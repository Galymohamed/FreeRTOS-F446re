#include "delay.h"


/* Delay for approximately `us` microseconds using DWT cycle counter.
 * Maximum safe input: UINT32_MAX / TICKS_PER_US microseconds.
 * Cast through uint64_t to prevent silent wrap of the tick count. */
void delay_us(uint32_t us)
{
    uint32_t start  = DWT->CYCCNT;
    uint32_t ticks  = (uint32_t)((uint64_t)us * TICKS_PER_US);

    while ((DWT->CYCCNT - start) < ticks)
    {
        __NOP();
    }
}


ErrorStatus w8ITFlg_STime(volatile uint32_t *flagReg, uint32_t mask, uint32_t strtTime, uint32_t timeOut_ms)
{
    ErrorStatus result = SUCCESS;

    if ((flagReg == NULL) || (mask == 0U))
    {
        result = ERROR;
    }
    else
    {
        uint32_t start = (strtTime != 0U) ? strtTime : HAL_GetTick();

        /* Wait until any requested flag appears */
        while (((*flagReg) & mask) == 0U)
        {
            /* Timeout handling */
            if (timeOut_ms != HAL_MAX_DELAY)
            {
                uint32_t now = HAL_GetTick();
                if ((now - start) >= timeOut_ms)
                {
                    *flagReg |= IT_TIMEOUT_FLAG;
                    result = ERROR;
                    break;
                }
            }
            __WFI();    /* Sleep until the next interrupt */
        }
    }

    return result;
}


ErrorStatus w8ITFlg(volatile uint32_t *flagReg, uint32_t mask, uint32_t timeOut_ms)
{
    ErrorStatus result = SUCCESS;

    if ((flagReg == NULL) || (mask == 0U))
    {
        result = ERROR;
    }
    else
    {
        uint32_t start = HAL_GetTick();

        /* Wait until any requested flag appears */
        while (((*flagReg) & mask) == 0U)
        {
            /* Timeout handling */
            if (timeOut_ms != HAL_MAX_DELAY)
            {
                uint32_t now = HAL_GetTick();
                if ((now - start) >= timeOut_ms)
                {
                    *flagReg |= IT_TIMEOUT_FLAG;
                    result = ERROR;
                    break;
                }
            }
            __WFI();    /* Sleep until the next interrupt */
        }
    }

    return result;
}


ErrorStatus w8Flg_STime(volatile uint32_t *flagReg, uint32_t mask, uint32_t strtTime, uint32_t timeOut_ms)
{
    ErrorStatus result = SUCCESS;

    if ((flagReg == NULL) || (mask == 0U))
    {
        result = ERROR;
    }
    else
    {
        uint32_t start = (strtTime != 0U) ? strtTime : HAL_GetTick();

        /* Wait until any requested flag appears */
        while (((*flagReg) & mask) == 0U)
        {
            /* Timeout handling */
            if (timeOut_ms != HAL_MAX_DELAY)
            {
                uint32_t now = HAL_GetTick();
                if ((now - start) >= timeOut_ms)
                {
                    *flagReg |= IT_TIMEOUT_FLAG;
                    result = ERROR;
                    break;
                }
            }
            __WFI();    /* Sleep until the next interrupt */
        }
    }

    return result;
}


ErrorStatus w8Flg(volatile uint32_t *flagReg, uint32_t mask, uint32_t timeOut_ms)
{
    ErrorStatus result = SUCCESS;

    if ((flagReg == NULL) || (mask == 0U))
    {
        result = ERROR;
    }
    else
    {
        uint32_t start = HAL_GetTick();

        /* Wait until any requested flag appears */
        while (((*flagReg) & mask) == 0U)
        {
            /* Timeout handling */
            if (timeOut_ms != HAL_MAX_DELAY)
            {
                uint32_t now = HAL_GetTick();
                if ((now - start) >= timeOut_ms)
                {
                    *flagReg |= IT_TIMEOUT_FLAG;
                    result = ERROR;
                    break;
                }
            }
            __WFI();    /* Sleep until the next interrupt */
        }
    }

    return result;
}
