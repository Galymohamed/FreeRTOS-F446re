#include "sysGlob.h"
#include "gpio.h"


/* Named masks for EXTI group handlers — avoid magic numbers (MISRA Rule 2.4) */
#define EXTI15_10_PIN_MASK  (GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | \
                             GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15)

#define EXTI9_5_PIN_MASK    (GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | \
                             GPIO_PIN_8 | GPIO_PIN_9)


void EXTI15_10_IRQHandler(void)
{
    uint32_t ifg = __HAL_GPIO_EXTI_GET_IT(EXTI15_10_PIN_MASK);

    if (ifg != 0U)
    {
        __HAL_GPIO_EXTI_CLEAR_IT(ifg);

        if ((ifg & BOARD_PSHBTN_PIN) != 0U)
        {
            HAL_GPIO_TogglePin(BOARD_LED_PORT, BOARD_LED_PIN);
        }
    }
}


void EXTI9_5_IRQHandler(void)
{
    uint32_t ifg = __HAL_GPIO_EXTI_GET_IT(EXTI9_5_PIN_MASK);

    if (ifg != 0U)
    {
        __HAL_GPIO_EXTI_CLEAR_IT(ifg);

        if ((ifg & GPIO_PIN_5) != 0U)
        {
            HAL_GPIO_TogglePin(BOARD_LED_PORT, BOARD_LED_PIN);
        }
    }
}
