/*
	2018年7月13日09:26:09
	作者：蓬莱博硕电子科技有限公司
	MODBUS回复消息文件
	使用在从机中，程序里调用CallBack函数，然后清零标志位 MD_InitArg
	*-超时3.5t为一帧数据
	*-首地址不是本地址的不理会
	*-实现功能：
		回复读输出线圈
		回复读输入线圈
		回复读输出寄存器
		回复读输入寄存器
		回复写线圈
		回复写单个寄存器
		回复写多个线圈
		回复写多个寄存器
*/
#ifndef	__MODBUS_RSP_H
#define	__MODBUS_RSP_H

#include "UHEAD.H"
#include "MODBUS.H"


void 			MD_RSP_Init(u16 baud,MD_datstr* pMD_Reqstr);			//MODBUS相关初始化
typeErro 	RSP_CallBack(MD_datstr* pMD_Reqstr);					//MODBUS回复数据回调函数
void 			RSP_Fun_InUart(MD_datstr* pMD_Reqstr,u8 receiveByte);	//在串口接收中的函数

//以下是需要根据具体情况改写的错误处理函数
__weak typeErro RSP_ErroFUN(MD_datstr* pMD_datstr); 
__weak typeErro RSP_ErroADDR(MD_datstr* pMD_datstr); 
__weak typeErro RSP_ErroNUM(MD_datstr* pMD_datstr); 
__weak typeErro RSP_Erro4(MD_datstr* pMD_datstr); 
__weak typeErro RSP_ErroCRC(MD_datstr* pMD_datstr);

#endif
