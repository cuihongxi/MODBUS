#include "myframe.h"

TouchXY press0;	//触摸点
static u32 length = 0;
u8 flag_shownum = 1;

//画X轴刻度
void DrawScale_X(CoordinateStr* cs_str,u8 state)
{
	u16 i = 0;	
	cs_str->scale_x = (u16)(((cs_str->ratio_unit_X)*cs_str->h_ratio));
	if(state) LCD_SetPointColor(cs_str->cs_color);
		else LCD_SetPointColor(cs_str->back_color);//背景色清除
    
	do{
		i++;
		LCD_DrawLine(cs_str->origin_x + i*cs_str->scale_x ,cs_str->origin_y+10,cs_str->origin_x + i*cs_str->scale_x,cs_str->origin_y+1);	//画Y轴
	}while(((i+1)*cs_str->scale_x)<(cs_str->length_x-10));
	
}

//画Y轴刻度
void DrawScale_Y(CoordinateStr* cs_str,u8 state)
{
	u16 i = 0;
	cs_str->scale_y = (u16)((cs_str->v_ratio)*(cs_str->ratio_unit_Y));
	
	if(state) LCD_SetPointColor(cs_str->cs_color);
	else LCD_SetPointColor(cs_str->back_color);//背景色清除
	
	do{
		
		LCD_DrawLine(cs_str->origin_x-1,cs_str->origin_y-i*cs_str->scale_y,cs_str->origin_x-10,cs_str->origin_y-i*cs_str->scale_y);	//画Y轴
        i++;
        }while((i*cs_str->scale_y)<((cs_str->origin_y-INIT_Y)-10));
        i=0;
      	do{
		i++;
		LCD_DrawLine(cs_str->origin_x-1,cs_str->origin_y+i*cs_str->scale_y,cs_str->origin_x-10,cs_str->origin_y+i*cs_str->scale_y);	//画Y轴
	  }while(((i+1)*cs_str->scale_y)<(INIT_Y+cs_str->high_y -cs_str->origin_y-10));
}

//将数组中的值转换成物理坐标点
u16 ToCoordDat(double dat,CoordinateStr* cs_str)
{
    float bitdat = ONE_INTERVAL_Y/ROTIO_UNIT_Y;   //1个像素点代表的值
    u16 lastdat = 0;
    if(dat>=0) 
    {
        lastdat = (cs_str->origin_y) - (u16)(dat/bitdat);
        //printf("lastdat = %d\r\n",lastdat);
    }else
    {
        dat = -dat;
        lastdat = (cs_str->origin_y) + (u16)(dat/bitdat);
        //printf("lastdat = %d\r\n",lastdat);
    }
    return lastdat;
}

/*******************************************************
*-函数名称	  ：
*-函数作用	  ：画坐标系
*-参数		  ：原点坐标：origin_x,origin_y。X轴长度：length_x；Y轴高：high_y；画笔颜色：color
*-备注		  ：画笔大小：size
*******************************************************/
void Myframe_DrawCoordinate(u16 origin_x,u16 origin_y,u16 length_x,u16 high_y,u16 color)
{
	LCD_SetPointColor(color);
    
    LCD_DrawLine(origin_x,INIT_Y,origin_x,INIT_Y+Coordinate_YHigh);	//画Y轴
    LCD_DrawLine(origin_x,origin_y,origin_x+length_x,origin_y);	//画X轴

	
	LCD_DrawLine(origin_x,INIT_Y,origin_x-SIZEPOWER/2,INIT_Y+SIZEPOWER);	//画Y轴箭头
	LCD_DrawLine(origin_x,INIT_Y,origin_x+SIZEPOWER/2,INIT_Y+SIZEPOWER);
	LCD_DrawLine(origin_x-SIZEPOWER/2,INIT_Y+SIZEPOWER,origin_x+SIZEPOWER/2,INIT_Y+SIZEPOWER);
	
	LCD_DrawLine(origin_x+length_x,origin_y,origin_x+length_x-SIZEPOWER,origin_y-SIZEPOWER/2);	//画X轴箭头
	LCD_DrawLine(origin_x+length_x,origin_y,origin_x+length_x-SIZEPOWER,origin_y+SIZEPOWER/2);
	LCD_DrawLine(origin_x+length_x-SIZEPOWER,origin_y-SIZEPOWER/2,origin_x+length_x-SIZEPOWER,origin_y+SIZEPOWER/2);

}

void MyFrame_Coordinate(CoordinateStr* cs_str)
{
	Myframe_DrawCoordinate(cs_str->origin_x,cs_str->origin_y,cs_str->length_x,cs_str->high_y,cs_str->cs_color);
	DrawScale_X(cs_str,1);
	DrawScale_Y(cs_str,1);
	
}


/*******************************************************
*-函数名称	  ：
*-函数作用	  ：画曲线
*-参数		  ：arraySize数组大小，coordinate_Xlength X轴的长度；原点坐标： origin_x,origin_y
*-备注		  ：
*******************************************************/

void MyFrame_CoordShow(CoordinateStr* cs_str,double* ArrayBuff,u32 sizebuf)
{
	u16 i = 0;				
	u32	 buf_size = 0;					//buf_size指在J定位下，屏幕每个点的位置对应数组中
	static	float  h_ratio0 = 1.0;
	static	float  v_ratio0 = 1.0;
	float  h_ratio1;
	float  v_ratio1;
	
	//清点
	for(i=1;i<cs_str->length_x;i++)
	{
		buf_size = (u32)(i/h_ratio0 + cs_str->j);//buf_size = (u32)(h_ratio0*(i+cs_str->j));
		buf_size %= sizebuf;		
		LCD_Fast_DrawPoint(i+cs_str->origin_x,(u16)(((ToCoordDat(v_ratio0*ArrayBuff[buf_size],cs_str)))),cs_str->back_color);	
		
	}
	
	//判断变化：横向
	if(h_ratio0 != cs_str->h_ratio) 
	{
		h_ratio1 = cs_str->h_ratio;
		
		cs_str->h_ratio = h_ratio0;
		
		DrawScale_X(cs_str,0);
		
		cs_str->h_ratio = h_ratio1;
		
		DrawScale_X(cs_str,1);
		h_ratio0 = h_ratio1;
		
	}
	
	//判断变化：纵向
	if(v_ratio0 != cs_str->v_ratio) 
	{
		v_ratio1 = cs_str->v_ratio;
		cs_str->v_ratio = v_ratio0;
		
		DrawScale_Y(cs_str,0);
		cs_str->v_ratio = v_ratio1;
		DrawScale_Y(cs_str,1);
		v_ratio0 = v_ratio1;
	}

	cs_str->j += cs_str->speed;
	cs_str->j %= sizebuf;
	
	//画点
	for(i=1;i<cs_str->length_x;i++)
	{
		buf_size = (u32)(i/h_ratio0 + cs_str->j);
		buf_size %= sizebuf;
		LCD_Fast_DrawPoint(i+cs_str->origin_x,(u16)(ToCoordDat(v_ratio0*ArrayBuff[buf_size],cs_str)),cs_str->piont_color);	

	}	
}

////按下显示刻度
//void LongPressDrawLine(CoordinateStr* cs_str,u16 lcolor)
//{	
//	if(lcolor != cs_str->back_color)
//	{
//			press0 = Read_Piont();	
//	}	
//	LCD_SetPointColor(lcolor);
//	LCD_ShowxNum(100,50,press0.x,6,16,1);
//	LCD_ShowxNum(100,75,press0.y,6,16,1);
//	LCD_DrawLine(cs_str->origin_x+1,press0.y,press0.x,press0.y);
//	LCD_DrawLine(press0.x,press0.y,press0.x,cs_str->origin_y+1);
//}

//画出按键
void DrawButton(Button button,u16 color,u8* str)
{
	LCD_Fill(button.x0,button.y0,button.x0+button.width,button.y0+button.height,color);
	LCD_ShowString(button.x0+10,button.y0+20,304,24,48,str);
}

//根据扫描的值，进行相应动作
void MyScan_Date(u8 state,CoordinateStr* cs_str)
{
	static u8	flag_b = 0;
	switch(state)
	{
		case DONone : flag_b = 0;//LongPressDrawLine(cs_str,cs_str->back_color);
			break;
		case BIG :	//宽
			if(flag_b==0)
			{
				flag_b = 1;
				if(cs_str->h_ratio<10)cs_str->h_ratio += 0.1;
			}					
			break;
		case SMALL: //窄
			if(flag_b==0)
			{
				flag_b = 1;
				if(cs_str->h_ratio>0.2)cs_str->h_ratio -= 0.1;
			}
			
			break;
			case SHORT: //矮
			if(flag_b==0)
			{
				flag_b = 1;
				if(cs_str->v_ratio>0.1)cs_str->v_ratio -= 0.1;
			}
			
			break;
			case HIGH: //高
			if(flag_b==0)
			{
				flag_b = 1;
				if(cs_str->v_ratio<6.0)cs_str->v_ratio += 0.1;
			}
			
			break;
		case SLOW: 
			if(cs_str->speed>0)cs_str->speed --;
			
			break;
		case FAST: 
			if(cs_str->speed<20)cs_str->speed ++;
			break;
		case LONGPRESSED:
		{
			//LongPressDrawLine(cs_str,YELLOW);
		}
			break;
	}
}

//显示相应的坐标值
void MyShowNum(CoordinateStr* cs_str)
{
    static u32 dat0=0;
   
    if((cs_str->j)< dat0 && flag_shownum == 0)
    {
        flag_shownum = 1;
        length = length + 1536;
    }
    if((cs_str->j) >dat0 && flag_shownum == 1)
    {
        flag_shownum = 0;
    }
     dat0 = cs_str->j;
	LCD_SetBACKColor(cs_str->back_color);
	LCD_SetPointColor(cs_str->cs_color);
    
	LCD_ShowxNum(50,420,length+(cs_str->j*cs_str->bitratio),10,16,0);
    
}
