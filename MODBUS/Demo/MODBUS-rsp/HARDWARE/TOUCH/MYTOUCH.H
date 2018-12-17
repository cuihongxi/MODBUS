#ifndef  __MYTOUCH_H_   
#define  __MYTOUCH_H_ 

//#include<P_HEAD.H>
#include "myframe.h"

//触摸点坐标
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

#define		FENBIAN			2	//定义分辨距离




u8  MyLCD_Scan(void);

TouchXY Read_Piont(void);		//读当前触摸点的坐标

//u8 IsPressed(Button button);//判断有没有按到按键，按到返回1，没按到返回0

#endif
