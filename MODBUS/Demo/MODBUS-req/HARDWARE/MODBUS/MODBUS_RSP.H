/*
	2018��7��13��09:26:09
	���ߣ�������˶���ӿƼ����޹�˾
	MODBUS�ظ���Ϣ�ļ�
	ʹ���ڴӻ��У����������CallBack������Ȼ�������־λ MD_InitArg
	*-��ʱ3.5tΪһ֡����
	*-�׵�ַ���Ǳ���ַ�Ĳ�����
	*-ʵ�ֹ��ܣ�
		�ظ��������Ȧ
		�ظ���������Ȧ
		�ظ�������Ĵ���
		�ظ�������Ĵ���
		�ظ�д��Ȧ
		�ظ�д�����Ĵ���
		�ظ�д�����Ȧ
		�ظ�д����Ĵ���
*/
#ifndef	__MODBUS_RSP_H
#define	__MODBUS_RSP_H

#include "UHEAD.H"
#include "MODBUS.H"


void 			MD_RSP_Init(u16 baud,MD_datstr* pMD_Reqstr);			//MODBUS��س�ʼ��
typeErro 	RSP_CallBack(MD_datstr* pMD_Reqstr);					//MODBUS�ظ����ݻص�����
void 			RSP_Fun_InUart(MD_datstr* pMD_Reqstr,u8 receiveByte);	//�ڴ��ڽ����еĺ���

//��������Ҫ���ݾ��������д�Ĵ���������
__weak typeErro RSP_ErroFUN(MD_datstr* pMD_datstr); 
__weak typeErro RSP_ErroADDR(MD_datstr* pMD_datstr); 
__weak typeErro RSP_ErroNUM(MD_datstr* pMD_datstr); 
__weak typeErro RSP_Erro4(MD_datstr* pMD_datstr); 
__weak typeErro RSP_ErroCRC(MD_datstr* pMD_datstr);

#endif