#pragma once


#include "f4gCore.h"


/*===============================================================================================================*/
#define RCC_GPIOX_CLK_ENABLE(_ENABLE_MSK)    do { \
                                             __IO uint32_t tmpreg = RCC->AHB1ENR; \
                                             tmpreg |= (_ENABLE_MSK); \
                                             RCC->AHB1ENR = tmpreg; \
                                             tmpreg = READ_BIT(RCC->AHB1ENR, (_ENABLE_MSK)); \
                                             UNUSED(tmpreg); \
                                             } while(0U)

#define RCC_GPIOX_CLK_DISABLE(_ENABLE_MSK)   do { \
                                             __IO uint32_t tmpreg = RCC->AHB1ENR; \
                                             tmpreg &= ~(_ENABLE_MSK); \
                                             RCC->AHB1ENR = tmpreg; \
                                             tmpreg = READ_BIT(RCC->AHB1ENR, (_ENABLE_MSK)); \
                                             UNUSED(tmpreg); \
                                             } while(0U)

#define IS_PIN_ITERRUPT_MODE(MODE)           (((MODE) == GPIO_MODE_IT_RISING)          || \
                                              ((MODE) == GPIO_MODE_IT_FALLING)         || \
                                              ((MODE) == GPIO_MODE_IT_RISING_FALLING)  || \
                                              ((MODE) == GPIO_MODE_EVT_RISING)         || \
                                              ((MODE) == GPIO_MODE_EVT_FALLING)        || \
                                              ((MODE) == GPIO_MODE_EVT_RISING_FALLING))
/*===============================================================================================================*/


/*===============================================================================================================*/
typedef struct {
    GPIO_TypeDef *GPIOx;           
    intrptParam_t *extiParam;       
    const char *description;     
    uint32_t mode;            
    uint16_t pinMsk;             
    uint8_t pullType;        
    uint8_t speed;           
    uint8_t altFunc;         
    bool b_lockPinConfig; 
} pinCfg_t;
/*===============================================================================================================*/

ErrorStatus GPIO_init(const pinCfg_t *IOPin);
ErrorStatus GPIO_initMulti(const pinCfg_t *configs, uint8_t count);
ErrorStatus GPIO_deInit(GPIO_TypeDef *GPIOx, uint32_t pinMsk);
ErrorStatus GPIO_deInitMulti(const pinCfg_t *configs, uint8_t count);


#ifdef DEBUG
/* Debug function not used in production */
const char *port2char(GPIO_TypeDef *port);
#endif
