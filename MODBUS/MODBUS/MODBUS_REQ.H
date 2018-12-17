/*
*-Header:MODBUS 请求数据函数
*-Name	: 
*-Autho	:******
*-Date	:2018年7月16日11:27:09
*/

/*
	发送时，将命令，地址，数量保存到 
*/
#ifndef	__MODBUS_REQ_H
#define	__MODBUS_REQ_H

#include "MODBUS.H"


void 			MD_REQ_Init(u16 baud,MD_datstr* pMD_Reqstr);
typeErro 	REQ_CallBack(MD_datstr* pMD_Reqstr);									//收到回复后的回调函数
void 			REQ_Fun_InUart(MD_datstr* pMD_Reqstr,u8 receiveByte);	//串口中调用函数
//DMA模式的主函数中调用
typeErro REQ_DMAInMain(MD_datstr* pMD_Reqstr);

//主机请求函数
typeErro REQ_Read(MD_datstr* pMD_Reqstr,u8 FunNum,u8 dev_addr,u16 startAddr,u16 datnum,void* pBuf);	//读指令
typeErro REQ_SingleWriteREG(MD_datstr* pMD_Reqstr,u8 dev_addr,u16 addr,u16 datval);				//写单个寄存器
typeErro REQ_SingleWriteCoil(MD_datstr* pMD_Reqstr,u8 dev_addr,u16 addr,MD_SWITCH datval);//写单个线圈

typeErro REQ_WriteCoils(MD_datstr* pMD_Reqstr,u8 dev_addr,u16 startAddr,u16 datnum,u8* pDatBuf);			//写多个线圈
typeErro REQ_WriteREGs(MD_datstr* pMD_Reqstr,u8 dev_addr,u16 startAddr,u16 datnum,u16* pDatBuf);				//写多个寄存器
 
//以下是需要根据具体情况改写的错误处理函数
__weak typeErro REQ_ErroFUN(MD_datstr* pMD_datstr); 
__weak typeErro REQ_ErroADDR(MD_datstr* pMD_datstr); 
__weak typeErro REQ_ErroNUM(MD_datstr* pMD_datstr); 
__weak typeErro REQ_Erro4(MD_datstr* pMD_datstr); 
__weak typeErro REQ_ErroCRC(MD_datstr* pMD_datstr); 
//超时处理函数
__weak void ErroOutTime(MD_datstr* pMD_Reqstr);

#endif

