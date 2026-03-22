#pragma once


#include "config.h"


/*======================================================================================================*/
/*=================================== Board PushButton Led =============================================*/
#define BOARD_PSHBTN_PORT					GPIOC
#define BOARD_PSHBTN_PIN					GPIO_PIN_13
#define BOARD_PSHBTN_IT_IRQN				EXTI15_10_IRQn
#define BOARD_PSHBTN_IT_PRIO				1U				

#define BOARD_LED_PORT						GPIOA
#define BOARD_LED_PIN						GPIO_PIN_5

/*======================================================================================================*/
/*=================================== USART-2 ==========================================================*/
#define UART2_TX_PORT						GPIOA
#define UART2_TX_PIN						GPIO_PIN_2
#define UART2_TX_ALT_FUN					GPIO_AF7_USART2

#define UART2_RX_PORT						GPIOA
#define UART2_RX_PIN						GPIO_PIN_3
#define UART2_RX_ALT_FUN					GPIO_AF7_USART2


