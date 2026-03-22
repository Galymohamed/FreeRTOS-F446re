#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifndef true
#define true 			1
#define false   		0
#endif



#define ARRAY_SIZE(a)   						  (sizeof(a) / sizeof((a)[0]))

#ifndef BIT
#define BIT(n) 									  (1UL << (n))
#endif

#define BIT_SET(value, mask, state)                do { if (state == true) {       \
														  	 (value) |= (mask);    \
													     } else {                  \
														    (value) &= ~(mask);    \
													     }                         \
												       } while (0)

#ifndef SET_BIT
#define SET_BIT(value, mask) 				  	  ((value) |= (mask))
#define CLEAR_BIT(value, mask) 				  	  ((value) &= ~(mask))
#endif

#define IS_BIT_SET(value, mask)        			  (((value) & (mask)) == (mask))
#define IS_BIT_CLR(value, mask)         		  (((value) & (mask)) == 0U)
#define BIT_ANY_SET(value, mask)   				  (((value) & (mask)) != 0U)

