#include "uart.h"




ErrorStatus UART_init(uart_t *uart)
{
    ErrorStatus status = ERROR;

    if ((uart != NULL) && (uart->huart != NULL))
    {
        /*=============================== Initialise GPIO pins first ===============================*/
        if ((uart->pinsCfg != NULL) && (uart->pinCount > 0U))
        {
            status = GPIO_initMulti(uart->pinsCfg, (uint8_t)uart->pinCount);
        }
        /*==========================================================================================*/

        /*======================== Enable peripheral clock & Initialise UART =======================*/
        if (status == SUCCESS)
        {
            ErrorStatus clkStatus = USARTx_enableCLK(uart->huart->Instance);
            if (clkStatus != SUCCESS)
            {
                status = ERROR;
            }
            else
            {
                HAL_StatusTypeDef halInitStatus = HAL_UART_Init(uart->huart);

                /*====================== Configure NVIC for UART interrupt if required =====================*/
                if (uart->intrptParam != NULL)
                {
                    if (uart->intrptParam->intrptCount != 1U)
                    {
                        status = ERROR;
                    }
                    else
                    {
                        /* Clear interrupt flags BEFORE enabling IRQ */
                        uart->intrptParam->intptFlg = 0U;
                        HAL_NVIC_SetPriority(uart->intrptParam->irqn[0],
                                             uart->intrptParam->intrptPriority[0], 0U);
                        HAL_NVIC_EnableIRQ(uart->intrptParam->irqn[0]);
                    }
                }
                /*==========================================================================================*/

                status = (halInitStatus == HAL_OK) ? SUCCESS : ERROR;
            }
        }
    }
    /* else: invalid parameter — status stays ERROR */
    return status;
}


ErrorStatus UART_deInit(uart_t *uart)
{
    ErrorStatus status = ERROR;

    if ((uart != NULL) && (uart->huart != NULL))
    {
        /*=============================== De-initialise UART peripheral ============================*/
        HAL_StatusTypeDef halDeInitStatus = HAL_UART_DeInit(uart->huart);
        /*==========================================================================================*/
        /*======================== Disable peripheral clock ========================================*/
        ErrorStatus clkStatus = USARTx_disableCLK(uart->huart->Instance);
        /*==========================================================================================*/

        /* GPIO deInit */
        if ((uart->pinsCfg == NULL) || (uart->pinCount == 0U))
        {
            /* No pins is not an error */
            status = SUCCESS;
        }
        else
        {
            status = GPIO_deInitMulti(uart->pinsCfg, (uint8_t)uart->pinCount);
        }

        if ((halDeInitStatus != HAL_OK) || (clkStatus != SUCCESS))
        {
            status = ERROR;
        }
    }
    /* else: invalid parameter — status stays ERROR */

    return status;
}


ErrorStatus UART_busRecover(uart_t *uart)
{
    ErrorStatus status = ERROR;
    ErrorStatus deInitStatus = UART_deInit(uart);
    ErrorStatus initStatus   = UART_init(uart);

    if ((deInitStatus == SUCCESS) && (initStatus == SUCCESS))
    {
        status = SUCCESS;
    }
    return status;
}


static inline void UART_handlErr(uart_t *uart, uint32_t err)
{
    if (uart == NULL)
    {
        return;
    }
    uart->errCnt++;
    uart->errCode = err;

    if (uart->errCnt >= UART_ERR_LIM)
    {
        (void)UART_busRecover(uart);
        uart->errCnt = 0U;
    }
}


ErrorStatus UART_txPoll(uart_t *uart, uint8_t *txData, uint16_t dataSZ, uint32_t timeOut_ms)
{
    ErrorStatus status = ERROR;

    if ((uart != NULL) && (uart->huart != NULL) && (txData != NULL) && (dataSZ > 0U))
    {
        HAL_StatusTypeDef halStatus = HAL_UART_Transmit(uart->huart, txData, dataSZ, timeOut_ms);

        if (halStatus != HAL_OK)
        {
            uint32_t err = HAL_UART_GetError(uart->huart);
            UART_handlErr(uart, err);
            status = ERROR;
        }
        else
        {
            status = SUCCESS;
        }
    }
    /* else: invalid parameter — status stays ERROR */

    return status;
}


ErrorStatus UART_rxPoll(uart_t *uart, uint8_t *rxData, uint16_t dataSZ, uint32_t timeOut_ms)
{
    ErrorStatus status = ERROR;

    if ((uart != NULL) && (uart->huart != NULL) && (rxData != NULL) && (dataSZ > 0U))
    {
        HAL_StatusTypeDef halStatus = HAL_UART_Receive(uart->huart, rxData, dataSZ, timeOut_ms);

        if (halStatus != HAL_OK)
        {
            uint32_t err = HAL_UART_GetError(uart->huart);
            UART_handlErr(uart, err);
            status = ERROR;
        }
        else
        {
            status = SUCCESS;
        }
    }
    /* else: invalid parameter — status stays ERROR */

    return status;
}


ErrorStatus UART_txIT(uart_t *uart, uint8_t *txData, uint16_t dataSZ, uint32_t timeOut_ms)
{
    ErrorStatus status = ERROR;

    if ((uart != NULL) && (uart->huart != NULL) && (txData != NULL) && (dataSZ > 0U) && (uart->intrptParam != NULL))
    {
        HAL_StatusTypeDef halStatus = HAL_OK;
        uint32_t err = 0U;

        /* Reset interrupt flags */
        CLEAR_BIT(uart->intrptParam->intptFlg, IT_TX_CMPLT_FLAG);
        CLEAR_BIT(uart->intrptParam->intptFlg, IT_ERR_FLAG);
        CLEAR_BIT(uart->intrptParam->intptFlg, IT_TIMEOUT_FLAG);

        halStatus = HAL_UART_Transmit_IT(uart->huart, (const uint8_t*)txData, dataSZ);
        if (halStatus != HAL_OK)
        {
            err = HAL_UART_GetError(uart->huart);
            UART_handlErr(uart, err);
            status = ERROR;
        }
        else
        {
            /* Wait for complete or error */
            ErrorStatus w8IT = w8ITFlg(&uart->intrptParam->intptFlg,
                                       (IT_TX_CMPLT_FLAG | IT_ERR_FLAG),
                                       timeOut_ms);

            if (w8IT != SUCCESS)
            {
                SET_BIT(uart->intrptParam->intptFlg, IT_TIMEOUT_FLAG);
                (void)HAL_UART_Abort_IT(uart->huart);
                err = ERROR_IT_TIMOUT_CUSTUM;
                UART_handlErr(uart, err);
                status = ERROR;
            }
            else if (IS_BIT_SET(uart->intrptParam->intptFlg, IT_ERR_FLAG))
            {
                err = HAL_UART_GetError(uart->huart);
                UART_handlErr(uart, err);
                status = ERROR;
            }
            else
            {
                status = SUCCESS;
            }
        }
    }
    /* else: invalid parameter — status stays ERROR */

    return status;
}


ErrorStatus UART_rxIT(uart_t *uart, uint8_t *rxData, uint16_t dataSZ, uint32_t timeOut_ms)
{
    ErrorStatus status = ERROR;

    if ((uart != NULL) && (uart->huart != NULL) && (rxData != NULL) && (dataSZ > 0U) && (uart->intrptParam != NULL))
    {
        HAL_StatusTypeDef halStatus = HAL_OK;
        uint32_t err = 0U;

        /* Reset interrupt flags */
        CLEAR_BIT(uart->intrptParam->intptFlg, IT_RX_CMPLT_FLAG);
        CLEAR_BIT(uart->intrptParam->intptFlg, IT_ERR_FLAG);
        CLEAR_BIT(uart->intrptParam->intptFlg, IT_TIMEOUT_FLAG);

        halStatus = HAL_UART_Receive_IT(uart->huart, (uint8_t*)rxData, dataSZ);
        if (halStatus != HAL_OK)
        {
            err = HAL_UART_GetError(uart->huart);
            UART_handlErr(uart, err);
            status = ERROR;
        }
        else
        {
            /* Wait for complete or error */
            ErrorStatus w8IT = w8ITFlg(&uart->intrptParam->intptFlg,
                                      (IT_RX_CMPLT_FLAG | IT_ERR_FLAG),
                                      timeOut_ms);

            if (w8IT != SUCCESS)
            {
                SET_BIT(uart->intrptParam->intptFlg, IT_TIMEOUT_FLAG);
                (void)HAL_UART_Abort_IT(uart->huart);
                err = ERROR_IT_TIMOUT_CUSTUM;
                UART_handlErr(uart, err);
                status = ERROR;
            }
            else if (IS_BIT_SET(uart->intrptParam->intptFlg, IT_ERR_FLAG))
            {
                err = HAL_UART_GetError(uart->huart);
                UART_handlErr(uart, err);
                status = ERROR;
            }
            else
            {
                status = SUCCESS;
            }
        }
    }
    /* else: invalid parameter — status stays ERROR */

    return status;
}


ErrorStatus UART_txDMA(uart_t *uart, uint8_t *txData, uint16_t dataSZ, uint32_t timeOut_ms)
{
    ErrorStatus status = ERROR;

    if ((uart != NULL) && (uart->huart != NULL) && (txData != NULL) && (dataSZ > 0U) && (uart->intrptParam != NULL))
    {
        HAL_StatusTypeDef halStatus = HAL_OK;
        uint32_t err = 0U;

        /* Reset interrupt flags */
        CLEAR_BIT(uart->intrptParam->intptFlg, IT_TX_CMPLT_FLAG);
        CLEAR_BIT(uart->intrptParam->intptFlg, IT_ERR_FLAG);
        CLEAR_BIT(uart->intrptParam->intptFlg, IT_TIMEOUT_FLAG);

        halStatus = HAL_UART_Transmit_DMA(uart->huart, (uint8_t*)txData, dataSZ);
        if (halStatus != HAL_OK)
        {
            err = HAL_UART_GetError(uart->huart);
            UART_handlErr(uart, err);
            status = ERROR;
        }
        else
        {
            /* Wait for complete or error */
            ErrorStatus w8IT = w8ITFlg(&uart->intrptParam->intptFlg,
                                      (IT_TX_CMPLT_FLAG | IT_ERR_FLAG),
                                      timeOut_ms);

            if (w8IT != SUCCESS)
            {
                SET_BIT(uart->intrptParam->intptFlg, IT_TIMEOUT_FLAG);

                /* Abort DMA + UART state */
                if (uart->huart->hdmatx != NULL) { (void)HAL_DMA_Abort(uart->huart->hdmatx); }
                (void)HAL_UART_AbortTransmit(uart->huart);

                err = ERROR_IT_TIMOUT_CUSTUM;
                UART_handlErr(uart, err);
                status = ERROR;
            }
            else if (IS_BIT_SET(uart->intrptParam->intptFlg, IT_ERR_FLAG))
            {
                err = HAL_UART_GetError(uart->huart);
                UART_handlErr(uart, err);
                status = ERROR;
            }
            else
            {
                status = SUCCESS;
            }
        }
    }
    /* else: invalid parameter — status stays ERROR */

    return status;
}


ErrorStatus UART_rxDMA(uart_t *uart, uint8_t *rxData, uint16_t dataSZ, uint32_t timeOut_ms)
{
    ErrorStatus status = ERROR;

    if ((uart != NULL) && (uart->huart != NULL) && (rxData != NULL) && (dataSZ > 0U) && (uart->intrptParam != NULL))
    {
        HAL_StatusTypeDef halStatus = HAL_OK;
        uint32_t err = 0U;

        /* Reset interrupt flags */
        CLEAR_BIT(uart->intrptParam->intptFlg, IT_RX_CMPLT_FLAG);
        CLEAR_BIT(uart->intrptParam->intptFlg, IT_ERR_FLAG);
        CLEAR_BIT(uart->intrptParam->intptFlg, IT_TIMEOUT_FLAG);

        halStatus = HAL_UART_Receive_DMA(uart->huart, (uint8_t*)rxData, dataSZ);
        if (halStatus != HAL_OK)
        {
            err = HAL_UART_GetError(uart->huart);
            UART_handlErr(uart, err);
            status = ERROR;
        }
        else
        {
            /* Wait for complete or error */
            ErrorStatus w8IT = w8ITFlg(&uart->intrptParam->intptFlg,
                                      (IT_RX_CMPLT_FLAG | IT_ERR_FLAG),
                                      timeOut_ms);

            if (w8IT != SUCCESS)
            {
                SET_BIT(uart->intrptParam->intptFlg, IT_TIMEOUT_FLAG);

                /* Abort DMA + UART state */
                if (uart->huart->hdmarx != NULL) { (void)HAL_DMA_Abort(uart->huart->hdmarx); }
                (void)HAL_UART_AbortReceive(uart->huart);

                err = ERROR_IT_TIMOUT_CUSTUM;
                UART_handlErr(uart, err);
                status = ERROR;
            }
            else if (IS_BIT_SET(uart->intrptParam->intptFlg, IT_ERR_FLAG))
            {
                err = HAL_UART_GetError(uart->huart);
                UART_handlErr(uart, err);
                status = ERROR;
            }
            else
            {
                status = SUCCESS;
            }
        }
    }
    /* else: invalid parameter — status stays ERROR */

    return status;
}
