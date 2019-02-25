/*

*** FreeRTOS Event Groups ***

Software Watchdog

*/

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_spi_flash.h"


const uint32_t good_task_id = (1 << 0); // 0th bit 1 -> 01
const uint32_t bad_task_id = (1 << 1);  // 1st bit 1 -> 10
const uint32_t tasks_all_bits = 0x03; //(good_task_id | bad_task_id); // 11

EventGroupHandle_t task_watchdog;


void task_good(void *p){


    while(1){
        
        // good task therefore always setting corresponding event group bit
        xEventGroupSetBits(task_watchdog, good_task_id);

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

}


void task_bad(void *p){

    int counter = 0;

    while(1){
        
        counter++;

        if(counter >= 3 && counter <= 6){
            // stop setting event group bit
            // bad behaviour
            printf("BAD!!!\n");
        } else {
            xEventGroupSetBits(task_watchdog, bad_task_id);
        }  

        vTaskDelay(1000 / portTICK_PERIOD_MS);     
    }

}


void task_software_watchdog(void *p){

    while(1){

        uint32_t result = xEventGroupWaitBits(task_watchdog, tasks_all_bits,
                                                pdTRUE,
                                                pdTRUE,
                                                5000);

        // printf("[%d] [%d] [%d]\n", result, !(result & good_task_id), !(result & bad_task_id));

        if( (result & tasks_all_bits) == tasks_all_bits ){
            printf("System is Healthy! Chill.\n");
        
        } else {

            if( !(result & good_task_id) ){
                printf("Good-Task is NOT responding :p\n");
            }

            if( !(result & bad_task_id) ){
                printf("Bad-Task is NOT responding :p\n");
            }

        }


    }


}

void app_main(){

    task_watchdog = xEventGroupCreate();

    xTaskCreate(&task_good, "task_good", 8000, NULL, 1, NULL);
    xTaskCreate(&task_bad, "task_bad", 8000, NULL, 1, NULL);
    xTaskCreate(&task_software_watchdog, "task_software_watchdog", 8000, NULL, 2, NULL); // should have highest priority

    
}
