/* MQTT (over TCP) Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

#include "ssd1306.h"

#include "dht11.h"

#define QUEUE_LENGTH 1
#define ITEM_SIZE    sizeof(int)

typedef struct {
    uint8_t tem;
    uint8_t hud;
} DHT;

QueueHandle_t xQueue;
static const char *TAG = "MQTT_EXAMPLE";
esp_mqtt_client_handle_t client;
static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");

            msg_id = esp_mqtt_client_subscribe(client, "Sensor_receive_data", 0);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb(event_data);
}

static void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .host = "mqtt.flespi.io",
        .port = 1883,
        .username = "FlespiToken PZvm0KGXr6g3FqedFCW7LQJsfEG0cSJPG41uCyhTsnMWBOX4hJ1Qny8t3Ch3lQIP",
        .client_id = "123xaa",
    };

    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
}

void DHT_reading()
{
    char Hud[20],Tem[20];
    char dataT[10],dataH[10];
    DHT tmp, dht;
    DHT11_init(GPIO_NUM_4);
    while(1) {
        task_ssd1306_display_clear();
        tmp.tem =  DHT11_read().temperature; 
        tmp.hud =  DHT11_read().humidity;
        if (tmp.tem > 0 && tmp.tem < 80){
            dht.tem = tmp.tem;
        }
        if (tmp.hud > 0 && tmp.hud < 100){
            dht.hud = tmp.hud;
        }
        xQueueSend(xQueue, &dht,0);
        snprintf(dataT, sizeof(dataT), "%d", dht.tem);
        snprintf(dataH, sizeof(dataH), "%d", dht.hud);
        strcpy(Hud,"Humidity: ");
        strcat(Hud,dataH);
        task_ssd1306_display_text_in_line(Hud,0);
        strcpy(Tem,"Temperature: ");
        strcat(Tem,dataT);
        task_ssd1306_display_text_in_line(Tem,1);
        vTaskDelay(1000 / portTICK_RATE_MS);
    }		
}

void Send_server()
{
    char dataT[10],dataH[10];
    DHT receivedData;
    while(1)
    {
        if (xQueueReceive(xQueue, &receivedData, portMAX_DELAY) == pdTRUE) {
            snprintf(dataT, sizeof(dataT), "%d", receivedData.tem);
            esp_mqtt_client_publish(client, "Sensor_receive_data", dataT, strlen(dataT), 0, 0);
            snprintf(dataH, sizeof(dataH), "%d", receivedData.hud);
            esp_mqtt_client_publish(client, "Sensor_receive_data", dataH, strlen(dataH), 0, 0);
            vTaskDelay(10000 / portTICK_RATE_MS);
        }
    }
}

void app_main(void)
{
    ssd1306_init();
    task_ssd1306_display_clear();
    //task_ssd1306_display_text("Hello");
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());
    mqtt_app_start();
    xQueue = xQueueCreate(QUEUE_LENGTH, ITEM_SIZE);
    printf("running task\n");
    xTaskCreate(DHT_reading,"read_DHT_data",4096,NULL,4,NULL);
    xTaskCreate(Send_server,"send_DHT_data",4096,NULL,5,NULL);
}
