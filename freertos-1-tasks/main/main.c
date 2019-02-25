/*

*** FreeRTOS Task ***

*/

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_system.h"
#include "esp_spi_flash.h"


void my_task(void *p){

    char counter = 0;

    UBaseType_t uxHighWaterMark; // variable to store available stack-size

    while(1){
        
        uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL); // returns value in words  words (for example, on a 32 bit machine a return value of 1 would indicate that 4 bytes of stack were unused)
        
        printf("counter = %d \t free-stack-size = %d\n", counter++, uxHighWaterMark); // %lld

        vTaskDelay(100 / portTICK_PERIOD_MS);   // create a 100ms delay
    }

}


void app_main(){

    xTaskCreate(&my_task, "my_task", 1600, NULL, 5, NULL);

    
    /*
        > vTaskStartScheduler(): Starts the real time kernel tick processing. After calling the kernel has control over which tasks are executed and when.
        > No need to call vTaskStartScheduler(), this function is called before app_main starts. In fact, app_main runs within a FreeRTOS task already.
    */
    
    // vTaskStartScheduler();

}
