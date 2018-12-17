/********************************************************************************
*Author :       explorersoftware.taobao.com 
*Time   :       2017-3-30 09:19:37
*Explain:       通用头文件
*
********************************************************************************/

#ifndef __CRC16_H
#define __CRC16_H

/* 函数以 unsigned short 类型返回 CRC */
/* puchMsg 指向需要计算的数组,用于计算 CRC 的报文 */
/* usDataLen报文中的字节数 */
unsigned short CRC16 (unsigned char *puchMsg,unsigned short usDataLen);


#endif
