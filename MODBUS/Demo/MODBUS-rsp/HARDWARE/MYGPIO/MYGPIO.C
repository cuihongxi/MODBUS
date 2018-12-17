#include<mygpio.h>
/*******************************************************
*-函数名称	：
*-函数作用	：GPIO推挽模式
*-参数		：
*-返回值	：
*-备注		：
*******************************************************/
void GPIO_OutPP_Init(GPIO_TypeDef* GPIOx,uint16_t GPIO_Pin)
{
    GPIO_InitTypeDef GPIO_TypeStruc;
    RCC_APB2PeriphClockCmd(0X00000004<<(((u32)GPIOx-(u32)GPIOA)/0x400),ENABLE);

    GPIO_TypeStruc.GPIO_Mode = GPIO_Mode_Out_PP;    //推挽
    GPIO_TypeStruc.GPIO_Pin = GPIO_Pin;
    GPIO_TypeStruc.GPIO_Speed = GPIO_Speed_50MHz;   
    GPIO_Init(GPIOx,&GPIO_TypeStruc);
}

/*******************************************************
*-函数名称	:
*-函数作用:GPIO复用推挽模式
*-参数	
*-返回值	
*-备注		：
*******************************************************/
void GPIO_AFPP_Init(GPIO_TypeDef* GPIOx,uint16_t GPIO_Pin)
{
    GPIO_InitTypeDef GPIO_TypeStruc;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO|(0X00000004<<(((u32)GPIOx-(u32)GPIOA)/0x400)), ENABLE);    //端口复用时钟打开,GPIOA时钟
    GPIO_TypeStruc.GPIO_Mode = GPIO_Mode_AF_PP;    //复用推挽
    GPIO_TypeStruc.GPIO_Pin = GPIO_Pin;
    GPIO_TypeStruc.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOx,&GPIO_TypeStruc);
}

/*******************************************************
*-函数名称	:
*-函数作用:GPIO模式
*-参数：	GPIO_Mode：	GPIO_Mode_AIN					模拟输入
											GPIO_Mode_IN_FLOATING	浮空输入
											GPIO_Mode_IPD					下拉输入
											GPIO_Mode_IPU 				上拉输入
											GPIO_Mode_Out_OD 			开漏输出
											GPIO_Mode_Out_PP			推挽输出
											GPIO_Mode_AF_OD 			复用开漏输出
											GPIO_Mode_AF_PP 			复用推挽输出
*-返回值	
*-备注		：
*******************************************************/
void My_GPIO_Init(GPIO_TypeDef* GPIOx,uint16_t GPIO_Pin,GPIOMode_TypeDef GPIO_Mode)
{
    GPIO_InitTypeDef GPIO_TypeStruc;
    RCC_APB2PeriphClockCmd(0X00000004<<(((u32)GPIOx-(u32)GPIOA)/0x400), ENABLE);    //时钟打开,GPIO时钟
    GPIO_TypeStruc.GPIO_Mode = GPIO_Mode;   
    GPIO_TypeStruc.GPIO_Pin = GPIO_Pin;
    GPIO_TypeStruc.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOx,&GPIO_TypeStruc);
}
