#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif


#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"

#include "sysGlob.h"

#include "gpio.h"
#include "uart.h"

#include "uartCB.h"
#include "brd_HWU.h"
#include "bsp_uart2.h"




ErrorStatus g_sysInit(void);


#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
