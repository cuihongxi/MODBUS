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

 //����һ��10us�Ķ�ʱ������MD��ʱ
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
	delay_init();	    	 //��ʱ������ʼ��	  
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

//��ʱ
 void TIM2_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM2,TIM_FLAG_Update)==SET)  //�ж��ǲ�������ж�
    {
		
			MD_Fun_InTime(&MD_str,100);

		TIM_ClearITPendingBit(TIM2,TIM_FLAG_Update); //�����־λ
	}

		
}

