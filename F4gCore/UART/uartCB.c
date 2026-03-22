#include "uartCB.h"




ErrorStatus UARTCB_init(uartCB_t *uartCB)
{
    ErrorStatus status = ERROR;

    if ((uartCB != NULL) && (uartCB->huart != NULL))
    {
        UART_HandleTypeDef *huart = uartCB->huart;

        /*=============================== Initialise GPIO pins first ===============================*/
        if ((uartCB->pinsCfg != NULL) && (uartCB->pinCount > 0U))
        {
            status = GPIO_initMulti(uartCB->pinsCfg, (uint8_t)uartCB->pinCount);
        }
        /*==========================================================================================*/

        /*======================== Enable peripheral clock & Initialise UART =======================*/
        if (status == SUCCESS)
        {
            ErrorStatus clkStatus = USARTx_enableCLK(uartCB->huart->Instance);
            if (clkStatus != SUCCESS)
            {
                status = ERROR;
            }
            else
            {
                HAL_StatusTypeDef halInitStatus = HAL_UART_Init(uartCB->huart);

                /*====================== Configure NVIC for UART interrupt if required =====================*/
                if (uartCB->intrptParam != NULL)
                {
                    if (uartCB->intrptParam->intrptCount != 1U)
                    {
                        status = ERROR;
                    }
                    else
                    {
                        /* Clear interrupt flags BEFORE enabling IRQ */
                        uartCB->intrptParam->intptFlg = 0U;
                        HAL_NVIC_SetPriority(uartCB->intrptParam->irqn[0],
                                             uartCB->intrptParam->intrptPriority[0], 0U);
                        HAL_NVIC_EnableIRQ(uartCB->intrptParam->irqn[0]);

                        /*============================================================*/
                        if (uartCB->rxCBuf != NULL)
                        {
                            /* if RX-Circular buffer is present then enable receive&errors interrupts */
                            uint8_t retries = 0U;
                            do {
                                status = UART_strtRCVIntrpt(huart);
                                if (retries < MAX_RX_START_RETRIES)
                                {
                                    retries++;
                                }
                                else
                                {
                                    status = ERROR; /* Failed to start RX after several trials */
                                    break;
                                }
                            } while (status != SUCCESS);
                        }
                        /*============================================================*/
                    }
                }
                /*==========================================================================================*/

                if (status == SUCCESS)
                {
                    status = (halInitStatus == HAL_OK) ? SUCCESS : ERROR;
                }
            }
        }
    }
    /* else: invalid parameter — status stays ERROR */

    return status;
}


ErrorStatus UARTCB_deInit(uartCB_t *uartCB)
{
    ErrorStatus status = ERROR;

    if ((uartCB != NULL) && (uartCB->huart != NULL))
    {
        /*=============================== De-initialise UART peripheral ============================*/
        HAL_StatusTypeDef halDeInitStatus = HAL_UART_DeInit(uartCB->huart);
        /*==========================================================================================*/
        /*======================== Disable peripheral clock ========================================*/
        ErrorStatus clkStatus = USARTx_disableCLK(uartCB->huart->Instance);
        /*==========================================================================================*/

        if (uartCB->rxCBuf != NULL)
        {
            /* De-initialize RX circular buffer if present */
            circBuff_reset(uartCB->rxCBuf);
        }
        if (uartCB->txCBuf != NULL)
        {
            /* De-initialize TX circular buffer if present */
            circBuff_reset(uartCB->txCBuf);
        }

        /* GPIO deInit */
        if ((uartCB->pinsCfg == NULL) || (uartCB->pinCount == 0U))
        {
            /* No pins is not an error */
            status = SUCCESS;
        }
        else
        {
            status = GPIO_deInitMulti(uartCB->pinsCfg, (uint8_t)uartCB->pinCount);
        }

        if ((halDeInitStatus != HAL_OK) || (clkStatus != SUCCESS))
        {
            status = ERROR;
        }
    }
    /* else: invalid parameter — status stays ERROR */

    return status;
}

/*================================================================================================*/
/*================================  Circular buffer Interrupts Functions =========================*/
/*================================================================================================*/
ErrorStatus UARTCB_write(uartCB_t *uartCB, uint8_t data)
{
    ErrorStatus status = ERROR;

    if ((uartCB != NULL) && (uartCB->huart != NULL) && (uartCB->txCBuf != NULL))
    {
        status = cBuff_write(uartCB->txCBuf, data);
        if (status == SUCCESS)
        {
            /* Enable TXE interrupt to start transmission */
            status = UART_strtXferIntrpt(uartCB->huart);
        }
    }
    /* else: invalid parameter — status stays ERROR */

    return status;
}


ErrorStatus UARTCB_sendArray(uartCB_t *uartCB, const uint8_t *data, size_t len)
{
    ErrorStatus status = ERROR;

    if ((uartCB != NULL) && (uartCB->huart != NULL) && (uartCB->txCBuf != NULL) && (data != NULL) && (len > 0U))
    {
        status = cBuff_sendArray(uartCB->txCBuf, data, len);
        if (status == SUCCESS)
        {
            /* Enable TXE interrupt to start transmission */
            status = UART_strtXferIntrpt(uartCB->huart);
        }
    }
    /* else: invalid parameter — status stays ERROR */

    return status;
}


ErrorStatus UARTCB_sendString(uartCB_t *uartCB, const char *s)
{
    ErrorStatus status = ERROR;

    if ((uartCB != NULL) && (uartCB->huart != NULL) && (uartCB->txCBuf != NULL) && (s != NULL))
    {
        status = cBuff_sendString(uartCB->txCBuf, s);
        if (status == SUCCESS)
        {
            /* Enable TXE interrupt to start transmission */
            status = UART_strtXferIntrpt(uartCB->huart);
        }
    }
    /* else: invalid parameter — status stays ERROR */

    return status;
}

ErrorStatus UARTCB_clearRxBuff(uartCB_t *uartCB)
{
    ErrorStatus status = ERROR;

    if ((uartCB != NULL) && (uartCB->rxCBuf != NULL))
    {
        circBuff_reset(uartCB->rxCBuf);
        status = SUCCESS;
    }
    /* else: invalid parameter — status stays ERROR */

    return status;
}

ErrorStatus UARTCB_peek(uartCB_t *uartCB, uint8_t *out)
{
    ErrorStatus status = ERROR;

    if ((uartCB != NULL) && (uartCB->huart != NULL) && (uartCB->rxCBuf != NULL) && (out != NULL))
    {
        status = UART_strtRCVIntrpt(uartCB->huart);
        if (status == SUCCESS)
        {
            status = cBuff_peek(uartCB->rxCBuf, out);
        }
    }
    /* else: invalid parameter — status stays ERROR */

    return status;
}


ErrorStatus UARTCB_read(uartCB_t *uartCB, uint8_t *out)
{
    ErrorStatus status = ERROR;

    if ((uartCB != NULL) && (uartCB->huart != NULL) && (uartCB->rxCBuf != NULL) && (out != NULL))
    {
        status = UART_strtRCVIntrpt(uartCB->huart);
        if (status == SUCCESS)
        {
            status = cBuff_read(uartCB->rxCBuf, out);
        }
    }
    /* else: invalid parameter — status stays ERROR */

    return status;
}


ErrorStatus UARTCB_readArray(uartCB_t *uartCB, uint8_t* out, size_t len)
{
    ErrorStatus status = ERROR;

    if ((uartCB != NULL) && (uartCB->huart != NULL) && (uartCB->rxCBuf != NULL) && (out != NULL) && (len > 0U))
    {
        status = UART_strtRCVIntrpt(uartCB->huart);
        if (status == SUCCESS)
        {
            size_t bytesRead = cBuff_readArray(uartCB->rxCBuf, out, len);
            status = (bytesRead == len) ? SUCCESS : ERROR;
        }
    }
    /* else: invalid parameter — status stays ERROR */

    return status;
}


uint32_t UARTCB_isData(uartCB_t *uartCB)
{
    uint32_t result = 0U;

    if ((uartCB != NULL) && (uartCB->huart != NULL) && (uartCB->rxCBuf != NULL))
    {
        ErrorStatus rcvStatus = UART_strtRCVIntrpt(uartCB->huart);
        if (rcvStatus == SUCCESS)
        {
            result = cBuff_isData(uartCB->rxCBuf);
        }
    }
    /* else: invalid parameter — result stays 0U */

    return result;
}


ErrorStatus UARTCB_isResponse_ASCII(uartCB_t *uartCB, const char *str, uint32_t timeout)
{
    ErrorStatus status = ERROR;

    if ((uartCB != NULL) && (uartCB->huart != NULL) && (uartCB->rxCBuf != NULL) && (str != NULL) && (timeout != 0U))
    {
        status = UART_strtRCVIntrpt(uartCB->huart);
        if (status == SUCCESS)
        {
            status = cBuff_isResponse_ASCII(uartCB->rxCBuf, str, timeout);
        }
    }
    /* else: invalid parameter — status stays ERROR */

    return status;
}


ErrorStatus UARTCB_isResponse_BIN(uartCB_t *uartCB, const uint8_t *pattern, size_t pattern_len, uint16_t timeout)
{
    ErrorStatus status = ERROR;

    if ((uartCB != NULL) && (uartCB->huart != NULL) && (uartCB->rxCBuf != NULL) && (pattern != NULL) && (pattern_len != 0U) && (timeout != 0U))
    {
        status = UART_strtRCVIntrpt(uartCB->huart);
        if (status == SUCCESS)
        {
            status = cBuff_isResponse_BIN(uartCB->rxCBuf, pattern, pattern_len, timeout);
        }
    }
    /* else: invalid parameter — status stays ERROR */

    return status;
}


ErrorStatus UARTCB_getNextstrg(uartCB_t *uartCB, const char *str,
							 uint8_t num_of_chars,
							 uint8_t *dest_buffer,
							 uint32_t timeout)
{
    ErrorStatus status = ERROR;

    if ((uartCB != NULL) && (uartCB->huart != NULL) && (uartCB->rxCBuf != NULL) && (dest_buffer != NULL) && (str != NULL) && (num_of_chars > 0U) && (timeout != 0U))
    {
        status = UART_strtRCVIntrpt(uartCB->huart);
        if (status == SUCCESS)
        {
            status = cBuff_getNextstrg(uartCB->rxCBuf,
                                       str,
                                       num_of_chars,
                                       dest_buffer,
                                       timeout);
        }
    }
    /* else: invalid parameter — status stays ERROR */

    return status;
}


ErrorStatus UARTCB_copyUpToString(uartCB_t *uartCB, const char *str,
								  uint8_t *dest,
								  uint32_t timeout)
{
    ErrorStatus status = ERROR;

    if ((uartCB != NULL) && (uartCB->huart != NULL) && (uartCB->rxCBuf != NULL) && (dest != NULL) && (str != NULL) && (timeout != 0U))
    {
        status = UART_strtRCVIntrpt(uartCB->huart);
        if (status == SUCCESS)
        {
            status = cBuff_copyUpToString(uartCB->rxCBuf,
                                          str,
                                          dest,
                                          timeout);
        }
    }
    /* else: invalid parameter — status stays ERROR */

    return status;
}

/*================================================================================================*/
