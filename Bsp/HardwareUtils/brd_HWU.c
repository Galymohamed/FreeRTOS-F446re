#include "brd_HWU.h"


/*==================================================================================*/
/*=========================== HardwareUtili Interrupt Parameters ==================*/
static intrptParam_t pshBtnExti = {
    .intptFlg       = 0U,
    .irqn           = { BOARD_PSHBTN_IT_IRQN, INVALID_IRQN },
    .intrptPriority = { BOARD_PSHBTN_IT_PRIO, INVALID_IRQ_PRIO },
    .intrptCount    = 1U
};
/*==================================================================================*/

static const pinCfg_t brdHW_utils[] = {
    {
        .GPIOx           = BOARD_PSHBTN_PORT,
        .pinMsk          = BOARD_PSHBTN_PIN,
        .mode            = GPIO_MODE_IT_FALLING,
        .pullType        = GPIO_NOPULL,
        .speed           = GPIO_SPEED_FREQ_LOW,
        .altFunc         = 0U,
        .b_lockPinConfig = false,
        .extiParam       = &pshBtnExti,
        .description     = "User Button on PC13"
    },
    {
        .GPIOx           = BOARD_LED_PORT,
        .pinMsk          = BOARD_LED_PIN,
        .mode            = GPIO_MODE_OUTPUT_PP,
        .pullType        = GPIO_NOPULL,
        .speed           = GPIO_SPEED_FREQ_LOW,
        .altFunc         = 0U,
        .b_lockPinConfig = false,
        .extiParam       = NULL,
        .description     = "Onboard LED on PA5"
    }
};


ErrorStatus board_hardWareUtils_init(void)
{
    return GPIO_initMulti(brdHW_utils, (uint8_t)ARRAY_SIZE(brdHW_utils));
}
