#pragma once



#include "f4gCore.h"
#include "gpio.h"
#include "intrpt.h"
#include "delay.h"
#include "circBuff.h"




/*===========================================================================================================*/
/*================================ HELPER-STATIC-Inline-FUNCTIONS ===========================================*/
static inline volatile uint32_t *USARTx_getBusReg(USART_TypeDef *USARTx) {
    if (USARTx == NULL) {
        return NULL;
    } else if (USARTx == USART1 || USARTx == USART6) {
        return &RCC->APB2ENR;
    } else if (USARTx != NULL) {
        return &RCC->APB1ENR;
    } else {
        return NULL;
    }
}


static inline uint32_t USARTx_getEnableMsk(USART_TypeDef *USARTx) {
    uint32_t mask;
    if (USARTx == NULL) {
        mask = 0U;
    } else if (USARTx == USART1) {
        mask = RCC_APB2ENR_USART1EN;
    } else if (USARTx == USART2) {
        mask = RCC_APB1ENR_USART2EN;
    } else if (USARTx == USART3) {
        mask = RCC_APB1ENR_USART3EN;
    } else if (USARTx == UART4) {
        mask = RCC_APB1ENR_UART4EN;
    } else if (USARTx == UART5) {
        mask = RCC_APB1ENR_UART5EN;
    } else if (USARTx == USART6) {
        mask = RCC_APB2ENR_USART6EN;
    } else {
        mask = 0U;  /* Default case to handle invalid USARTx */
    }
    return mask;
}


/* Enable the clock for a specific USART */
static inline ErrorStatus USARTx_enableCLK(USART_TypeDef *USARTx) {
    volatile uint32_t *busReg = USARTx_getBusReg(USARTx);
    uint32_t mask = USARTx_getEnableMsk(USARTx);
    ErrorStatus status;

    if ((busReg == NULL) || (mask == 0U)) {
        status = ERROR;
    } else {
        /* Explicit read-modify-write + readback delay (same pattern as RCC_GPIOX_CLK_ENABLE) */
        __IO uint32_t tmpreg = *busReg;
        tmpreg |= mask;
        *busReg = tmpreg;
        /* Mandatory delay: read back bit just set (RM0390 SS6.3.9) */
        tmpreg = READ_BIT(*busReg, mask);
        UNUSED(tmpreg);
        status = SUCCESS;
    }
    return status;
}


/* Disable the clock for a specific USART */
static inline ErrorStatus USARTx_disableCLK(USART_TypeDef *USARTx) {
    volatile uint32_t *busReg = USARTx_getBusReg(USARTx);
    uint32_t mask = USARTx_getEnableMsk(USARTx);
    ErrorStatus status;

    if ((busReg == NULL) || (mask == 0U)) {
        status = ERROR;
    } else {
        /* Explicit read-modify-write + readback delay (same pattern as RCC_GPIOX_CLK_DISABLE) */
        __IO uint32_t tmpreg = *busReg;
        tmpreg &= ~mask;
        *busReg = tmpreg;
        /* Mandatory delay: read back register after clock change (RM0390 SS6.3.9) */
        tmpreg = READ_BIT(*busReg, mask);
        UNUSED(tmpreg);
        status = SUCCESS;
    }
    return status;
}


static inline IRQn_Type USARTx_getIrqn(USART_TypeDef *uartX)
{
    IRQn_Type irqn;
    if      (uartX == USART1) { irqn = USART1_IRQn; }
    else if (uartX == USART2) { irqn = USART2_IRQn; }
    else if (uartX == USART3) { irqn = USART3_IRQn; }
    else if (uartX == UART4)  { irqn = UART4_IRQn; }
    else if (uartX == UART5)  { irqn = UART5_IRQn; }
    else if (uartX == USART6) { irqn = USART6_IRQn; }
    else                      { irqn = INVALID_IRQN; } /* Invalid pointer */
    return irqn;
}


static inline ErrorStatus UART_strtRCVIntrpt(UART_HandleTypeDef *huart)
{
    ErrorStatus status;
    if (huart == NULL) {
        status = ERROR;  /* Invalid parameter */
    } else if (huart->RxState != HAL_UART_STATE_READY) {
        status = ERROR; /* UART not ready for RX */
    } else {
        /* Set Reception type to Standard reception */
        huart->ReceptionType = HAL_UART_RECEPTION_STANDARD;

        huart->ErrorCode = HAL_UART_ERROR_NONE;
        huart->RxState = HAL_UART_STATE_BUSY_RX;
        __HAL_UART_ENABLE_IT(huart, UART_IT_RXNE);

        status = SUCCESS;
    }
    return status;
}


static inline ErrorStatus UART_strtXferIntrpt(UART_HandleTypeDef *huart)
{
    ErrorStatus status;
    if (huart == NULL) {
        status = ERROR;  /* Invalid parameter */
    } else if (huart->gState != HAL_UART_STATE_READY) {
        status = ERROR; /* UART not ready for TX */
    } else {
        huart->ErrorCode = HAL_UART_ERROR_NONE;
        huart->gState = HAL_UART_STATE_BUSY_TX;

        /* Enable the UART Transmit data register empty Interrupt */
        __HAL_UART_ENABLE_IT(huart, UART_IT_TXE);

        status = SUCCESS;
    }
    return status;
}


/* End ongoing Tx transfer on UART peripheral (following error detection or Transmit completion) */
static inline void UART_endTxIntrpt(UART_HandleTypeDef *huart)
{
  /* Disable TXEIE and TCIE interrupts */
  ATOMIC_CLEAR_BIT(huart->Instance->CR1, (USART_CR1_TXEIE | USART_CR1_TCIE));
  /* At end of Tx process, restore huart->uartState to Ready */
  huart->gState = HAL_UART_STATE_READY;
}


/* End ongoing Rx transfer on UART peripheral (following error detection or Reception completion) */
static inline void UART_endRxIntrpt(UART_HandleTypeDef *huart)
{
  /* Disable RXNE, PE and ERR (Frame error, noise error, overrun error) interrupts */
  ATOMIC_CLEAR_BIT(huart->Instance->CR1, (USART_CR1_RXNEIE | USART_CR1_PEIE | USART_CR1_IDLEIE));
  ATOMIC_CLEAR_BIT(huart->Instance->CR3, USART_CR3_EIE);
  /* At end of Rx process, restore huart->RxState to Ready */
  huart->RxState = HAL_UART_STATE_READY;
  huart->ReceptionType = HAL_UART_RECEPTION_STANDARD;
}
/*===========================================================================================================*/


