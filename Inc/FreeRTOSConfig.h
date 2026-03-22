/**
 * @file        FreeRTOSConfig.h
 * @author      Mohamed GALY
 * @brief       FreeRTOS kernel configuration for the STM32F446RE (Cortex-M4F @ 180 MHz).
 * @details     Tune the values below to match your application's timing, stack,
 *              and memory requirements.  Every macro that is commented out is left
 *              at its kernel default; un-comment only what you need to override.
 *
 * @target      STM32F446RE
 * @board       Nucleo-F446RE
 * @date        15-06-2026
 * @version     1.0.0
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*===========================================================================
 *  Scheduler behaviour
 *===========================================================================*/
#define configUSE_PREEMPTION                    1
#define configUSE_TIME_SLICING                  1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 1   /* Cortex-M4 CLZ instruction */
#define configUSE_TICKLESS_IDLE                 0   /* set to 1 for low-power apps */
#define configUSE_IDLE_HOOK                     1   /* calls vApplicationIdleHook() */
#define configUSE_TICK_HOOK                     1   /* calls vApplicationTickHook() → HAL_IncTick() */
#define configUSE_MALLOC_FAILED_HOOK            1   /* calls vApplicationMallocFailedHook() */
#define configUSE_DAEMON_TASK_STARTUP_HOOK      1   /* calls vApplicationDaemonTaskStartupHook() */
/* 2 = fill pattern check (catches overflows that don't immediately crash).
 * Remove or set to 0 for release builds.                                   */
#define configCHECK_FOR_STACK_OVERFLOW          2

/*===========================================================================
 *  Clock & tick
 *===========================================================================*/
#define configCPU_CLOCK_HZ                      ( ( uint32_t ) 180000000UL )
#define configTICK_RATE_HZ                      ( ( TickType_t ) 1000 )     /* 1 ms tick */
/* 0 = 32-bit tick counter (recommended for Cortex-M, supports long delays). */
#define configUSE_16_BIT_TICKS                  0

/*===========================================================================
 *  Task management
 *===========================================================================*/
#define configMAX_PRIORITIES                    ( 7 )
#define configMINIMAL_STACK_SIZE                ( ( uint16_t ) 128 )        /* in words */
#define configMAX_TASK_NAME_LEN                 ( 16 )
#define configIDLE_SHOULD_YIELD                 1
#define configUSE_TASK_NOTIFICATIONS            1
#define configTASK_NOTIFICATION_ARRAY_ENTRIES   3
#define configUSE_TASK_FPU_SUPPORT              1   /* auto-save FPU registers */

/*===========================================================================
 *  Memory allocation
 *===========================================================================*/
#define configSUPPORT_STATIC_ALLOCATION         0   /* set to 1 to use static allocation */
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configTOTAL_HEAP_SIZE                   ( ( size_t ) ( 32 * 1024 ) )/* 32 KB */
#define configAPPLICATION_ALLOCATED_HEAP        0

/*===========================================================================
 *  Software timers
 *===========================================================================*/
#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               ( configMAX_PRIORITIES - 1 )
#define configTIMER_QUEUE_LENGTH                10
#define configTIMER_TASK_STACK_DEPTH            ( configMINIMAL_STACK_SIZE * 2 )

/*===========================================================================
 *  Queue / mutex / semaphore
 *===========================================================================*/
#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             1
#define configUSE_COUNTING_SEMAPHORES           1
#define configQUEUE_REGISTRY_SIZE               8
#define configUSE_QUEUE_SETS                    0

/*===========================================================================
 *  Run-time stats & trace
 *===========================================================================*/
#define configGENERATE_RUN_TIME_STATS           0
#define configUSE_TRACE_FACILITY                0
#define configUSE_STATS_FORMATTING_FUNCTIONS    0

/*===========================================================================
 *  Co-routines (deprecated — keep disabled)
 *===========================================================================*/
#define configUSE_CO_ROUTINES                   0
#define configMAX_CO_ROUTINE_PRIORITIES         1

/*===========================================================================
 *  Cortex-M interrupt priority configuration
 *  STM32F4 implements __NVIC_PRIO_BITS = 4 priority bits (0–15).
 *
 *  configKERNEL_INTERRUPT_PRIORITY    must be the LOWEST priority (15 << shift).
 *  configMAX_SYSCALL_INTERRUPT_PRIORITY sets the ceiling for ISRs that call
 *  FreeRTOS "FromISR" API functions (5 << shift).  Any ISR with a NUMERICALLY
 *  LOWER priority value (i.e. higher hardware priority) must NOT call the
 *  FreeRTOS API.
 *===========================================================================*/
/* STM32F446RE (Cortex-M4) implements 4 NVIC priority bits. */
#define configPRIO_BITS                         4

#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY         15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY    5

#define configKERNEL_INTERRUPT_PRIORITY     \
    ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << ( 8 - configPRIO_BITS ) )

#define configMAX_SYSCALL_INTERRUPT_PRIORITY \
    ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << ( 8 - configPRIO_BITS ) )

/*===========================================================================
 *  API function inclusion
 *===========================================================================*/
#define INCLUDE_vTaskPrioritySet                1
#define INCLUDE_uxTaskPriorityGet               1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_xResumeFromISR                  1
#define INCLUDE_vTaskDelayUntil                 1
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_xTaskGetSchedulerState          1
#define INCLUDE_xTaskGetCurrentTaskHandle       1
#define INCLUDE_uxTaskGetStackHighWaterMark     1
#define INCLUDE_xTaskGetIdleTaskHandle          0
#define INCLUDE_eTaskGetState                   1
#define INCLUDE_xEventGroupSetBitFromISR        1
#define INCLUDE_xTimerPendFunctionCall          1
#define INCLUDE_xTaskAbortDelay                 0
#define INCLUDE_xTaskGetHandle                  0

/*===========================================================================
 *  Assertion handler
 *  Halts execution with a breakpoint in Debug builds; optimised away otherwise.
 *===========================================================================*/
#if defined(DEBUG)
    #define configASSERT( x )   \
        if( ( x ) == 0 ) { taskDISABLE_INTERRUPTS(); __asm volatile("bkpt #0"); for(;;); }
#else
    #define configASSERT( x )   ( ( void )( x ) )
#endif

/*===========================================================================
 *  Map FreeRTOS port interrupt handlers to the CMSIS/HAL names used in the
 *  STM32 startup file and vector table.
 *===========================================================================*/
#define vPortSVCHandler     SVC_Handler
#define xPortPendSVHandler  PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

#endif /* FREERTOS_CONFIG_H */
