#include <stdio.h>
#include "hk32f0xxa_flash.h"
#include "flash.h"
#include "persistent_layout.h"

#define FLASH_USER_START_ADDR    ((uint32_t)PERSISTENT_PRIMARY_ADDRESS)
#define FLASH_BACKUPDATA_ADDR    ((uint32_t)PERSISTENT_BACKUP_ADDRESS)
#define FLASH_USER_INDEX   0U
#define FLASH_BACK_IDNEX   1U
#define FLASH_CHECK_DATA_OFFSET   PERSISTENT_CHECK_OFFSET
#define FLASH_DATA_NUM            PERSISTENT_DATA_SIZE
unsigned int FlashCheckData[2] = {0};//判断flash数据是否丢失
static unsigned char cSaveChar[PERSISTENT_DATA_SIZE];

/******************************************************************** 
*
* 名称: ReadEepromInt
* 说明: 读eeprom中数据INT型
		eAdd 	   int&0xff
		eAdd+1	 (int >>8 )&0xff
* 功能: 
* 调用:	
* 输入: eAdd 只能是偶数 0~255

* 返回值: 
***********************************************************************/
unsigned short int ReadEepromInt( unsigned int eAdd)//
{
	volatile FLASH_Status FLASHStatus;	
	unsigned int Address;
	unsigned short int Data;
  FlashCheckData[FLASH_USER_INDEX] = *(__IO uint16_t *)(FLASH_USER_START_ADDR + 258);
  FlashCheckData[FLASH_BACK_IDNEX] = *(__IO uint16_t *)(FLASH_BACKUPDATA_ADDR + 258);
  FlashCheckData[FLASH_USER_INDEX] =(FlashCheckData[FLASH_USER_INDEX])&0xFFFF;
	FlashCheckData[FLASH_BACK_IDNEX] =(FlashCheckData[FLASH_BACK_IDNEX])&0xFFFF;
		
  if((FlashCheckData[FLASH_BACK_IDNEX] == 0xffff)&&((FlashCheckData[FLASH_USER_INDEX]) == 0xffff))//两个扇区都为空
  {
    FlashCheckData[FLASH_USER_INDEX] = 0X0001;
    FlashCheckData[FLASH_BACK_IDNEX] = 0X0000;
  }
  else if((FlashCheckData[FLASH_BACK_IDNEX]) == 0xffff)//备份存储区为空
  {
    FlashCheckData[FLASH_BACK_IDNEX] = 0X0000;
  }
  else if((FlashCheckData[FLASH_USER_INDEX]) == 0xffff)//存储区为空
  {
    FlashCheckData[FLASH_USER_INDEX] = 0X0000;
  }
  if(FlashCheckData[FLASH_USER_INDEX] > FlashCheckData[FLASH_BACK_IDNEX])//存储扇区保存的数据较新
  {
		eAdd = eAdd&0xfe;		//eAdd只能是偶数 最低为只能时候偶数
	  Address = FLASH_USER_START_ADDR + eAdd;
  }
  else
  {
   	eAdd = eAdd&0xfe;		//eAdd只能是偶数 最低为只能时候偶数
	  Address = FLASH_BACKUPDATA_ADDR + eAdd;
  }
	
	Data = *(__IO uint16_t *)Address;
	return Data;
}	

/******************************************************************** 
*
* 名称: ReadEeprom
* 说明: 读eeprom中数据
* 功能: 
* 调用:	
* 输入: tFlag
* 返回值: 
***********************************************************************/
char ReadEeprom( unsigned char eAdd)//
{
	unsigned int Data;
		
  Data =  ReadEepromInt(eAdd);
	
	if(eAdd&0x01)	//	去高位
	{
		Data = Data>>8;
	}
	return(Data&0xFF);
}


/******************************************************************** 
*
* 名称: ReadEepromLong
* 说明: 读eeprom中数据Long型
		eAdd 	long&0xff
		eAdd+1	(long >>8 )&0xff
		eAdd+2	(long >>16)&0xff
		eAdd+3	(long >>24)&0xff				
* 功能: 
* 调用:	
* 输入: tFlag
* 返回值: 
***********************************************************************/
long ReadEepromLong( unsigned char eAdd)//
{
	unsigned long int  Data;
		
  Data =  ReadEepromInt(eAdd+2);
	Data = Data<<16;
	Data |=  ReadEepromInt(eAdd);
	
	return(Data);
}

/******************************************************************** 
*
* 名称: ReadEepromFloat
* 说明: 读eeprom中数据Long型
		eAdd 	long&0xff
		eAdd+1	(long >>8 )&0xff
		eAdd+2	(long >>16)&0xff
		eAdd+3	(long >>24)&0xff				
* 功能: 
* 调用:	
* 输入: tFlag
* 返回值: 
**********************************************************************
float ReadEepromFloat( unsigned char eAdd)//
{
	unsigned char tempchar[4];
	float fReadData;
	tempchar[0]= ReadEeprom(eAdd+0);
	tempchar[1]= ReadEeprom(eAdd+1);
	tempchar[2]= ReadEeprom(eAdd+2);
	tempchar[3]= ReadEeprom(eAdd+3);				
	memcpy(&fReadData,tempchar,4);	
	
	return fReadData;	
}
*/
/******************************************************************** 
*
* 名称: 
* 说明: 
* 功能: 
* 调用:	
* 输入: 
* 返回值:
	1 写成功
	0 写入失败 
***********************************************************************/
char WriteEeprom( unsigned int eAdd,char eData)
{
	cSaveChar[eAdd] = eData;
	return 0;
}

/******************************************************************** 
*
* 名称: 
* 说明: 
* 功能: 
* 调用:	
* 输入: 
* 返回值:
	1 写成功
	0 写入失败 
***********************************************************************/
char WriteEepromInt( unsigned int eAdd,int eData)
{
	cSaveChar[eAdd]   = eData;
	cSaveChar[eAdd+1] = eData >>8;
	return 0;
}

/******************************************************************** 
*
* 名称: WriteEepromLong
* 说明: 
* 功能: 
* 调用:	
* 输入: 
* 返回值:
	1 写成功
	0 写入失败 
***********************************************************************/
char WriteEepromLong( unsigned char eAdd,long eData)
{
	cSaveChar[eAdd]   = eData;
	cSaveChar[eAdd+1] = eData >>8*1;
	cSaveChar[eAdd+2] = eData >>8*2;
	cSaveChar[eAdd+3] = eData >>8*3;
	return 0;
}	

/******************************************************************** 
*
* 名称: WriteEepromFloat
* 说明: 写入一个float型字符
* 功能: 
* 调用:	
* 输入: 
* 返回值:
	1 写成功
	0 写入失败 
**********************************************************************
char WriteEepromFloat( unsigned char eAdd,float eData)
{
 unsigned char tempchar[4];
 char cWriteFlag;
 	cWriteFlag = 0;
		memcpy(tempchar,&eData,4);
	 	cWriteFlag =  WriteEeprom(eAdd+0,tempchar[0]);
	 	cWriteFlag |= WriteEeprom(eAdd+1,tempchar[1]);
		cWriteFlag |= WriteEeprom(eAdd+2,tempchar[2]);
		cWriteFlag |= WriteEeprom(eAdd+3,tempchar[3]);		 	
 return	cWriteFlag;	
}	
*/
/******************************************************************** 
*
* 名称: ReadFlashAll()
* 说明: 
* 功能: 
* 调用:	
* 输入: 
* 返回值:
***********************************************************************/
void ReadFlashAll(void)
{
	unsigned int i;
	for(i=0;i<260;i++)
	{
		cSaveChar[i] =  ReadEeprom( i );
		//		printf("%ld, ",cSaveChar[i]);	
	}
}
/******************************************************************** 
*
* 名称: WriteFlashAll()
* 说明: 
* 功能: 
* 调用:	
* 输入: 
* 返回值:
	1 写成功
	0 写入失败 
***********************************************************************/
unsigned char WriteFlashAll(void)
{
	volatile FLASH_Status FLASHStatus;	

	uint32_t StartAddr;
	uint32_t tempLong;

	uint32_t i;
	uint32_t TempAddr;
//  StartAddr = FLASH_USER_START_ADDR;

	 if(FlashCheckData[FLASH_USER_INDEX]<FlashCheckData[FLASH_BACK_IDNEX])//确定谁的数据较旧 当前保存在用户区
  {
    FlashCheckData[FLASH_USER_INDEX] = FlashCheckData[FLASH_BACK_IDNEX]+1;
		StartAddr = FLASH_USER_START_ADDR;
//    WriteFlash_16Bit(FLASH_USER_START_ADDR + FLASH_CHECK_DATA_OFFSET,&FlashCheckData[FLASH_USER_INDEX],1);
	  WriteEepromInt(258,FlashCheckData[FLASH_USER_INDEX]);
  }
  else//当前保存在备份区
  {
    FlashCheckData[FLASH_BACK_IDNEX] = FlashCheckData[FLASH_USER_INDEX]+1;

		StartAddr = FLASH_BACKUPDATA_ADDR;
    WriteEepromInt(258,FlashCheckData[FLASH_BACK_IDNEX]);
//		cSaveChar[257]=FlashCheckData[FLASH_BACK_IDNEX];		
  }
//		printf("Flash %d %d\n",FlashCheckData[0],FlashCheckData[1]);
	FLASHStatus=FLASH_COMPLETE;
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
	FLASHStatus=FLASH_ErasePage(StartAddr + 30 );	
	if(FLASHStatus!=FLASH_COMPLETE)		 return(0); //擦除异常
	//开始写数据	
	TempAddr=StartAddr;
	for(i=0;i<260;i=i+4 )
	{
		tempLong  = cSaveChar[i+3];
		tempLong <<= 8;
		tempLong |= cSaveChar[i+2];	
		tempLong <<= 8;
		tempLong |= cSaveChar[i+1];	
		tempLong <<= 8;
		tempLong |= cSaveChar[i+0];	
		
		FLASHStatus=FLASH_ProgramWord(TempAddr,tempLong);
		if(FLASHStatus!=FLASH_COMPLETE)		 return(0); //写数据异常
		TempAddr+=4;	//写入四个字节
	}
	
	//检验写入数据是否出错
	TempAddr=StartAddr;
	for(i=0;i<260;i=i+4)
	{
		tempLong  = cSaveChar[i+3];
		tempLong <<= 8;
		tempLong |= cSaveChar[i+2];	
		tempLong <<= 8;
		tempLong |= cSaveChar[i+1];	
		tempLong <<= 8;
		tempLong |= cSaveChar[i+0];	
		
		if((*(__IO uint32_t *) TempAddr) != tempLong)
		{	
				return 0;//写入数据不符
		}
		TempAddr += 4;			
	}
	FLASH_Lock();
	return(1);//操作完成
}



////AddrIndex:表示在存储的FLASH_DATA_NUM中第几个数据
////Data:表示存储的数据 
////此函数一次只能更新存在flash里面的一个数据 更新一个数据会擦除扇区一次
//void WriteFlashWord(unsigned int AddrIndex,unsigned int long Data)
//{
//	unsigned int long FlashTemp[FLASH_DATA_NUM];
//	unsigned int short i;

//	if(AddrIndex >= FLASH_DATA_NUM )  	return;		
//	
//	for(i=0;i<FLASH_DATA_NUM;i++)
//	{
//		FlashTemp[i] = ReadFlash(i);
//	}
//	FlashTemp[AddrIndex] = Data;

//	//USART_Cmd(USART1,DISABLE);
//	FLASH_Unlock();
//	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR); 
//	FLASH_ErasePage(FLASH_USER_START_ADDR + 30 );
//	
//	for(i=0;i<FLASH_DATA_NUM;i++)
//	{
//		FLASH_ProgramWord(FLASH_USER_START_ADDR + i*4,FlashTemp[i]);	
//	}

//	FLASH_Lock(); 
//	//USART_Cmd(USART1,ENABLE);
//}


/*
unsigned int ReadFlash(unsigned int short Addr)
{
	unsigned int Address,Data;
	Address = FLASH_USER_START_ADDR + Addr*4;
	Data = *(__IO uint32_t *)Address;
	return Data;
}
*/


////size 表示存储short型数据的个数
//unsigned char WriteFlash_16Bit(unsigned int StartAddr,unsigned short *p_Data,unsigned int Size)
//{
//	volatile FLASH_Status FLASHStatus;	
//	uint32_t EndAddr;
//	//uint32_t ClrPageCount;//有多少页需要被擦除
//	uint32_t i;
//	uint32_t TempAddr;
//	
//	EndAddr=StartAddr+(Size<<1);
//	//ClrPageCount=((EndAddr-StartAddr)/FLASH_PAGE_SIZE)+1;
//	FLASHStatus=FLASH_COMPLETE;
//	
//	FLASH_Unlock();
//	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR); 
//	FLASHStatus=FLASH_ErasePage(FLASH_USER_START_ADDR + 30 );	
//	if(FLASHStatus!=FLASH_COMPLETE)		 return(0); //擦除异常
//	
////	for(i=0;i<ClrPageCount;i++)//擦除页
////	{
////		FLASHStatus=FLASH_ErasePage(StartAddr+(FLASH_PAGE_SIZE*i));	
////    if(FLASHStatus!=FLASH_COMPLETE)		 return(0); //擦除异常
////	}
//	//开始写数据
//	TempAddr=StartAddr;
//	i=0;
//	while(TempAddr<EndAddr)
//	{
//		FLASHStatus=FLASH_ProgramHalfWord(TempAddr,p_Data[i++]);
//		if(FLASHStatus!=FLASH_COMPLETE)		 return(0); //写数据异常
//		TempAddr+=2;	//写入2个字节
//	}
//	//检验写入数据是否出错
//	TempAddr=StartAddr;
//	i=0;
//	while(TempAddr < EndAddr)
//	{
//		if((*(__IO uint16_t *) TempAddr) != p_Data[i++])
//		{	
//				return 0;//写入数据不符
//		}
//		TempAddr += 2;			
//	}
//	FLASH_Lock();
//	return(1);//操作完成
//}

//unsigned char ReadFlash_16Bit(unsigned int StartAddr,unsigned short *p_Data,unsigned int Size)
//{
//  uint32_t EndAddr=StartAddr+Size*2;
//	uint32_t Address = 0x0;
//	uint32_t i=0;
//	Address = StartAddr;
//	while(Address < EndAddr)
//	{
//		p_Data[i++]=(*(__IO uint16_t *) Address);
//		Address += 2;	
//	}
//	return 1;	
//}

////size 表示存储int型数据的个数
//unsigned char WriteFlash_32Bit(unsigned int StartAddr,unsigned int *p_Data,unsigned int Size)
//{
//	volatile FLASH_Status FLASHStatus;	
//	uint32_t EndAddr;
//	//uint32_t ClrPageCount;//有多少页需要被擦除
//	uint32_t i;
//	uint32_t TempAddr;
//	
//	EndAddr=StartAddr+(Size<<2);
//	//ClrPageCount=((EndAddr-StartAddr)/FLASH_PAGE_SIZE)+1;
//	FLASHStatus=FLASH_COMPLETE;
//	
//	FLASH_Unlock();
//	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR); 
//	FLASHStatus=FLASH_ErasePage(FLASH_USER_START_ADDR + 30 );	
//	if(FLASHStatus!=FLASH_COMPLETE)		 return(0); //擦除异常
//	
////	for(i=0;i<ClrPageCount;i++)//擦除页
////	{
////		FLASHStatus=FLASH_ErasePage(StartAddr+(FLASH_PAGE_SIZE*i));	
////    if(FLASHStatus!=FLASH_COMPLETE)		 return(0); //擦除异常
////	}
//	//开始写数据
//	TempAddr=StartAddr;
//	i=0;
//	while(TempAddr<EndAddr)
//	{
//		FLASHStatus=FLASH_ProgramWord(TempAddr,p_Data[i++]);
//		if(FLASHStatus!=FLASH_COMPLETE)		 return(0); //写数据异常
//		TempAddr+=4;	//写入四个字节
//	}
//	//检验写入数据是否出错
//	TempAddr=StartAddr;
//	i=0;
//	while(TempAddr < EndAddr)
//	{
//		if((*(__IO uint32_t *) TempAddr) != p_Data[i++])
//		{	
//				return 0;//写入数据不符
//		}
//		TempAddr += 4;			
//	}
//	FLASH_Lock();
//	return(1);//操作完成
//}


//unsigned char ReadFlash_32Bit(unsigned int StartAddr,unsigned int *p_Data,unsigned int Size)
//{
//  uint32_t EndAddr=StartAddr+Size*4;
//	uint32_t Address = 0x0;
//	uint32_t i=0;
//	Address = StartAddr;
//	while(Address < EndAddr)
//	{
//		p_Data[i++]=(*(__IO uint32_t *) Address);
//		Address += 4;	
//	}
//	return 1;	
//}
