#ifndef __MYGPIO__H__
#define __MYGPIO__H__
#include "sys.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

void GPIO_OutPP_Init(GPIO_TypeDef* GPIOx,uint16_t GPIO_Pin);//GPIO推挽模式
void GPIO_AFPP_Init(GPIO_TypeDef* GPIOx,uint16_t GPIO_Pin);//GPIO复用推挽模式
void My_GPIO_Init(GPIO_TypeDef* GPIOx,uint16_t GPIO_Pin,GPIOMode_TypeDef GPIO_Mode);
#endif
