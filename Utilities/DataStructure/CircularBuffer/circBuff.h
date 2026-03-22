#pragma once


#include "sysGlob.h"
#include <stdint.h>


#define CIRC_BUFF_ERROR          0xFFU


typedef struct {
    uint8_t *buffer;     
    volatile size_t head;      
    volatile size_t tail;      
    size_t maxSize;    
} circularBuffer_t;


typedef struct {
    const char *pattern;       
    size_t len;           
    size_t pos;           
    uint32_t start_tick;   
    uint32_t timeout;      
    uint8_t active;        
} cBuff_Matcher_t;


static inline void circBuff_reset(circularBuffer_t *cBuf)
{
    if (cBuf == NULL) { return; }
    cBuf->head = 0U;
    cBuf->tail = 0U;
}

static inline bool cBuff_isEmpty(circularBuffer_t *cBuf)
{
    if (cBuf == NULL) { return true; }
    return (cBuf->head == cBuf->tail);
}

static inline void circBuff_flush(circularBuffer_t *cBuf)
{
    if (cBuf == NULL) { return; }
    cBuf->tail = cBuf->head;
}

static inline size_t circBuff_capacity(circularBuffer_t *cBuf)
{
    if (cBuf == NULL) { return 0U; }
    return cBuf->maxSize;
}

static inline size_t circBuff_size(circularBuffer_t *cBuf)
{
    if (cBuf == NULL) { return 0U; }

    size_t head = cBuf->head;
    size_t tail = cBuf->tail;
    size_t max  = cBuf->maxSize;

    return (head + max - tail) % max;
}

static inline size_t circBuff_freeSpace(circularBuffer_t *cBuf)
{
    if (cBuf == NULL) { return 0U; }

    size_t head = cBuf->head;
    size_t tail = cBuf->tail;
    size_t max  = cBuf->maxSize;
    size_t used = (head + max - tail) % max;

    return (max - 1U) - used;  
}


void circBuff_init(circularBuffer_t *cBuf, uint8_t *buffer, size_t size);

ErrorStatus cBuff_write(circularBuffer_t *xferBuff, uint8_t data);
ErrorStatus cBuff_sendString(circularBuffer_t *xferBuff, const char *s);
ErrorStatus cBuff_sendArray(circularBuffer_t *xferBuff, const uint8_t *data, size_t len);

ErrorStatus cBuff_storeChar(circularBuffer_t *rcvBuf, uint8_t data);
ErrorStatus cBuff_peek(circularBuffer_t *rcvBuf, uint8_t *out);
uint8_t     cBuff_returnPeek(circularBuffer_t *rcvBuf);
ErrorStatus cBuff_read(circularBuffer_t *rcvBuf, uint8_t *out);
uint8_t     cBuff_returnRead(circularBuffer_t *rcvBuf);
size_t      cBuff_readArray(circularBuffer_t *rcvBuf, uint8_t *out, size_t len);

uint32_t    cBuff_isData(circularBuffer_t *rcvBuff);
ErrorStatus cBuff_getAChar(circularBuffer_t *cBuf, uint8_t target, uint32_t timeout);
ErrorStatus cBuff_isResponse_ASCII(circularBuffer_t *cBuf, const char *str, uint32_t timeout);
ErrorStatus cBuff_contains_ASCII_noLose(circularBuffer_t *cBuf, const char *str, uint32_t timeout);
ErrorStatus cBuff_isResponse_BIN(circularBuffer_t *cBuf,
                                 const uint8_t *pattern,
                                 size_t pattern_len,
                                 uint32_t timeout);
ErrorStatus cBuff_getNextstrg(circularBuffer_t *rcvBuff,
                              const char *str,
                              uint8_t num_of_chars,
                              uint8_t *dest_buffer,
                              uint32_t timeout);
ErrorStatus cBuff_copyUpToString(circularBuffer_t *buf,
                                 const char *str,
                                 uint8_t *dest,
                                 uint32_t timeout);

void        cBuff_matchStart(cBuff_Matcher_t *m, const char *pattern, uint32_t timeout);
ErrorStatus cBuff_matchProcess(cBuff_Matcher_t *m, circularBuffer_t *cBuf);
void        cBuff_matchReset(cBuff_Matcher_t *m);
