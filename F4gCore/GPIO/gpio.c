#include "gpio.h"


static uint32_t GPIO_getEnableMsk(GPIO_TypeDef* GPIOx);




ErrorStatus GPIO_init(const pinCfg_t *pinCfg)
{
    ErrorStatus status = ERROR;

    if (pinCfg != NULL)
    {
        uint32_t enMsk = GPIO_getEnableMsk(pinCfg->GPIOx);

        if (enMsk != 0U)
        {
            GPIO_InitTypeDef GPIO_InitStruct = {0};

            status = SUCCESS;

            /* Enable GPIO clock if not already enabled */
            if ((RCC->AHB1ENR & enMsk) == 0U)
            {
                RCC_GPIOX_CLK_ENABLE(enMsk);
            }

            GPIO_InitStruct.Pin       = (uint32_t)pinCfg->pinMsk;     
            GPIO_InitStruct.Mode      = pinCfg->mode;
            GPIO_InitStruct.Pull      = (uint32_t)pinCfg->pullType; 
            GPIO_InitStruct.Speed     = (uint32_t)pinCfg->speed;    
            GPIO_InitStruct.Alternate = (uint32_t)pinCfg->altFunc;  

            HAL_GPIO_Init(pinCfg->GPIOx, &GPIO_InitStruct);

            if (pinCfg->b_lockPinConfig == true)
            {
                HAL_StatusTypeDef lockResult = HAL_GPIO_LockPin(pinCfg->GPIOx, pinCfg->pinMsk);
                if (lockResult != HAL_OK)
                {
                    status = ERROR;
                }
            }

            /* Configure EXTI only if no prior error & mode is interrupt-enabled */
            if ((status == SUCCESS) && (IS_PIN_ITERRUPT_MODE(pinCfg->mode) != 0U))
            {
                if (pinCfg->extiParam == NULL)
                {
                    status = ERROR;
                }
                else
                {
                    uint8_t intrptCount = pinCfg->extiParam->intrptCount;

                    if (intrptCount != 1U)
                    {
                        status = ERROR;
                    }
                    else
                    {
                        /* Clear interrupt flags*/
                        pinCfg->extiParam->intptFlg = 0U;
                        /* Configure EXTI */
                        IRQn_Type irqn = pinCfg->extiParam->irqn[0];
                        uint8_t intrptPrio = pinCfg->extiParam->intrptPriority[0];
                        HAL_NVIC_SetPriority(irqn, intrptPrio, 0U);
                        HAL_NVIC_EnableIRQ(irqn);
                    }
                }
            }
        }
    }

    return status;
}


ErrorStatus GPIO_initMulti(const pinCfg_t *configs, uint8_t count)
{
    ErrorStatus status = ERROR;

    if ((configs != NULL) && (count != 0U))
    {
        status = SUCCESS;
        for (uint8_t i = 0U; (i < count) && (status == SUCCESS); i++)
        {
            status = GPIO_init(&configs[i]);
        }
    }

    return status;
}


ErrorStatus GPIO_deInit(GPIO_TypeDef* GPIOx, uint32_t pinMsk)
{
    ErrorStatus status = ERROR;

    if ((GPIOx != NULL) && (pinMsk != 0U))
    {
        HAL_GPIO_DeInit(GPIOx, pinMsk);
        status = SUCCESS;
    }
    
    return status;
}


ErrorStatus GPIO_deInitMulti(const pinCfg_t *configs, uint8_t count)
{
    ErrorStatus status = ERROR;

    if ((configs != NULL) && (count != 0U))
    {
        status = SUCCESS;
        for (uint8_t i = 0U; (i < count) && (status == SUCCESS); i++)
        {
            status = GPIO_deInit(configs[i].GPIOx, configs[i].pinMsk);
        }
    }

    return status;
}


static uint32_t GPIO_getEnableMsk(GPIO_TypeDef *GPIOx)
{
    uint32_t mask = 0U;

    if (GPIOx == NULL)       { mask = 0U; }
    else if (GPIOx == GPIOA) { mask = RCC_AHB1ENR_GPIOAEN; }
    else if (GPIOx == GPIOB) { mask = RCC_AHB1ENR_GPIOBEN; }
    else if (GPIOx == GPIOC) { mask = RCC_AHB1ENR_GPIOCEN; }
    else if (GPIOx == GPIOD) { mask = RCC_AHB1ENR_GPIODEN; }
    else if (GPIOx == GPIOE) { mask = RCC_AHB1ENR_GPIOEEN; }
#ifdef GPIOF
    else if (GPIOx == GPIOF) { mask = RCC_AHB1ENR_GPIOFEN; }
#endif
#ifdef GPIOG
    else if (GPIOx == GPIOG) { mask = RCC_AHB1ENR_GPIOGEN; }
#endif
#ifdef GPIOH
    else if (GPIOx == GPIOH) { mask = RCC_AHB1ENR_GPIOHEN; }
#endif
#ifdef GPIOI
    else if (GPIOx == GPIOI) { mask = RCC_AHB1ENR_GPIOIEN; }
#endif
#ifdef GPIOJ
    else if (GPIOx == GPIOJ) { mask = RCC_AHB1ENR_GPIOJEN; }
#endif
#ifdef GPIOK
    else if (GPIOx == GPIOK) { mask = RCC_AHB1ENR_GPIOKEN; }
#endif
    else { mask = 0U; }  /* Invalid GPIOx */

    return mask;
}


#ifdef DEBUG

const char *port2char(GPIO_TypeDef *port)
{
    const char *portName;
    switch ((uintptr_t)port)
    {
        case (uintptr_t)GPIOA: { portName = "Port-A"; } break;
        case (uintptr_t)GPIOB: { portName = "Port-B"; } break;
        case (uintptr_t)GPIOC: { portName = "Port-C"; } break;
        case (uintptr_t)GPIOD: { portName = "Port-D"; } break;
        case (uintptr_t)GPIOE: { portName = "Port-E"; } break;
#ifdef GPIOF
        case (uintptr_t)GPIOF: { portName = "Port-F"; } break;
#endif
#ifdef GPIOG
        case (uintptr_t)GPIOG: { portName = "Port-G"; } break;
#endif
#ifdef GPIOH
        case (uintptr_t)GPIOH: { portName = "Port-H"; } break;
#endif
#ifdef GPIOI
        case (uintptr_t)GPIOI: { portName = "Port-I"; } break;
#endif
#ifdef GPIOJ
        case (uintptr_t)GPIOJ: { portName = "Port-J"; } break;
#endif
#ifdef GPIOK
        case (uintptr_t)GPIOK: { portName = "Port-K"; } break;
#endif
        default: { portName = "Port Not Defined"; } break;
    }

    return portName;
}
#endif
