/*

*** FreeRTOS Queue Set ***

*/

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_system.h"
#include "esp_spi_flash.h"


void my_task(void *p){


    while(1){

        vTaskDelay(100 / portTICK_PERIOD_MS);   // create a 100ms delay
    }

}


void app_main(){

    xTaskCreate(&my_task, "my_task", 1600, NULL, 5, NULL);

}
