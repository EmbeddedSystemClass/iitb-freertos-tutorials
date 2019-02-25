/*

*** FreeRTOS Semaphores (Mutex) ***

*/

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"    // Header file to use FreeRTOS Semaphores

#include "esp_system.h"
#include "esp_spi_flash.h"

xSemaphoreHandle key = 0;

void access_precious_resource(){
    printf("** My Precious!! **\n\n");
}

void user_1(void *p){

    int gatekeeper = 0;

    while(1){
        

        gatekeeper = xSemaphoreTake(key, 1000);

        if( gatekeeper == pdTRUE){
           printf("[User-1] I have the key!! ");
           access_precious_resource();

           // vTaskDelay(500/portTICK_PERIOD_MS);

           xSemaphoreGive(key);
        
        } else {
           printf("[User-1] Failed to get the key after waiting for 1 sec :/\n\n"); 
        }

        vTaskDelay(1000/portTICK_PERIOD_MS);
    }

}

void user_2(void *p){

    int gatekeeper = 0;

    while(1){
        
        gatekeeper = xSemaphoreTake(key, 1000);

        if( gatekeeper == pdTRUE){
           printf("\t[User-2] I have the key!! ");
           
           access_precious_resource();
           
           xSemaphoreGive(key); 
        
        } else {
           printf("\t[User-2] Failed to get the key after waiting for 1 sec :/\n\n"); 
        }

        vTaskDelay(1000/portTICK_PERIOD_MS);
    }

}


void app_main(){

    key = xSemaphoreCreateMutex();

    xTaskCreate(&user_1, "user_1", 1600, NULL, 5, NULL);
    xTaskCreate(&user_2, "user_2", 1600, NULL, 1, NULL);


}
