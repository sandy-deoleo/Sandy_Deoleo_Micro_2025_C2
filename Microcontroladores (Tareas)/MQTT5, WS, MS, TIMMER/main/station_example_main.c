#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/timers.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "mqtt_client.h"
#include "driver/gpio.h"
#include "esp_timer.h"

#define EXAMPLE_ESP_WIFI_SSID           "WYSPLUS - Familia de oleo"
#define EXAMPLE_ESP_WIFI_PASS           "1p29384756Ds"
#define EXAMPLE_ESP_MAXIMUM_RETRY  5
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK

#define WIFI_CONNECTED_BIT          BIT0
#define WIFI_FAIL_BIT               BIT1

#define ESTADO_INICIAL  0
#define ESTADO_CERRANDO 1
#define ESTADO_ABRIENDO 2
#define ESTADO_CERRADO  3
#define ESTADO_ABIERTO  4
#define ESTADO_ERR      5
#define ESTADO_STOP     6

#define LAMP_OFF  0
#define LAMP_ON   1
#define LAMP_FAST 2
#define LAMP_SLOW 3

#define ERR_OK  0
#define ERR_OT  1
#define ERR_LSM 2

#define PIN1 GPIO_NUM_4

static const char *TAG = "MAIN";

static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;
static esp_mqtt_client_handle_t global_client;

struct IO 
{
    unsigned int LSC : 1;
    unsigned int LSA : 1;
    unsigned int BA : 1;
    unsigned int BC : 1;
    unsigned int PP : 1;
    unsigned int SE : 1;
    unsigned int MA : 1;
    unsigned int MC : 1;
    unsigned int LAMP : 1;
    unsigned int Buzzer : 1;
} io;

struct STATUS 
{
    unsigned int cntTimerCA;
    unsigned int cntRunTimer;
    int ERR_COD;
} status = { 0, 0, ERR_OK };

struct CONFIG 
{
    unsigned int RunTimer;
    unsigned int TimerCA;
} config = { 180, 120 };

int EstadoSiguiente = ESTADO_INICIAL;
int EstadoActual = ESTADO_INICIAL;
int EstadoAnterior = ESTADO_INICIAL;
int EstadoLamp = LAMP_OFF;
bool init_estado = true;

void lamp_control(void) 
{
    static int cnt = 0;
    cnt++;
    switch (EstadoLamp) 
    {
        case LAMP_OFF:
            io.LAMP = false;
            cnt = 0;
            break;

        case LAMP_ON:
            io.LAMP = true;
            cnt = 0;
            break;

        case LAMP_SLOW:
            if (cnt >= 10) cnt = 0;
            io.LAMP = (cnt <= 5);
            break;

        case LAMP_FAST:
            if (cnt >= 6) cnt = 0;
            io.LAMP = (cnt <= 3);
            break;
    }
}

int Func_ESTADO_INICIAL(void);
int Func_ESTADO_CERRANDO(void);
int Func_ESTADO_ABRIENDO(void);
int Func_ESTADO_CERRADO(void);
int Func_ESTADO_ABIERTO(void);
int Func_ESTADO_ERR(void);
int Func_ESTADO_STOP(void);

void ejecutar_maquina_estados(void)
{
    int nuevo_estado = EstadoSiguiente;

    switch (EstadoSiguiente) 
    {
        case ESTADO_INICIAL:
            nuevo_estado = Func_ESTADO_INICIAL();
            break;

        case ESTADO_CERRANDO:
            nuevo_estado = Func_ESTADO_CERRANDO();
            break;

        case ESTADO_ABRIENDO:
            nuevo_estado = Func_ESTADO_ABRIENDO();
            break;

        case ESTADO_CERRADO:
            nuevo_estado = Func_ESTADO_CERRADO();
            break;

        case ESTADO_ABIERTO:
            nuevo_estado = Func_ESTADO_ABIERTO();
            break;

        case ESTADO_ERR:
            nuevo_estado = Func_ESTADO_ERR();
            break;

        case ESTADO_STOP:
            nuevo_estado = Func_ESTADO_STOP();
            break;

        default:
            nuevo_estado = ESTADO_INICIAL;
            break;
    }

    if (nuevo_estado != EstadoSiguiente) 
    {
        init_estado = true;
        EstadoSiguiente = nuevo_estado;
    }
}

void my_timer_callback(void *arg) 
{
    status.cntRunTimer++;
    status.cntTimerCA++;
    lamp_control();
    ejecutar_maquina_estados();
    io.LSA = gpio_get_level(PIN1);
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) 
{
    esp_mqtt_event_handle_t event = event_data;
    global_client = event->client;

    switch ((esp_mqtt_event_id_t)event_id) 
    {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT conectado");
            esp_mqtt_client_subscribe(global_client, "mqtt5_01/temp", 0);
            break;

        case MQTT_EVENT_DATA:
            char msg[50] = {0};
            int len = (event->data_len < sizeof(msg) - 1) ? event->data_len : sizeof(msg) - 1;
            strncpy(msg, event->data, len);
            msg[len] = '\0';
            ESP_LOGI(TAG, "Comando MQTT recibido: %s", msg);
            if (strcmp(msg, "abrir") == 0) EstadoSiguiente = ESTADO_ABRIENDO;
            else if (strcmp(msg, "cerrar") == 0) EstadoSiguiente = ESTADO_CERRANDO;
            else if (strcmp(msg, "stop") == 0) EstadoSiguiente = ESTADO_STOP;
            else if (strcmp(msg, "reset") == 0) EstadoSiguiente = ESTADO_INICIAL;
            break;
        default:
            break;
    }
}

static void mqtt_app_start(void) 
{
    const esp_mqtt_client_config_t mqtt_cfg = 
    {
        .broker.address.uri = "ws://broker.emqx.io:8083/mqtt",
        .credentials.username = "mqtt5_01",
        .credentials.authentication.password = "12345678"
    };

    global_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(global_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(global_client);
}

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) 
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) 
    {
        esp_wifi_connect();
    } 
    
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) 
    {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) 
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "Reintentando WiFi");
        }
            else 
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
    } 
    
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) 
    {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Conectado IP: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void) 
{
    s_wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    esp_event_handler_instance_t instance_any_id, instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));

    wifi_config_t wifi_config = 
    {
        .sta = 
        {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
    if (bits & WIFI_CONNECTED_BIT) mqtt_app_start();
}

void app_main(void) 
{
    ESP_ERROR_CHECK(nvs_flash_init());
    wifi_init_sta();

    gpio_set_direction(PIN1, GPIO_MODE_INPUT);
    const esp_timer_create_args_t periodic_timer_args = 
    {
        .callback = &my_timer_callback,
        .name = "my_50ms_timer"
    };
;

    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 50 * 1000));

    while (true) 
    {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

int Func_ESTADO_INICIAL(void) 
{
    ESP_LOGI(TAG, "Entrando en ESTADO_INICIAL");
    EstadoAnterior = EstadoActual;
    EstadoActual = ESTADO_INICIAL;

    if (io.LSC == true && io.LSA == true) {
        return ESTADO_ERR;
    }

    if (io.LSC == true && io.LSA == false) {
        return ESTADO_CERRADO;
    }

    if (io.LSC == false && io.LSA == true) {
        return ESTADO_CERRANDO;
    }

    if (io.LSC == false && io.LSA == false) {
        return ESTADO_STOP;
    }

    return ESTADO_ERR;
}

int Func_ESTADO_CERRANDO(void) 
{
    ESP_LOGI(TAG, "Entrando en ESTADO_CERRANDO");
    EstadoAnterior = EstadoActual;
    EstadoActual = ESTADO_CERRANDO;
    EstadoLamp = LAMP_FAST;
    status.cntRunTimer = 0;
    io.MA = false;
    io.MC = true;
    io.BA = false;
    io.BC = false;

    if (io.BA == true || io.BC == true) 
    {
        return ESTADO_STOP;
    }

    if (io.LSA == false && io.LSC == true) {
        return ESTADO_CERRADO;
    }

    if (status.cntRunTimer > config.RunTimer) {
        return ESTADO_ERR;
    }

    return ESTADO_CERRANDO;
}

int Func_ESTADO_ABRIENDO(void) 
{
    ESP_LOGI(TAG, "Entrando en ESTADO_ABRIENDO");

    EstadoAnterior = EstadoActual;
    EstadoActual = ESTADO_ABRIENDO;
    EstadoLamp = LAMP_SLOW;

    status.cntRunTimer = 0;
    io.MA = true;
    io.MC = false;
    io.BA = false;
    io.BC = false;
    if (io.LSA == true && io.LSC == false)
    {
        return ESTADO_ABIERTO;
    }

    if (io.BA == true || io.BC == true) 
    {
        return ESTADO_STOP;
    }

    if (status.cntRunTimer > config.RunTimer) 
    {
        status.ERR_COD = ERR_OT;
        return ESTADO_ERR;
    }

    return ESTADO_ABRIENDO;
}

int Func_ESTADO_CERRADO(void) 
{
    ESP_LOGI(TAG, "Entrando en ESTADO_CERRADO");
    EstadoAnterior = EstadoActual;
    EstadoActual = ESTADO_CERRADO;
    EstadoLamp = LAMP_OFF;

    io.MA = false;
    io.MC = false;
    io.BA = false;
    io.BC = false;

    if (io.BA == true && io.BC == false) 
    {
        return ESTADO_ABRIENDO;
    }

    if (io.PP == true) 
    {
        return ESTADO_ABRIENDO;
    }

    return ESTADO_CERRADO;
}

int Func_ESTADO_ABIERTO(void) 
{
    ESP_LOGI(TAG, "Entrando en ESTADO_ABIERTO");
    EstadoAnterior = EstadoActual;
    EstadoActual = ESTADO_ABIERTO;
    EstadoLamp = LAMP_ON;

    io.MA = false;
    io.MC = false;
    io.BA = false;
    io.BC = false;

    if (io.BA == false && io.BC == true) 
    {
        return ESTADO_CERRANDO;
    }

    if (io.PP == true) {
        return ESTADO_CERRANDO;
    }

    return ESTADO_ABIERTO;
}

int Func_ESTADO_ERR(void) 
{
    ESP_LOGI(TAG, "Entrando en ESTADO_ERR");
    EstadoAnterior = EstadoActual;
    EstadoActual = ESTADO_ERR;
    EstadoLamp = LAMP_OFF;

    io.MA = false;
    io.MC = false;
    io.BA = false;
    io.BC = false;

    if (status.ERR_COD == ERR_LSM) 
    {
        if (io.LSA == false || io.LSC == false) 
        {
            status.ERR_COD = ERR_OK;
            return ESTADO_INICIAL;
        }
    }
    return ESTADO_ERR;
}

int Func_ESTADO_STOP(void) 
{
    ESP_LOGI(TAG, "Entrando en ESTADO_STOP");
    EstadoAnterior = EstadoActual;
    EstadoActual = ESTADO_STOP;
    EstadoLamp = LAMP_OFF;

    io.MA = false;
    io.MC = false;
    io.BA = false;
    io.BC = false;

    if (io.BA == true && io.BC == false) 
    {
        return ESTADO_ABRIENDO;
    }

    if (io.BA == false && io.BC == true) 
    {
        return ESTADO_CERRANDO;
    }

    if (io.PP == true) 
    {
        return ESTADO_CERRANDO;
    }

    return ESTADO_STOP;
}