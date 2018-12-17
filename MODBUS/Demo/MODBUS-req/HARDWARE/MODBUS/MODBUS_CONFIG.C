#include "MODBUS_CONFIG.H"
#include "MODBUS.H"

#ifdef	USE_MODBUS_RSP
#ifdef   BUFF_OutCoil 
u8 rspOutCoilDatBuf[L_OC]={0Xff,0X53,0X45,0X55,0X55};		//�����ɢ������
#endif

#ifdef   BUFF_InCoil
u8 rspInputCoilDatBuf[L_IC]={0X12,0X34,0X56,0X55,0X55};		//������ɢ������
#endif

#ifdef   BUFF_HoldREG
u16 rspHoldREGDatBuf[L_HoldREG]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18};
#endif

#ifdef   BUFF_InREG
u16 rspInputREGDatBuf[L_INREG]={11,12,13,14,15,16,17,18,19,110,111,112,113,114,115,116,117,118};			//����Ĵ�����������
#endif
/************************************************************************************
*-��������	���ӻ��ظ�����,����һ����Ϣ
*-����			��array��������,������num
*-����ֵ		��
*-��ע			��
*-������		��******
*/
void RSPSendMessage(u8* array,u16 num)
{
    //������    
		RS485_Send_Bytes(array,num);
}

#endif
#ifdef USE_MODBUS_REQ
/************************************************************************************
*-��������	����������,����һ����Ϣ
*-����			��array��������,������num
*-����ֵ		��
*-��ע			��
*-������		��******
*/
void REQSendMessage(u8* array,u16 num)
{
  //������
	RS485_Send_Bytes(array,num);
}

#endif

//��MODBUS�����λ���������������ת����float��
float MDint2float(u32* array)
{
	return *(float*)array;
}
//��MODBUS���鱣�������������ת����int��
u32 MDint2int(u32* array)
{
	return *array;
}
//��foat��ת��һ�´浽MODBUS������
void MDfloat2Array(float num,u32* array)
{
	*(float*)array = num;
}
