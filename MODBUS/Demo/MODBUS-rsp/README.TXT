2018��12��15��15:13:09
�汾 V2.0

��������: 
	1. ԭ�ȵ���洢��ģʽ,��Ϊ����洢
	2. �޸���MODBUS_CONFIG�����ļ�
	3. REQ����ģʽ���Դ�������ӻ�,��д����ָ���洢λ��

������
	���MODBUSЭ���Ǵ�2018.7.13��ʼд,������7.18�ܹ�д��5�졣ʱ��Ƚ϶̣��϶��кܶ಻��֮����ϣ����Ұѷ��ֵ�BUG����Ҫ�󶼸����ң�
	�һ���иĽ�,�������֪ͨ�����ҵ���ϵ��ʽ�� QQ: 108287205

�ļ��ṹ��
		UHEAD.H		- ���ͷ�ļ����������u8,u16,u32���������Ͷ��壬������ûʲô�ˡ�
		MODBUS		- ���ﶨ����REQ(����)��RSP(�ظ�)������ģʽ��ʹ�õĹ����ĺ������쳣���ź͹����붼������������Ͷ�������
					- ����һ����ʱ����ʹ�õĺ������������Ҳ��REQ,RSP����
					- ע���������Ҫ����һ���ط���MD_BUFNUM �㹻��Ϳ��ԡ����Ƕ�����MODBUS���ݵĴ�С,ȡ���ڹ��̵���Ҫ
		MODBUS_REQ	- ����ģʽʹ�ã�������
		MODBUS_RSP	- �ظ�ģʽʹ�ã��ӻ���
		MODBUS_CONFIG	- ����MD_BUFNUM�����е����ö�������
		
���ݴ��λ�ã�
		�ӻ�RSPģʽ :��MODBUS_CONFIG.C�ж������ĸ����飬�ֱ��������Ȧ��������Ȧ������Ĵ���������Ĵ�����
		����REQģʽ����д��Ҫָ�����ݱ�����ȡ��λ�ã�
		

��ֲ������
	�� MODBUS.H ��MD_BUFNUM�㹻�󣬱�֤�����ݴ���ʱ���������

	�� MODBUS_CONFIG.H ������
		- ��������ͷ�ļ���
		- �ֱ�����REQ,RSPģʽ�Ĵ��ڳ�ʼ��������
		- ����Ǵӻ������õ�ַ����ַ��Χ��1~247����
		- ������Ҫʹ�ܶ���BUFF_OutCoil,BUFF_InCoil,BUFF_HoldREG,BUFF_InREG,ָ�򱣴����ݵ�����,����������Ĵ�С
		  
	�� MODBUS_CONFIG.C ������
		- REQģʽ���ô��ڷ��ͺ���: void REQSendMessage(u8* array,u16 num)
		- RSPģʽ���ô��ڷ��ͺ���: void RSPSendMessage(u8* array,u16 num)

ʹ�÷�����

--->���ȣ�����һ��MD_datstr���͵ı�������������������һ������һ����������һ������֮��ĺ��������ĸ��������Ƕ��ĸ���������
��Ҫһ����ʱ������������ʱ�жϣ�
��Ҫһ�����ڣ������������жϣ�
��MODBUS.H�е�	MD_Fun_InTime(MD_datstr* pMD_datstr,u32 period)���ڶ�ʱ���ж��У�periodֵ�Ƕ�ʱ���жϵ�����ʱ�䣬��λus

--->�ӻ�RSPģʽ��
	
	ʹ��RSP.H�еĺ��� MD_RSP_Init(u16 baud,MD_datstr* pMD_Reqstr) ��ʼ���������ò����ʣ�
			�� RSP_Fun_InUart(MD_datstr* pMD_datstr,u8 receiveByte)	���ڴ��ڽ����ж��У�
	����������
		�����ж��Ƿ���յ����ݣ�Ȼ����� RSP_CallBack(MD_datstr* pMD_datstr)���д������ú��������쳣�����Խ��ս��д�����
	
	������	
		if(MD_str.flag_receiveOK == MD_OK)
		{
			RSP_CallBack(&MD_str);					
		}
		
--->����REQģʽ��ʹ��
			MD_REQ_Init(u16 baud,MD_datstr* pMD_Reqstr)��ʼ���������ò����� 
			ʹ��req.H�еĺ����Դӻ�����ѯ�ʣ�
			�������в����ж��Ƿ���յ����ݣ�Ȼ�����REQ_CallBack(MD_datstr* pMD_Reqstr);���д�����
			__weak���εĺ�����Ҫ��Ӧ�����쳣�������������Ҫ��д��Ĭ�ϴ���ʱ������ʾ
			
		������
			
		if(MD_str.flag_listener)					//�������ݺ����
		{
			if(MD_str.flag_waitRelpy == MD_NO)		//�ȴ���ʱ		
			{
				ErroOutTime(&MD_str);				//��ʱ����
				MD_str.flag_listener = 0;			//ֹͣ����
			}
			else
			if(MD_str.flag_receiveOK == MD_OK)		//�����ݽ��յ�
			{
				REQ_CallBack(&MD_str);							
			}
		}
	