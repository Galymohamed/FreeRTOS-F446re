#pragma once


#include "stm32f4xx_hal.h"

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "config.h"
#include "mcuMap.h"
#include "gUtilis.h"




static inline void Error_Handler(void)
{
  __disable_irq();
  while (1) { }
}
