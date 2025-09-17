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

static const char *TAG = "HTTP_CLIENT";
char api_key[] = "5018608"; // WhatsApp API write key
char phone[] = ""; // WhatsApp phone number

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
    	ESP_LOGI(TAG, "Temperature in Celsius: %.2f C", TempC);
//    	ESP_LOGI(TAG, "Temperature in Fahrenheit: %.2f F", TempF);
	}
    
		
}	

void send_data_to_whatsapp(void *pvParameters)
{
	char whatsapp_url[] = "https://api.callmebot.com/whatsapp.php?phone=%s&text=Temp:+%.2f&apikey=%s"; 
	char data[] = "/update?api_key=%s&field1=%.2f";
	char post_data[200];
	esp_err_t err;

	esp_http_client_config_t config = {
		.url = whatsapp_url,
		.method = HTTP_METHOD_GET,
		.transport_type = HTTP_TRANSPORT_OVER_SSL,  // added: Specify transport type
		.crt_bundle_attach = esp_crt_bundle_attach, // added: Attach the certificate bundle
	};
	esp_http_client_handle_t client = esp_http_client_init(&config);
	esp_http_client_set_header(client, "Content-Type", "application/x-www-form-urlencoded");
	while (1)
	{
		vTaskDelay(30000 / portTICK_PERIOD_MS);
		strcpy(post_data, "");
        if (TempC>30.0){  // send message only if temperature exceeds 30C
		    snprintf(post_data, sizeof(post_data), whatsapp_url, phone, temp, api_key);
        }
		ESP_LOGI(TAG, "post = %s", post_data);
		//esp_http_client_set_post_field(client, post_data, strlen(post_data)); // removed
		esp_http_client_set_url(client, post_data);  // replaces set_post_field
		err = esp_http_client_perform(client);
		if (err == ESP_OK)
		{
			int status_code = esp_http_client_get_status_code(client);
			if (status_code == 200)
			{
				ESP_LOGI(TAG, "Message sent Successfully");
			}
			else
			{
				ESP_LOGI(TAG, "Message sent Failed");				
				goto exit;
			}
		}
		else
		{
			ESP_LOGI(TAG, "Message sent Failed");
			goto exit;
		}
	}
exit:
	esp_http_client_cleanup(client);
	vTaskDelete(NULL);
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
		xTaskCreate(&DS18B20_readings, "get_readings", 2048, NULL, 5, NULL);
		xTaskCreate(&send_data_to_whatsapp, "send_data_to_whatsapp", 8192, NULL, 6, NULL);
	}
}
