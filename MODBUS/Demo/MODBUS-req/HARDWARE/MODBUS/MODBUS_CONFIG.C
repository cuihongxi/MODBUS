#include "MODBUS_CONFIG.H"
#include "MODBUS.H"

#ifdef	USE_MODBUS_RSP
#ifdef   BUFF_OutCoil 
u8 rspOutCoilDatBuf[L_OC]={0Xff,0X53,0X45,0X55,0X55};		//输出离散量数组
#endif

#ifdef   BUFF_InCoil
u8 rspInputCoilDatBuf[L_IC]={0X12,0X34,0X56,0X55,0X55};		//输入离散量数组
#endif

#ifdef   BUFF_HoldREG
u16 rspHoldREGDatBuf[L_HoldREG]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18};
#endif

#ifdef   BUFF_InREG
u16 rspInputREGDatBuf[L_INREG]={11,12,13,14,15,16,17,18,19,110,111,112,113,114,115,116,117,118};			//输入寄存器数据数组
#endif
/************************************************************************************
*-函数名称	：从机回复请求,发送一条信息
*-参数			：array发送数组,个数是num
*-返回值		：
*-备注			：
*-创建者		：******
*/
void RSPSendMessage(u8* array,u16 num)
{
    //发数据    
		RS485_Send_Bytes(array,num);
}

#endif
#ifdef USE_MODBUS_REQ
/************************************************************************************
*-函数名称	：主机请求,发送一条信息
*-参数			：array发送数组,个数是num
*-返回值		：
*-备注			：
*-创建者		：******
*/
void REQSendMessage(u8* array,u16 num)
{
  //发数据
	RS485_Send_Bytes(array,num);
}

#endif

//在MODBUS数组低位保存的数组里整型转换成float型
float MDint2float(u32* array)
{
	return *(float*)array;
}
//在MODBUS数组保存的数组里整型转换成int型
u32 MDint2int(u32* array)
{
	return *array;
}
//将foat型转换一下存到MODBUS数组中
void MDfloat2Array(float num,u32* array)
{
	*(float*)array = num;
}

