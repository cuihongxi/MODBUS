#include "MYTOUCH.H"

//����ǰ�����������
TouchXY Read_Piont(void)
{
	u8	buf[4];
	TouchXY press0;
	//����һ��������
	GT9147_RD_Reg(GT_TP1_REG,buf,4);
	press0.y = ((u16)buf[1]<<8)|buf[0];
	press0.x = 800-(((u16)buf[3]<<8)|buf[2]);
	
	return press0;
}


/*******************************************************
*-��������	  ������ɨ��
*-��������	  ��
*-����		  �����أ���ָ���������£�˫ָ��������С
*-��ע		  ��
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
	
	GT9147_WR_Reg(GT_GSTID_REG,&mode,1);	//���־
	delay_ms(10);
	GT9147_RD_Reg(GT_GSTID_REG,&mode,1);	//��ȡ�������״̬ 
	if(mode&0x80)	//�д���
	{	
		if((mode&0x0f) == 1)					//�����������1��
		{
			if(flag_pressed_1 == 0)
			{
				//����һ��������
				GT9147_RD_Reg(GT_TP1_REG,buf,4);
				press0.y = ((u16)buf[1]<<8)|buf[0];
				press0.x = 800-(((u16)buf[3]<<8)|buf[2]);	
				flag_pressed_1 ++;
			}else
			{
				flag_pressed_1 = 0;
				//��������
				GT9147_RD_Reg(GT_TP1_REG,buf,4);
				press1.y = ((u16)buf[1]<<8)|buf[0];
				press1.x = 800-(((u16)buf[3]<<8)|buf[2]);
				if(abs(press0.x - press1.x)> abs(press0.y - press1.y))
				{
					if(abs(press0.x - press1.x)>FENBIAN)
					{
						if(press0.x <press1.x)
						return SLOW;//����
						else
							return FAST;
					}

				}
				
				if(abs(press0.x - press1.x) < abs(press0.y - press1.y))
				{
					if(abs(press0.y - press1.y)>FENBIAN)
					{
						if(press0.y <press1.y)
						return SHORT;//��
						else
							return HIGH;//��
					}

				}
				//�����ж�
				if((press0.x ==press1.x) && (press0.y == press1.y))
				{
						return LONGPRESSED;
				}					
			}

			
		}
		else if((mode&0x0f) == 2)					//�����������2��
		{
			
			//����һ��������
			GT9147_RD_Reg(GT_TP1_REG,buf,4);
			press0.y = ((u16)buf[1]<<8)|buf[0];
			press0.x = 800-(((u16)buf[3]<<8)|buf[2]);
			//����2��������
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
				if(abs(dis - dis0)> FENBIAN)	//������Ч
				{						
					if(dis< dis0)
					{
						dis = 0;							
						return BIG;	//�Ŵ�
						
					}
					else
					{
						dis = 0;
						return SMALL;//��С
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


//�ж���û�а�����������������1��û��������0
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

