#ifndef  __MYFRAME_H_   
#define  __MYFRAME_H_

#include<P_HEAD.H>


#define		SIZEPOWER	6

#define	Coordinate_YHigh	360				//y��߶�
#define	Coordinate_XLength	512			    //x�᳤��
#define ORIGIN_X			80				//ԭ������
#define ORIGIN_Y			240				//ԭ������
#define INIT_Y              40              //Y���ԭʼ
#define	ROTIO_UNIT_X		20				//ROTIO_UNIT_X�����ص㣬��X�ửһ����
#define	ROTIO_UNIT_Y		10				//ROTIO_UNIT_Y�����ص㣬��Y�ửһ����

#define ONE_INTERVAL_Y      200.0            //1С�����

typedef struct 
{
	u16 x0 ;//�������x
	u16 y0 ;//�������y
	u16 width ;//����
	u16 height ;//�߶�
}Button;

//����ϵ��
typedef struct csStr
{
	u16 origin_x ;										//�������x
	u16 origin_y ;										//�������y
	u16 length_x ;										//X�᳤��
	u16 high_y ;										//Y��߶�
	
	u16 cs_color;										//����ϵ��ɫ
	u16 piont_color;									//�����ɫ
	u16 back_color;										//������ɫ
	
	float h_ratio;										//������
	float v_ratio;										//�߶ȱ���			
	char speed;											//��ʾ�н��ٶ�	
	
	u16 ratio_unit_X;									//ROTIO_UNIT_X�����ص㣬��X�ửһ����
	u16 ratio_unit_Y;									//ROTIO_UNIT_Y�����ص㣬��Y�ửһ����
	u32 j;												//jλ��ǰ���λ���������е�ƫ��
	u16 bitratio;										//��λ
	u16 scale_x;
	u16	scale_y;
	
}CoordinateStr;


extern 	u8	flag_button;

void DrawButton(Button button,u16 color,u8* str);				//��������

void MyFrame_Coordinate(CoordinateStr* cs_str);					//������ϵ
void MyFrame_CoordShow(CoordinateStr* cs_str,double* ArrayBuff,u32 sizebuf);//������ϵ�л���
void MyScan_Date(u8 state,CoordinateStr* cs_str);

void MyShowNum(CoordinateStr* cs_str);//��ʾ��Ӧ������ֵ

u16 ToCoordDat(double dat,CoordinateStr* cs_str);//�������е�ֵת�������������
#define	_MyFrame_CoordShow(cs_str,ArrayBuff)	 MyFrame_CoordShow(cs_str,ArrayBuff,sizeof(ArrayBuff)/(sizeof(double)))
#endif