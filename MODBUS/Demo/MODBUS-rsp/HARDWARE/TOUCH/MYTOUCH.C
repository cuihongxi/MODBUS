#include "MYTOUCH.H"

//读当前触摸点的坐标
TouchXY Read_Piont(void)
{
	u8	buf[4];
	TouchXY press0;
	//读第一个点坐标
	GT9147_RD_Reg(GT_TP1_REG,buf,4);
	press0.y = ((u16)buf[1]<<8)|buf[0];
	press0.x = 800-(((u16)buf[3]<<8)|buf[2]);
	
	return press0;
}


/*******************************************************
*-函数名称	  ：触摸扫描
*-函数作用	  ：
*-参数		  ：返回：单指：左右上下，双指：扩大，缩小
*-备注		  ：
*******************************************************/
u8  MyLCD_Scan(void)
{
	u8	mode = 0;
	u8	buf[4];
	static TouchXY press0 ={0,0},press1 = {0,0};
	u16	dis0 = 0;
	static 	u8 flag_pressed = 0;
	static 	u8 flag_pressed_1 = 0;
	static	u16 dis = 0;
	
	GT9147_WR_Reg(GT_GSTID_REG,&mode,1);	//清标志
	delay_ms(10);
	GT9147_RD_Reg(GT_GSTID_REG,&mode,1);	//读取触摸点的状态 
	if(mode&0x80)	//有触摸
	{	
		if((mode&0x0f) == 1)					//如果触摸点是1个
		{
			if(flag_pressed_1 == 0)
			{
				//读第一个点坐标
				GT9147_RD_Reg(GT_TP1_REG,buf,4);
				press0.y = ((u16)buf[1]<<8)|buf[0];
				press0.x = 800-(((u16)buf[3]<<8)|buf[2]);	
				flag_pressed_1 ++;
			}else
			{
				flag_pressed_1 = 0;
				//读点坐标
				GT9147_RD_Reg(GT_TP1_REG,buf,4);
				press1.y = ((u16)buf[1]<<8)|buf[0];
				press1.x = 800-(((u16)buf[3]<<8)|buf[2]);
				if(abs(press0.x - press1.x)> abs(press0.y - press1.y))
				{
					if(abs(press0.x - press1.x)>FENBIAN)
					{
						if(press0.x <press1.x)
						return SLOW;//慢速
						else
							return FAST;
					}

				}
				
				if(abs(press0.x - press1.x) < abs(press0.y - press1.y))
				{
					if(abs(press0.y - press1.y)>FENBIAN)
					{
						if(press0.y <press1.y)
						return SHORT;//矮
						else
							return HIGH;//高
					}

				}
				//长按判断
				if((press0.x ==press1.x) && (press0.y == press1.y))
				{
						return LONGPRESSED;
				}					
			}

			
		}
		else if((mode&0x0f) == 2)					//如果触摸点是2个
		{
			
			//读第一个点坐标
			GT9147_RD_Reg(GT_TP1_REG,buf,4);
			press0.y = ((u16)buf[1]<<8)|buf[0];
			press0.x = 800-(((u16)buf[3]<<8)|buf[2]);
			//读第2个点坐标
			GT9147_RD_Reg(GT_TP2_REG,buf,4);
			press1.y = ((u16)buf[1]<<8)|buf[0];
			press1.x = 800-(((u16)buf[3]<<8)|buf[2]);
			if(flag_pressed == 0)
			{
				flag_pressed = 1;
				dis = (u16)sqrt(pow(abs(press0.x - press1.x),2)+pow(abs( press0.y - press1.y),2));
				
			}else
			{
				flag_pressed = 0;
				dis0 = (u16)sqrt(pow(abs(press0.x - press1.x),2)+pow(abs( press0.y - press1.y),2));
				//printf("dis = %d\r\n",dis);
				//printf("dis0 = %d\r\n",dis0);
				if(abs(dis - dis0)> FENBIAN)	//滑动有效
				{						
					if(dis< dis0)
					{
						dis = 0;							
						return BIG;	//放大
						
					}
					else
					{
						dis = 0;
						return SMALL;//缩小
					}							
				}
				else dis = 0;
			}
			
			
		}
		
		else
		{
			flag_pressed_1 = 0;
			flag_pressed = 0;
			dis = 0;
		}
		
		

	}

	return	DONone;		
}


//判断有没有按到按键，按到返回1，没按到返回0
//u8 IsPressed(Button button)
//{
//	u8 i = 0;
//	for(i=0; i<5; i++)
//	{
//		
//		if(tp_dev.x[i]>button.x0 && tp_dev.y[i]>button.y0 && tp_dev.x[i]<(button.x0+ button.width)&& tp_dev.y[i]<(button.y0+button.height))
//		{
//			
//			return 1;
//		}
//	
//	}
//	return 0;
//}


