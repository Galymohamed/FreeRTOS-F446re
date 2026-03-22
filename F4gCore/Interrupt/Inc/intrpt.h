#pragma once

#include "sysGlob.h"


#define INVALID_IRQN							((IRQn_Type)(-99))
#define INVALID_IRQ_PRIO                        ((uint8_t)0xFFU)   /* 0xFF is outside valid 0-15 range */                                                                                                              

#define IT_TX_CMPLT_FLAG            			(1U << 0)
#define IT_RX_CMPLT_FLAG            			(1U << 1)
#define IT_ERR_FLAG              				(1U << 2)
#define IT_TIMEOUT_FLAG          				(1U << 3)


/* Extra define to HAL instance Error */
#define ERROR_BUSY_CUSTUM   						0x00000400U
#define ERROR_IT_TIMOUT_CUSTUM   					0x00000800U


#define MAX_IRQN_PER_PERIPH		  					2U

typedef struct {
    volatile uint32_t intptFlg;                      
    IRQn_Type irqn[MAX_IRQN_PER_PERIPH];         /* IRQ numbers  — max 2 (e.g. I2C event + error) */
    uint8_t intrptPriority[MAX_IRQN_PER_PERIPH]; /* NVIC preempt priority per IRQ */
    uint8_t intrptCount;                                                                                    
} intrptParam_t;

