#include "MODBUS_RSP.H"
#include "MODBUS.H"
#include "MODBUS_CONFIG.H"

typeErro RSP_ReadCoil(MD_datstr* pMD_datstr);				//�ظ��������Ȧ
typeErro RSP_ReadInputCoil(MD_datstr* pMD_datstr);	//�ظ���������Ȧ
typeErro RSP_ReadHoldREG(MD_datstr* pMD_datstr);		//�ظ�������Ĵ���
typeErro RSP_ReadInputREG(MD_datstr* pMD_datstr);		//�ظ�������Ĵ���
typeErro RSP_WriteCoil(MD_datstr* pMD_datstr);			//�ظ�д��Ȧ
typeErro RSP_WriteREG(MD_datstr* pMD_datstr);				//�ظ�д�����Ĵ���
typeErro RSP_WriteCoils(MD_datstr* pMD_datstr);			//�ظ�д�����Ȧ
typeErro RSP_WriteREGs(MD_datstr* pMD_datstr);			//�ظ�д����Ĵ���


//��Ϣ��Ӧ��������
mesUnit switchMes[] = {

	#ifdef   BUFF_OutCoil 	
		MD_READ_COIL,				RSP_ReadCoil,				//����Ȧ,��ɢ�����
	  MD_WRITE_COIL,			RSP_WriteCoil,			//д������Ȧ
		MD_WRITE_COILS,			RSP_WriteCoils,			//д�����Ȧ  	
	#endif
	
	#ifdef   BUFF_InCoil
		MD_READ_INPUT,			RSP_ReadInputCoil,	//����ɢ������
	#endif
	
	#ifdef   BUFF_HoldREG
		MD_READ_HOLD_REG, 	RSP_ReadHoldREG,		//������Ĵ���,����Ĵ���
		MD_WRITE_REG, 			RSP_WriteREG, 			//д�����Ĵ���
    MD_WRITE_REGS,			RSP_WriteREGs,			//д����Ĵ���   
	#endif
	
	#ifdef   BUFF_InREG
    MD_READ_INPUT_REG, 	RSP_ReadInputREG,		//������Ĵ���
	#endif
};

//�����Ӧ��������
mesUnit switchErroMes[] = {
	MD_ERROR_FUN,		RSP_ErroFUN,	//�Ƿ�����
	MD_ERROR_ADDR,	RSP_ErroADDR,	//�Ƿ���ַ
	MD_ERROR_NUM, 	RSP_ErroNUM,	//�Ƿ�����ֵ
  MD_ERROR_4, 		RSP_Erro4,		//��վ�豸����
	MD_ERROR_CRC,		RSP_ErroCRC,	//crc����
};
/************************************************************************************
*-��������	��ö�ٹ��ܺ���
*-����		��
*����ֵ		��
*-��������	��
*-������	�������в�˶���ӿƼ�
*/
static typeErro RSPEnumAddrFun(MD_datstr* pMD_datstr)
{
	u16 i;
	typeErro erro = 0;
	for (i = 0;i < GETSIZE(switchMes);i++)
	{
		if(switchMes[i].funNum == pMD_datstr->funNum)
		{
			erro = (*switchMes[i].fun)(pMD_datstr);
		}
	}
	return erro;
}

/************************************************************************************
*-��������	��ö�ٴ���������
*-����		��
*����ֵ		��
*-��������	��
*-������	�������в�˶���ӿƼ�
*/
static typeErro RSPEnumErroFun(MD_datstr* pMD_datstr)
{
	u16 i;
	typeErro erro = 0;
	for(i = 0;i < GETSIZE(switchMes);i++)
	{
		if(switchErroMes[i].funNum == (pMD_datstr->datBuf[0]))
		{
			erro = (*switchErroMes[i].fun)(pMD_datstr);
		}
	}
	return erro;
}

/************************************************************************************
*-��������	��ĩβ�Զ�����CRCУ��Ļظ�
*-����		��num ��Ч�ֽ�����������CRC�ֽڣ�,num���ڵ���2��devaddr ������ַ
*-����ֵ		��
*-��������	��
*-������		�������в�˶���ӿƼ�
*/

typeErro RSP_SendWithCRC(MD_datstr* pMD_datstr,u8 num,u8 dev_addr)
{
	u16 crcdat = 0;
	if(num < 2) return MD_ERROR_NUM;
	if(pMD_datstr->addr == 0) return MD_ERROR_OK;		//�㲥ģʽ
	pMD_datstr->addr = dev_addr;
	crcdat = CRC16 ((u8 *)pMD_datstr,num);
	pMD_datstr->datBuf[num-2] = (u8)crcdat;
	pMD_datstr->datBuf[num-1] = (u8)(crcdat>>8);	//CRC�ȷ��͵��ֽ��ٷ��͸��ֽ�
#if MODBUS_UART_DMA == 0
	crcdat = 1000;
	while(pMD_datstr->flag_CanSend == MD_NO && crcdat)crcdat--;		//�ȴ�3.5���ֽ�֡���
#endif
	RSPSendMessage((u8 *)pMD_datstr,num + 2);   	//����
	pMD_datstr->flag_CanSend = MD_NO;				//��ʱ֡���
	return MD_ERROR_OK;
	
}

/************************************************************************************
*-��������	������ظ�
*-����	��erro ������ funNum ������ arraySend ���յķ�������
*����ֵ	��
*-��������	��
*-������	�������в�˶���ӿƼ�
*/
static void Modbus_Exception_Rsp(typeErro erro,MD_datstr* pMD_datstr)
{
	pMD_datstr->addr = RSP_LOCAL_ADDR;
	pMD_datstr->funNum |= 0x80;
	pMD_datstr->datBuf[0] = erro;
	RSP_SendWithCRC(pMD_datstr,3,RSP_LOCAL_ADDR);
}

#if (defined BUFF_OutCoil)||(defined BUFF_InCoil)
/************************************************************************************
*-��������	������Ȧ
*-����		��startaddr,��ʼ��ַ��num Ҫ��ȡ������(1~2000),datBuf���뻺������pBuf��Ȧ������
*-����ֵ		������datBufռ�õ�����
*-��������	��
*-������		�������в�˶���ӿƼ�
*/
static u8 MD_ReadCoil(u16 startaddr,u16 num,u8* datBuf,u8* pBuf)
{
	u16 i = 0;
	u16 bytepos;
	u8  bitstart;
	
  bitstart = startaddr&7;										//bit���ֽ��п�ʼ��λ��
	bytepos = startaddr/8;										//��ʼ���ֽ�
	//num����8
	for(i = 0;num>=8;i++)
	{
		datBuf[i] = (pBuf[bytepos + i] >> bitstart)|(pBuf[bytepos + 1 + i]<<(8 - bitstart));
		num -= 8;
	}
	//numС��8
	if(num != 0)
		datBuf[i] = ((u8)(((pBuf[bytepos + i] >> bitstart)|(pBuf[bytepos + 1 + i]<<(8 - bitstart)))<<(8-num))>>(8-num));
	else i--;
	return i+1;
}
#endif

#ifdef   BUFF_OutCoil
/************************************************************************************
*-��������	���ظ�����Ȧ
*-����		��
*-����ֵ		��
*-��������	������Ȧ����������
*-������		�������в�˶���ӿƼ�
*/
static typeErro RSP_ReadCoil(MD_datstr* pMD_datstr)
{
	typeErro erro = MD_ERROR_OK;
	u16	num = 0;
	u16	addr_start  = 0;     																		//��ʼ��ַ,0~0xFFFF
	u16	num_bit    = 0 ;      																	//����,1~0x7d0

	addr_start = GetBigEndValu(pMD_datstr->datBuf,2);  					//��ȡ��ʼ��ַ          
	num_bit = GetBigEndValu(&(pMD_datstr->datBuf[2]),2);				//��ȡҪ��������	 
	if(num_bit==0 || num_bit>0x7d0 || num_bit > (GETSIZE(BUFF_OutCoil)*8))    
		return MD_ERROR_NUM;  																		//�������� 
	if((u32)(addr_start+num_bit)>(GETSIZE(BUFF_OutCoil)*8))
		return MD_ERROR_ADDR;																			//�����ַ
	pMD_datstr->datBuf[0] = (num_bit+7)/8;											//�ֽ���
	num = MD_ReadCoil(addr_start,num_bit,&(pMD_datstr->datBuf[1]),pMD_datstr->pBuf_OutCoil);//������Ȧ����
	RSP_SendWithCRC(pMD_datstr,num+3,RSP_LOCAL_ADDR);						//�Զ�����CRC���ظ�
	return erro;    
}


/************************************************************************************
*-��������	��д������Ȧ
*-����			��startaddr,��ʼ��ַ��off_onֵֻ�������֣�0xff00Ϊ����0Ϊ�أ�
							pBuf��Ȧ������
*-����ֵ		��
*-��������	��
*-������		�������в�˶���ӿƼ�
*/
static u8 MD_WriteCoil(u16 startaddr,MD_SWITCH off_on,u8* pBuf)
{
	u16 bytepos;
	u8 bitstart;
	
  bitstart = startaddr&7;										//bit���ֽ��п�ʼ��λ��
	bytepos = startaddr/8;										//��ʼ���ֽ�
	if(off_on == MD_OFF)
	{
		MD_TURN_OFF(pBuf[bytepos],bitstart);			//�ر����λ
	}else
	{		
		MD_TURN_ON(pBuf[bytepos],bitstart);			//�����λ
	}

	return 0;
}

/************************************************************************************
*-��������	���ظ�д������Ȧ
*-����			��
*����ֵ			��
*-��������	��
*-������		�������в�˶���ӿƼ�
*/
static typeErro RSP_WriteCoil(MD_datstr* pMD_datstr)
{
	typeErro erro = MD_ERROR_OK;
	u16   addr_start  = 0   ;     															//��ʼ��ַ, 0~0xFFFF
	u16   sw    = 0   ;      																		//������,   0xff00ΪON,0ΪOFF
      
	addr_start = GetBigEndValu(pMD_datstr->datBuf,2);  					//��ȡ��ʼ��ַ          
	sw = GetBigEndValu(&(pMD_datstr->datBuf[2]),2);							//��ȡ���ֵ 
 	
	//��Ч���ж�
	if(addr_start > GETSIZE(BUFF_OutCoil)*8) return MD_ERROR_ADDR;
	if(sw == 0 || sw == 0xff00)
	{
		MD_WriteCoil(addr_start,(MD_SWITCH)sw,pMD_datstr->pBuf_OutCoil);	//д����Ȧ
	}else  return MD_ERROR_NUM;																			//�������� 
	RSP_SendWithCRC(pMD_datstr,6,RSP_LOCAL_ADDR);										//�Զ�����CRC���ظ�
	return erro;          
}
/************************************************************************************
*-��������	��д�����Ȧ
*-����			��startaddr,��ʼ��ַ��num Ҫд�������,datBufд�뻺������pBuf��Ȧ������
*-����ֵ		��
*-��������	��
*-������		�������в�˶���ӿƼ�
*/
static u8 MD_WriteCoils(u16 startaddr,u16 num,u8* datBuf,u8* pBuf)
{
	u16 i = 0;
	u16 bytepos;
	u8 bitstart;
	u8 dat = 0xff;
  bitstart = startaddr&7;							//bit���ֽ��п�ʼ��λ��
	bytepos = startaddr/8;							//��ʼ���ֽ�
	//num����8
	if(num >= 8)
	{
		for(i = 0;num>=8;i++)
		{		
			pBuf[bytepos + i] = (((u8)(pBuf[bytepos + i]<<(8 - bitstart)))>>(8 - bitstart))| (datBuf[i]<<bitstart);
			pBuf[bytepos + i + 1] = (((u8)(pBuf[bytepos + i + 1] >> bitstart))<<bitstart) |(datBuf[i]>>(8-bitstart));
			num -= 8;
		}		
	}
	//numС��8
	 if(num +bitstart >8)
	{
		pBuf[bytepos + i] = ((pBuf[bytepos + i]<<(8 - bitstart))>>(8 - bitstart))| (datBuf[i]<<bitstart);
		num = num - (8-bitstart);
		pBuf[bytepos + i + 1] =  ((pBuf[bytepos + i + 1] >> num)<<num) |(datBuf[i]>>(8-num));
	}else
	if(num > 0)
	{
		dat = ~((u8)(((dat>>bitstart)<<bitstart)<<(8-bitstart-num))>>(8-bitstart-num));
		pBuf[bytepos + i] =  (pBuf[bytepos + i]&dat)| (datBuf[i]<<bitstart);
	}	
	return 0;
}

/************************************************************************************
*-��������	���ظ�д�����Ȧ
*-����		��
*����ֵ		��
*-��������	��
*-������		�������в�˶���ӿƼ�
*/
static typeErro RSP_WriteCoils(MD_datstr* pMD_datstr)
{
	typeErro erro = MD_ERROR_OK;
	u16   addr_start  = 0   ;       //��ʼ��ַ,     0~0xFFFF
	u16   out_num     = 0   ;       //�������,     1~0x7b0
	u8    bytenum     = 0   ;       //�ֽ���

	addr_start = GetBigEndValu(pMD_datstr->datBuf,2);  						//��ȡ�Ĵ����ĵ�ַ         
	out_num = GetBigEndValu(&(pMD_datstr->datBuf[2]),2);					//��ȡҪд������
	//�ֽ���           
	bytenum = (out_num+7)/8 ;
	if(bytenum != pMD_datstr->datBuf[4] ||out_num==0||out_num>0X7B ||out_num>(GETSIZE(BUFF_OutCoil)*8))  
		return MD_ERROR_NUM;		//�������� 
	if((u32)(addr_start + bytenum) > GETSIZE(BUFF_OutCoil)*8) 
		return MD_ERROR_ADDR;		//��ַ���� 
	if(MD_WriteCoils(addr_start,out_num,&(pMD_datstr->datBuf[5]),pMD_datstr->pBuf_OutCoil))  
		return  MD_ERROR_4;   		//���豸�쳣   

	RSP_SendWithCRC(pMD_datstr,6,RSP_LOCAL_ADDR);											//�Զ�����CRC���ظ�
	return erro;          

}
#endif

#ifdef   BUFF_InCoil
/************************************************************************************
*-��������	���ظ�����ɢ������
*-����		��
*-����ֵ		��
*-��������	��
*-������		�������в�˶���ӿƼ�
*/
static typeErro RSP_ReadInputCoil(MD_datstr* pMD_datstr)
{
	typeErro erro = MD_ERROR_OK;
	u16 	num = 0;
	u16   addr_start  = 0   ;     															//��ʼ��ַ,0~0xFFFF
	u16   num_bit    	= 0   ;      															//����,1~0x7d0

	addr_start = GetBigEndValu(pMD_datstr->datBuf,2);  					//��ȡ��ʼ��ַ          
	num_bit = GetBigEndValu(&(pMD_datstr->datBuf[2]),2);				//��ȡҪ��������
	if(num_bit==0 || num_bit>0x7d0 || num_bit > (GETSIZE(BUFF_InCoil)*8)) 
	  return MD_ERROR_NUM;																			//��������  2000
	if((u32)(addr_start + num_bit)>(GETSIZE(BUFF_InCoil)*8))
	{
		return MD_ERROR_ADDR;																			//�����ַ
	}											
	pMD_datstr->datBuf[0] = (num_bit+7)/8;											//�ֽ���
	num = MD_ReadCoil(addr_start,num_bit,&(pMD_datstr->datBuf[1]),pMD_datstr->pBuf_InputCoil);//������Ȧ����
	RSP_SendWithCRC(pMD_datstr,num+3,RSP_LOCAL_ADDR);										//�Զ�����CRC���ظ�
	return erro;            
}

#endif

  
#if (defined BUFF_HoldREG) || (defined BUFF_InREG)
/************************************************************************************
*-��������	�����Ĵ���
*-����		��startaddr,��ʼ��ַ��num Ҫ��ȡ������,datBuf���뻺������pBuf�Ĵ���������
*-����ֵ		������datBufռ�õ�����
*-��������	��
*-������		�������в�˶���ӿƼ�
*/
u8 MD_ReadREG(u16 startaddr,u16 num,u8* datBuf,u16* pBuf)
{
	u16 i = 0;
	for(i = 0;i<num;i++)
	{
		datBuf[i*2] = (u8)(pBuf[startaddr+i]>>8);
		datBuf[i*2+1] = (u8)pBuf[startaddr+i];
	}
	return i*2;
}
#endif 
#ifdef   BUFF_HoldREG 
/************************************************************************************
*-��������	���ظ�������Ĵ���
*-����		��
*����ֵ		��
*-��������	��
*-������	�������в�˶���ӿƼ�
*/
static typeErro RSP_ReadHoldREG(MD_datstr* pMD_datstr)
{
		typeErro erro = MD_ERROR_OK;
		u16   addr_start  = 0   ;     															//��ʼ��ַ,  0~0xFFFF
		u16   num_reg     = 0   ;      															//����,     1~0x7d
		addr_start = GetBigEndValu(pMD_datstr->datBuf,2);  					//��ȡ��ʼ��ַ   
		num_reg = GetBigEndValu(&(pMD_datstr->datBuf[2]),2);				//��ȡҪ���ļĴ�������
		if(num_reg==0 || num_reg>0x7d || num_reg > GETSIZE(BUFF_HoldREG))   
			return MD_ERROR_NUM;																			//�������� 
		if((u32)(addr_start + num_reg) > GETSIZE(BUFF_HoldREG))
			return MD_ERROR_ADDR;																			//�����ַ
    pMD_datstr->datBuf[0] = num_reg*2;													//�ֽ���
	  num_reg = MD_ReadREG(addr_start,num_reg,&(pMD_datstr->datBuf[1]),pMD_datstr->pBuf_HoldREG);
	  RSP_SendWithCRC(pMD_datstr,num_reg+3,RSP_LOCAL_ADDR);				//�Զ�����CRC���ظ�
    return erro;          
      
}


/************************************************************************************
*-��������	��д�����Ĵ���
*-����			��addr,��ʼ��ַ��,datд���ֵ��pBuf�Ĵ���������
*-����ֵ		��
*-��������	��
*-������		�������в�˶���ӿƼ�
*/
static u8 MD_WriteREG(u16 addr,u16 dat,u16* pBuf)
{
	pBuf[addr] = dat;

	return 0;
}
/************************************************************************************
*-��������	���ظ�д�����Ĵ���
*-����			��
*����ֵ			��
*-��������	��
*-������		�������в�˶���ӿƼ�
*/
static typeErro RSP_WriteREG(MD_datstr* pMD_datstr)
{
	typeErro erro = MD_ERROR_OK;
	u16   addr_start  = 0   ;     //��ʼ��ַ,        0~0xFFFF
	u16   reg_num    = 0   ;      //�Ĵ�����ֵ,      0~0xFFFF

	addr_start = GetBigEndValu(pMD_datstr->datBuf,2);  												//��ȡ�Ĵ����ĵ�ַ         
	reg_num = GetBigEndValu(&(pMD_datstr->datBuf[2]),2);											//��ȡҪд��Ĵ�����ֵ       
	if(addr_start> GETSIZE(BUFF_HoldREG))   return  MD_ERROR_ADDR;   					//��ַ�쳣 	
	if(MD_WriteREG(addr_start,reg_num,pMD_datstr->pBuf_HoldREG))							//д��Ĵ���
		return MD_ERROR_4;																											//���豸�쳣
	RSP_SendWithCRC(pMD_datstr,6,RSP_LOCAL_ADDR);															//�Զ�����CRC���ظ�
	return erro;                
}

/************************************************************************************
*-��������	��д����Ĵ���
*-����		��startaddr ��ʼ��ַ,num д�������, datBufд�뻺������pBuf�Ĵ���������
*����ֵ		��
*-��������	��
*-������		�������в�˶���ӿƼ�
*/
static typeErro MD_WriteREGs(u16 startaddr,u16 num,u8* datBuf,u16* pBuf)
{
	u8 i = 0;
	for(i=0;i<num;i++)
	{
		pBuf[startaddr + i] = ((u16)datBuf[i*2]<<8)|datBuf[i*2+1];
		
	}
    return 0;                
}

/************************************************************************************
*-��������	���ظ�д����Ĵ���
*-����		��
*����ֵ		��
*-��������	��
*-������		�������в�˶���ӿƼ�
*/
static typeErro RSP_WriteREGs(MD_datstr* pMD_datstr)
{
	typeErro erro = MD_ERROR_OK;
	u16   addr_start  = 0   ;     //��ʼ��ַ,        0~0xFFFF
	u16   reg_num    = 0    ;     //�Ĵ���������,      1~0x7B
	u8    bytenum     = 0   ;     //�ֽ���

	addr_start = GetBigEndValu(pMD_datstr->datBuf,2);  							//��ȡ��ʼ�Ĵ�����ַ          
	reg_num = GetBigEndValu(&(pMD_datstr->datBuf[2]),2);						//��ȡҪд��Ĵ��������� 
	bytenum =  pMD_datstr->datBuf[4];

	if(reg_num==0 || reg_num>0x7B ||(u16)bytenum != 2*reg_num ||reg_num>GETSIZE(BUFF_HoldREG))
		return  MD_ERROR_NUM;																					//��������
	if((u32)(addr_start + reg_num) > GETSIZE(BUFF_HoldREG))
		return MD_ERROR_ADDR;																					//��ַ���� 
	if(MD_WriteREGs(addr_start,reg_num,&(pMD_datstr->datBuf[5]),pMD_datstr->pBuf_HoldREG))    
		return MD_ERROR_4;   																					//�����쳣 
	RSP_SendWithCRC(pMD_datstr,6,RSP_LOCAL_ADDR);										//�Զ�����CRC���ظ�
	return erro;                
}
#endif



#ifdef   BUFF_InREG
/************************************************************************************
*-��������	��������Ĵ���
*-����		��addrStart ��ʼ��ַ,num_bit Ҫ���ĸ��� , datbuf ��ʱ�������鱣����������� 
            , num_byte ��ʱ����,����Ӧ���� num_bit*2
*����ֵ		��
*-��������	��
*-������		�������в�˶���ӿƼ�
*/
static typeErro RSP_ReadInputREG(MD_datstr* pMD_datstr)
{
		typeErro erro = MD_ERROR_OK;
		u16   addr_start  = 0   ;     //��ʼ��ַ,  0~0xFFFF
		u16   num_reg     = 0   ;      //����,     1~0x7d

		addr_start = GetBigEndValu(pMD_datstr->datBuf,2);  							//��ȡ��ʼ��ַ          
		num_reg = GetBigEndValu(&(pMD_datstr->datBuf[2]),2);						//��ȡҪ���ļĴ�������
		if(num_reg==0 || num_reg>0x7d|| num_reg > GETSIZE(BUFF_InREG))   
			return MD_ERROR_NUM;																					//��������
		if((addr_start + num_reg) > GETSIZE(BUFF_InREG))
			return MD_ERROR_ADDR;																					//�����ַ	  
		pMD_datstr->datBuf[0] = num_reg*2;															//�ֽ���
	  num_reg = MD_ReadREG(addr_start,num_reg,&(pMD_datstr->datBuf[1]),pMD_datstr->pBuf_INREG);
	  RSP_SendWithCRC(pMD_datstr,num_reg+3,RSP_LOCAL_ADDR);									//�Զ�����CRC���ظ�
    return erro;            
      
}
#endif

/************************************************************************************
*-��������	���ظ�����Ļص�����������MODEBUS����
*-����		��
*����ֵ		���쳣��
*-��������	��
*-������	�������в�˶���ӿƼ�
*/

typeErro RSP_CallBack(MD_datstr* pMD_Reqstr)
{
	u16 crcdat;
	typeErro erro = MD_ERROR_OK;

	//���ж�CRC
	if(pMD_Reqstr->num_byte < 4)
		{
			MD_InitArg(pMD_Reqstr);						//���ݸ�λ
			return REQ_RECEIVE_ERROR_NUM;
		}
	crcdat = CRC16 ((u8*)pMD_Reqstr,pMD_Reqstr->num_byte - 2);
	if(pMD_Reqstr->datBuf[pMD_Reqstr->num_byte - 4] != (u8)(crcdat) ||pMD_Reqstr->datBuf[pMD_Reqstr->num_byte-3] != (u8)(crcdat>>8) )										//����ӦCRC����
	{
		pMD_Reqstr->datBuf[0] = MD_ERROR_CRC;
		RSPEnumErroFun(pMD_Reqstr);					//CRC����
		MD_InitArg(pMD_Reqstr);						//���ݸ�λ
		return MD_ERROR_CRC;
	}
		
	//�жϹ����뷶Χ
	if(pMD_Reqstr->funNum > MAX_FUNNUM || pMD_Reqstr->funNum == 0) 
	{
		Modbus_Exception_Rsp(MD_ERROR_FUN ,pMD_Reqstr);  			//�ظ�������
		RSPEnumErroFun(pMD_Reqstr);									//ִ�д���������
		MD_InitArg(pMD_Reqstr);										//���ݸ�λ
		return MD_ERROR_FUN;
	}
	
	//����������
	erro = RSPEnumAddrFun(pMD_Reqstr);
	if(erro != MD_ERROR_OK) 
	{
		Modbus_Exception_Rsp(erro,pMD_Reqstr);  					//�ظ�������  
		RSPEnumErroFun(pMD_Reqstr);									//ִ�д���������
	}
	MD_InitArg(pMD_Reqstr);
	return erro;
}

/************************************************************************************
*-��������	����ʼ��MODBUS�ӻ������ʣ���ʱ���ĳ�ʱ��׼������Ӧ�Ĵ�������
*-����		��timerbase ��ʱ��׼����λus���ֽڼ䳬ʱ1.5����׼�Ͷ�������֡��֡��ļ������3.5����׼
*-����ֵ		��
*-��������	��
*-������		�������в�˶���ӿƼ�
*/

void MD_RSP_Init(u16 baud,MD_datstr* pMD_Reqstr)
{
	pMD_Reqstr->timerbase = (u32)(3.5*(1000000/baud)*8);
	
#ifdef   BUFF_OutCoil 
	pMD_Reqstr->pBuf_OutCoil = (u8*)&BUFF_OutCoil;
#endif
#ifdef   BUFF_InCoil
	pMD_Reqstr->pBuf_InputCoil = (u8*)&BUFF_InCoil;
#endif

#ifdef   BUFF_HoldREG
	pMD_Reqstr->pBuf_HoldREG = (u16*)&BUFF_HoldREG;
#endif

#ifdef   BUFF_InREG
	pMD_Reqstr->pBuf_INREG = (u16*)&BUFF_InREG;
#endif
	
	RSP_UART_INIT(baud);					//���ڳ�ʼ��
}


/************************************************************************************
*-��������	���ڴ��ڽ����еĺ���
*-����		��receiveByte ���ڽ��յ�������
*-����ֵ		��
*-��������	��֮ǰ���͵����֮���յ��Ļظ�
*-������		�������в�˶���ӿƼ�
*/

void RSP_Fun_InUart(MD_datstr* pMD_Reqstr,u8 receiveByte)
{
	u8* datBuf = (u8*)pMD_Reqstr;
	if(pMD_Reqstr->flag_headTimer == MD_OK)			//����������ַ����
	{
		pMD_Reqstr->head_counter = 0;
		return ;
	}
	if(pMD_Reqstr->flag_receiveOK == MD_NO)			//���ڿ���״̬
	{
		//���յ���Ϣ
		if(pMD_Reqstr->num_byte == 0) 
		{
			if(receiveByte == RSP_LOCAL_ADDR || receiveByte==0)	//����Ǳ��ص���Ϣ�����߹㲥��Ϣ
				pMD_Reqstr->flag_startTimer = MD_OK;			//������ʱ����ʱ
			else
			{
				pMD_Reqstr->flag_headTimer = MD_OK;
				pMD_Reqstr->head_counter = 0;					//���֡ͷ��ʱ
			}
				
			
		}
		if(pMD_Reqstr->flag_startTimer == MD_OK)
		{
			pMD_Reqstr->frame_counter = 0;				//����֡��ʱ��
			datBuf[pMD_Reqstr->num_byte] = receiveByte;	//��������
			if(pMD_Reqstr->num_byte < MD_BUFNUM)		//��ֹ�������	
				pMD_Reqstr->num_byte ++;				//�����ֽ�������		
		}

		
	}
	
}



//����������
__weak typeErro RSP_ErroFUN(MD_datstr* pMD_datstr)
{
	ERROR_Log("funNum = 0x%x\r\n",pMD_datstr->funNum);
	ERROR_Log("RSP_ErroFUN\r\n");
	return pMD_datstr->datBuf[0];
}

__weak typeErro RSP_ErroADDR(MD_datstr* pMD_datstr)
{
	ERROR_Log("funNum = 0x%x\r\n",pMD_datstr->funNum);
	ERROR_Log("RSP_ErroADDR\r\n");
	return pMD_datstr->datBuf[0];
}

__weak typeErro RSP_ErroNUM(MD_datstr* pMD_datstr)
{
	ERROR_Log("funNum = 0x%x\r\n",pMD_datstr->funNum);
	ERROR_Log("RSP_ErroNUM\r\n");
	return pMD_datstr->datBuf[0];
}	

__weak typeErro RSP_Erro4(MD_datstr* pMD_datstr)
{
	ERROR_Log("funNum = 0x%x\r\n",pMD_datstr->funNum);
	ERROR_Log("RSP_Erro4\r\n");
	return pMD_datstr->datBuf[0];
}
__weak typeErro RSP_ErroCRC(MD_datstr* pMD_datstr)
{
	ERROR_Log("funNum = 0x%x\r\n",pMD_datstr->funNum);
	ERROR_Log("RSP_ErroCRC\r\n");
	return pMD_datstr->datBuf[0];
}