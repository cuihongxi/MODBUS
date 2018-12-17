#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "lcd.h"
#include "usart.h"	 
#include "rs485.h"
#include "modbus_rsp.h"
 #include "it.h"
 #include "modbus_config.h"
 #include "modbus_req.h"
 
MD_datstr MD_str = {0};	

 //产生一个10us的定时器，给MD计时
void Time_Init(void)
{
	TIM_Struct tim_str;
	tim_str.NVIC_IRQChannelPreemptionPriority = 1;
	tim_str.NVIC_IRQChannelSubPriority = 1;
	tim_str.Period = 100;
	tim_str.Prescaler = 71;
	tim_str.TIMx = TIM2;
	TIM_Init(&tim_str);
	
}

 int main(void)
 {	 
	delay_init();	    	 //延时函数初始化	  
	uart_init(115200);	
	printf("----start----\r\n");
	MD_RSP_Init(19200,&MD_str);
	Time_Init();
	while(1)
	{
		if(MD_str.flag_receiveOK == MD_OK)
		{
			//UartSend((u8*)&(MD_str.addr),MD_str.num_byte);
			RSP_CallBack(&MD_str);
					
		}
			
	} 
}

//定时
 void TIM2_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM2,TIM_FLAG_Update)==SET)  //判断是不是这个中断
    {
		
			MD_Fun_InTime(&MD_str,100);

		TIM_ClearITPendingBit(TIM2,TIM_FLAG_Update); //清除标志位
	}

		
}

