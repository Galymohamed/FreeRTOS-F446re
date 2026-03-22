#include "bsp_uart2.h"



/*====================================================================================================================*/
/*====================================================== UART-2 ======================================================*/
/*====================================================================================================================*/

/*========================================================================*/
/*=========================== UART2 TX/RX pins ===========================*/
static const pinCfg_t uart2_pins[] = {
	{
		.GPIOx           = UART2_TX_PORT,
		.pinMsk          = UART2_TX_PIN,
		.mode            = GPIO_MODE_AF_PP,
		.pullType        = GPIO_NOPULL,
		.speed           = GPIO_SPEED_FREQ_VERY_HIGH,
		.altFunc         = UART2_TX_ALT_FUN,
		.b_lockPinConfig = true,
		.extiParam		 = NULL,
		.description	 = "UART2 TX Pin"
	},
	{
		.GPIOx           = UART2_RX_PORT,
		.pinMsk          = UART2_RX_PIN,
		.mode            = GPIO_MODE_AF_PP,
		.pullType        = GPIO_NOPULL,
		.speed           = GPIO_SPEED_FREQ_VERY_HIGH,
		.altFunc         = UART2_RX_ALT_FUN,
		.b_lockPinConfig = true,
		.extiParam		 = NULL,
		.description	 = "UART2 RX Pin"
	}
};

/*========================================================================*/
/*=========================== Interrupt Handlers =========================*/
/*====================== UART2 TX/RX Circular Buffers ====================*/
#define UART2_TX_BUFF_SIZEx 				256U
#define UART2_RX_BUFF_SIZEx 				256U

static uint8_t uart2_txBufx[UART2_TX_BUFF_SIZEx] = {0};
static uint8_t uart2_rxBufx[UART2_RX_BUFF_SIZEx] = {0};

static circularBuffer_t uart2x_txCBuf = {
		.buffer  = uart2_txBufx,
		.head    = 0u,
		.tail    = 0u,
		.maxSize = UART2_TX_BUFF_SIZEx,
};

static circularBuffer_t uart2x_rxCBuf = {
		.buffer  = uart2_rxBufx,
		.head    = 0u,
		.tail    = 0u,
		.maxSize = UART2_RX_BUFF_SIZEx,
};
/*========================================================================*/
/*=========================== UART2 Interrupt Parameters =================*/
static intrptParam_t uart2_intrptParam = {
    .intptFlg       = 0U,
    .irqn           = { USART2_IRQn, INVALID_IRQN },
    .intrptPriority = { 1U, INVALID_IRQ_PRIO},
    .intrptCount    = 1U
};
/*========================================================================*/

/*========================================================================*/
/*=========================== UART2 DMA Config ===========================*/
static DMA_HandleTypeDef hdma_usart2_tx = {
	.Instance                 = DMA1_Stream6,
	.Init                     = {
		.Channel              = DMA_CHANNEL_4,
		.Direction            = DMA_MEMORY_TO_PERIPH,
		.PeriphInc            = DMA_PINC_DISABLE,
		.MemInc               = DMA_MINC_ENABLE,
		.PeriphDataAlignment  = DMA_PDATAALIGN_BYTE,
		.MemDataAlignment     = DMA_MDATAALIGN_BYTE,
		.Mode                 = DMA_NORMAL,
		.Priority             = DMA_PRIORITY_LOW,
		.FIFOMode             = DMA_FIFOMODE_DISABLE,
	}
};
/*========================================================================*/

/*========================================================================*/
/*=========================== UART2 CB Handle ============================*/
static UART_HandleTypeDef huart2CB = {
	.Instance        = USART2,
	.Init            = {
		.BaudRate     = 115200UL,
		.WordLength   = UART_WORDLENGTH_8B,
		.StopBits     = UART_STOPBITS_1,
		.Parity       = UART_PARITY_NONE,
		.Mode         = UART_MODE_TX_RX,
		.HwFlowCtl    = UART_HWCONTROL_NONE,
		.OverSampling = UART_OVERSAMPLING_16
	},
};

uartCB_t uart2CB =
{
    .huart             = &huart2CB,
    .pinsCfg           = uart2_pins,
    .pinCount          = (uint32_t)ARRAY_SIZE(uart2_pins),
	.intrptParam       = &uart2_intrptParam,
	.txCBuf            = &uart2x_txCBuf,
	.rxCBuf            = &uart2x_rxCBuf,
};

/*========================================================================*/
/*=========================== UART2 Handle ===============================*/
static UART_HandleTypeDef huart2 = {
	.Instance        = USART2,
	.Init            = {
		.BaudRate     = 115200UL,
		.WordLength   = UART_WORDLENGTH_8B,
		.StopBits     = UART_STOPBITS_1,
		.Parity       = UART_PARITY_NONE,
		.Mode         = UART_MODE_TX_RX,
		.HwFlowCtl    = UART_HWCONTROL_NONE,
		.OverSampling = UART_OVERSAMPLING_16
	},
	.hdmatx         = &hdma_usart2_tx
};

uart_t uart2 =
{
    .huart             = &huart2,
    .pinsCfg           = uart2_pins,
    .pinCount          = (uint32_t)ARRAY_SIZE(uart2_pins),
	.intrptParam       = &uart2_intrptParam,
    .errCode           = 0U,
    .errCnt            = 0U,
};

bool uart2_isUseCB = false;

/*====================================================================================================================*/
/*================================================= Init Functions ===================================================*/
/*====================================================================================================================*/
ErrorStatus uart2CB_init(void)
{
	ErrorStatus status = UARTCB_init(&uart2CB);
	if (status == SUCCESS)
	{
		uart2_isUseCB = true;
	}
	return status;
}


ErrorStatus uart2DMA_init(void)
{
	ErrorStatus status = UART_init(&uart2);
	HAL_StatusTypeDef halStatus = HAL_ERROR;

	if (status == SUCCESS)
	{
		/* DMA configuration */
		__HAL_RCC_DMA1_CLK_ENABLE();
		__HAL_LINKDMA(&huart2, hdmatx, hdma_usart2_tx);
		halStatus = HAL_DMA_Init(&hdma_usart2_tx);

		/* Enable DMA stream NVIC */
		HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 1U, 0U);
		HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);
	}

	const bool uartOk = (status    == SUCCESS);
	const bool dmaOk  = (halStatus == HAL_OK);
	return ((uartOk && dmaOk) ? SUCCESS : ERROR);
}


/*====================================================================================================================*/
/*================================================= Poll Functions ===================================================*/
/*====================================================================================================================*/
ErrorStatus uart2_txPoll(uint8_t *txData, uint16_t dataSZ, uint32_t timeOut_ms)
{
	return UART_txPoll(&uart2, txData, dataSZ, timeOut_ms);
}

ErrorStatus uart2_rxPoll(uint8_t *rxData, uint16_t dataSZ, uint32_t timeOut_ms)
{
	return UART_rxPoll(&uart2, rxData, dataSZ, timeOut_ms);
}


/*====================================================================================================================*/
/*==================================================== IT Functions ==================================================*/
/*====================================================================================================================*/
ErrorStatus uart2_txIT(uint8_t *txData, uint16_t dataSZ, uint32_t timeOut_ms)
{
	return UART_txIT(&uart2, txData, dataSZ, timeOut_ms);
}

ErrorStatus uart2_rxIT(uint8_t *rxData, uint16_t dataSZ, uint32_t timeOut_ms)
{
	return UART_rxIT(&uart2, rxData, dataSZ, timeOut_ms);
}


/*====================================================================================================================*/
/*=================================================== DMA Functions ==================================================*/
/*====================================================================================================================*/
ErrorStatus uart2_txDMA(uint8_t *txData, uint16_t dataSZ, uint32_t timeOut_ms)
{
	return UART_txDMA(&uart2, txData, dataSZ, timeOut_ms);
}

ErrorStatus uart2_rxDMA(uint8_t *rxData, uint16_t dataSZ, uint32_t timeOut_ms)
{
	return UART_rxDMA(&uart2, rxData, dataSZ, timeOut_ms);
}


/*====================================================================================================================*/
/*============================================= Circular Buffer Functions ============================================*/
/*====================================================================================================================*/
ErrorStatus uart2_cbWrite(uint8_t data)
{
	return UARTCB_write(&uart2CB, data);
}

ErrorStatus uart2_cbSendArray(const uint8_t *data, size_t len)
{
	return UARTCB_sendArray(&uart2CB, data, len);
}

ErrorStatus uart2_cbSendString(const char *s)
{
	return UARTCB_sendString(&uart2CB, s);
}

ErrorStatus uart2_cbClearRxBuff(void)
{
	return UARTCB_clearRxBuff(&uart2CB);
}

ErrorStatus uart2_cbPeek(uint8_t *out)
{
	return UARTCB_peek(&uart2CB, out);
}

ErrorStatus uart2_cbRead(uint8_t *out)
{
	return UARTCB_read(&uart2CB, out);
}

ErrorStatus uart2_cbReadArray(uint8_t *out, size_t len)
{
	return UARTCB_readArray(&uart2CB, out, len);
}

uint32_t uart2_cbIsData(void)
{
	return UARTCB_isData(&uart2CB);
}

ErrorStatus uart2_cbIsResponseASCII(const char *str, uint32_t timeout)
{
	return UARTCB_isResponse_ASCII(&uart2CB, str, timeout);
}

ErrorStatus uart2_cbIsResponseBIN(const uint8_t *pattern, size_t pattern_len, uint16_t timeout)
{
	return UARTCB_isResponse_BIN(&uart2CB, pattern, pattern_len, timeout);
}

ErrorStatus uart2_cbGetNextstrg(const char *str, uint8_t num_of_chars, uint8_t *dest_buffer, uint32_t timeout)
{
	return UARTCB_getNextstrg(&uart2CB, str, num_of_chars, dest_buffer, timeout);
}

ErrorStatus uart2_cbCopyUpToString(const char *str, uint8_t *dest, uint32_t timeout)
{
	return UARTCB_copyUpToString(&uart2CB, str, dest, timeout);
}
