#include "IT.H"


/*******************************************************
*-函数名称	：
*-函数作用	：使能相应的定时器时钟
*-参数		：
*-返回值	：
*-备注		：
*******************************************************/

void RCC_Time(TIM_TypeDef* TIMx)
{

    if(TIMx == TIM1)RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    if(TIMx == TIM2)RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    if(TIMx == TIM3)RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    if(TIMx == TIM4)RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    if(TIMx == TIM5)RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
    if(TIMx == TIM6)RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
    if(TIMx == TIM7)RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
    if(TIMx == TIM8)RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);
    if(TIMx == TIM9)RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM9, ENABLE);
    if(TIMx == TIM10)RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM10, ENABLE);
    if(TIMx == TIM11)RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM11, ENABLE);
    if(TIMx == TIM12)RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM12, ENABLE) ;
    if(TIMx == TIM13)RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM13, ENABLE) ;
    if(TIMx == TIM14)RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE) ;
    if(TIMx == TIM15)RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM15, ENABLE) ;
    if(TIMx == TIM16)RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM16, ENABLE) ;
    if(TIMx == TIM17)RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM17, ENABLE) ;   
}

/*******************************************************
*-函数名称	：
*-函数作用	：中断优先级设置
*-参数		：preemption 抢占优先级，sub 从优先级
*-返回值	：
*-备注		：函数中默认2个抢占2个从优先级
*******************************************************/
void NVIC_ITInit(TIM_TypeDef* TIMx,u8 preemption,u8 sub)
{
    NVIC_InitTypeDef NVIC_InitStruct;
    if(TIMx ==TIM2) NVIC_InitStruct.NVIC_IRQChannel = TIM2_IRQn;
    if(TIMx ==TIM3) NVIC_InitStruct.NVIC_IRQChannel = TIM3_IRQn;
    if(TIMx ==TIM4) NVIC_InitStruct.NVIC_IRQChannel = TIM4_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = preemption; 
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = sub;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;                        //使能 
   
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);                     //设置2抢占优先级2从优先级
    NVIC_Init(&NVIC_InitStruct);
}


/*******************************************************
*-函数名称	：
*-函数作用	：通用定时器配置，T2~T7
*-参数		：arr为自动重装值，psc为分频系数，TIMx为定时器编号（TIM2~7）
*-返回值	：
*-备注		：默认使用内部时钟源，本函数使用了内部时钟更加精确
*******************************************************/
void IT_Config(TIM_TypeDef* TIMx,u16 Period,u16 Prescaler) //Period为自动重装值，Prescaler为分频系数，72M/Prescaler为1秒定时器时钟跳动的次数
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	RCC_Time(TIMx);                                                     //使能时钟
    //TIM_DeInit(TIMx);                                                   //将外设 TIMx 寄存器重设为缺省值
   // TIM_InternalClockConfig(TIMx);                                      //使用内部时钟，更加精确
    
    TIM_TimeBaseInitStruct.TIM_Prescaler = Prescaler;                   //设置分频系数
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;      //向上计数
	TIM_TimeBaseInitStruct.TIM_Period = Period;                         //预装载值
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIMx,&TIM_TimeBaseInitStruct);                     //初始化时基    
    TIM_ClearFlag(TIMx,TIM_FLAG_Update);                                 //清除标志位
	TIM_ITConfig(TIMx,TIM_IT_Update,ENABLE);	                        //开启定时器中断
	TIM_Cmd(TIMx,ENABLE);                                                      //使能定时器
    //TIM_PrescalerConfig(TIMx,Prescaler,TIM_PSCReloadMode_Immediate);//立即装载预分频值
}

/*******************************************************
*-函数名称	：
*-函数作用	：初始化定时器
*-参数		：
*-返回值	：
*-备注		：
*******************************************************/
void TIM_Init(TIM_Struct* tim_stru)
{

	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;		
	RCC_Time(tim_stru->TIMx); 
	NVIC_ITInit(tim_stru->TIMx,tim_stru->NVIC_IRQChannelPreemptionPriority,tim_stru->NVIC_IRQChannelSubPriority);
	TIM_DeInit(tim_stru->TIMx);                                        //将外设 TIMx 寄存器重设为缺省值
	TIM_InternalClockConfig(tim_stru->TIMx);                                      //使用内部时钟，更加精确    
    TIM_TimeBaseInitStruct.TIM_Prescaler =tim_stru->Prescaler;                   //设置分频系数
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;      //向上计数
	TIM_TimeBaseInitStruct.TIM_Period = tim_stru->Period;                         //预装载值
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(tim_stru->TIMx,&TIM_TimeBaseInitStruct);                     //初始化时基    
    TIM_ClearFlag(tim_stru->TIMx,TIM_FLAG_Update);                                 //清除标志位
	TIM_ITConfig(tim_stru->TIMx,TIM_IT_Update,ENABLE);	                        //开启定时器中断更新中断
	TIM_Cmd(tim_stru->TIMx,ENABLE);                                                      //使能定时器
    TIM_PrescalerConfig(tim_stru->TIMx,tim_stru->Prescaler,TIM_PSCReloadMode_Immediate);//立即装载预分频值
}
/*******************************************************
*-函数名称	：
*-函数作用	：初始化输出比较参数
*-参数		：Pulse 比较值 Cannel通道号
*-返回值	：
*-备注		：
*******************************************************/

void TIM_OC_Init(TIM_TypeDef* TIMx,u16 Pulse,u8 Cannel)
{
	TIM_OCInitTypeDef TIM_OCInitTypeStruc;
	TIM_OCInitTypeStruc.TIM_OCMode = TIM_OCMode_PWM2 ;  //少于值为低电平
	TIM_OCInitTypeStruc.TIM_Pulse = Pulse;//占空比脉冲
    TIM_OCInitTypeStruc.TIM_OCPolarity = TIM_OCPolarity_Low;   //输出比较极性为低
    TIM_OCInitTypeStruc.TIM_OutputState = TIM_OutputState_Enable;//使能输出
    
	switch (Cannel)
	{
		case 1:			TIM_OC1Init(TIMx, &TIM_OCInitTypeStruc);
								TIM_OC1PreloadConfig(TIMx,TIM_OCPreload_Enable);	//使能预装载
		break;
		case 2:			TIM_OC2Init(TIMx, &TIM_OCInitTypeStruc);
								TIM_OC2PreloadConfig(TIMx,TIM_OCPreload_Enable);	//使能预装载
			break;
		case 3:			TIM_OC3Init(TIMx, &TIM_OCInitTypeStruc);
								TIM_OC3PreloadConfig(TIMx,TIM_OCPreload_Enable);	//使能预装载
			break;
		case 4:			TIM_OC4Init(TIMx, &TIM_OCInitTypeStruc);
								TIM_OC4PreloadConfig(TIMx,TIM_OCPreload_Enable);	//使能预装载
			break;

	};
}

/*******************************************************
*-函数名称	：
*-函数作用	：PWM初始化
*-参数		：
*-返回值	：
*-备注		：设置2抢占优先级2从优先级,先定义结构体再调用
*******************************************************/
	
void MyPWM_Init(PWM_Struct* pwm_stru)
{
	// NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置2抢占优先级2从优先级
	// 初始化GPIO复用推挽模式
	GPIO_AFPP_Init(pwm_stru->GPIOx,pwm_stru->GPIO_Pin);
	//初始化定时器
    IT_Config(pwm_stru->TIMx,pwm_stru->Period,pwm_stru->Prescaler);
	//初始化输出比较参数 
	TIM_OC_Init(pwm_stru->TIMx,pwm_stru->Pulse,pwm_stru->Cannel);
}

/*******************************************************
*-函数名称	：
*-函数作用	：PWM重新设置比较值，占空比
*-参数		：
*-返回值	：
*-备注		：设置2抢占优先级2从优先级,先定义结构体，并初始化后再调用
*******************************************************/
void PWM_TIMSetCompare(PWM_Struct* pwm_stru,u16 Pulse)
{
	switch (pwm_stru->Cannel)
	{
		case 1:			TIM_SetCompare1(pwm_stru->TIMx,Pulse);
		break;
		case 2:			TIM_SetCompare2(pwm_stru->TIMx,Pulse);
			break;
		case 3:			TIM_SetCompare3(pwm_stru->TIMx,Pulse);
			break;
		case 4:			TIM_SetCompare4(pwm_stru->TIMx,Pulse);
			break;
	}
}


/*******************************************************
*-函数名称	：
*-函数作用	：外部中断初始化
*-参数		：EXTI_Line（0~15），EXTITrigger_TypeDef ：	EXTI_Trigger_Rising，(上升沿)
																											EXTI_Trigger_Falling,（下降沿）
																											EXTI_Trigger_Rising_Falling（双边沿）
*-返回值	：
*-备注		：函数中默认2个抢占2个从优先级
*******************************************************/

void My_EXTI_Init(GPIO_TypeDef* GPIOx,uint16_t GPIO_Pin,GPIOMode_TypeDef GPIO_Mode,EXTI_Struct* exti_stru)
{
		//变量定义
		NVIC_InitTypeDef NVIC_InitStruct;
		EXTI_InitTypeDef EXTI_InitStruct;
		u8 NVIC_IRQChannel,n=0;
		uint16_t dat0 = GPIO_Pin;
		u8 GPIO_PortSource;
		uint8_t GPIO_PinSource;
		//使能时钟
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	//使能端口复用时钟、
		My_GPIO_Init(GPIOx,GPIO_Pin,GPIO_Mode);
		
		//根据GPIO_PIN获得中断通道号
		while(dat0)
		{
			dat0 = dat0>>1;
			n++;
		}
		NVIC_IRQChannel = n+5;	
		//端口映射
		GPIO_PinSource = n-1;
		GPIO_PortSource = ((u32)GPIOx-(u32)GPIOA)/0x400;
		GPIO_EXTILineConfig(GPIO_PortSource, GPIO_PinSource);	//Selects the GPIO pin used as EXTI Line.
		
		if(NVIC_IRQChannel<11)
			NVIC_InitStruct.NVIC_IRQChannel = NVIC_IRQChannel;		//中断通道
		if(NVIC_IRQChannel<16&&NVIC_IRQChannel>10)
			NVIC_InitStruct.NVIC_IRQChannel = EXTI9_5_IRQn;		//中断通道
		if(NVIC_IRQChannel<22&&NVIC_IRQChannel>15)
		NVIC_InitStruct.NVIC_IRQChannel = EXTI15_10_IRQn;		//中断通道
		
		NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
		
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置2抢占优先级2从优先级
		NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = exti_stru->NVIC_IRQChannelPreemptionPriority;	//设置抢占优先级
		NVIC_InitStruct.NVIC_IRQChannelSubPriority = exti_stru->NVIC_IRQChannelSubPriority;	//设置从优先级
		NVIC_Init(&NVIC_InitStruct);
		
		
		EXTI_InitStruct.EXTI_Line = GPIO_Pin;	//中断线跟引脚的地址相同
		EXTI_InitStruct.EXTI_LineCmd = ENABLE;	//使能
		EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;	//中断模式
		EXTI_InitStruct.EXTI_Trigger = exti_stru->EXTI_Trigger;		//中断触发类型
		EXTI_Init(&EXTI_InitStruct);
}

/*******************************************************
*-函数名称	：
*-函数作用	：输入捕获初始化
*-参数		：
*-返回值	：
*-备注		：
*******************************************************/
void MY_EXTI_ICInit(EXTI_IC_Struct* exti_ic_stru)
{
	
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_ICInitTypeDef TIM_ICInitStructure;
	
	//初始化IO口
	My_GPIO_Init(exti_ic_stru->GPIOx,exti_ic_stru->GPIO_Pin,exti_ic_stru->GPIO_Mode);
	RCC_Time(exti_ic_stru->TIMx);                                                     //使能时钟
	//初始化定时器参数
	TIM_TimeBaseStructure.TIM_Period = exti_ic_stru->TIM_Period; //周期
	TIM_TimeBaseStructure.TIM_Prescaler =exti_ic_stru->TIM_Prescaler; 	//分频系数
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时间分割
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //向上计数
	TIM_TimeBaseInit(exti_ic_stru->TIMx, &TIM_TimeBaseStructure); //初始化时间基数单位
  
	//初始化输入捕获参数
	TIM_ICInitStructure.TIM_Channel = exti_ic_stru->TIM_Channel; //	选择输入端IC映射到TI上
	TIM_ICInitStructure.TIM_ICPolarity = exti_ic_stru->TIM_ICPolarity;	//?????
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //映射到TI上
	TIM_ICInitStructure.TIM_ICPrescaler = exti_ic_stru->TIM_ICPrescaler;	 //配置输入分频
	TIM_ICInitStructure.TIM_ICFilter = exti_ic_stru->TIM_ICFilter;//配置输入滤波器(0~f)
	TIM_ICInit(exti_ic_stru->TIMx, &TIM_ICInitStructure);
	

	if(exti_ic_stru->TIMx ==TIM2) NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	if(exti_ic_stru->TIMx ==TIM3) NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	if(exti_ic_stru->TIMx ==TIM4) NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
//	if(exti_ic_stru->TIMx ==TIM5) NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
//	if(exti_ic_stru->TIMx ==TIM6) NVIC_InitStructure.NVIC_IRQChannel = TIM6_IRQn;
//	if(exti_ic_stru->TIMx ==TIM7) NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = exti_ic_stru->NVIC_IRQChannelPreemptionPriority;  //抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = exti_ic_stru->NVIC_IRQChannelSubPriority;  //从优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //使能
	NVIC_Init(&NVIC_InitStructure);  
	
	//TIM_ITConfig(exti_ic_stru->TIMx,exti_ic_stru->TIM_IT,ENABLE);//使能比较中断
//	TIM_Cmd(exti_ic_stru->TIMx,DISABLE); 	//不使能
}

//使能外部中断线
void Enable_ExtiLine(uint32_t EXTI_Line)
{
      EXTI->IMR |= EXTI_Line;
}

//失能外部中断线
void Disable_ExtiLine(uint32_t EXTI_Line)
{
      EXTI->IMR &= ~EXTI_Line;
}
