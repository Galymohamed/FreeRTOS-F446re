#pragma once


#include "uartShared.h"


#define MAX_RX_START_RETRIES ((uint8_t)5U)


typedef struct {
	const pinCfg_t *pinsCfg;
	uint8_t pinCount;
	UART_HandleTypeDef *huart;
	intrptParam_t *intrptParam;
    circularBuffer_t *txCBuf;
    circularBuffer_t *rxCBuf;
} uartCB_t;



ErrorStatus UARTCB_init(uartCB_t *uartCB);
ErrorStatus UARTCB_deInit(uartCB_t *uartCB);
ErrorStatus UARTCB_write(uartCB_t *uartCB, uint8_t data);
ErrorStatus UARTCB_sendArray(uartCB_t *uartCB, const uint8_t *data, size_t len);
ErrorStatus UARTCB_sendString(uartCB_t *uartCB, const char *s);
ErrorStatus UARTCB_clearRxBuff(uartCB_t *uartCB);
ErrorStatus UARTCB_peek(uartCB_t *uartCB, uint8_t *out);
ErrorStatus UARTCB_read(uartCB_t *uartCB, uint8_t *out);
ErrorStatus UARTCB_readArray(uartCB_t *uartCB, uint8_t* out, size_t len);
uint32_t UARTCB_isData(uartCB_t *uartCB);
ErrorStatus UARTCB_isResponse_ASCII(uartCB_t *uartCB, const char *str, uint32_t timeout);
ErrorStatus UARTCB_isResponse_BIN(uartCB_t *uartCB, const uint8_t *pattern, size_t pattern_len, uint16_t timeout);
ErrorStatus UARTCB_getNextstrg(uartCB_t *uartCB, const char *str,
							 uint8_t num_of_chars,
							 uint8_t *dest_buffer,
							 uint32_t timeout);
ErrorStatus UARTCB_copyUpToString(uartCB_t *uartCB, const char *str,
								uint8_t *dest,
								uint32_t timeout);


/*=============================================================================================================*/
/*===================================== UART Circular Buffer Interrupt Handler ================================*/
__attribute__((always_inline)) static inline void UARTCB_intrptCallback(uartCB_t *uartCB)
{
	USART_TypeDef *UARTx = uartCB->huart->Instance;
	UART_HandleTypeDef *huart = uartCB->huart;
    circularBuffer_t *rx = uartCB->rxCBuf;
    circularBuffer_t *tx = uartCB->txCBuf;

    /* Refresh flags in case they changed while handling errors */
    uint32_t sr  = UARTx->SR;
    uint32_t cr1 = UARTx->CR1;

    /* ----------- RX Handler ----------- */
    if (rx != NULL)
    {
        if (((sr & USART_SR_RXNE) != 0U) && ((cr1 & USART_CR1_RXNEIE) != 0U))
        {
            /* RXNE set: read DR once and store into RX circular buffer */
            uint8_t received = (uint8_t)(UARTx->DR & 0xFFu);
            cBuff_storeChar(rx, received);
        }
    }
    else
    {
        /* No RX buffer: still must read DR to clear RXNE, then stop RX interrupt */
        if (((sr & USART_SR_RXNE) != 0U) && ((cr1 & USART_CR1_RXNEIE) != 0U))
        {
            volatile uint32_t tmp = UARTx->DR;
            (void)tmp;
            UART_endRxIntrpt(huart);
        }
    }

    /* ----------- TX Handler (TXE) ----------- */
    if (tx != NULL)
    {
        if (((sr & USART_SR_TXE) != 0U) && ((cr1 & USART_CR1_TXEIE) != 0U))
        {
            if (tx->head == tx->tail)
            {
                /* No more data: disable TXE, enable TC to catch final shift */
                __HAL_UART_DISABLE_IT(huart, UART_IT_TXE);
                __HAL_UART_ENABLE_IT(huart, UART_IT_TC);
            }
            else
            {
            	UARTx->DR = tx->buffer[tx->tail];
                if (tx->maxSize != 0U) {
                    tx->tail = (tx->tail + 1u) % tx->maxSize;
                }
            }
        }
    }
    else
    {
        /* No TX buffer: make sure TX side is fully disabled/closed */
        UART_endTxIntrpt(huart);
    }
    /* ----------- TX Complete (TC) ----------- */
    if (((sr & USART_SR_TC) != 0U) && ((cr1 & USART_CR1_TCIE) != 0U))
    {
        UART_endTxIntrpt(huart);
    }
}
/*=============================================================================================================*/


