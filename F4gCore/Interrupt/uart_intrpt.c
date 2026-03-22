#include "uart.h"
#include "uartCB.h"


void USART2_IRQHandler(void)
{
    if (uart2_isUseCB)
    {
        UARTCB_intrptCallback(&uart2CB);
    }
    else
    {
        HAL_UART_IRQHandler(uart2.huart);
    }
}


void DMA1_Stream6_IRQHandler(void)
{
    if ((uart2.huart != NULL) && (uart2.huart->hdmatx != NULL))
    {
        HAL_DMA_IRQHandler(uart2.huart->hdmatx);
    }
}


void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if ((huart != NULL) && (huart->Instance == USART2))
    {
        if (uart2.intrptParam != NULL)
        {
            SET_BIT(uart2.intrptParam->intptFlg, IT_TX_CMPLT_FLAG);
        }
    }
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if ((huart != NULL) && (huart->Instance == USART2))
    {
        if (uart2.intrptParam != NULL)
        {
            SET_BIT(uart2.intrptParam->intptFlg, IT_RX_CMPLT_FLAG);
        }
    }
}


void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if ((huart != NULL) && (huart->Instance == USART2))
    {
        if (uart2.intrptParam != NULL)
        {
            SET_BIT(uart2.intrptParam->intptFlg, IT_ERR_FLAG);
        }
    }
}
