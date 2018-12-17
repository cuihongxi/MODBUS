#ifndef __IT__H__
#define __IT__H__
#include "stm32f10x.h"
#include<stm32f10x_tim.h>
#include<MYGPIO.H>
#include "stm32f10x_exti.h"

//////////定时器中断//////////////////
/*    if(TIM_GetITStatus(TIM2,TIM_FLAG_Update)==SET)  //判断是不是这个中断
			TIM_ClearITPendingBit(TIM2,TIM_FLAG_Update); //清除标志位*/
typedef struct 
	{
		TIM_TypeDef* TIMx;  
		u16 Period;			//周期
		u16 Prescaler;		//分频系数
		uint8_t NVIC_IRQChannelPreemptionPriority; //中断抢占优先级
		uint8_t NVIC_IRQChannelSubPriority; //中断从优先级
		
	}TIM_Struct;
void TIM_Init(TIM_Struct* tim_stru);
	
//void NVIC_ITInit(TIM_TypeDef* TIMx,u8 preemption,u8 sub); //中断优先级设置，默认是2抢占2从优先级
//void IT_Config(TIM_TypeDef* TIMx,u16 Period,u16 Prescaler); //Period为自动重装值，Prescaler为分频系数，72M/Prescaler为1秒定时器时钟跳动的次数


/////////////PWM函数///////////////////////
	/*
	(1)	??????0.5mS,?0.5mS---2.5mS??;周期20MS
	*/
typedef struct 
	{
		TIM_TypeDef* TIMx;  
		u16 Period;			//周期
		u16 Prescaler;	//分频系数
		GPIO_TypeDef* GPIOx;
		uint16_t GPIO_Pin;
		u16 Pulse;	//占空比
		u8 Cannel;	//通道号
	}PWM_Struct;
void MyPWM_Init(PWM_Struct* pwm_stru);
void PWM_TIMSetCompare(PWM_Struct* pwm_stru,u16 Pulse);
	
//////////////外部中断函数设定/////////////////////////////
	/*中断服务函数中要调用void EXTI_ClearITPendingBit(uint32_t EXTI_Line)清除标志位*/
	typedef struct 
	{
		EXTITrigger_TypeDef EXTI_Trigger;	//中断触发类型
		uint8_t NVIC_IRQChannelPreemptionPriority;	//中断抢占优先级
		uint8_t NVIC_IRQChannelSubPriority;					//中断从优先级
	}EXTI_Struct;
void My_EXTI_Init(GPIO_TypeDef* GPIOx,uint16_t GPIO_Pin,GPIOMode_TypeDef GPIO_Mode,EXTI_Struct* exti_stru);

void Enable_ExtiLine(uint32_t EXTI_Line);	      //使能外部中断线

void Disable_ExtiLine(uint32_t EXTI_Line);      //失能外部中断线   
      
///////////////输入捕获中断设定///////////////////////////////////	
	/* TIM_OC2PolarityConfig(TIM_TypeDef* TIMx, uint16_t TIM_OCPolarity);改变极性*/
	typedef struct 
	{
		TIM_TypeDef* TIMx;
		GPIO_TypeDef* GPIOx;
		uint16_t GPIO_Pin;
		GPIOMode_TypeDef GPIO_Mode;
		uint16_t TIM_Period; 							//重装值，周期
		uint16_t TIM_Prescaler; 					//分频系数
		uint16_t TIM_IT;
		uint8_t NVIC_IRQChannelPreemptionPriority;	//中断抢占优先级
		uint8_t NVIC_IRQChannelSubPriority;					//中断从优先级
		uint16_t TIM_Channel;     	//通道号
		uint16_t TIM_ICPolarity;  //捕获极性，上升沿捕获还是下降沿捕获
		uint16_t TIM_ICPrescaler;  //输入捕获多少次，执行一次
		uint16_t TIM_ICFilter;    //滤波，取值在（0~F）
	}EXTI_IC_Struct;	
	
void MY_EXTI_ICInit(EXTI_IC_Struct* exti_ic_stru);	
	
#endif
