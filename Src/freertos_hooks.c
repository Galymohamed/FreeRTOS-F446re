#include "FreeRTOS.h"
#include "task.h"
#include "stm32f4xx_hal.h"



static void fatal_error_handler(void)
{
    taskDISABLE_INTERRUPTS();
#if defined(DEBUG)
    __asm volatile("bkpt #0");
    for (;;) {}
#else
    NVIC_SystemReset();
#endif
}


void vApplicationTickHook(void)
{
    HAL_IncTick();
}


void vApplicationIdleHook(void)
{
    __WFI();
}


void vApplicationMallocFailedHook(void)
{
    fatal_error_handler();
}


void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;
    fatal_error_handler();
}


void vApplicationDaemonTaskStartupHook(void)
{
    /* Called once by the FreeRTOS timer/daemon task on first run.
     * Place any one-time initialisation that must run in a task context here. */
}
