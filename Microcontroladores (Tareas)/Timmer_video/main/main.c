#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"

#define LED 2

uint8_t estado_led = 0;
static const char *tag = "Main";
TimerHandle_t xTimers;
int interval = 1000;
int timerID = 1;

// Declaraciones
esp_err_t prender_led(void);
esp_err_t intermitar_led(void);
esp_err_t encender_temporizador(void);

// Callback del temporizador
void vTimerCallback(TimerHandle_t xTimer)
{
    intermitar_led();
    ESP_LOGI(tag, "Temporizador activado y led intermitando");
}

// Función principal
void app_main(void)
{
    prender_led();
    encender_temporizador();

    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay largo y el timer ya maneja la intermitencia
    }
}

// Inicialización del LED
esp_err_t prender_led(void)
{
    gpio_reset_pin(LED);
    gpio_set_direction(LED, GPIO_MODE_OUTPUT);
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