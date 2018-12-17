2018年12月15日15:13:09
版本 V2.0

更新内容: 
	1. 原先倒序存储的模式,改为正序存储
	2. 修改了MODBUS_CONFIG配置文件
	3. REQ主机模式可以创建多个从机,读写可以指定存储位置

声明：
	这个MODBUS协议是从2018.7.13开始写,到今天7.18总共写了5天。时间比较短，肯定有很多不足之处。希望大家把发现的BUG或者要求都告诉我，
	我会进行改进,并将结果通知您。我的联系方式是 QQ: 108287205

文件结构：
		UHEAD.H		- 这个头文件里包含的是u8,u16,u32的数据类型定义，其他的没什么了。
		MODBUS		- 这里定义了REQ(请求)和RSP(回复)这两种模式所使用的公共的函数。异常代号和功能码都在这里，数据类型都在这里
					- 还有一个定时器所使用的函数。这个函数也被REQ,RSP公用
					- 注意这里仅需要配置一个地方：MD_BUFNUM 足够大就可以。这是定义存放MODBUS数据的大小,取决于工程的需要
		MODBUS_REQ	- 请求模式使用（主机）
		MODBUS_RSP	- 回复模式使用（从机）
		MODBUS_CONFIG	- 除了MD_BUFNUM，所有的配置都在这里
		
数据存放位置：
		从机RSP模式 :在MODBUS_CONFIG.C中定义了四个数组，分别是输出线圈，输入线圈，输出寄存器，输入寄存器。
		主机REQ模式，读写需要指定数据保存或读取的位置；
		

移植方法：
	① MODBUS.H 中MD_BUFNUM足够大，保证在数据传输时不会溢出。

	② MODBUS_CONFIG.H 中配置
		- 包含串口头文件；
		- 分别设置REQ,RSP模式的串口初始化函数；
		- 如果是从机，设置地址，地址范围（1~247）；
		- 根据需要使能定义BUFF_OutCoil,BUFF_InCoil,BUFF_HoldREG,BUFF_InREG,指向保存数据的数组,并配置数组的大小
		  
	③ MODBUS_CONFIG.C 中配置
		- REQ模式配置串口发送函数: void REQSendMessage(u8* array,u16 num)
		- RSP模式配置串口发送函数: void RSPSendMessage(u8* array,u16 num)

使用方法：

--->首先，定义一个MD_datstr类型的变量。把这个变量想象成一个对象，一个变量就是一个对象。之后的函数调用哪个变量就是对哪个对象处理；
需要一个定时器，并开启定时中断；
需要一个串口，并开启接收中断；
将MODBUS.H中的	MD_Fun_InTime(MD_datstr* pMD_datstr,u32 period)放在定时器中断中，period值是定时器中断的周期时间，单位us

--->从机RSP模式下
	
	使用RSP.H中的函数 MD_RSP_Init(u16 baud,MD_datstr* pMD_Reqstr) 初始化对象，配置波特率；
			将 RSP_Fun_InUart(MD_datstr* pMD_datstr,u8 receiveByte)	放在串口接收中断中；
	在主程序中
		不断判断是否接收到数据，然后调用 RSP_CallBack(MD_datstr* pMD_datstr)进行处理，该函数返回异常，可以接收进行处理。
	
	举例：	
		if(MD_str.flag_receiveOK == MD_OK)
		{
			RSP_CallBack(&MD_str);					
		}
		
--->主机REQ模式下使用
			MD_REQ_Init(u16 baud,MD_datstr* pMD_Reqstr)初始化对象，配置波特率 
			使用req.H中的函数对从机进行询问，
			主程序中不断判断是否接收到数据，然后调用REQ_CallBack(MD_datstr* pMD_Reqstr);进行处理。
			__weak修饰的函数主要对应的是异常处理，请根据需要改写。默认错误时串口显示
			
		举例：
			
		if(MD_str.flag_listener)					//发送数据后监听
		{
			if(MD_str.flag_waitRelpy == MD_NO)		//等待超时		
			{
				ErroOutTime(&MD_str);				//超时处理
				MD_str.flag_listener = 0;			//停止监听
			}
			else
			if(MD_str.flag_receiveOK == MD_OK)		//有数据接收到
			{
				REQ_CallBack(&MD_str);							
			}
		}
	
