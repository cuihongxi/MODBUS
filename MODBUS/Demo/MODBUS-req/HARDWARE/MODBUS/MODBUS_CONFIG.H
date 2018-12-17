/*
*-Header:MODBUS�����ļ�
*-Name	: 
*-Autho	:******
*-Date	:2018��7��13��10:31:21
*		��Ҫ�������ĵ�
*	����������˵����
*- 	�ĵ��Ƽ���������19200��3.5���ֽڼ��Ϊ1458.3us��������115200�����Ϊ243us
*- 	��ʱ��Ӧ����100us�����ж�
*- 	�����жϼ�⵽����ʱ������ʱ������1.5���ֽ�ʱ��ļ�������ж�
*- 	����3.5���ֽڼ������Ϊ��һ֡���ݣ������д���
*/

#ifndef	__MODBUS_CONFIG_H
#define	__MODBUS_CONFIG_H

#include "UHEAD.H"

/*��������ͷ�ļ�*/
#include "rs485.h"
/*DMA���ڽ�������*/

//����ʹ������ģʽ
#define	USE_MODBUS_REQ
//����ʹ�ôӻ�ģʽ
//#define	USE_MODBUS_RSP

//����ʹ��DMAģʽ
#define	MODBUS_UART_DMA			0			//DMAģʽ	:	0 ��ʹ��;	1	ʹ��.

//����OFF����0��������1
#define		MD_TURN_OFF(a,b)		a=((a)&(~(0x01<<(b))))			//д��ȦΪOFF״̬,b��λ
#define		MD_TURN_ON(a,b)			a=((a)|((0x01<<(b))))				//д��ȦΪON״̬,b��λ

//���ù���,����ò����ͱ���,���Խ�ʡ��Դ,RSP�д������ݵĴ洢��,REQ�ж���ʹ���ʹ��
#define		BUFF_OutCoil			rspOutCoilDatBuf		//RSP�д�����ɢ�������
#define		BUFF_InCoil				rspInputCoilDatBuf	//RSP�д�����ɢ��������
#define		BUFF_HoldREG			rspHoldREGDatBuf		//RSP�д�������Ĵ�����
#define		BUFF_InREG				rspInputREGDatBuf		//RSP�д�������Ĵ�����

#ifdef USE_MODBUS_RSP
/*******************************�ӻ�RSP������***************************************************/

//���ô��ڳ�ʼ������
#define		RSP_UART_INIT(a)		RS485_Init(a)

//���ôӻ����ص�ַ
#define		RSP_LOCAL_ADDR			1		//(1~247)Ϊ�Ϸ��ӵ�ַ��0Ϊ�㲥��ַ���������豸��Ҫʶ��


//���ø����������Ĵ�С
#ifdef   BUFF_OutCoil
	#define		L_OC				10												//�����Ȧ�����С
#endif
#ifdef   BUFF_InCoil
	#define		L_IC				5													//������Ȧ�����С
#endif
#ifdef   BUFF_HoldREG
	#define		L_HoldREG		18//512-0xdc									//������ּĴ��������С
#endif	
#ifdef   BUFF_InREG
	#define		L_INREG			18												//����Ĵ��������С
#endif

/*************************���������д******************************/

#define	externfun(type,fun,length)	extern type fun##[##length##]##;

#ifdef BUFF_OutCoil
externfun(u8,BUFF_OutCoil, L_OC)							//�Զ�����
#endif

#ifdef BUFF_InCoil
externfun(u8,BUFF_InCoil, L_IC)								//�Զ�����
#endif

#ifdef BUFF_HoldREG
externfun(u16,BUFF_HoldREG, L_HoldREG)				//�Զ�����
#endif

#ifdef BUFF_InREG
externfun(u16,BUFF_InREG, L_INREG)						//�Զ�����
#endif

//���ӻ�������Ϣ����
void RSPSendMessage(u8* array,u16 num);

#endif
#ifdef USE_MODBUS_REQ
/*********************************����REQ������****************************************************/

//�����������ڳ�ʼ������
#define		REQ_UART_INIT(a)		RS485_Init(a)

//��������ȴ���ʱʱ��
#define		REQWAIT_TIME		0.5		//��λs


/*************************����ѡ����޸�******************************/
float MDint2float(u32* array);
u32 	MDint2int(u32* array);
void 	MDfloat2Array(float num,u32* array);//��foat��ת��һ�´浽MODBUS������
//���ӻ�������Ϣ����
void REQSendMessage(u8* array,u16 num);

#endif

#endif

