/*
*-Header:MODBUS配置文件
*-Name	: 
*-Autho	:******
*-Date	:2018年7月13日10:31:21
*		重要的配置文档
*	波特率配置说明：
*- 	文档推荐波特率是19200，3.5个字节间隔为1458.3us，波特率115200，间隔为243us
*- 	定时器应配置100us以上中断
*- 	接收中断检测到数据时启动定时器，对1.5个字节时间的间隔不做判断
*- 	大于3.5个字节间隔就认为是一帧数据，并进行处理
*/

#ifndef	__MODBUS_CONFIG_H
#define	__MODBUS_CONFIG_H

#include "UHEAD.H"

/*包含串口头文件*/
#include "rs485.h"
/*DMA串口接收配置*/

//配置使用主机模式
#define	USE_MODBUS_REQ
//配置使用从机模式
//#define	USE_MODBUS_RSP

//配置使用DMA模式
#define	MODBUS_UART_DMA			0			//DMA模式	:	0 不使用;	1	使用.

//配置OFF是置0，还是置1
#define		MD_TURN_OFF(a,b)		a=((a)&(~(0x01<<(b))))			//写线圈为OFF状态,b是位
#define		MD_TURN_ON(a,b)			a=((a)|((0x01<<(b))))				//写线圈为ON状态,b是位

//配置功能,如果用不到就别定义,可以节省资源,RSP中代表数据的存储区,REQ中定义就代表使能
#define		BUFF_OutCoil			rspOutCoilDatBuf		//RSP中代表离散量输出区
#define		BUFF_InCoil				rspInputCoilDatBuf	//RSP中代表离散量输入区
#define		BUFF_HoldREG			rspHoldREGDatBuf		//RSP中代表保存寄存器区
#define		BUFF_InREG				rspInputREGDatBuf		//RSP中代表输入寄存器区

#ifdef USE_MODBUS_RSP
/*******************************从机RSP的配置***************************************************/

//设置串口初始化函数
#define		RSP_UART_INIT(a)		RS485_Init(a)

//设置从机本地地址
#define		RSP_LOCAL_ADDR			1		//(1~247)为合法子地址，0为广播地址，所有子设备都要识别


//配置各个保存区的大小
#ifdef   BUFF_OutCoil
	#define		L_OC				10												//输出线圈数组大小
#endif
#ifdef   BUFF_InCoil
	#define		L_IC				5													//输入线圈数组大小
#endif
#ifdef   BUFF_HoldREG
	#define		L_HoldREG		18//512-0xdc									//输出保持寄存器数组大小
#endif	
#ifdef   BUFF_InREG
	#define		L_INREG			18												//输入寄存器数组大小
#endif

/*************************以下无需改写******************************/

#define	externfun(type,fun,length)	extern type fun##[##length##]##;

#ifdef BUFF_OutCoil
externfun(u8,BUFF_OutCoil, L_OC)							//自动调用
#endif

#ifdef BUFF_InCoil
externfun(u8,BUFF_InCoil, L_IC)								//自动调用
#endif

#ifdef BUFF_HoldREG
externfun(u16,BUFF_HoldREG, L_HoldREG)				//自动调用
#endif

#ifdef BUFF_InREG
externfun(u16,BUFF_InREG, L_INREG)						//自动调用
#endif

//主从机发送消息函数
void RSPSendMessage(u8* array,u16 num);

#endif
#ifdef USE_MODBUS_REQ
/*********************************主机REQ的配置****************************************************/

//设置主机串口初始化函数
#define		REQ_UART_INIT(a)		RS485_Init(a)

//设置请求等待超时时间
#define		REQWAIT_TIME		0.5		//单位s


/*************************可以选择的修改******************************/
float MDint2float(u32* array);
u32 	MDint2int(u32* array);
void 	MDfloat2Array(float num,u32* array);//将foat型转换一下存到MODBUS数组中
//主从机发送消息函数
void REQSendMessage(u8* array,u16 num);

#endif

#endif


