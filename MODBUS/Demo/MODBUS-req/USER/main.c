
#include "delay.h"
#include "sys.h"
#include "usart.h"	 
#include "rs485.h"
#include "it.h"
#include "modbus_config.h"
#include "modbus_req.h"
 
#define SLAVE_ADDR	1
MD_datstr MD_str = {0};	//һ��MODBUS����ʵ��
u8 flag_send = 0;		//����һ����д�����־
u8 saveAddr[5] = {0xaa,0x55,3,4,5};	//�������ݱ���
u16 saveAddrREG[5] = {0x1234,0x5678};


 //����һ��10us�Ķ�ʱ������MD��ʱ
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
		if(MD_str.flag_listener)							//�������ݺ����
		{
			if(MD_str.flag_waitRelpy == MD_NO)	//�ȴ���ʱ		
			{
				ErroOutTime(&MD_str);
				MD_str.flag_listener = 0;					//ֹͣ����
			}
			if(MD_str.flag_receiveOK == MD_OK)	//�����ݽ��յ�
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

			//REQ_SingleWriteREG(&MD_str,SLAVE_ADDR,8,0xaa55);//д�����Ĵ���
			//REQ_SingleWriteCoil(&MD_str,SLAVE_ADDR,7,MD_ON);						//д������Ȧ
			//REQ_WriteCoils(&MD_str,SLAVE_ADDR,0,3,saveAddr);						//д�����Ȧ
			//REQ_WriteREGs(&MD_str,SLAVE_ADDR,0xe4,2,saveAddrREG);				//д����Ĵ���
			REQ_Read(&MD_str,MD_READ_HOLD_REG,SLAVE_ADDR,0,2,saveAddrREG);			//�������Ȧ�����Ĵ���
			flag_send = 0;
		}
			
	} 
}

//��ʱ
 void TIM2_IRQHandler(void)
{
	static u32 counter_t2 = 0;
	if(TIM_GetITStatus(TIM2,TIM_FLAG_Update)==SET)  //�ж��ǲ�������ж�
    {
		
			MD_Fun_InTime(&MD_str,200);	//MODBUS��ʱ���еĺ���
				
			//���1�뷢����������
			if(flag_send == 0)
			{
				counter_t2 ++;
				if(counter_t2 > 5000)	
				{
					counter_t2 = 0;
					flag_send = 1;
				}		
			}	
		TIM_ClearITPendingBit(TIM2,TIM_FLAG_Update); //�����־λ
	}

		
}

