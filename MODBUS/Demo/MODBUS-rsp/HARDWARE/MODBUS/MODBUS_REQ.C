#include "modbus_req.h"
#include "MODBUS_CONFIG.H"

typeErro REQ_SaveRead(MD_datstr* pMD_Reqstr); 
typeErro REQ_SingleWriteReceive(MD_datstr* pMD_Reqstr); 
typeErro REQ_WriteDatasReceive(MD_datstr* pMD_Reqstr);

//消息对应函数数组
mesUnit switchReqMes[] = {
	
	#ifdef   BUFF_OutCoil 	
		MD_READ_COIL,				REQ_SaveRead,								//读线圈,离散量输出
	  MD_WRITE_COIL,			REQ_SingleWriteReceive,			//写单个线圈
		MD_WRITE_COILS,			REQ_WriteDatasReceive,			//写多个线圈  	
	#endif
	
	#ifdef   BUFF_InCoil
		MD_READ_INPUT,			REQ_SaveRead,								//读离散量输入
	#endif
	
	#ifdef   BUFF_HoldREG
		MD_READ_HOLD_REG, 	REQ_SaveRead,								//读保存寄存器,输出寄存器
		MD_WRITE_REG, 			REQ_SingleWriteReceive, 		//写单个寄存器
    MD_WRITE_REGS,			REQ_WriteDatasReceive,			//写多个寄存器   
	#endif
	
	#ifdef   BUFF_InREG
    MD_READ_INPUT_REG, 	REQ_SaveRead,								//读输入寄存器
	#endif
	
//	MD_READ_FILE,			REQ_NoFun,  		//读文件记录          
//	MD_WRITE_FILE,		REQ_NoFun,			//写文件记录       
//	MD_CLOSE_WRREG,		REQ_NoFun, 			//屏蔽写寄存器          
//	MD_WANDR_REGS,		REQ_NoFun, 			//读写多个寄存器           
//	MD_READ_ID, 			REQ_NoFun,  		//读设备识别号          

};

//错误对应函数数组
mesUnit switchReqErroMes[] = {

	MD_ERROR_FUN,		REQ_ErroFUN,	//非法功能
	MD_ERROR_ADDR,	REQ_ErroADDR,	//非法地址
	MD_ERROR_NUM, 	REQ_ErroNUM,	//非法数据值
  MD_ERROR_4, 		REQ_Erro4,		//从站设备故障
	MD_ERROR_CRC,		REQ_ErroCRC,	//crc错误
};


/************************************************************************************
*-函数名称	：末尾自动添加CRC校验的请求
*-参数		：num 有效字节数，（出掉CRC字节）,num大于等于2，devaddr 器件地址
*-返回值		：
*-函数功能	：
*-创建者		：蓬莱市博硕电子科技
*/

typeErro REQ_SendWithCRC(MD_datstr* pMD_datstr,u8 num,u8 dev_addr)
{
	u16 crcdat = 1000;
#if MODBUS_UART_DMA == 0
	while(pMD_datstr->flag_listener == MD_OK && crcdat) crcdat --;
	if(crcdat == 0)return  REQ_SEND_ERROR_TOOFAST;				//等待上一帧数据完成
	crcdat = 1000;
#endif
	while(pMD_datstr->flag_CanSend == MD_NO && crcdat)crcdat --;
	if(crcdat == 0)return REQ_SEND_ERROR_WAIT3B5T;				//等待3.5个字节帧间隔
	if(num < 2) return REQ_SEND_ERROR_NUM;
	if(dev_addr>247) return  REQ_SEND_ERROR_DEVADDR;			//1~247,0为广播地址
	pMD_datstr->addr = dev_addr;
	crcdat = CRC16 ((u8 *)pMD_datstr,num);
	pMD_datstr->datBuf[num-2] = (u8)crcdat;
	pMD_datstr->datBuf[num-1] = (u8)(crcdat>>8);					//CRC先发送低字节再发送高字节
	REQSendMessage((u8 *)pMD_datstr,num + 2);   					//发送
	pMD_datstr->flag_CanSend = MD_NO;											//计时帧间隔
	pMD_datstr->flag_waitRelpy = MD_OK;										//等待回复计时
	pMD_datstr->waitRelpy_counter = 0;										//计时清零
	pMD_datstr->flag_listener = MD_OK;										//开始监听
	return MD_ERROR_OK;
	
}

/************************************************************************************
*-函数名称	：请求读指令
*-参数		：dev_addr,器件地址，FunNum 功能码，startAddr 起始地址，datnum数量,pBuf 数据返回后保存数据区
*-返回值		：
*-备注		：0x读输出线圈；1x读输入线圈；3x读输入寄存器；4x读保存寄存器
*-创建者		：蓬莱市博硕电子科技
*/

 typeErro REQ_Read(MD_datstr* pMD_Reqstr,u8 FunNum,u8 dev_addr,u16 startAddr,u16 datnum,void* pBuf)
{		
	if(FunNum > MD_READ_INPUT_REG) return REQ_SEND_ERROR_FUN;		//不是读命令
	if(dev_addr>247) return REQ_SEND_ERROR_DEVADDR;
	pMD_Reqstr->addr = dev_addr;
	pMD_Reqstr->funNum = FunNum;	
	pMD_Reqstr->datBuf[0] = (u8)(startAddr>>8);
	pMD_Reqstr->datBuf[1] = (u8)startAddr;
	pMD_Reqstr->datBuf[2] = (u8)(datnum>>8);
	pMD_Reqstr->datBuf[3] = (u8)datnum;
	pMD_Reqstr->pBuf.req_pBuf.cmdBuf[0] = dev_addr;					//保存器件地址用于校验
	pMD_Reqstr->pBuf.req_pBuf.cmdBuf[1] = FunNum;						//保存功能码用于校验
	if(FunNum == MD_READ_COIL || FunNum == MD_READ_INPUT)
		pMD_Reqstr->pBuf.req_pBuf.pBuf.pBuf_Coil = (u8*)pBuf;
	else pMD_Reqstr->pBuf.req_pBuf.pBuf.pBuf_REG = (u16*)pBuf;
	
	return REQ_SendWithCRC(pMD_Reqstr,6,dev_addr);
}

/************************************************************************************
*-函数名称	：写单个寄存器
*-参数			：addr 地址 datval 值
*-返回值		：
*-备注			：保存器件地址，功能码，值，用于下次接收校验;
*-创建者		：蓬莱市博硕电子科技
*/

typeErro REQ_SingleWriteREG(MD_datstr* pMD_Reqstr,u8 dev_addr,u16 addr,u16 datval)
{
	if(dev_addr>247) return REQ_SEND_ERROR_DEVADDR;
	pMD_Reqstr->addr = dev_addr;
	pMD_Reqstr->funNum = MD_WRITE_REG;	
	pMD_Reqstr->datBuf[0] = (u8)(addr>>8);
	pMD_Reqstr->datBuf[1] = (u8)addr;
	pMD_Reqstr->datBuf[2] = (u8)(datval>>8);
	pMD_Reqstr->datBuf[3] = (u8)datval;
	
	//保存参数	
	pMD_Reqstr->pBuf.req_pBuf.cmdBuf[0] = dev_addr;			
	pMD_Reqstr->pBuf.req_pBuf.cmdBuf[1] = MD_WRITE_REG;			
	pMD_Reqstr->pBuf.req_pBuf.cmdBuf[2] = pMD_Reqstr->datBuf[0];
	pMD_Reqstr->pBuf.req_pBuf.cmdBuf[3] = pMD_Reqstr->datBuf[1];
	pMD_Reqstr->pBuf.req_pBuf.cmdBuf[4] = pMD_Reqstr->datBuf[2];
	pMD_Reqstr->pBuf.req_pBuf.cmdBuf[5] = pMD_Reqstr->datBuf[3];
	
	return REQ_SendWithCRC(pMD_Reqstr,6,dev_addr);	

}

/************************************************************************************
*-函数名称	：写单个线圈
*-参数			：addr 地址 datval 值
*-返回值		：
*-备注			：值只能是0或者0xff00
*-创建者		：蓬莱市博硕电子科技
*/

typeErro REQ_SingleWriteCoil(MD_datstr* pMD_Reqstr,u8 dev_addr,u16 addr,MD_SWITCH datval)
{
	if(dev_addr>247) return REQ_SEND_ERROR_DEVADDR;
	pMD_Reqstr->addr = dev_addr;
	pMD_Reqstr->funNum = MD_WRITE_COIL;	
	pMD_Reqstr->datBuf[0] = (u8)(addr>>8);
	pMD_Reqstr->datBuf[1] = (u8)addr;
	pMD_Reqstr->datBuf[2] = (u8)(datval>>8);
	pMD_Reqstr->datBuf[3] = (u8)datval;
	
	//保存参数	
	pMD_Reqstr->pBuf.req_pBuf.cmdBuf[0] = dev_addr;			
	pMD_Reqstr->pBuf.req_pBuf.cmdBuf[1] = MD_WRITE_COIL;			
	pMD_Reqstr->pBuf.req_pBuf.cmdBuf[2] = pMD_Reqstr->datBuf[0];
	pMD_Reqstr->pBuf.req_pBuf.cmdBuf[3] = pMD_Reqstr->datBuf[1];
	pMD_Reqstr->pBuf.req_pBuf.cmdBuf[4] = pMD_Reqstr->datBuf[2];
	pMD_Reqstr->pBuf.req_pBuf.cmdBuf[5] = pMD_Reqstr->datBuf[3];
	
	return REQ_SendWithCRC(pMD_Reqstr,6,dev_addr);	

}

/************************************************************************************
*-函数名称	：写多个线圈
*-参数			：dev_addr,器件地址，startAddr 起始地址，datnum数量 ,pDatBuf 要写入值的数据区
*-返回值		：
*-备注			：
*-创建者		：蓬莱市博硕电子科技
*/

typeErro REQ_WriteCoils(MD_datstr* pMD_Reqstr,u8 dev_addr,u16 startAddr,u16 datnum,u8* pDatBuf)
{
	u8 i = 0;									
	if(dev_addr>247) return REQ_SEND_ERROR_DEVADDR;
	if(datnum>0x07b0 ||datnum == 0) return REQ_SEND_ERROR_NUM;
	pMD_Reqstr->addr = dev_addr;
	pMD_Reqstr->funNum = MD_WRITE_COILS;	
	pMD_Reqstr->datBuf[0] = (u8)(startAddr>>8);
	pMD_Reqstr->datBuf[1] = (u8)startAddr;
	pMD_Reqstr->datBuf[2] = (u8)(datnum>>8);
	pMD_Reqstr->datBuf[3] = (u8)datnum;
	pMD_Reqstr->datBuf[4] = (datnum+7)/8;	//字节数
	datnum &= 7;	//获得余数
	
	for(i=0;i<pMD_Reqstr->datBuf[4]-1;i++)
	{
		pMD_Reqstr->datBuf[5+i] = pDatBuf[i];
	}
	if(datnum>0)
		pMD_Reqstr->datBuf[5+i]  = (~(0xff<<datnum)) & pDatBuf[i];
	else pMD_Reqstr->datBuf[5+i]  = pDatBuf[i];
	//保存参数	
	pMD_Reqstr->pBuf.req_pBuf.cmdBuf[0] = dev_addr;			
	pMD_Reqstr->pBuf.req_pBuf.cmdBuf[1] = MD_WRITE_COILS;			
	pMD_Reqstr->pBuf.req_pBuf.cmdBuf[2] = pMD_Reqstr->datBuf[0];
	pMD_Reqstr->pBuf.req_pBuf.cmdBuf[3] = pMD_Reqstr->datBuf[1];
	pMD_Reqstr->pBuf.req_pBuf.cmdBuf[4] = pMD_Reqstr->datBuf[2];
	pMD_Reqstr->pBuf.req_pBuf.cmdBuf[5] = pMD_Reqstr->datBuf[3];	
	
	return REQ_SendWithCRC(pMD_Reqstr,7+pMD_Reqstr->datBuf[4],dev_addr);	

}


/************************************************************************************
*-函数名称	：写多个寄存器
*-参数		：dev_addr,器件地址，startAddr 起始地址，datnum u16类型的数量 ,pDatBuf 要写入值的数据区
*-返回值		：
*-备注		：
*-创建者		：蓬莱市博硕电子科技
*/

typeErro REQ_WriteREGs(MD_datstr* pMD_Reqstr,u8 dev_addr,u16 startAddr,u16 datnum,u16* pDatBuf)
{
	u8 i = 0;
	if(dev_addr>247) return REQ_SEND_ERROR_DEVADDR;
	if(datnum>0x0078 ||datnum == 0) return REQ_SEND_ERROR_NUM;
	pMD_Reqstr->addr = dev_addr;
	pMD_Reqstr->funNum = MD_WRITE_REGS;	
	pMD_Reqstr->datBuf[0] = (u8)(startAddr>>8);
	pMD_Reqstr->datBuf[1] = (u8)startAddr;
	pMD_Reqstr->datBuf[2] = (u8)(datnum>>8);
	pMD_Reqstr->datBuf[3] = (u8)datnum;
	pMD_Reqstr->datBuf[4] = datnum*2;	//字节数

	for(i=0;i< datnum;i++)
	{
		pMD_Reqstr->datBuf[5+i*2] = (u8)(pDatBuf[i] >> 8);
		pMD_Reqstr->datBuf[5+i*2+1] = (u8)pDatBuf[i];
	}
	//保存参数	
	pMD_Reqstr->pBuf.req_pBuf.cmdBuf[0] = dev_addr;			
	pMD_Reqstr->pBuf.req_pBuf.cmdBuf[1] = MD_WRITE_REGS;			
	pMD_Reqstr->pBuf.req_pBuf.cmdBuf[2] = pMD_Reqstr->datBuf[0];
	pMD_Reqstr->pBuf.req_pBuf.cmdBuf[3] = pMD_Reqstr->datBuf[1];
	pMD_Reqstr->pBuf.req_pBuf.cmdBuf[4] = pMD_Reqstr->datBuf[2];
	pMD_Reqstr->pBuf.req_pBuf.cmdBuf[5] = pMD_Reqstr->datBuf[3];	
	
	return REQ_SendWithCRC(pMD_Reqstr,7+pMD_Reqstr->datBuf[4],dev_addr);	

}
/***********************以下为串口接收处理函数****************************************************/
/************************************************************************************
*-函数名称	：枚举执行功能函数
*-参数		：
*返回值		：
*-函数功能	：
*-创建者	：蓬莱市博硕电子科技
*/
static typeErro REQEnumAddrFun(MD_datstr* pMD_datstr)
{
	u16 i;
	typeErro erro = 0;
	for (i = 0;i < GETSIZE(switchReqMes);i++)
	{
		if(switchReqMes[i].funNum == pMD_datstr->funNum)
		{
			erro = (*switchReqMes[i].fun)(pMD_datstr);
		}
	}
	return erro;
}

/************************************************************************************
*-函数名称	：枚举错误处理函数
*-参数		：
*返回值		：
*-函数功能	：
*-创建者	：蓬莱市博硕电子科技
*/
static typeErro REQEnumErroFun(MD_datstr* pMD_datstr)
{
	u16 i;
	typeErro erro = MD_ERROR_OK;
	for (i = 0;i < GETSIZE(switchReqErroMes);i++)
	{
		if(switchReqErroMes[i].funNum == (pMD_datstr->datBuf[0]))
		{
			erro = (*switchReqErroMes[i].fun)(pMD_datstr);
		}
	}
	return erro;
}

/************************************************************************************
*-函数名称	：收到回复的回调函数
*-参数		：
*-返回值		：异常码,这个已经在程序中处理，但是也抛出
*-函数功能	：
*-创建者	：蓬莱市博硕电子科技
*/

typeErro REQ_CallBack(MD_datstr* pMD_Reqstr)
{
	u16 crcdat;
	typeErro erro = MD_ERROR_OK;

		if(pMD_Reqstr->num_byte < 4)
		{
			MD_InitArg(pMD_Reqstr);									//数据复位
			return REQ_RECEIVE_ERROR_NUM;
		}
		//判断CRC
		crcdat = CRC16 ((u8*)pMD_Reqstr,pMD_Reqstr->num_byte - 2);
		if(pMD_Reqstr->datBuf[pMD_Reqstr->num_byte - 4] != (u8)(crcdat) ||pMD_Reqstr->datBuf[pMD_Reqstr->num_byte-3] != (u8)(crcdat>>8) )
		{

			MD_InitArg(pMD_Reqstr);									//数据复位
			
			return REQ_RECEIVE_ERROR_CRC;
		}  
		//判断功能码
		if(pMD_Reqstr->funNum & 0x80)							//是错误回复
		{
	
			erro = REQEnumErroFun(pMD_Reqstr);
		}else
			erro = REQEnumAddrFun(pMD_Reqstr);				//解析功能码
		MD_InitArg(pMD_Reqstr);										//数据复位	
	return erro;
}

/************************************************************************************
*-函数名称	：初始化MODBUS从机波特率，定时器的超时基准，及对应的处理函数
*-参数		：timerbase 超时基准，单位us。字节间超时1.5个基准就丢弃接收帧，帧间的间隔至少3.5个基准
*-返回值		：
*-函数功能	：
*-创建者		：蓬莱市博硕电子科技
*/

void MD_REQ_Init(u16 baud,MD_datstr* pMD_Reqstr)
{
	pMD_Reqstr->timerbase = (u32)(3.5*(1000000/baud)*8);
	pMD_Reqstr->flag_CanSend = MD_OK;
	REQ_UART_INIT(baud);					//串口初始化

}



/************************************************************************************
*-函数名称	：在串口接收中的函数
*-参数		：receiveByte 串口接收到的数据
*-返回值		：
*-备注		：之前发送的命令，之后收到的回复。注意：没有对功能码是否一致进行判断
*-创建者		：蓬莱市博硕电子科技
*/

void REQ_Fun_InUart(MD_datstr* pMD_Reqstr,u8 receiveByte)
{
	u8* datBuf = (u8*)pMD_Reqstr; 
	if(pMD_Reqstr->flag_headTimer == MD_OK)						//两个错误的字符间距
	{
		pMD_Reqstr->head_counter = 0;
		return ;
	}
	if(pMD_Reqstr->flag_receiveOK == MD_NO)						//处于空闲状态
	{
		//接收到消息
		if(pMD_Reqstr->num_byte == 0) 
		{
			if(receiveByte == pMD_Reqstr->addr)					//如果是之前发送的对象
				pMD_Reqstr->flag_startTimer = MD_OK;			//启动定时器计时
			else
			{
				pMD_Reqstr->flag_headTimer = MD_OK;				//接收到错误的地址，越过这一帧
				pMD_Reqstr->head_counter = 0;							//清空帧头计时
			}
		}
		if(pMD_Reqstr->flag_startTimer == MD_OK)
		{
			pMD_Reqstr->frame_counter = 0;						//清零帧计时器
			datBuf[pMD_Reqstr->num_byte] = receiveByte;			//保存数据
			if(pMD_Reqstr->num_byte < MD_BUFNUM)				//防止数组出界	
				pMD_Reqstr->num_byte ++;						//接收字节数增加		
		}
	}
	
}

/************************************************************************************
*-函数名称	：离散量保存读数据
*-参数			：num 要保存的字节数量,datBuf读出源地址，pBuf保存地址
*-返回值		：
*-备注			：
*-创建者		：蓬莱市博硕电子科技
*/
static u8 MD_SaveRead(u8 num,u8* datBuf,u8* pBuf)
{
	u8 i = 0;
	for(i=0;i<num;i++)
	{
		pBuf[i] = datBuf[i];
	}
	return 0;
}

/************************************************************************************
*-函数名称	：寄存器保存读数据
*-参数			：num 要保存的字节数量,datBuf读出源地址，pBuf保存地址
*-返回值		：
*-备注			：将前后字节颠倒
*-创建者		：蓬莱市博硕电子科技
*/

static u8 MD_SaveREGRead(u8 num,u8* datBuf,u16* pBuf)
{
	u8 i = 0;
	for(i=0;i<num/2;i++)
	{
		pBuf[i] =  (((u16)datBuf[i*2])<<8)|datBuf[i*2+1];
	}
	return 0;
}

/************************************************************************************
*-函数名称	：保存读数据的处理
*-参数			：pBuf 保存数据区
*-返回值		：
*-备注			：
*-创建者		：蓬莱市博硕电子科技
*/
typeErro REQ_SaveRead(MD_datstr* pMD_Reqstr)
{
	//检验是不是上次发的命令数据的返回
	if(pMD_Reqstr->pBuf.req_pBuf.cmdBuf[0] != pMD_Reqstr->addr \
	|| pMD_Reqstr->pBuf.req_pBuf.cmdBuf[1] != pMD_Reqstr->funNum)
	{
		return MD_ERROR_NUM;
	}
	switch(pMD_Reqstr->funNum)
	{
		case MD_READ_COIL: 
			MD_SaveRead(pMD_Reqstr->datBuf[0],&(pMD_Reqstr->datBuf[1]),pMD_Reqstr->pBuf.req_pBuf.pBuf.pBuf_Coil);
			break;
		case MD_READ_INPUT:
			MD_SaveRead(pMD_Reqstr->datBuf[0],&(pMD_Reqstr->datBuf[1]),pMD_Reqstr->pBuf.req_pBuf.pBuf.pBuf_Coil);
			break;
		case MD_READ_HOLD_REG:
			MD_SaveREGRead(pMD_Reqstr->datBuf[0],&(pMD_Reqstr->datBuf[1]),pMD_Reqstr->pBuf.req_pBuf.pBuf.pBuf_REG);
			break;
		case MD_READ_INPUT_REG:
			MD_SaveREGRead(pMD_Reqstr->datBuf[0],&(pMD_Reqstr->datBuf[1]),pMD_Reqstr->pBuf.req_pBuf.pBuf.pBuf_REG);
			break;
	}
	pMD_Reqstr->waitRelpy_counter = 0;					//计时清零
	pMD_Reqstr->flag_listener = MD_NO;					//停止超时监听
	return MD_ERROR_OK;
}

/************************************************************************************
*-函数名称	：收到写单个数据的处理
*-参数			：
*-返回值		：
*-备注			：
*-创建者		：蓬莱市博硕电子科技
*/
typeErro REQ_SingleWriteReceive(MD_datstr* pMD_Reqstr)
{
	//检验是不是上次发的命令数据的返回
	if((pMD_Reqstr->pBuf.req_pBuf.cmdBuf[0] != pMD_Reqstr->addr)||(pMD_Reqstr->pBuf.req_pBuf.cmdBuf[1] != pMD_Reqstr->funNum)\
		||(pMD_Reqstr->pBuf.req_pBuf.cmdBuf[2]!= pMD_Reqstr->datBuf[0])||(pMD_Reqstr->pBuf.req_pBuf.cmdBuf[3]!= pMD_Reqstr->datBuf[1]))	
	{
		return MD_ERROR_NUM;
	}
	pMD_Reqstr->waitRelpy_counter = 0;					//计时清零
	pMD_Reqstr->flag_listener = MD_NO;					//停止超时监听
	return MD_ERROR_OK;
}

/************************************************************************************
*-函数名称	：收到写多个数据的处理
*-参数			：
*-返回值		：
*-备注			：
*-创建者		：蓬莱市博硕电子科技
*/
static u8 REQ_WriteDatasReceive(MD_datstr* pMD_Reqstr)
{
	//检验是不是上次发的命令数据的返回
	if(pMD_Reqstr->pBuf.req_pBuf.cmdBuf[0] != pMD_Reqstr->addr || pMD_Reqstr->pBuf.req_pBuf.cmdBuf[1] != pMD_Reqstr->funNum ||\
		pMD_Reqstr->pBuf.req_pBuf.cmdBuf[2]!= pMD_Reqstr->datBuf[0]|| pMD_Reqstr->pBuf.req_pBuf.cmdBuf[3]!= pMD_Reqstr->datBuf[1]||\
		pMD_Reqstr->pBuf.req_pBuf.cmdBuf[4]!= pMD_Reqstr->datBuf[2]|| pMD_Reqstr->pBuf.req_pBuf.cmdBuf[5]!= pMD_Reqstr->datBuf[3])
	{
		return MD_ERROR_NUM;
	}
	pMD_Reqstr->waitRelpy_counter = 0;				//计时清零
	pMD_Reqstr->flag_listener = MD_NO;					//停止超时监听
	return MD_ERROR_OK;
}


/*错误处理函数*/
__weak typeErro REQ_ErroFUN(MD_datstr* pMD_Reqstr)
{
	ERROR_Log("funNum = 0x%x\r\n",pMD_Reqstr->funNum);
	ERROR_Log("REQ_ErroFUN\r\n");
	return pMD_Reqstr->datBuf[0];
}
__weak typeErro REQ_ErroADDR(MD_datstr* pMD_Reqstr)
{
	ERROR_Log("funNum = 0x%x\r\n",pMD_Reqstr->funNum);
	ERROR_Log("REQ_ErroADDR\r\n");
	return pMD_Reqstr->datBuf[0];
}
__weak typeErro REQ_ErroNUM(MD_datstr* pMD_Reqstr)
{
	ERROR_Log("funNum = 0x%x\r\n",pMD_Reqstr->funNum);
	ERROR_Log("REQ_ErroNUM\r\n");
	return pMD_Reqstr->datBuf[0];
}	
__weak typeErro REQ_Erro4(MD_datstr* pMD_Reqstr)
{
	ERROR_Log("funNum = 0x%x\r\n",pMD_Reqstr->funNum);
	ERROR_Log("REQ_Erro4\r\n");
	return pMD_Reqstr->datBuf[0];
}
__weak typeErro REQ_ErroCRC(MD_datstr* pMD_Reqstr)
{
	ERROR_Log("funNum = 0x%x\r\n",pMD_Reqstr->funNum);
	ERROR_Log("REQ_ErroCRC\r\n");
	return pMD_Reqstr->datBuf[0];
}
//超时处理函数
__weak void ErroOutTime(MD_datstr* pMD_Reqstr)
{

	ERROR_Log("ErroOutTime\r\n");
}

/**************************2018年9月27日07:08:27*****************************************/
/************************************************************************************
*-函数名称	：主函数中DMA处理函数
*-参数			：
*-返回值		：
*-备注			：
*-创建者		：蓬莱市博硕电子科技
*/

typeErro REQ_DMAInMain(MD_datstr* pMD_Reqstr)
{ 
		u8 error = MD_ERROR_NO;
		u16 i;	
		
		if(pMD_Reqstr->flag_listener == MD_OK)				//发送数据后监听
		{
			if(pMD_Reqstr->flag_receiveOK == MD_OK)	//有数据接收到
			{
				error = REQ_CallBack(pMD_Reqstr);
				if(error)
				{
					ERROR_Log("error = 0X%X\r\n",error);
					for(i=0;i<pMD_Reqstr->num_byte;i++)
					{
						ERROR_Log("%x  ",((u8*)pMD_Reqstr)[i]);
					}
				}else
				{					
				}
			}
			if(pMD_Reqstr->flag_waitRelpy == MD_NO)	//等待超时		
			{
				ErroOutTime(pMD_Reqstr);
				pMD_Reqstr->flag_listener = MD_NO;		//停止监听
			}

		}
		return error;
}

