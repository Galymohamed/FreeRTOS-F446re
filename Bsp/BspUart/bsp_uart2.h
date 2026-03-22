#pragma once


#include "uart.h"
#include "uartCB.h"
#include "gpio.h"
#include "circBuff.h"


/*======================================== Init =============================================*/
ErrorStatus uart2CB_init(void);
ErrorStatus uart2DMA_init(void);

/*======================================== Poll =============================================*/
ErrorStatus uart2_txPoll(uint8_t *txData, uint16_t dataSZ, uint32_t timeOut_ms);
ErrorStatus uart2_rxPoll(uint8_t *rxData, uint16_t dataSZ, uint32_t timeOut_ms);

/*======================================== IT ===============================================*/
ErrorStatus uart2_txIT(uint8_t *txData, uint16_t dataSZ, uint32_t timeOut_ms);
ErrorStatus uart2_rxIT(uint8_t *rxData, uint16_t dataSZ, uint32_t timeOut_ms);

/*======================================== DMA ==============================================*/
ErrorStatus uart2_txDMA(uint8_t *txData, uint16_t dataSZ, uint32_t timeOut_ms);
ErrorStatus uart2_rxDMA(uint8_t *rxData, uint16_t dataSZ, uint32_t timeOut_ms);

/*======================================== Circular Buffer ==================================*/
ErrorStatus uart2_cbWrite(uint8_t data);
ErrorStatus uart2_cbSendArray(const uint8_t *data, size_t len);
ErrorStatus uart2_cbSendString(const char *s);
ErrorStatus uart2_cbClearRxBuff(void);
ErrorStatus uart2_cbPeek(uint8_t *out);
ErrorStatus uart2_cbRead(uint8_t *out);
ErrorStatus uart2_cbReadArray(uint8_t *out, size_t len);
uint32_t uart2_cbIsData(void);
ErrorStatus uart2_cbIsResponseASCII(const char *str, uint32_t timeout);
ErrorStatus uart2_cbIsResponseBIN(const uint8_t *pattern, size_t pattern_len, uint16_t timeout);
ErrorStatus uart2_cbGetNextstrg(const char *str, uint8_t num_of_chars, uint8_t *dest_buffer, uint32_t timeout);
ErrorStatus uart2_cbCopyUpToString(const char *str, uint8_t *dest, uint32_t timeout);
