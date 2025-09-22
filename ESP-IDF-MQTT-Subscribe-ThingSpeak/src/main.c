#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include "esp_crt_bundle.h"    //added for TLS certificate verification
#include "esp_tls.h"		   //added for enabling TLS 
#include "esp_http_client.h"

#include "connect_wifi.h"
#include "ds18b20.h" 
#include "thingspeak_mqtt.h"
#include "credentials.h"


static const char *TAG = "HTTP_CLIENT";
char api_key[] = "40ABSE6JQXZYMGJB"; // Thingspeak API write key

const int DS_PIN = 3;

float TempC;
float TempF;

volatile double temp = 10.0;
void get_readings(void *ignore)
{
	while (true)
	{
		vTaskDelay(4000 / portTICK_PERIOD_MS);
		temp = temp + 0.01;  //simulate measurements
	}
	vTaskDelete(NULL);
}

void DS18B20_readings()
{
	ds18b20_init(DS_PIN);
	while(true){
		ds18b20_requestTemperatures();

    	TempC = ds18b20_get_temp();
    	TempF = TempC*9/5 + 32;
		temp = TempC;
//    	ESP_LOGI(TAG, "Temperature in Celsius: %.2f C", TempC);
//    	ESP_LOGI(TAG, "Temperature in Fahrenheit: %.2f F", TempF);
	}
    
		
}	

void send_data_to_thingspeak(void *arg)
{
	
    thingspeak_mqtt_client_t mqtt_client;
    thingspeak_mqtt_client_init(&mqtt_client);
	vTaskDelay(5000 / portTICK_PERIOD_MS);
	thingspeak_mqtt_client_subscribe(&mqtt_client);
//	thingspeak_mqtt_client_subscribe_field(&mqtt_client, 1);
    while(1)
    {
      vTaskDelay(15000 / portTICK_PERIOD_MS);
//      thingspeak_set_field(&mqtt_client,THINGSPEAK_FIELD1,(uint16_t) (TempC * 100));
//      thingspeak_mqtt_client_publish(&mqtt_client);
    }


}

void app_main(void)
{
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);
	connect_wifi();
	if (wifi_connect_status)
	{
//		xTaskCreate(&DS18B20_readings, "get_readings", 2048, NULL, 5, NULL);
		xTaskCreate(&send_data_to_thingspeak, "send_data_to_thingspeak", 8192, NULL, 6, NULL);
	}
}
