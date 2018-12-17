#include "MODBUS_RSP.H"
#include "MODBUS.H"
#include "MODBUS_CONFIG.H"

typeErro RSP_ReadCoil(MD_datstr* pMD_datstr);				//回复读输出线圈
typeErro RSP_ReadInputCoil(MD_datstr* pMD_datstr);	//回复读输入线圈
typeErro RSP_ReadHoldREG(MD_datstr* pMD_datstr);		//回复读输出寄存器
typeErro RSP_ReadInputREG(MD_datstr* pMD_datstr);		//回复读输入寄存器
typeErro RSP_WriteCoil(MD_datstr* pMD_datstr);			//回复写线圈
typeErro RSP_WriteREG(MD_datstr* pMD_datstr);				//回复写单个寄存器
typeErro RSP_WriteCoils(MD_datstr* pMD_datstr);			//回复写多个线圈
typeErro RSP_WriteREGs(MD_datstr* pMD_datstr);			//回复写多个寄存器


//消息对应函数数组
mesUnit switchMes[] = {

	#ifdef   BUFF_OutCoil 	
		MD_READ_COIL,				RSP_ReadCoil,				//读线圈,离散量输出
	  MD_WRITE_COIL,			RSP_WriteCoil,			//写单个线圈
		MD_WRITE_COILS,			RSP_WriteCoils,			//写多个线圈  	
	#endif
	
	#ifdef   BUFF_InCoil
		MD_READ_INPUT,			RSP_ReadInputCoil,	//读离散量输入
	#endif
	
	#ifdef   BUFF_HoldREG
		MD_READ_HOLD_REG, 	RSP_ReadHoldREG,		//读保存寄存器,输出寄存器
		MD_WRITE_REG, 			RSP_WriteREG, 			//写单个寄存器
    MD_WRITE_REGS,			RSP_WriteREGs,			//写多个寄存器   
	#endif
	
	#ifdef   BUFF_InREG
    MD_READ_INPUT_REG, 	RSP_ReadInputREG,		//读输入寄存器
	#endif
};

//错误对应函数数组
mesUnit switchErroMes[] = {
	MD_ERROR_FUN,		RSP_ErroFUN,	//非法功能
	MD_ERROR_ADDR,	RSP_ErroADDR,	//非法地址
	MD_ERROR_NUM, 	RSP_ErroNUM,	//非法数据值
  MD_ERROR_4, 		RSP_Erro4,		//从站设备故障
	MD_ERROR_CRC,		RSP_ErroCRC,	//crc错误
};
/************************************************************************************
*-函数名称	：枚举功能函数
*-参数		：
*返回值		：
*-函数功能	：
*-创建者	：******
*/
static typeErro RSPEnumAddrFun(MD_datstr* pMD_datstr)
{
	u16 i;
	typeErro erro = 0;
	for (i = 0;i < GETSIZE(switchMes);i++)
	{
		if(switchMes[i].funNum == pMD_datstr->funNum)
		{
			erro = (*switchMes[i].fun)(pMD_datstr);
		}
	}
	return erro;
}

/************************************************************************************
*-函数名称	：枚举错误处理函数
*-参数		：
*返回值		：
*-函数功能	：
*-创建者	：******
*/
static typeErro RSPEnumErroFun(MD_datstr* pMD_datstr)
{
	u16 i;
	typeErro erro = 0;
	for(i = 0;i < GETSIZE(switchMes);i++)
	{
		if(switchErroMes[i].funNum == (pMD_datstr->datBuf[0]))
		{
			erro = (*switchErroMes[i].fun)(pMD_datstr);
		}
	}
	return erro;
}

/************************************************************************************
*-函数名称	：末尾自动添加CRC校验的回复
*-参数		：num 有效字节数，（出掉CRC字节）,num大于等于2，devaddr 器件地址
*-返回值		：
*-函数功能	：
*-创建者		：******
*/

typeErro RSP_SendWithCRC(MD_datstr* pMD_datstr,u8 num,u8 dev_addr)
{
	u16 crcdat = 0;
	if(num < 2) return MD_ERROR_NUM;
	if(pMD_datstr->addr == 0) return MD_ERROR_OK;		//广播模式
	pMD_datstr->addr = dev_addr;
	crcdat = CRC16 ((u8 *)pMD_datstr,num);
	pMD_datstr->datBuf[num-2] = (u8)crcdat;
	pMD_datstr->datBuf[num-1] = (u8)(crcdat>>8);	//CRC先发送低字节再发送高字节
#if MODBUS_UART_DMA == 0
	while(pMD_datstr->flag_CanSend == MD_NO);		//等待3.5个字节帧间隔
#endif
	RSPSendMessage((u8 *)pMD_datstr,num + 2);   	//发送
	pMD_datstr->flag_CanSend = MD_NO;				//计时帧间隔
	return MD_ERROR_OK;
	
}

/************************************************************************************
*-函数名称	：错误回复
*-参数	：erro 错误码 funNum 功能码 arraySend 最终的发送数组
*返回值	：
*-函数功能	：
*-创建者	：******
*/
static void Modbus_Exception_Rsp(typeErro erro,MD_datstr* pMD_datstr)
{
	pMD_datstr->addr = RSP_LOCAL_ADDR;
	pMD_datstr->funNum |= 0x80;
	pMD_datstr->datBuf[0] = erro;
	RSP_SendWithCRC(pMD_datstr,3,RSP_LOCAL_ADDR);
}

#if (defined BUFF_OutCoil)||(defined BUFF_InCoil)
/************************************************************************************
*-函数名称	：读线圈
*-参数		：startaddr,起始地址，num 要读取的数量(1~2000),datBuf读入缓冲区，pBuf线圈保存区
*-返回值		：返回datBuf占用的数量
*-函数功能	：
*-创建者		：******
*/
static u8 MD_ReadCoil(u16 startaddr,u16 num,u8* datBuf,u8* pBuf)
{
	u16 i = 0;
	u16 bytepos;
	u8  bitstart;
	
  bitstart = startaddr&7;										//bit在字节中开始的位置
	bytepos = startaddr/8;										//开始的字节
	//num大于8
	for(i = 0;num>=8;i++)
	{
		datBuf[i] = (pBuf[bytepos + i] >> bitstart)|(pBuf[bytepos + 1 + i]<<(8 - bitstart));
		num -= 8;
	}
	//num小于8
	if(num != 0)
		datBuf[i] = ((u8)(((pBuf[bytepos + i] >> bitstart)|(pBuf[bytepos + 1 + i]<<(8 - bitstart)))<<(8-num))>>(8-num));
	else i--;
	return i+1;
}
#endif

#ifdef   BUFF_OutCoil
/************************************************************************************
*-函数名称	：回复读线圈
*-参数		：
*-返回值		：
*-函数功能	：读线圈函数并处理
*-创建者		：******
*/
static typeErro RSP_ReadCoil(MD_datstr* pMD_datstr)
{
	typeErro erro = MD_ERROR_OK;
	u16	num = 0;
	u16	addr_start  = 0;     																		//起始地址,0~0xFFFF
	u16	num_bit    = 0 ;      																	//数量,1~0x7d0

	addr_start = GetBigEndValu(pMD_datstr->datBuf,2);  					//获取起始地址          
	num_bit = GetBigEndValu(&(pMD_datstr->datBuf[2]),2);				//获取要读的数量	 
	if(num_bit==0 || num_bit>0x7d0 || num_bit > (GETSIZE(BUFF_OutCoil)*8))    
		return MD_ERROR_NUM;  																		//数量错误 
	if((u32)(addr_start+num_bit)>(GETSIZE(BUFF_OutCoil)*8))
		return MD_ERROR_ADDR;																			//错误地址
	pMD_datstr->datBuf[0] = (num_bit+7)/8;											//字节数
	num = MD_ReadCoil(addr_start,num_bit,&(pMD_datstr->datBuf[1]),pMD_datstr->pBuf.rsp_pBuf.pBuf_OutCoil);//读入线圈数据
	RSP_SendWithCRC(pMD_datstr,num+3,RSP_LOCAL_ADDR);						//自动添加CRC并回复
	return erro;    
}


/************************************************************************************
*-函数名称	：写单个线圈
*-参数			：startaddr,起始地址，off_on值只能有两种，0xff00为开，0为关，
							pBuf线圈保存区
*-返回值		：
*-函数功能	：
*-创建者		：******
*/
static u8 MD_WriteCoil(u16 startaddr,MD_SWITCH off_on,u8* pBuf)
{
	u16 bytepos;
	u8 bitstart;
	
  bitstart = startaddr&7;										//bit在字节中开始的位置
	bytepos = startaddr/8;										//开始的字节
	if(off_on == MD_OFF)
	{
		MD_TURN_OFF(pBuf[bytepos],bitstart);			//关闭这个位
	}else
	{		
		MD_TURN_ON(pBuf[bytepos],bitstart);			//打开这个位
	}

	return 0;
}

/************************************************************************************
*-函数名称	：回复写单个线圈
*-参数			：
*返回值			：
*-函数功能	：
*-创建者		：******
*/
static typeErro RSP_WriteCoil(MD_datstr* pMD_datstr)
{
	typeErro erro = MD_ERROR_OK;
	u16   addr_start  = 0   ;     															//起始地址, 0~0xFFFF
	u16   sw    = 0   ;      																		//开关量,   0xff00为ON,0为OFF
      
	addr_start = GetBigEndValu(pMD_datstr->datBuf,2);  					//获取起始地址          
	sw = GetBigEndValu(&(pMD_datstr->datBuf[2]),2);							//获取输出值 
 	
	//有效性判断
	if(addr_start > GETSIZE(BUFF_OutCoil)*8) return MD_ERROR_ADDR;
	if(sw == 0 || sw == 0xff00)
	{
		MD_WriteCoil(addr_start,(MD_SWITCH)sw,pMD_datstr->pBuf.rsp_pBuf.pBuf_OutCoil);	//写入线圈
	}else  return MD_ERROR_NUM;																			//数量错误 
	RSP_SendWithCRC(pMD_datstr,6,RSP_LOCAL_ADDR);										//自动添加CRC并回复
	return erro;          
}
/************************************************************************************
*-函数名称	：写多个线圈
*-参数			：startaddr,起始地址，num 要写入的数量,datBuf写入缓冲区，pBuf线圈保存区
*-返回值		：
*-函数功能	：
*-创建者		：******
*/
static u8 MD_WriteCoils(u16 startaddr,u16 num,u8* datBuf,u8* pBuf)
{
	u16 i = 0;
	u16 bytepos;
	u8 bitstart;
	u8 dat = 0xff;
  bitstart = startaddr&7;							//bit在字节中开始的位置
	bytepos = startaddr/8;							//开始的字节
	//num大于8
	if(num >= 8)
	{
		for(i = 0;num>=8;i++)
		{		
			pBuf[bytepos + i] = (((u8)(pBuf[bytepos + i]<<(8 - bitstart)))>>(8 - bitstart))| (datBuf[i]<<bitstart);
			pBuf[bytepos + i + 1] = (((u8)(pBuf[bytepos + i + 1] >> bitstart))<<bitstart) |(datBuf[i]>>(8-bitstart));
			num -= 8;
		}		
	}
	//num小于8
	 if(num +bitstart >8)
	{
		pBuf[bytepos + i] = ((pBuf[bytepos + i]<<(8 - bitstart))>>(8 - bitstart))| (datBuf[i]<<bitstart);
		num = num - (8-bitstart);
		pBuf[bytepos + i + 1] =  ((pBuf[bytepos + i + 1] >> num)<<num) |(datBuf[i]>>(8-num));
	}else
	if(num > 0)
	{
		dat = ~((u8)(((dat>>bitstart)<<bitstart)<<(8-bitstart-num))>>(8-bitstart-num));
		pBuf[bytepos + i] =  (pBuf[bytepos + i]&dat)| (datBuf[i]<<bitstart);
	}	
	return 0;
}

/************************************************************************************
*-函数名称	：回复写多个线圈
*-参数		：
*返回值		：
*-函数功能	：
*-创建者		：******
*/
static typeErro RSP_WriteCoils(MD_datstr* pMD_datstr)
{
	typeErro erro = MD_ERROR_OK;
	u16   addr_start  = 0   ;       //起始地址,     0~0xFFFF
	u16   out_num     = 0   ;       //输出数量,     1~0x7b0
	u8    bytenum     = 0   ;       //字节数

	addr_start = GetBigEndValu(pMD_datstr->datBuf,2);  						//获取寄存器的地址         
	out_num = GetBigEndValu(&(pMD_datstr->datBuf[2]),2);					//获取要写的数量
	//字节数           
	bytenum = (out_num+7)/8 ;
	if(bytenum != pMD_datstr->datBuf[4] ||out_num==0||out_num>0X7B ||out_num>(GETSIZE(BUFF_OutCoil)*8))  
		return MD_ERROR_NUM;		//数量错误 
	if((u32)(addr_start + bytenum) > GETSIZE(BUFF_OutCoil)*8) 
		return MD_ERROR_ADDR;		//地址错误 
	if(MD_WriteCoils(addr_start,out_num,&(pMD_datstr->datBuf[5]),pMD_datstr->pBuf.rsp_pBuf.pBuf_OutCoil))  
		return  MD_ERROR_4;   		//从设备异常   

	RSP_SendWithCRC(pMD_datstr,6,RSP_LOCAL_ADDR);											//自动添加CRC并回复
	return erro;          

}
#endif

#ifdef   BUFF_InCoil
/************************************************************************************
*-函数名称	：回复读离散量输入
*-参数		：
*-返回值		：
*-函数功能	：
*-创建者		：******
*/
static typeErro RSP_ReadInputCoil(MD_datstr* pMD_datstr)
{
	typeErro erro = MD_ERROR_OK;
	u16 	num = 0;
	u16   addr_start  = 0   ;     															//起始地址,0~0xFFFF
	u16   num_bit    	= 0   ;      															//数量,1~0x7d0

	addr_start = GetBigEndValu(pMD_datstr->datBuf,2);  					//获取起始地址          
	num_bit = GetBigEndValu(&(pMD_datstr->datBuf[2]),2);				//获取要读的数量
	if(num_bit==0 || num_bit>0x7d0 || num_bit > (GETSIZE(BUFF_InCoil)*8)) 
	  return MD_ERROR_NUM;																			//数量错误  2000
	if((u32)(addr_start + num_bit)>(GETSIZE(BUFF_InCoil)*8))
	{
		return MD_ERROR_ADDR;																			//错误地址
	}											
	pMD_datstr->datBuf[0] = (num_bit+7)/8;											//字节数
	num = MD_ReadCoil(addr_start,num_bit,&(pMD_datstr->datBuf[1]),pMD_datstr->pBuf.rsp_pBuf.pBuf_InputCoil);//读入线圈数据
	RSP_SendWithCRC(pMD_datstr,num+3,RSP_LOCAL_ADDR);										//自动添加CRC并回复
	return erro;            
}

#endif

  
#if (defined BUFF_HoldREG) || (defined BUFF_InREG)
/************************************************************************************
*-函数名称	：读寄存器
*-参数		：startaddr,起始地址，num 要读取的数量,datBuf读入缓冲区，pBuf寄存器保存区
*-返回值		：返回datBuf占用的数量
*-函数功能	：
*-创建者		：******
*/
u8 MD_ReadREG(u16 startaddr,u16 num,u8* datBuf,u16* pBuf)
{
	u16 i = 0;
	for(i = 0;i<num;i++)
	{
		datBuf[i*2] = (u8)(pBuf[startaddr+i]>>8);
		datBuf[i*2+1] = (u8)pBuf[startaddr+i];
	}
	return i*2;
}
#endif 
#ifdef   BUFF_HoldREG 
/************************************************************************************
*-函数名称	：回复读保存寄存器
*-参数		：
*返回值		：
*-函数功能	：
*-创建者	：******
*/
static typeErro RSP_ReadHoldREG(MD_datstr* pMD_datstr)
{
		typeErro erro = MD_ERROR_OK;
		u16   addr_start  = 0   ;     															//起始地址,  0~0xFFFF
		u16   num_reg     = 0   ;      															//数量,     1~0x7d
		addr_start = GetBigEndValu(pMD_datstr->datBuf,2);  					//获取起始地址   
		num_reg = GetBigEndValu(&(pMD_datstr->datBuf[2]),2);				//获取要读的寄存器数量
		if(num_reg==0 || num_reg>0x7d || num_reg > GETSIZE(BUFF_HoldREG))   
			return MD_ERROR_NUM;																			//数量错误 
		if((u32)(addr_start + num_reg) > GETSIZE(BUFF_HoldREG))
			return MD_ERROR_ADDR;																			//错误地址
    pMD_datstr->datBuf[0] = num_reg*2;													//字节数
	  num_reg = MD_ReadREG(addr_start,num_reg,&(pMD_datstr->datBuf[1]),pMD_datstr->pBuf.rsp_pBuf.pBuf_HoldREG);
	  RSP_SendWithCRC(pMD_datstr,num_reg+3,RSP_LOCAL_ADDR);				//自动添加CRC并回复
    return erro;          
      
}


/************************************************************************************
*-函数名称	：写单个寄存器
*-参数			：addr,起始地址，,dat写入的值，pBuf寄存器保存区
*-返回值		：
*-函数功能	：
*-创建者		：******
*/
static u8 MD_WriteREG(u16 addr,u16 dat,u16* pBuf)
{
	pBuf[addr] = dat;

	return 0;
}
/************************************************************************************
*-函数名称	：回复写单个寄存器
*-参数			：
*返回值			：
*-函数功能	：
*-创建者		：******
*/
static typeErro RSP_WriteREG(MD_datstr* pMD_datstr)
{
	typeErro erro = MD_ERROR_OK;
	u16   addr_start  = 0   ;     //起始地址,        0~0xFFFF
	u16   reg_num    = 0   ;      //寄存器的值,      0~0xFFFF

	addr_start = GetBigEndValu(pMD_datstr->datBuf,2);  												//获取寄存器的地址         
	reg_num = GetBigEndValu(&(pMD_datstr->datBuf[2]),2);											//获取要写入寄存器的值       
	if(addr_start> GETSIZE(BUFF_HoldREG))   return  MD_ERROR_ADDR;   					//地址异常 	
	if(MD_WriteREG(addr_start,reg_num,pMD_datstr->pBuf.rsp_pBuf.pBuf_HoldREG))							//写入寄存器
		return MD_ERROR_4;																											//从设备异常
	RSP_SendWithCRC(pMD_datstr,6,RSP_LOCAL_ADDR);															//自动添加CRC并回复
	return erro;                
}

/************************************************************************************
*-函数名称	：写多个寄存器
*-参数		：startaddr 起始地址,num 写入的数量, datBuf写入缓冲区，pBuf寄存器保存区
*返回值		：
*-函数功能	：
*-创建者		：******
*/
static typeErro MD_WriteREGs(u16 startaddr,u16 num,u8* datBuf,u16* pBuf)
{
	u8 i = 0;
	for(i=0;i<num;i++)
	{
		pBuf[startaddr + i] = ((u16)datBuf[i*2]<<8)|datBuf[i*2+1];
		
	}
    return 0;                
}

/************************************************************************************
*-函数名称	：回复写多个寄存器
*-参数		：
*返回值		：
*-函数功能	：
*-创建者		：******
*/
static typeErro RSP_WriteREGs(MD_datstr* pMD_datstr)
{
	typeErro erro = MD_ERROR_OK;
	u16   addr_start  = 0   ;     //起始地址,        0~0xFFFF
	u16   reg_num    = 0    ;     //寄存器的数量,      1~0x7B
	u8    bytenum     = 0   ;     //字节数

	addr_start = GetBigEndValu(pMD_datstr->datBuf,2);  							//获取起始寄存器地址          
	reg_num = GetBigEndValu(&(pMD_datstr->datBuf[2]),2);						//获取要写入寄存器的数量 
	bytenum =  pMD_datstr->datBuf[4];

	if(reg_num==0 || reg_num>0x7B ||(u16)bytenum != 2*reg_num ||reg_num>GETSIZE(BUFF_HoldREG))
		return  MD_ERROR_NUM;																					//数量错误
	if((u32)(addr_start + reg_num) > GETSIZE(BUFF_HoldREG))
		return MD_ERROR_ADDR;																					//地址错误 
	if(MD_WriteREGs(addr_start,reg_num,&(pMD_datstr->datBuf[5]),pMD_datstr->pBuf.rsp_pBuf.pBuf_HoldREG))    
		return MD_ERROR_4;   																					//数据异常 
	RSP_SendWithCRC(pMD_datstr,6,RSP_LOCAL_ADDR);										//自动添加CRC并回复
	return erro;                
}
#endif



#ifdef   BUFF_InREG
/************************************************************************************
*-函数名称	：读输入寄存器
*-参数		：addrStart 起始地址,num_bit 要读的个数 , datbuf 临时数据数组保存读到的数据 
            , num_byte 临时数据,个数应该是 num_bit*2
*返回值		：
*-函数功能	：
*-创建者		：******
*/
static typeErro RSP_ReadInputREG(MD_datstr* pMD_datstr)
{
		typeErro erro = MD_ERROR_OK;
		u16   addr_start  = 0   ;     //起始地址,  0~0xFFFF
		u16   num_reg     = 0   ;      //数量,     1~0x7d

		addr_start = GetBigEndValu(pMD_datstr->datBuf,2);  							//获取起始地址          
		num_reg = GetBigEndValu(&(pMD_datstr->datBuf[2]),2);						//获取要读的寄存器数量
		if(num_reg==0 || num_reg>0x7d|| num_reg > GETSIZE(BUFF_InREG))   
			return MD_ERROR_NUM;																					//数量错误
		if((addr_start + num_reg) > GETSIZE(BUFF_InREG))
			return MD_ERROR_ADDR;																					//错误地址	  
		pMD_datstr->datBuf[0] = num_reg*2;															//字节数
	  num_reg = MD_ReadREG(addr_start,num_reg,&(pMD_datstr->datBuf[1]),pMD_datstr->pBuf.rsp_pBuf.pBuf_INREG);
	  RSP_SendWithCRC(pMD_datstr,num_reg+3,RSP_LOCAL_ADDR);									//自动添加CRC并回复
    return erro;            
      
}
#endif

/************************************************************************************
*-函数名称	：回复命令的回调函数，解析MODEBUS数据
*-参数		：
*返回值		：异常码
*-函数功能	：
*-创建者	：******
*/

typeErro RSP_CallBack(MD_datstr* pMD_Reqstr)
{
	u16 crcdat;
	typeErro erro = MD_ERROR_OK;

	//先判断CRC
	if(pMD_Reqstr->num_byte < 4)
		{
			MD_InitArg(pMD_Reqstr);						//数据复位
			return REQ_RECEIVE_ERROR_NUM;
		}
	crcdat = CRC16 ((u8*)pMD_Reqstr,pMD_Reqstr->num_byte - 2);
	if(pMD_Reqstr->datBuf[pMD_Reqstr->num_byte - 4] != (u8)(crcdat) ||pMD_Reqstr->datBuf[pMD_Reqstr->num_byte-3] != (u8)(crcdat>>8) )										//不响应CRC错误
	{
		pMD_Reqstr->datBuf[0] = MD_ERROR_CRC;
		RSPEnumErroFun(pMD_Reqstr);					//CRC错误
		MD_InitArg(pMD_Reqstr);						//数据复位
		return MD_ERROR_CRC;
	}
		
	//判断功能码范围
	if(pMD_Reqstr->funNum > MAX_FUNNUM || pMD_Reqstr->funNum == 0) 
	{
		Modbus_Exception_Rsp(MD_ERROR_FUN ,pMD_Reqstr);  			//回复错误码
		RSPEnumErroFun(pMD_Reqstr);									//执行错误处理函数
		MD_InitArg(pMD_Reqstr);										//数据复位
		return MD_ERROR_FUN;
	}
	
	//解析功能码
	erro = RSPEnumAddrFun(pMD_Reqstr);
	if(erro != MD_ERROR_OK) 
	{
		Modbus_Exception_Rsp(erro,pMD_Reqstr);  					//回复错误码  
		RSPEnumErroFun(pMD_Reqstr);									//执行错误处理函数
	}
	MD_InitArg(pMD_Reqstr);
	return erro;
}

/************************************************************************************
*-函数名称	：初始化MODBUS从机波特率，定时器的超时基准，及对应的处理函数
*-参数		：timerbase 超时基准，单位us。字节间超时1.5个基准就丢弃接收帧，帧间的间隔至少3.5个基准
*-返回值		：
*-函数功能	：
*-创建者		：******
*/

void MD_RSP_Init(u16 baud,MD_datstr* pMD_Reqstr)
{
	pMD_Reqstr->timerbase = (u32)(3.5*(1000000/baud)*8);
	
#ifdef   BUFF_OutCoil 
	pMD_Reqstr->pBuf.rsp_pBuf.pBuf_OutCoil = (u8*)&BUFF_OutCoil;
#endif
#ifdef   BUFF_InCoil
	pMD_Reqstr->pBuf.rsp_pBuf.pBuf_InputCoil = (u8*)&BUFF_InCoil;
#endif

#ifdef   BUFF_HoldREG
	pMD_Reqstr->pBuf.rsp_pBuf.pBuf_HoldREG = (u16*)&BUFF_HoldREG;
#endif

#ifdef   BUFF_InREG
	pMD_Reqstr->pBuf.rsp_pBuf.pBuf_INREG = (u16*)&BUFF_InREG;
#endif
	
	RSP_UART_INIT(baud);					//串口初始化
}


/************************************************************************************
*-函数名称	：在串口接收中的函数
*-参数		：receiveByte 串口接收到的数据
*-返回值		：
*-函数功能	：之前发送的命令，之后收到的回复
*-创建者		：******
*/

void RSP_Fun_InUart(MD_datstr* pMD_Reqstr,u8 receiveByte)
{
	u8* datBuf = (u8*)pMD_Reqstr;
	if(pMD_Reqstr->flag_headTimer == MD_OK)			//两个错误的字符间距
	{
		pMD_Reqstr->head_counter = 0;
		return ;
	}
	if(pMD_Reqstr->flag_receiveOK == MD_NO)			//处于空闲状态
	{
		//接收到消息
		if(pMD_Reqstr->num_byte == 0) 
		{
			if(receiveByte == RSP_LOCAL_ADDR || receiveByte==0)	//如果是本地的信息，或者广播信息
				pMD_Reqstr->flag_startTimer = MD_OK;			//启动定时器计时
			else
			{
				pMD_Reqstr->flag_headTimer = MD_OK;
				pMD_Reqstr->head_counter = 0;					//清空帧头计时
			}
				
			
		}
		if(pMD_Reqstr->flag_startTimer == MD_OK)
		{
			pMD_Reqstr->frame_counter = 0;				//清零帧计时器
			datBuf[pMD_Reqstr->num_byte] = receiveByte;	//保存数据
			if(pMD_Reqstr->num_byte < MD_BUFNUM)		//防止数组出界	
				pMD_Reqstr->num_byte ++;				//接收字节数增加		
		}

		
	}
	
}



//错误处理函数
__weak typeErro RSP_ErroFUN(MD_datstr* pMD_datstr)
{
	ERROR_Log("funNum = 0x%x\r\n",pMD_datstr->funNum);
	ERROR_Log("RSP_ErroFUN\r\n");
	return pMD_datstr->datBuf[0];
}

__weak typeErro RSP_ErroADDR(MD_datstr* pMD_datstr)
{
	ERROR_Log("funNum = 0x%x\r\n",pMD_datstr->funNum);
	ERROR_Log("RSP_ErroADDR\r\n");
	return pMD_datstr->datBuf[0];
}

__weak typeErro RSP_ErroNUM(MD_datstr* pMD_datstr)
{
	ERROR_Log("funNum = 0x%x\r\n",pMD_datstr->funNum);
	ERROR_Log("RSP_ErroNUM\r\n");
	return pMD_datstr->datBuf[0];
}	

__weak typeErro RSP_Erro4(MD_datstr* pMD_datstr)
{
	ERROR_Log("funNum = 0x%x\r\n",pMD_datstr->funNum);
	ERROR_Log("RSP_Erro4\r\n");
	return pMD_datstr->datBuf[0];
}
__weak typeErro RSP_ErroCRC(MD_datstr* pMD_datstr)
{
	ERROR_Log("funNum = 0x%x\r\n",pMD_datstr->funNum);
	ERROR_Log("RSP_ErroCRC\r\n");
	return pMD_datstr->datBuf[0];
}
