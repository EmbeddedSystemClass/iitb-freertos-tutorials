/*

*** FreeRTOS Queues ***

*/

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"  // Required to use FreeRTOS Queues

#include "esp_system.h"
#include "esp_spi_flash.h"

xQueueHandle global_queue_handle = 0;


void sender_task(void *p){

    int tx_i = 0;   // variable to send values to the queue
    int ret = 0;

    while(1){

        // This line will send value stored in tx_i to global_queue_handle
        // If the queue is full it will wait for a maximum of 1000 ticks for the queue to get empty before returning error.
        ret = xQueueSend(global_queue_handle, &tx_i, 1000);

        if( ret == pdTRUE ){
            printf("[Tx] Sent: %i\n", tx_i);
        } else {
            printf("[Tx] Failed :/\n");
        }

        tx_i++;

        vTaskDelay(3000 / portTICK_PERIOD_MS);   // create a 3 sec delay
    }

}


void receiver_task(void *p){

    int rx_i;      // variable to receive value from the queue
    int ret = 0;

    while(1){
        
        // save the value sent to global_queue_handle in rx_i
        // wait for 1000 ticks to receive something
        ret = xQueueReceive(global_queue_handle, &rx_i, 1000);

        if(ret == pdTRUE){
            printf("[Rx] Received: %i\n\n", rx_i);
        } else {
            printf("[Rx] Failed!\n\n");
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);   // create a 1 sec delay
    }

}

void app_main(){

    global_queue_handle = xQueueCreate(3, sizeof(int) ); // this queue can return 3 items of size int

    xTaskCreate(&sender_task, "tx_task", 1600, NULL, 5, NULL);
    xTaskCreate(&receiver_task, "rx_task", 1600, NULL, 4, NULL); // reciever at lower priority because at equal priority printing sequence is affected.
    
}
