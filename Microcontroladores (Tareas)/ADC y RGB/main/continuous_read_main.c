#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "driver/adc.h"

#define LED 2
#define LEDR 33
#define LEDG 25
#define LEDB 26


uint8_t estado_led = 0;
static const char *tag = "Main";
TimerHandle_t xTimers;
int interval = 1000;
int timerID = 1;

int adc_val = 0;

// Declaraciones
esp_err_t prender_led(void);
esp_err_t intermitar_led(void);
esp_err_t encender_temporizador(void);
esp_err_t set_adc(void);


// Callback del temporizador
void vTimerCallback(TimerHandle_t xTimer)
{
    intermitar_led();
    adc_val = adc1_get_raw(ADC1_CHANNEL_4);
    int adc_case= adc_val / 1000;  //MAXIMO DE BIT = 4094

    ESP_LOGI(tag, "ADC VAL: %i", adc_val);
    switch (adc_case)
    {
    case 0:

        gpio_set_level(LEDR, 0);
        gpio_set_level(LEDG, 0);
        gpio_set_level(LEDB, 0);
        break;

    case 1:

        gpio_set_level(LEDR, 1);
        gpio_set_level(LEDG, 0);
        gpio_set_level(LEDB, 0);
        break;

    case 2:

        gpio_set_level(LEDR, 1);
        gpio_set_level(LEDG, 1);
        gpio_set_level(LEDB, 0);
        break;

    case 3:
    case 4:

        gpio_set_level(LEDR, 1);
        gpio_set_level(LEDG, 1);
        gpio_set_level(LEDB, 1);
        break;

    default:
        break;
    }

}

// Función principal
void app_main(void)
{
    prender_led();
    encender_temporizador();
}

// Inicialización del LED
esp_err_t prender_led(void)
{
    gpio_reset_pin(LED);
    gpio_set_direction(LED, GPIO_MODE_OUTPUT);

    gpio_reset_pin(LEDR);
    gpio_set_direction(LEDR, GPIO_MODE_OUTPUT);

    gpio_reset_pin(LEDG);
    gpio_set_direction(LEDG, GPIO_MODE_OUTPUT);

    gpio_reset_pin(LEDB);
    gpio_set_direction(LEDB, GPIO_MODE_OUTPUT);
    
    return ESP_OK;
}

// Encendido y apagado del LED
esp_err_t intermitar_led(void)
{
    estado_led = !estado_led;
    gpio_set_level(LED, estado_led);
    return ESP_OK;
}

// Configuración del temporizador
esp_err_t encender_temporizador(void)
{
    ESP_LOGI(tag, "Configuracion inicial del timer");
    xTimers = xTimerCreate("Timer",             // Nombre del temporizador
                           pdMS_TO_TICKS(interval), // Periodo de 1 segundo
                           pdTRUE,              // Auto-reload
                           (void *)timerID,                // ID opcional
                           vTimerCallback);     // Callback

    if (xTimers == NULL)
    {
        ESP_LOGE(tag, "Fallo al crear el temporizador");
        return ESP_FAIL;
    }
    else
    {
        if (xTimerStart(xTimers, 0) != pdPASS)
        {
            ESP_LOGE(tag, "Fallo al iniciar el temporizador");
            return ESP_FAIL;
        }
    }

    return ESP_OK;
}

esp_err_t set_adc(void)
{
    adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_12);
    adc1_config_width(ADC_WIDTH_BIT_12);

    return ESP_OK;
}