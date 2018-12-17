#include "MODBUS.H"
#include "MODBUS_RSP.H"
#include "MODBUS_CONFIG.H"

/*******************************************************
*-函数名称	：
*-函数作用	：将大端数组转换成int值
*-参数		：array，数组名，sizeofarray应该使用sizeof获得数组长度
*-返回值	：dat 为转换的INT值
*-备注		：
*******************************************************/

unsigned long int GetBigEndValu(unsigned char* array,unsigned char sizeofarray)
{
	unsigned char i;
	unsigned long int dat = 0;

	for(i = 0;i<sizeofarray;i++)
	{
		dat = dat | ((unsigned long int)array[i]<<(8*(sizeofarray - i-1)));
	}
	return dat;
}


//MODBUS复位参数
void MD_InitArg(MD_datstr* pMD_datstr)
{
	pMD_datstr->flag_receiveOK = MD_NO;
	pMD_datstr->flag_startTimer = MD_NO;
	pMD_datstr->num_byte = 0;
		
}

//在定时器中断中调用,判断3.5字节间隔时间
//period为定时器设定的更新周期,us
MDState MD_TimeJudge(MD_datstr* pMD_datstr,u32 period)
{
	static u32 counter = 0;
	if(pMD_datstr->flag_CanSend == MD_NO)
	{
		counter ++;
		if(counter > 2*(pMD_datstr->timerbase/period))
		{
			counter = 0;
			pMD_datstr->flag_CanSend = MD_OK;
		}
	}
	return pMD_datstr->flag_CanSend;
}

//在定时器中的函数
void MD_Fun_InTime(MD_datstr* pMD_datstr,u32 period)
{
	//3.5T允许发送定时
	MD_TimeJudge(pMD_datstr,period);
#if (MODBUS_UART_DMA == 0)
	//传输完成判断，大于3.5T
	if(pMD_datstr->flag_startTimer == MD_OK)
	{
		pMD_datstr->frame_counter ++;
		if(pMD_datstr->frame_counter > pMD_datstr->timerbase/period)	//大于3.5字符间距，表示传输完成
		{
			pMD_datstr->flag_receiveOK = MD_OK;
			pMD_datstr->flag_startTimer = MD_NO;
		}
	}
	//帧头不是本地地址时，最后个字符要停留3.5T才可以接收下一帧
	if(pMD_datstr->flag_headTimer == MD_OK)
	{
		pMD_datstr->head_counter ++;
		if(pMD_datstr->head_counter > pMD_datstr->timerbase/period)		//大于3.5字符间距，表示两个字符的间距超时
		{
			pMD_datstr->flag_headTimer = MD_NO;							//允许接收
			
		}
	}

#endif
	//REQ中响应超时
#ifdef USE_MODBUS_REQ
	if(pMD_datstr->flag_waitRelpy == MD_OK)
	{
		pMD_datstr->waitRelpy_counter ++;
		if(pMD_datstr->waitRelpy_counter > (u32)(REQWAIT_TIME*(1000000/period)))
		{
			pMD_datstr->waitRelpy_counter = 0;
			pMD_datstr->flag_waitRelpy = MD_NO;	//超时停止计时
		}
	}	
#endif	

}

/************************************************************************************
*-函数名称	：DMA串口接收函数
*-参数			：huart 串口句柄, hdma_usart_rx DMA 串口句柄
*-返回值		：
*-备注			：
*-创建者		：******
*/
#if MODBUS_UART_DMA > 0
void MD_DMAInUART_IDLE(MD_datstr* pMD_str,UART_HandleTypeDef *huart,DMA_HandleTypeDef* hdma_usart_rx)
{
		uint16_t i = 0; 
		static BaseType_t baseVal = 0;	
		static BaseType_t baseVal2 = 0;			
		if(UART_CheckIdleState(huart) == HAL_OK) //IDLE中断
			{ 				

						i = hdma_usart_rx->Instance->CNDTR; 
		
						if(i< MD_BUFNUM)
						{
								HAL_DMA_Abort(hdma_usart_rx);
								hdma_usart_rx->Instance->CNDTR = MD_BUFNUM;//这个寄存器只能在通道不工作(DMA_CCRx的EN=0)时写入  
								pMD_str->num_byte = MD_BUFNUM - i;
				
//										printf("##########\r\npMD_Reqstr->num_byte = %d\r\n",pMD_str->num_byte);
//									  for(i=0;i<pMD_str->num_byte;i++)
//										{    
//											printf("0x%x ",((u8*)pMD_str)[i]);
//										}
//										printf("\r\n");							
									/* 重新接收 */
									pMD_str->flag_receiveOK = MD_OK;									

									vTaskNotifyGiveFromISR(PWM_T1OUT1Handle,&baseVal2);		//发送信号给PWM
									if(huart == &huart1)
									{										
											vTaskNotifyGiveFromISR(modbusReq_Handle,&baseVal);
											portYIELD_FROM_ISR(baseVal);
									}
									if(huart == &huart2)
									{										
											vTaskNotifyGiveFromISR(modbusRsp_Handle,&baseVal);
											portYIELD_FROM_ISR(baseVal);
									}						
						}
		
				}
						__HAL_UART_CLEAR_IDLEFLAG(huart); 
				HAL_UART_Receive_DMA(huart,(u8*)pMD_str,MD_BUFNUM);		
			

				
}
#endif








