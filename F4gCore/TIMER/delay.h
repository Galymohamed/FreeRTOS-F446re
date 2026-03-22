#pragma once



#include "f4gCore.h"
#include "intrpt.h"

void delay_us(uint32_t us);
ErrorStatus w8ITFlg(volatile uint32_t *flagReg, uint32_t mask, uint32_t timeOut_ms);
ErrorStatus w8ITFlg_STime(volatile uint32_t *flagReg, uint32_t mask, uint32_t strtTime, uint32_t timeOut_ms);
ErrorStatus w8Flg_STime(volatile uint32_t *flagReg, uint32_t mask, uint32_t strtTime, uint32_t timeOut_ms);
ErrorStatus w8Flg(volatile uint32_t *flagReg, uint32_t mask, uint32_t timeOut_ms);

