#ifndef  __MYTOUCH_H_   
#define  __MYTOUCH_H_ 

//#include<P_HEAD.H>
#include "myframe.h"

//����������
typedef	struct 
{
	u16	x;
	u16 y;
}TouchXY;

#define		DONone			0
#define		BIG				1
#define		SMALL			2
#define		SLOW			3
#define		FAST			4
#define		HIGH			5
#define		SHORT			6
#define		LONGPRESSED		7

#define		FENBIAN			2	//����ֱ����




u8  MyLCD_Scan(void);

TouchXY Read_Piont(void);		//����ǰ�����������

//u8 IsPressed(Button button);//�ж���û�а�����������������1��û��������0

#endif