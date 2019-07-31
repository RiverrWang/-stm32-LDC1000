#include "ldc1000.h"
/*----------------------------------------
				ȫ�ֱ���
----------------------------------------*/
u16 RegArray[REG_LEN];
/*----------------------------------------
 				�궨��
----------------------------------------*/
#define CSB_0 	GPIOC->BRR  = GPIO_Pin_13
#define CSB_1 	GPIOC->BSRR = GPIO_Pin_13
/*----------------------------------------
 ��������: LDC1000��ʼ��
 ��������: ��
----------------------------------------*/

void LDC1000_Init(void)
{
	LDC1000_GPIO_Init();
	LDC1000_SPI_Init();
	
  LDC1000_WriteReg( LDC1000_CMD_PWRCONFIG,  0x00);	 							//0b  ����ģʽ
  LDC1000_WriteReg(LDC1000_CMD_RPMAX,       0x0c);              //01   38
  LDC1000_WriteReg(LDC1000_CMD_RPMIN,       0x2e);					 		//02    21
	LDC1000_WriteReg(LDC1000_CMD_SENSORFREQ,  0xae);								//03   N=181 =0Xb5
	LDC1000_WriteReg(LDC1000_CMD_LDCCONFIG,   0x17);								//04   ��ֵ 4V  ��Ӧʱ�� 6144
	LDC1000_WriteReg(LDC1000_CMD_CLKCONFIG,   0x00);								//05   
	LDC1000_WriteReg(LDC1000_CMD_INTCONFIG,   0x00);								//0a     
	LDC1000_WriteReg(LDC1000_CMD_THRESHILSB,  0x00);								//06   Ĭ��
	LDC1000_WriteReg(LDC1000_CMD_THRESHIMSB,  0x80);							//07Ĭ��
	LDC1000_WriteReg(LDC1000_CMD_THRESLOLSB,  0xc0);     						//08Ĭ��
	LDC1000_WriteReg(LDC1000_CMD_THRESLOMSB,  0x12);					//09Ĭ��
	LDC1000_WriteReg( LDC1000_CMD_PWRCONFIG,  0x01);	 							//0b	 ��Դģʽ ����
	
}

/*----------------------------------------
 ��������: LDC1000 GPIO��ʼ�� 
 ��������: IO������ͼ

 	+5V 	<-> 	+5V
	DCLK	<-> 	PB6 <-> TIM4_CH1
	SCLK    <-> 	PA5 <-> SPI1_SCK
	SDI     <-> 	PA7 <-> SPI1_MOSI
	SDO     <-> 	PA6 <-> SPI1_MISO
	VIO		<-> 	3V3
	GND		<-> 	GND
	INT     <-> 	PA2 <-> GPIO
	CSB		<-> 	Pc13	<-> GPIO 
	GND		<->		GND
----------------------------------------*/
void LDC1000_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	//��סʹ��AFIOʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB| RCC_APB2Periph_GPIOC , ENABLE);
	
//	//PB6->PWM_OUTPUT
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;				    
//  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;		//PB6�����������
//  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
//  	GPIO_Init(GPIOB, &GPIO_InitStructure);

	//A5->SCK  A6->MISO A7->MOSI
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	   //SPI�ø����������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);

	//PA2->INT �Ĺ�
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);

	//PB7->CSB   //C13
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC,&GPIO_InitStructure);	
	
}
/*----------------------------------------
 ��������: SPI������ʼ��
 ��������: LDC1000��ʱ��Ƶ�ʱ�����4.2MHz
 		   �Ҳ��ܳ���5MHz,���ܵ���4MHz,����
		   ģ�鲻�ܹ�����ͨ��,����SPI������
		   ��Ƶϵ��ѡ��16��Ƶ.
----------------------------------------*/
void LDC1000_SPI_Init(void)
{
	SPI_InitTypeDef SPI_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);		//ʹ��APB2����ʱ��

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;	
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;			//SPI��
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//���ݸ�ʽ8λ
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;				//ʱ�Ӽ��Ե�,�߶�����
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;		    //ʱ����λ��1,2���ش���������
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler	= SPI_BaudRatePrescaler_16; //������16��Ƶ
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;		//MSB��ǰ,����
	SPI_InitStructure.SPI_CRCPolynomial = 7;				//CRC����ʽУ��,���п���
	SPI_Init(SPI1,&SPI_InitStructure);

	SPI_Cmd(SPI1,ENABLE);
}
/*----------------------------------------
 ��������: LDC1000д�Ĵ���
 ��������: - addr �Ĵ�����ַ
 		       - dat  ����
----------------------------------------*/
void LDC1000_WriteReg(int8_t addr, uint8_t dat)
{
	addr = addr & 0x7F;     		//MSBΪ0д�Ĵ���,Ϊ1���Ĵ���
	CSB_0;
	ReadWriteByte(addr);
  ReadWriteByte(dat);
//	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
//	SPI_I2S_SendData(SPI1, addr);
//	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
//	SPI_I2S_SendData(SPI1, dat);
//	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
	CSB_1; 	

}
/*----------------------------------------
 ��������: LDC1000���Ĵ���
 ��������: - addr �Ĵ�����ַ
----------------------------------------*/
u8 SPI_LDC1000_Read(int8_t addr)
{
	u16 Temp = 0;	  
	CSB_0;		
 	ReadWriteByte(addr | 0x80);
	Temp= ReadWriteByte(0xff);
//	SPIx_ReadWriteByte(addr | 0x80);//���Ͷ�ȡID����,          //����ID
//										         //���λΪ0��ʾд�룬���λΪ1��ʾ������
//										         //ʣ��7λΪ�Ĵ����ĵ�ַ,LDC1000_ID | 0x80 ʹ���λ��1��ʾ����	     	 			     
//	Temp = SPIx_ReadWriteByte(0x34);	 	     //���Ĵ����ж������� �浽temp��
	CSB_1;		
	
	return Temp;
	
}


//u8 SPIx_ReadWriteByte(u8 TxData)
//{		
//	u8 retry=0;				 	
//	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) //���ָ����SPI��־λ�������:���ͻ���ձ�־λ
//		{
//		retry++;
//		if(retry>200)return 0;
//		}			  
//	SPI_I2S_SendData(SPI1, TxData); //ͨ������SPIx����һ������
//	retry=0;
//		
//	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) //���ָ����SPI��־λ�������:���ܻ���ǿձ�־λ
//	
//		{
//		retry++;
//		if(retry>200)return 0;
//		}	 
//	
//	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET) //���ָ����SPI��־λ�������:���ܻ���ǿձ�־λ
//		SPI_I2S_SendData(SPI1, TxData);
//	return SPI_I2S_ReceiveData(SPI1); //����ͨ��SPIx������յ�����					    
//}


/*----------------------------------------
 ��������: ��ȡLDC1000 len���ֽ�
 ��������: - addr
 		   - pBuffer
		   - len
����ʵ����	 SPI_LDC1000_ReadBytes(LDC1000_CMD_PROXLSB,&proximtyData[0],2);	       //��ȡ  ����ֵ  ����������  8λһ��
        SPI_LDC1000_ReadBytes(LDC1000_CMD_FREQCTRLSB,&frequencyData[0],3);
----------------------------------------*/
u8 ReadBytes(char ReadAddr,char* pBuffer,u8 NumByteToRead) //main()�����ж�ȡRpʱ���õ��������   
{ 
 	u8 i,readback;
	u8 txReadAddr;
	txReadAddr = ReadAddr | 0x80;                //ȡ���λ  Ϊ0  ����д��    Ϊ1  ��ʾ���� 												    
	CSB_0;                            // ʹ������   
   ReadWriteByte(txReadAddr);              //���Ͷ�ȡ����     
//    SPIx_ReadWriteByte((u8)txReadAddr);   
    for(i=0;i<NumByteToRead;i++)	             //��ʾ  �������Ĵ���   ��Ϊ���� ����16λ  ��о���24λ
	{ 
        pBuffer[i]=ReadWriteByte(0XFF);     //ѭ������  
    }
	CSB_1;                            //ȡ��Ƭѡ 
	return readback;    	      
}  


u8 ReadWriteByte(u8 TxData)
{		
	u8 retry=0;				 	
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) //���ָ����SPI��־λ�������:���ͻ���ձ�־λ
		{
		retry++;
		if(retry>200)return 0;
		}			  
	SPI_I2S_SendData(SPI1, TxData); //ͨ������SPIx����һ������
	retry=0;

	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET) //���ָ����SPI��־λ�������:���ܻ���ǿձ�־λ
		{
		retry++;
		if(retry>200)return 0;
		}	  						    
	return SPI_I2S_ReceiveData(SPI1); //����ͨ��SPIx������յ�����					    
}

