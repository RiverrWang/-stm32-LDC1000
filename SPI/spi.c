/*-----------------------------------------------------------
			STM32 SPI&SST25VF016B������������
 ˵��: SPI�������ú�SST25V016B FLASH������������,д�˳����
       ʱ�򻨷��˹�����,�����˰�����Բ�����,����һ������
	   ֮��������������Ȼ����ԭ����CSƬѡ��ƽ�㷴��,һ���Ե�
	   ����ȫ��ȷ��,�Դ�,SPI�������Ѿ�������Ϥ.����Ĳ���ʱ��
	   ��ָ�������SST25VF016B�ֲ�.
 ʱ��: 2013��8��8��      BoX��д
-----------------------------------------------------------*/
#include "stm32f10x.h"
#include "spi.h"

#define CS_1 GPIO_ResetBits(GPIOC,GPIO_Pin_4)
#define CS_0 GPIO_SetBits(GPIOC,GPIO_Pin_4)
/*----------------------------------------
 ��������: FLASH��ʼ��
 ��������: ��
 �� �� ֵ: ��
----------------------------------------*/
void SPI_Flash_Init(void)
{
	SPI_Config();
}
/*----------------------------------------
 ��������: STM32 SPI1��������
 ��������: ��
 �� �� ֵ: ��
----------------------------------------*/
void SPI_Config(void)	  //�ܶ��廹��������SPI��������Ҫ���޹صĹص�
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef SPI_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);	//ʹ��APB2����ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	//��סʹ��AFIOʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC , ENABLE);
	//A5->SCK  A6->MISO A7->MOSI
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	  //SPI�ø����������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	//C4->CS  FLASH��Ƭѡ
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC,&GPIO_InitStructure);
	//B7->CS  ������Ƭѡ
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	//A4->CS ��̫��Ƭѡ
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	//SPI1����
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;	//2��ȫ˫��
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;			//SPI��
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//���ݸ�ʽ8λ
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;				//ʱ�Ӽ��Ը�
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;		    //ʱ����λ�ڶ����ش���
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler	= SPI_BaudRatePrescaler_8; //8��Ƶ
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;		//MSB��ǰ
	SPI_InitStructure.SPI_CRCPolynomial = 7;				//CRC����ʽУ��
	SPI_Init(SPI1,&SPI_InitStructure);

	SPI_Cmd(SPI1,ENABLE);

	GPIO_SetBits(GPIOB,GPIO_Pin_7);	//��ֹ����Ƭѡ
	GPIO_SetBits(GPIOA,GPIO_Pin_4);	//��ֹ��̫��Ƭѡ
	GPIO_SetBits(GPIOC,GPIO_Pin_4); //����ʱ��ֹFLASHƬѡ
}
/*----------------------------------------
 ��������: ��FLASH����һ�ֽ�����
 ��������: Ҫ���͵�����dat
 �� �� ֵ: ����������������
----------------------------------------*/
unsigned char SPI_Flash_SendByte(unsigned char dat)
{
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  SPI_I2S_SendData(SPI1, dat);
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
  return SPI_I2S_ReceiveData(SPI1);
}
/*----------------------------------------
 ��������: ��ȡFLASHһ���ֽ�
 ��������: ��
 �� �� ֵ: ������������һ�ֽ�����
----------------------------------------*/
unsigned char SPI_Flash_ReadByte(void)
{
	return(SPI_Flash_SendByte(0));	//��סMOSI����MISO����Ҳ����
}	
/*----------------------------------------
 ��������: ��ȡFLASH�ĳ���ID
 ��������: ��
 �� �� ֵ: ��
----------------------------------------*/
void Read_Flash_ID(void)
{
	CS_1;
	SPI_Flash_SendByte(0x90);  //90H��ABH����
	SPI_Flash_SendByte(0x00);
	SPI_Flash_SendByte(0x00);
	SPI_Flash_SendByte(0x00);  //���������ID,���������ID
	CS_0;
}
/*----------------------------------------
 ��������: ��ȡFLASH��״̬�Ĵ���
 ��������: ��
 �� �� ֵ: ����һ�ֽڼĴ���ֵ
----------------------------------------*/
unsigned char Read_Status_Reg(void)
{
	unsigned char temp;
	CS_1;
    SPI_Flash_SendByte(0x05);
	temp = SPI_Flash_ReadByte();  //����8λ״̬�Ĵ�������
	CS_0;
	return temp;
}
/*----------------------------------------
 ��������: д״̬�Ĵ���
 ��������: ��
 �� �� ֵ: ��
----------------------------------------*/
void Write_Status_Reg(void)
{	
	CS_1;
	SPI_Flash_SendByte(0x50);  //0x50��0x06������
	CS_0; 
	CS_1;
	SPI_Flash_SendByte(0x01);
	SPI_Flash_SendByte(0x00);  //��״̬�Ĵ���д��0x00
	CS_0;
    Busy();
}
/*----------------------------------------
 ��������: ������æ
 ��������: ��
 �� �� ֵ: ��
----------------------------------------*/
void Busy(void)
{
	unsigned char a=1;
	while( (a&0x01)==1) 
	a=Read_Status_Reg();	
}
/*----------------------------------------
 ��������: �������
 ��������:	32λ��ַdat
 �� �� ֵ:	��
----------------------------------------*/
void Clear_Sector(unsigned int dat)
{
	Write_Status_Reg();
	Write_Enable();     
	CS_1;	  
	SPI_Flash_SendByte(0x20);   //4KByte����ָ��
	SPI_Flash_SendByte((dat & 0xFFFFFF)>>16);//ȡ��dat[16..23]λ
	SPI_Flash_SendByte((dat & 0xFFFF)>>8);   //ȡ��dat[8..15]λ 
	SPI_Flash_SendByte( dat & 0xFF);         //ȡ��dat[0..7]λ
	CS_0;
	Busy();
}
/*----------------------------------------
 ��������: FLASHд��ʹ��
 ��������: ��
 �� �� ֵ: ��
----------------------------------------*/
void Write_Enable(void)
{
	CS_1;
	SPI_Flash_SendByte(0x06);  //д��ʹ��ָ��
	CS_0;
}
/*----------------------------------------
 ��������: FLASHҳ��ȡ
 ��������: 32λ��ַaddr
 		   ��ȡ�������ݴ���readbuff��
		   ��ȡ���ֽ���BlockSize
 �� �� ֵ: ��
----------------------------------------*/
void Flash_Page_Read(unsigned int addr, unsigned char *readbuff, unsigned int BlockSize)
{
	unsigned int i=0; 	
	CS_1;
	SPI_Flash_SendByte(0x0B); //���ٶ�ȡģʽ50MHz
	SPI_Flash_SendByte((addr&0xFFFFFF)>>16); //ȡ��dat[16..23]λ
	SPI_Flash_SendByte((addr&0xFFFF)>>8);	 //ȡ��dat[8..15]λ
	SPI_Flash_SendByte (addr&0xFF);			 //ȡ��dat[0..7]λ
	SPI_Flash_SendByte(0);					 //���ŷ���Dummy�ֽ�
	while(i<BlockSize)
	{	
		readbuff[i++]=SPI_Flash_ReadByte();
	}
	CS_0;	 	
}
/*----------------------------------------
 ��������: FLASHҳд��
 ��������: 32λ��ַaddr
 		   ��Ҫд������ݴ�readbuff����
		   д����ֽ���BlockSize
 �� �� ֵ: ��
----------------------------------------*/
void Flash_Page_Write(unsigned int addr,unsigned char *readbuff,unsigned int BlockSize)
{
	unsigned int i=0,a2;	//AAIģʽ
	Clear_Sector(addr);   	//ɾ��ҳ		  
	Write_Status_Reg();
  	Write_Enable();	
	CS_1;    
	SPI_Flash_SendByte(0xAD);
	SPI_Flash_SendByte((addr&0xFFFFFF)>>16);
	SPI_Flash_SendByte((addr&0xFFFF)>>8);
	SPI_Flash_SendByte (addr&0xFF);
  	SPI_Flash_SendByte(readbuff[0]);  //��ַ��������ֽ�����
	SPI_Flash_SendByte(readbuff[1]);
	CS_0;
	i=2;   //ǰ��0,1�ֽ��Ѿ�����,��������2��ʼ������
	while(i<BlockSize)
	{
		for(a2=120;a2>0;a2--);	 //��̵���ʱ
		CS_1;
		SPI_Flash_SendByte(0xAD);			//AD��������������ֽ�����
		SPI_Flash_SendByte(readbuff[i++]);
		SPI_Flash_SendByte(readbuff[i++]);
		CS_0;
	}
	for(a2=100;a2>0;a2--);
	Write_Forbid();		  //д��0x04������AAIģʽ
	CS_1;	
	Busy();
}
/*----------------------------------------
 ��������: д���ֹ
 ��������: ��
 �� �� ֵ: ��
----------------------------------------*/
void Write_Forbid(void)
{
	CS_1;
	SPI_Flash_SendByte(0x04);  //WRDI����
	CS_0;
	Busy();	
}


	