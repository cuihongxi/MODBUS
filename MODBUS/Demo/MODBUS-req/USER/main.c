
#include "delay.h"
#include "sys.h"
#include "usart.h"	 
#include "rs485.h"
#include "it.h"
#include "modbus_config.h"
#include "modbus_req.h"
 
#define SLAVE_ADDR	1
MD_datstr MD_str = {0};	//一个MODBUS请求实例
u8 flag_send = 0;		//发送一个读写命令标志
u8 saveAddr[5] = {0xaa,0x55,3,4,5};	//接收数据保存
u16 saveAddrREG[5] = {0x1234,0x5678};


 //产生一个10us的定时器，给MD计时
void Time_Init(void)
{
	TIM_Struct tim_str;
	tim_str.NVIC_IRQChannelPreemptionPriority = 3;
	tim_str.NVIC_IRQChannelSubPriority = 3;
	tim_str.Period = 200;
	tim_str.Prescaler = 71;
	tim_str.TIMx = TIM2;
	TIM_Init(&tim_str);
	
}

 int main(void)
 {	 
	u8 erro = 0;
	u8 i = 0;
	delay_init();	    	   
	uart_init(115200); 
	MD_REQ_Init(19200,&MD_str);
	Time_Init();
	while(1)
	{
		if(MD_str.flag_listener)							//发送数据后监听
		{
			if(MD_str.flag_waitRelpy == MD_NO)	//等待超时		
			{
				ErroOutTime(&MD_str);
				MD_str.flag_listener = 0;					//停止监听
			}
			if(MD_str.flag_receiveOK == MD_OK)	//有数据接收到
			{
				erro = REQ_CallBack(&MD_str);
				if(erro)
				{
					printf("erro = 0X%X\r\n",erro);
				}else
				{
					printf("buff[ ] = {");
					for(i=0;i<GETSIZE(saveAddrREG);i++)
					{
						printf(" %#x ",saveAddrREG[i]);
					}
					printf("}\r\n");
					
					printf("OK--------------------\r\n");
				}
					
				
			}
		}
		if(flag_send )
		{

			//REQ_SingleWriteREG(&MD_str,SLAVE_ADDR,8,0xaa55);//写单个寄存器
			//REQ_SingleWriteCoil(&MD_str,SLAVE_ADDR,7,MD_ON);						//写单个线圈
			//REQ_WriteCoils(&MD_str,SLAVE_ADDR,0,3,saveAddr);						//写多个线圈
			//REQ_WriteREGs(&MD_str,SLAVE_ADDR,0xe4,2,saveAddrREG);				//写多个寄存器
			REQ_Read(&MD_str,MD_READ_HOLD_REG,SLAVE_ADDR,0,2,saveAddrREG);			//读多个线圈或多个寄存器
			flag_send = 0;
		}
			
	} 
}

//定时
 void TIM2_IRQHandler(void)
{
	static u32 counter_t2 = 0;
	if(TIM_GetITStatus(TIM2,TIM_FLAG_Update)==SET)  //判断是不是这个中断
    {
		
			MD_Fun_InTime(&MD_str,200);	//MODBUS定时器中的函数
				
			//间隔1秒发送数据请求
			if(flag_send == 0)
			{
				counter_t2 ++;
				if(counter_t2 > 5000)	
				{
					counter_t2 = 0;
					flag_send = 1;
				}		
			}	
		TIM_ClearITPendingBit(TIM2,TIM_FLAG_Update); //清除标志位
	}

		
}

