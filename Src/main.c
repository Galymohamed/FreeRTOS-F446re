#include "main.h"


uint32_t ledTskCnt = 0U;

/*===========================================================================
 *  Task handles
 *===========================================================================*/
static TaskHandle_t hTaskLed    = NULL;
static TaskHandle_t hTaskReport = NULL;

static void taskUartReport(void *pvParameters);
static void taskLedBlink(void *pvParameters);


int main(void)
{
    g_sysInit();

    uart2_cbSendString("[FreeRTOS] Scheduler starting...\r\n");

    xTaskCreate(taskLedBlink, "LED", 256, NULL, 2, &hTaskLed);
    xTaskCreate(taskUartReport,"REPORT", 256, NULL, 1, &hTaskReport);

    vTaskStartScheduler();

    for (;;) {}
}



static void taskLedBlink(void *pvParameters)
{
    (void)pvParameters;

    for (;;)
    {
        ledTskCnt++;
        HAL_GPIO_TogglePin(BOARD_LED_PORT, BOARD_LED_PIN);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}


static void taskUartReport(void *pvParameters)
{
    (void)pvParameters;

    uint32_t tick = 0U;
    char     msg[48];

    for (;;)
    {
        snprintf(msg, sizeof(msg), "heartbeat: %lu\r\n", tick++);
        uart2_cbSendString(msg);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


#ifdef USE_FULL_ASSERT                                                                                                                                    
void assert_failed(uint8_t *file, uint32_t line)                                                                                                          
{                                                                                                                                                                                                                                                                                            
    taskDISABLE_INTERRUPTS();                                                                                                                             
#if defined(DEBUG)                                                                                                                                 
    (void)file;                                                                                               
    (void)line;                                                                                                  
     __asm volatile("bkpt #0");                                                                                                                            
    for (;;) {}                                                                                                                                          
#else                                                                                                                                                     
    NVIC_SystemReset();                                                                                                                                   
#endif                                                                                                                                                    
}                                                                                                                                                         
#endif 