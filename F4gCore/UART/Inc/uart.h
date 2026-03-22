#pragma once


#include "uartShared.h"
#include "uartCB.h"


#define UART_ERR_LIM						((uint8_t)5U)


/*===========================================================================================================*/
/*======================================== UART Driver Structure ============================================*/
typedef struct {
	const pinCfg_t *pinsCfg;
	uint8_t pinCount;
	UART_HandleTypeDef *huart;
	intrptParam_t *intrptParam;
	uint32_t errCode;
	uint8_t errCnt;
} uart_t;
/*=============================================================================================================*/


/*======================================== UART Driver Functions ============================================*/
ErrorStatus UART_init(uart_t *uart);
ErrorStatus UART_deInit(uart_t *uart);
ErrorStatus UART_busRecover(uart_t *uart);
/*=============================================================================================================*/

ErrorStatus UART_txPoll(uart_t *uart, uint8_t *txData, uint16_t dataSZ, uint32_t timeOut_ms);
ErrorStatus UART_rxPoll(uart_t *uart, uint8_t *rxData, uint16_t dataSZ, uint32_t timeOut_ms);

ErrorStatus UART_txIT(uart_t *uart, uint8_t *txData, uint16_t dataSZ, uint32_t timeOut_ms);
ErrorStatus UART_rxIT(uart_t *uart, uint8_t *rxData, uint16_t dataSZ, uint32_t timeOut_ms);

ErrorStatus UART_txDMA(uart_t *uart, uint8_t *txData, uint16_t dataSZ, uint32_t timeOut_ms);
ErrorStatus UART_rxDMA(uart_t *uart, uint8_t *rxData, uint16_t dataSZ, uint32_t timeOut_ms);


extern uart_t uart2;
extern uartCB_t uart2CB;
extern bool uart2_isUseCB;
