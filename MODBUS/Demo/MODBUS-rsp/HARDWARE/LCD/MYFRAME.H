#ifndef  __MYFRAME_H_   
#define  __MYFRAME_H_

#include<P_HEAD.H>


#define		SIZEPOWER	6

#define	Coordinate_YHigh	360				//y轴高度
#define	Coordinate_XLength	512			    //x轴长度
#define ORIGIN_X			80				//原点坐标
#define ORIGIN_Y			240				//原点坐标
#define INIT_Y              40              //Y轴的原始
#define	ROTIO_UNIT_X		20				//ROTIO_UNIT_X个像素点，在X轴画一个线
#define	ROTIO_UNIT_Y		10				//ROTIO_UNIT_Y个像素点，在Y轴画一个线

#define ONE_INTERVAL_Y      200.0            //1小格代表

typedef struct 
{
	u16 x0 ;//起点坐标x
	u16 y0 ;//起点坐标y
	u16 width ;//宽度
	u16 height ;//高度
}Button;

//坐标系型
typedef struct csStr
{
	u16 origin_x ;										//起点坐标x
	u16 origin_y ;										//起点坐标y
	u16 length_x ;										//X轴长度
	u16 high_y ;										//Y轴高度
	
	u16 cs_color;										//坐标系颜色
	u16 piont_color;									//描点颜色
	u16 back_color;										//背景颜色
	
	float h_ratio;										//宽比率
	float v_ratio;										//高度比率			
	char speed;											//演示行进速度	
	
	u16 ratio_unit_X;									//ROTIO_UNIT_X个像素点，在X轴画一个线
	u16 ratio_unit_Y;									//ROTIO_UNIT_Y个像素点，在Y轴画一个线
	u32 j;												//j位当前描点位置在数组中的偏移
	u16 bitratio;										//单位
	u16 scale_x;
	u16	scale_y;
	
}CoordinateStr;


extern 	u8	flag_button;

void DrawButton(Button button,u16 color,u8* str);				//画出按键

void MyFrame_Coordinate(CoordinateStr* cs_str);					//画坐标系
void MyFrame_CoordShow(CoordinateStr* cs_str,double* ArrayBuff,u32 sizebuf);//在坐标系中画点
void MyScan_Date(u8 state,CoordinateStr* cs_str);

void MyShowNum(CoordinateStr* cs_str);//显示相应的坐标值

u16 ToCoordDat(double dat,CoordinateStr* cs_str);//将数组中的值转换成物理坐标点
#define	_MyFrame_CoordShow(cs_str,ArrayBuff)	 MyFrame_CoordShow(cs_str,ArrayBuff,sizeof(ArrayBuff)/(sizeof(double)))
#endif
