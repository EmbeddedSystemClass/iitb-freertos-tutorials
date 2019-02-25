/*

*** FreeRTOS Binary Semaphores ***

> Both Binary semaphores and Mutex are used to protect guarded resources
> Mutex works on Priority Inversion Mechanism
> Mutex is used for Mutual Exclusion so that only one task can use the guarded resource
> Binary semaphores are used to give signals to functions
> This signal can be from a interrupt to a task

*/

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"    // Header file to use FreeRTOS Semaphores

#include "esp_system.h"
#include "esp_spi_flash.h"

xSemaphoreHandle signal = 0;

void slave_works(void){
    printf("\t[Slave] Done Master!!\n");
}

void master_task(void *p){


    while(1){
        
        printf("[Master] Giving signal to slave\n");
        
        xSemaphoreGive(signal);     // Master gives signal to slave
        
        printf("[Master] Finished giving signal\n\n");

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

}


void slave_task(void *p){

    while(1){
        
        if( xSemaphoreTake(signal, portMAX_DELAY) ){
            slave_works();
        }        
    }

}

void app_main(){

    vSemaphoreCreateBinary(signal);

    xTaskCreate(&master_task, "master_task", 8000, NULL, 1, NULL);
    // xTaskCreate(&master_task, "master_task", 8000, NULL, 2, NULL); // Mater_Task at higher priority > In this case slave will do its task after "[Master] Finished giving signal"
    xTaskCreate(&slave_task, "slave_task", 8000, NULL, 1, NULL);

    
}
