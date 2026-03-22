#include "circBuff.h"


void circBuff_init(circularBuffer_t *cBuf, uint8_t *buffer, size_t size)
{
    if ((cBuf == NULL) || (buffer == NULL) || (size == 0U))
    {
        return;
    }

    cBuf->buffer  = buffer;
    cBuf->maxSize = size;
    cBuf->head    = 0U;
    cBuf->tail    = 0U;
}


/*===========================================================================================*/
/*============================= Circular buffer transfer functions ==========================*/

ErrorStatus cBuff_write(circularBuffer_t *xferBuff, uint8_t data)
{
    if ((xferBuff == NULL) || (xferBuff->buffer == NULL) || (xferBuff->maxSize == 0U))
    {
        return ERROR;
    }

    size_t next = (xferBuff->head + 1U) % xferBuff->maxSize;

    /* Block until space is available, ISR must advance tail.
     * Use __WFI() to yield the CPU instead of busy-spinning. */
    while (next == xferBuff->tail)
    {
        __WFI();    /* TODO: add timeout handling if needed */
    }

    /* Store byte and advance head */
    xferBuff->buffer[xferBuff->head] = data;
    xferBuff->head = next;

    return SUCCESS;
}


ErrorStatus cBuff_sendString(circularBuffer_t *xferBuff, const char *s)
{
    if ((xferBuff == NULL) || (xferBuff->buffer == NULL) ||
        (xferBuff->maxSize == 0U) || (s == NULL))
    {
        return ERROR;
    }

    ErrorStatus result = SUCCESS;

    while ((*s != '\0') && (result == SUCCESS))
    {
        result = cBuff_write(xferBuff, (uint8_t)(*s));
        s++;
    }

    return result;
}


ErrorStatus cBuff_sendArray(circularBuffer_t *xferBuff, const uint8_t *data, size_t len)
{
    if ((xferBuff == NULL) || (xferBuff->buffer == NULL) ||
        (data == NULL) || (len == 0U))
    {
        return ERROR;
    }

    ErrorStatus result = SUCCESS;

    for (size_t i = 0U; (i < len) && (result == SUCCESS); i++)
    {
        result = cBuff_write(xferBuff, data[i]);
    }

    return result;
}
/*===========================================================================================*/


/*===========================================================================================*/
/*============================= Circular buffer receive functions ===========================*/

/* Store one received byte into the circular buffer */
ErrorStatus cBuff_storeChar(circularBuffer_t *rcvBuf, uint8_t data)
{
    ErrorStatus result;

    if ((rcvBuf == NULL) || (rcvBuf->buffer == NULL) || (rcvBuf->maxSize == 0U))
    {
        result = ERROR;
    }
    else
    {
        size_t next = (rcvBuf->head + 1U) % rcvBuf->maxSize;

        if (next != rcvBuf->tail)
        {
            rcvBuf->buffer[rcvBuf->head] = data;
            rcvBuf->head = next;
            result = SUCCESS;
        }
        else
        {
            result = ERROR;     /* Buffer full — drop byte */
        }
    }

    return result;
}


/*===========================================================================================*/
/*============= Peek the next character from the buffer without removing it =================*/

ErrorStatus cBuff_peek(circularBuffer_t *rcvBuf, uint8_t *out)
{
    ErrorStatus result;

    if ((rcvBuf == NULL) || (rcvBuf->buffer == NULL) || (out == NULL))
    {
        result = ERROR;
    }
    else if (rcvBuf->head == rcvBuf->tail)
    {
        result = ERROR;     /* Buffer empty */
    }
    else
    {
        *out   = rcvBuf->buffer[rcvBuf->tail];
        result = SUCCESS;
    }

    return result;
}


/* Peek variant that returns the byte directly; CIRC_BUFF_ERROR (0xFF) on failure */
uint8_t cBuff_returnPeek(circularBuffer_t *rcvBuf)
{
    uint8_t out;

    if ((rcvBuf == NULL) || (rcvBuf->buffer == NULL))
    {
        out = CIRC_BUFF_ERROR;
    }
    else if (rcvBuf->head == rcvBuf->tail)
    {
        out = CIRC_BUFF_ERROR;  /* Buffer empty */
    }
    else
    {
        out = rcvBuf->buffer[rcvBuf->tail];
    }

    return out;
}


/*===========================================================================================*/
/*======================= Read the next character from the buffer ===========================*/

ErrorStatus cBuff_read(circularBuffer_t *rcvBuf, uint8_t *out)
{
    ErrorStatus result;

    if ((rcvBuf == NULL) || (rcvBuf->buffer == NULL) || (out == NULL))
    {
        result = ERROR;
    }
    else
    {
        /* Take a snapshot to avoid race with ISR */
        size_t head = rcvBuf->head;
        size_t tail = rcvBuf->tail;

        if (head == tail)
        {
            result = ERROR;     /* Buffer empty */
        }
        else
        {
            *out        = rcvBuf->buffer[tail];
            rcvBuf->tail = (tail + 1U) % rcvBuf->maxSize;
            result      = SUCCESS;
        }
    }

    return result;
}


/* Read variant that returns the byte directly; CIRC_BUFF_ERROR (0xFF) on failure */
uint8_t cBuff_returnRead(circularBuffer_t *rcvBuf)
{
    uint8_t out;

    if ((rcvBuf == NULL) || (rcvBuf->buffer == NULL))
    {
        out = CIRC_BUFF_ERROR;
    }
    else
    {
        /* Take a snapshot to avoid race with ISR */
        size_t head = rcvBuf->head;
        size_t tail = rcvBuf->tail;

        if (head != tail)
        {
            out          = rcvBuf->buffer[tail];
            rcvBuf->tail = (tail + 1U) % rcvBuf->maxSize;
        }
        else
        {
            out = CIRC_BUFF_ERROR;  /* Buffer empty */
        }
    }

    return out;
}


size_t cBuff_readArray(circularBuffer_t *rcvBuf, uint8_t *out, size_t len)
{
    if ((rcvBuf == NULL) || (rcvBuf->buffer == NULL) ||
        (out == NULL) || (len == 0U))
    {
        return 0U;
    }

    size_t i;
    for (i = 0U; i < len; i++)
    {
        if (cBuff_read(rcvBuf, &out[i]) != SUCCESS)
        {
            break;  /* Stop when buffer is empty */
        }
    }

    return i;   /* Number of bytes actually read */
}
/*===========================================================================================*/


/*===========================================================================================*/
/*========================== Checking if there is data in the buffer ========================*/

uint32_t cBuff_isData(circularBuffer_t *rcvBuff)
{
    if ((rcvBuff == NULL) || (rcvBuff->buffer == NULL))
    {
        return 0U;
    }

    /* Take a snapshot to avoid race with ISR */
    size_t head = rcvBuff->head;
    size_t tail = rcvBuff->tail;

    return (uint32_t)((rcvBuff->maxSize + head - tail) % rcvBuff->maxSize);
}
/*===========================================================================================*/


/*
 * Search for a specific byte in the circular buffer, consuming non-matching bytes.
 * Returns SUCCESS if the target byte is found (target byte is left in the buffer).
 * Returns ERROR on timeout.
 */
ErrorStatus cBuff_getAChar(circularBuffer_t *cBuf, uint8_t target, uint32_t timeout)
{
    ErrorStatus result = ERROR;

    if (cBuf == NULL)
    {
        return ERROR;
    }

    uint32_t tick_start = HAL_GetTick();

    while (result == ERROR)
    {
        if ((uint32_t)(HAL_GetTick() - tick_start) > timeout)
        {
            break;
        }

        if (cBuff_isData(cBuf) == 0U)
        {
            continue;
        }

        uint8_t ch = 0U;
        if (cBuff_peek(cBuf, &ch) != SUCCESS)
        {
            continue;
        }

        if (ch == target)
        {
            result = SUCCESS;   /* Match found — leave byte in buffer */
        }
        else
        {
            (void)cBuff_read(cBuf, &ch);    /* Discard non-matching byte */
        }
    }

    return result;
}


ErrorStatus cBuff_isResponse_ASCII(circularBuffer_t *cBuf, const char *str, uint32_t timeout)
{
    if ((cBuf == NULL) || (str == NULL))
    {
        return ERROR;
    }

    size_t   target_len = strlen(str);
    size_t   match_pos  = 0U;
    uint8_t  byte       = 0U;
    uint32_t start      = HAL_GetTick();
    ErrorStatus result  = ERROR;

    while (result == ERROR)
    {
        if ((uint32_t)(HAL_GetTick() - start) > timeout)
        {
            break;
        }

        if (cBuff_isData(cBuf) == 0U)
        {
            continue;
        }

        if (cBuff_read(cBuf, &byte) != SUCCESS)
        {
            continue;
        }

        if (byte == (uint8_t)str[match_pos])
        {
            match_pos++;

            if (match_pos == target_len)
            {
                result = SUCCESS;
            }
        }
        else
        {
            /* Overlap-safe restart (handles patterns like "ATAT") */
            match_pos = (byte == (uint8_t)str[0U]) ? 1U : 0U;
        }
    }

    return result;
}


/* Search without consuming bytes — uses a local scan pointer (tmp_tail).
 * Real buffer tail is untouched; the matched bytes remain available for reading. */
ErrorStatus cBuff_contains_ASCII_noLose(circularBuffer_t *cBuf, const char *str, uint32_t timeout)
{
    if ((cBuf == NULL) || (str == NULL))
    {
        return ERROR;
    }

    size_t      len       = strlen(str);
    size_t      match_pos = 0U;
    uint32_t    start     = HAL_GetTick();
    uint8_t     byte      = 0U;
    size_t      tmp_tail  = cBuf->tail;
    size_t      max       = cBuf->maxSize;
    ErrorStatus result    = ERROR;

    while (result == ERROR)
    {
        if ((uint32_t)(HAL_GetTick() - start) > timeout)
        {
            break;
        }

        /* Wait until a new byte is available at tmp_tail */
        if (tmp_tail == cBuf->head)
        {
            continue;
        }

        byte     = cBuf->buffer[tmp_tail];
        tmp_tail = (tmp_tail + 1U) % max;

        if (byte == (uint8_t)str[match_pos])
        {
            match_pos++;
            if (match_pos == len)
            {
                result = SUCCESS;
            }
        }
        else
        {
            match_pos = (byte == (uint8_t)str[0U]) ? 1U : 0U;
        }
    }

    return result;
}


ErrorStatus cBuff_isResponse_BIN(circularBuffer_t *cBuf,
                                 const uint8_t *pattern,
                                 size_t pattern_len,
                                 uint32_t timeout)
{
    if ((cBuf == NULL) || (pattern == NULL) || (pattern_len == 0U))
    {
        return ERROR;
    }

    size_t      match_pos = 0U;
    uint32_t    start     = HAL_GetTick();
    uint8_t     byte      = 0U;
    ErrorStatus result    = ERROR;

    while (result == ERROR)
    {
        if ((uint32_t)(HAL_GetTick() - start) > timeout)
        {
            break;
        }

        if (cBuff_isData(cBuf) == 0U)
        {
            continue;
        }

        if (cBuff_read(cBuf, &byte) != SUCCESS)
        {
            continue;
        }

        if (byte == pattern[match_pos])
        {
            match_pos++;

            if (match_pos == pattern_len)
            {
                result = SUCCESS;
            }
        }
        else
        {
            match_pos = (byte == pattern[0U]) ? 1U : 0U;
        }
    }

    return result;
}


ErrorStatus cBuff_getNextstrg(circularBuffer_t *rcvBuff,
                              const char *str,
                              uint8_t num_of_chars,
                              uint8_t *dest_buffer,
                              uint32_t timeout)
{
    if ((rcvBuff == NULL) || (str == NULL) || (dest_buffer == NULL) || (num_of_chars == 0U))
    {
        return ERROR;
    }

    uint32_t    start  = HAL_GetTick();
    ErrorStatus result = SUCCESS;

    /* Step 1: wait for the response string */
    if (cBuff_isResponse_ASCII(rcvBuff, str, timeout) != SUCCESS)
    {
        return ERROR;
    }

    /* Step 2: read the next num_of_chars bytes */
    uint8_t i = 0U;
    while (i < num_of_chars)
    {
        if ((uint32_t)(HAL_GetTick() - start) > timeout)
        {
            result = ERROR;
            break;
        }

        if (cBuff_isData(rcvBuff) == 0U)
        {
            continue;
        }

        uint8_t byte = 0U;
        if (cBuff_read(rcvBuff, &byte) == SUCCESS)
        {
            dest_buffer[i] = byte;
            i++;
        }
    }

    return result;
}



ErrorStatus cBuff_copyUpToString(circularBuffer_t *buf,
                                 const char *str,
                                 uint8_t *dest,
                                 uint32_t timeout)
{
    if ((buf == NULL) || (str == NULL) || (dest == NULL))
    {
        return ERROR;
    }

    uint32_t    start  = HAL_GetTick();
    size_t      pos    = 0U;
    size_t      len    = strlen(str);
    uint32_t    idx    = 0U;
    uint8_t     byte   = 0U;
    ErrorStatus result = ERROR;

    while (result == ERROR)
    {
        if ((uint32_t)(HAL_GetTick() - start) > timeout)
        {
            break;
        }

        if (cBuff_isData(buf) == 0U)
        {
            continue;
        }

        if (cBuff_read(buf, &byte) != SUCCESS)
        {
            continue;
        }

        dest[idx] = byte;
        idx++;

        if (byte == (uint8_t)str[pos])
        {
            pos++;

            if (pos == len)
            {
                result = SUCCESS;
            }
        }
        else
        {
            /* Overlap-safe restart */
            pos = (byte == (uint8_t)str[0U]) ? 1U : 0U;
        }
    }

    return result;
}


/*===========================================================================================*/
/*============================= Non-blocking matcher ========================================*/
void cBuff_matchStart(cBuff_Matcher_t *m, const char *pattern, uint32_t timeout)
{
    if ((m == NULL) || (pattern == NULL))
    {
        return;
    }

    m->pattern    = pattern;
    m->len        = strlen(pattern);
    m->pos        = 0U;
    m->timeout    = timeout;
    m->start_tick = HAL_GetTick();
    m->active     = 1U;
}


ErrorStatus cBuff_matchProcess(cBuff_Matcher_t *m, circularBuffer_t *cBuf)
{
    if ((m == NULL) || (cBuf == NULL))
    {
        return ERROR;
    }

    ErrorStatus result = ERROR;

    if (m->active == 0U)
    {
        return ERROR;   /* Nothing to match */
    }

    uint32_t now = HAL_GetTick();

    if ((now - m->start_tick) > m->timeout)
    {
        m->active = 0U;
        return ERROR;   /* Timeout */
    }

    /* Process available bytes (non-blocking) */
    uint8_t byte = 0U;
    while ((result == ERROR) && (cBuff_read(cBuf, &byte) == SUCCESS))
    {
        if (byte == (uint8_t)m->pattern[m->pos])
        {
            m->pos++;

            if (m->pos == m->len)
            {
                m->active = 0U;
                result    = SUCCESS;
            }
        }
        else
        {
            m->pos = (byte == (uint8_t)m->pattern[0U]) ? 1U : 0U;
        }
    }

    return result;  
}


void cBuff_matchReset(cBuff_Matcher_t *m)
{
    if (m == NULL)
    {
        return;
    }

    m->active = 0U;
}
