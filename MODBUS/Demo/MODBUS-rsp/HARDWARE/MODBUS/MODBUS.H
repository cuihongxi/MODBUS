/*
	2018年7月14日11:54:44

*/

/*  说明
---->    MODBUS 协议定义了三种 PDU。它们是：
    ? MODBUS 请求 PDU，mb_req_pdu
    ? MODBUS 响应 PDU，mb_rsp_pdu
    ? MODBUS 异常响应 PDU，mb_excep_rsp_pdu
    定义 mb_req_pdu 为：
    mb_req_pdu = { function_code, request_data}，其中
    function_code - [1 个字节] MODBUS 功能码
    request_data - [n 个字节]，这个域与功能码有关，并且通常包括诸如可变参考、变量、数据偏移
    量、子功能码等信息。
    定义 mb_rsp_pdu 为：
    mb_rsp_pdu = { function_code, response_ data}，其中
    function_code - [1 个字节] MODBUS 功能码
    response_data - [n 个字节]，这个域与功能码有关，并且通常包括诸如可变参考、变量、数据偏
    移量、子功能码等信息。
    定义 mb_excep_rsp_pdu 为：
    mb_excep_rsp_pdu = { function_code, request_data}，其中
    function_code - [1 个字节] MODBUS 功能码 + 0x80
    exception_code - [1 个字节]，在下表中定义了 MODBUS 异常码。
    
---->   有两个用户定义功能码的定义范围，即 十进制 65 至 72 和100 至 110 

---->   MODBUS 使用一个‘big-Endian’ 表示地址和数据项   

        
----> 串口设置:1 起始位,8 数据位， 首先发送最低有效位,1 位作为偶校验,1 停止位  推荐 19200bps 波特率。   
---->	超时：bit之间不能大于750us,帧间的超时时间是1.75ms
*/

#ifndef  __MODBUS_H   
#define  __MODBUS_H 

#include "UHEAD.H"
#include "CRC16.H"
#include "MODBUS_CONFIG.H"

#define     DEBUG_MD_LEVEL   0     //调试开关   

#if (DEBUG_MD_LEVEL > 0)
#include "stdio.h"
#define  MD_Log(...)         printf(__VA_ARGS__);
#else
#define MD_Log(...)   
#endif 

#define     ERROR_LOG_LEVEL   1     //错误显示开关   

#if (ERROR_LOG_LEVEL > 0)
#include "stdio.h"
#define  ERROR_Log(...)         printf(__VA_ARGS__);
#else
#define ERROR_Log(...)   
#endif 


typedef enum{
	MD_OFF = 0,
	MD_ON = 0xff00,
}MD_SWITCH;	


typedef enum {
	MD_NO = 0, 
	MD_OK = !MD_NO
} MDState;//modbus状态


//功能码
#define         MD_READ_COIL       0x01        //读线圈,输出状态 离散量输出
#define         MD_READ_INPUT      0x02        //读离散量输入        
#define         MD_READ_HOLD_REG   0x03        //读输出保持寄存器       
#define         MD_READ_INPUT_REG  0X04        //读输入寄存器        
#define         MD_WRITE_COIL      0X05        //写单个线圈
#define         MD_WRITE_REG       0X06        //写单个寄存器
#define         MD_WRITE_COILS     0x0f        //写多个线圈
#define         MD_WRITE_REGS      0X10        //写多个寄存器
#define         MD_READ_FILE       0X14        //读文件记录
#define         MD_WRITE_FILE      0x15        //写文件记录
#define         MD_CLOSE_WRREG     0x16        //屏蔽写寄存器
#define         MD_WANDR_REGS      0X17        //读写多个寄存器
#define         MD_READ_ID         0x2b        //读设备识别码

//异常码
#define         MD_ERROR_OK         0              //OK
#define         MD_ERROR_FUN        0X01           //非法功能
#define         MD_ERROR_ADDR       0X02           //非法数据地址
#define         MD_ERROR_NUM        0X03           //非法数据值
#define         MD_ERROR_4          0X04           //从站设备故障
#define         MD_ERROR_5          0X05           //确认。与编程命令一起使用
#define         MD_ERROR_6          0X06           //从属设备忙。与编程命令一起使用
#define         MD_ERROR_8          0X08           //奇偶错误
#define         MD_ERROR_A          0X0A           //不可用网关路径
#define         MD_ERROR_B          0X0B           //网关目标设备响应失败
#define         MD_ERROR_NO					0xff						//未知错误

//自定义异常码
#define     MD_ERROR_CRC        		0X0C           //CRC错误,不响应
#define			MD_ERROR_NONFUN					0X0E		   //无此功能

//请求发送错误码
#define			REQ_SEND_ERROR_FUN 		0x30			//发送功能码不正确
#define			REQ_SEND_ERROR_NUM  	0x31			//发送数量不正确
#define			REQ_SEND_ERROR_DEVADDR 	0x32			//子节点地址不正确
#define			REQ_SEND_ERROR_TOOFAST  0x33			//还没等到上次命令的回复
#define			REQ_SEND_ERROR_WAIT3B5T 0x34			//还没到3.5T的间隔时间
//请求接收时错误码
#define			REQ_RECEIVE_ERROR_CRC 		0x40			//CRC错误
#define			REQ_RECEIVE_ERROR_NUM 		0x41			//接收数据数量错误

typedef     unsigned char typeMBfun;            			//定义功能码数据类型
typedef     unsigned char typeErro ;            			//异常码的数据类型
typedef     unsigned char typeFun ;             			//功能码的数据类型
		


#define			MD_BUFNUM		200									//MODBUS数据域数组大小


typedef	union{
	u8* 		pBuf_Coil;						//REQ时线圈数据返回时,保存的地址
	u16*		pBuf_REG;							//REG时寄存器数据返回时,保存的地址
}union_REQpBuf;

typedef struct{
	u8 			cmdBuf[6];						//REQ时保存发送的器件地址，命令，数量。地址等
	union_REQpBuf	pBuf;						//REG时数据返回时,保存的地址	
}struct_REQpBuf;

typedef struct{
	u8*   	pBuf_OutCoil;					//指向保存离散量输出数组
	u8* 		pBuf_InputCoil;				//指向保存离散量输入的数组
	u16* 		pBuf_HoldREG;					//指向保存保持寄存器的数组；
	u16* 		pBuf_INREG;						//指向保存输入寄存器的数组；	
}struct_RSPpBuf;

typedef	union{
	struct_REQpBuf 		req_pBuf;
	struct_RSPpBuf		rsp_pBuf;	
}union_RSPREQ;

//MODBUS 结构
typedef	struct{
	
	u8						addr;									//地址
	typeFun				funNum;								//功能码
	u8 						datBuf[MD_BUFNUM];		//数据域+CRC
	union_RSPREQ 	pBuf;									//指向数据保存区,RSP,REQ模式下功能不同
	u8 						num_byte;     				//接收到的字节总数

	u32 					timerbase;						//timerbase 超时基准，单位us。字节间超时1.5个基准就丢弃接收帧，帧间的间隔至少3.5个基准
	MDState				flag_receiveOK;				//接收完成标志位，当为OK时不再接收数据，处理完数据才复位
	u16 					frame_counter;				//帧间隔计数器
	MDState 			flag_CanSend;					//3.5个间隔到到标志
	MDState 			flag_startTimer;			//启动定时器标志
	u16 					head_counter;					//帧头计数器
	MDState 			flag_headTimer;				//启动帧头定时器标志
	MDState 			flag_waitRelpy;				//等待回复标志
	u32 					waitRelpy_counter;		//等待回复计时器
	u8 						flag_listener;				//为1才监听回复时间，为0 不监听
	
}MD_datstr;


//消息及对应的处理函数
typedef struct {
	typeFun		funNum;							//消息类型
	typeErro 	(*fun)(MD_datstr*);	//消息对应的处理函数
}mesUnit;


#define           MAX_FUNNUM     MD_WRITE_REGS  						//定义最大的功能码 ，目前只支持到写多个寄存器
#define						GETSIZE(x)		(sizeof(x)/sizeof(x[0]))		//获得数组大小

unsigned long int GetBigEndValu(unsigned char* array,unsigned char sizeofarray);//将大端数组转换成int值
void 		MD_Fun_InTime(MD_datstr* pMD_datstr,u32 period);												//在定时器中的函数
void 		MD_InitArg(MD_datstr* pMD_datstr);																			//MODBUS复位参数

//DMA串口空闲中断接收
#if MODBUS_UART_DMA > 0
void 		MD_DMAInUART_IDLE(MD_datstr* pMD_str,UART_HandleTypeDef *huart,DMA_HandleTypeDef* hdma_usart_rx);
#endif

#endif



