/*

*** FreeRTOS  ***
* Interrupt Processing using Semaphores *

> How to use semaphores inside interrupts
> Your ISR routine should not do all the work. It should just give a semaphore and your task should be waiting for the semaphore.
    > We do this way because if your ISR routine takes 5 seconds to complete then you don't want FreeRTOS to be disabled for those 5 seconds.
    > So using semaphores, a FreeRTOS task will be waiting for a semaphore from the ISR and as soon as it receives the seamphore the task does 
        all the things that the ISR was suppose to do. This way FreeRTOS wont be blocked during ISR as context switching would still take place.

> In this program ESP32's Timer-1 is used to call my_ISR() function every 2 seconds.

*/

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"    // Header file to use FreeRTOS Semaphores

#include "esp_system.h"
#include "esp_spi_flash.h"

#include "soc/timer_group_struct.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"

#define TIMER_DIVIDER         16  //  Hardware timer clock divider
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER)  // convert counter value to seconds
#define TIMER_INTERVAL0_SEC   (3.4179) // sample test interval for the first timer
#define TIMER_INTERVAL1_SEC   (1.78)   // sample test interval for the second timer
#define TEST_WITHOUT_RELOAD   0        // testing will be done without auto reload
#define TEST_WITH_RELOAD      1        // testing will be done with auto reload

/*
 * A sample structure to pass events
 * from the timer interrupt handler to the main program.
 */
typedef struct {
    int type;  // the type of timer's event
    int timer_group;
    int timer_idx;
    uint64_t timer_counter_value;
} timer_event_t;



xSemaphoreHandle binary_semaphore;

void my_ISR(void){

    long freertos_task_woken = 0; // it will tell us the task which is suppose to perform ISR has been woken by the semaphore or not

    xSemaphoreGiveFromISR(binary_semaphore, &freertos_task_woken);
    // xSemaphoreGiveFromISR(binary_semaphore, NULL);

    // if(freertos_task_woken){
    //     vPortYieldFromISR();
    // }

    /*
    
    xSemaphoreGiveFromISR(binary_semaphore, NULL);
    Task-1 --> 
                ISR --> Give Semaphore
                                        --> Back to Task-1

    
    xSemaphoreGiveFromISR(binary_semaphore, &freertos_task_woken);
    Task-1 -->
                ISR --> Give Semaphore --> YieldFromISR()
                                                            --> Back to semaphore_task() instead of Task-1

    */

}


void IRAM_ATTR timer_group0_isr(void *para)
{
    int timer_idx = (int) para;

    /* Retrieve the interrupt status and the counter value
       from the timer that reported the interrupt */
    uint32_t intr_status = TIMERG0.int_st_timers.val;
    TIMERG0.hw_timer[timer_idx].update = 1;
    uint64_t timer_counter_value = 
        ((uint64_t) TIMERG0.hw_timer[timer_idx].cnt_high) << 32
        | TIMERG0.hw_timer[timer_idx].cnt_low;

    /* Prepare basic event data
       that will be then sent back to the main program task */
    timer_event_t evt;
    evt.timer_group = 0;
    evt.timer_idx = timer_idx;
    evt.timer_counter_value = timer_counter_value;

    /* Clear the interrupt
       and update the alarm time for the timer with without reload */
    if ((intr_status & BIT(timer_idx)) && timer_idx == TIMER_0) {
        evt.type = TEST_WITHOUT_RELOAD;
        TIMERG0.int_clr_timers.t0 = 1;
        timer_counter_value += (uint64_t) (TIMER_INTERVAL0_SEC * TIMER_SCALE);
        TIMERG0.hw_timer[timer_idx].alarm_high = (uint32_t) (timer_counter_value >> 32);
        TIMERG0.hw_timer[timer_idx].alarm_low = (uint32_t) timer_counter_value;
    } else if ((intr_status & BIT(timer_idx)) && timer_idx == TIMER_1) {
        evt.type = TEST_WITH_RELOAD;
        TIMERG0.int_clr_timers.t1 = 1;
    } else {
        evt.type = -1; // not supported even type
    }

    /* After the alarm has been triggered
      we need enable it again, so it is triggered the next time */
    TIMERG0.hw_timer[timer_idx].config.alarm_en = TIMER_ALARM_EN;

    /* Now just send the event data back to the main program task */
    // xQueueSendFromISR(timer_queue, &evt, NULL);

    my_ISR();
}


static void example_tg0_timer_init(int timer_idx, 
    bool auto_reload, double timer_interval_sec)
{
    /* Select and initialize basic parameters of the timer */
    timer_config_t config;
    config.divider = TIMER_DIVIDER;
    config.counter_dir = TIMER_COUNT_UP;
    config.counter_en = TIMER_PAUSE;
    config.alarm_en = TIMER_ALARM_EN;
    config.intr_type = TIMER_INTR_LEVEL;
    config.auto_reload = auto_reload;
    timer_init(TIMER_GROUP_0, timer_idx, &config);

    /* Timer's counter will initially start from value below.
       Also, if auto_reload is set, this value will be automatically reload on alarm */
    timer_set_counter_value(TIMER_GROUP_0, timer_idx, 0x00000000ULL);

    /* Configure the alarm value and the interrupt on alarm. */
    timer_set_alarm_value(TIMER_GROUP_0, timer_idx, timer_interval_sec * TIMER_SCALE);
    timer_enable_intr(TIMER_GROUP_0, timer_idx);
    timer_isr_register(TIMER_GROUP_0, timer_idx, timer_group0_isr, 
        (void *) timer_idx, ESP_INTR_FLAG_IRAM, NULL);

    timer_start(TIMER_GROUP_0, timer_idx);
}



void semaphore_task(void *p){
    int counter = 0;
    
    while(1){
    
        if( xSemaphoreTake(binary_semaphore, portMAX_DELAY) ){
            printf("[%d] Tick!!\n", counter++);
        }


    }

}


void app_main(){

    vSemaphoreCreateBinary(binary_semaphore);

    example_tg0_timer_init(TIMER_1, TEST_WITH_RELOAD, TIMER_INTERVAL1_SEC);

    xTaskCreate(&semaphore_task, "semaphore_task", 1600, NULL, 5, NULL);


}



