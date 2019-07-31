#include "ldc1000.h"
/*----------------------------------------
				全局变量
----------------------------------------*/
u16 RegArray[REG_LEN];
/*----------------------------------------
 				宏定义
----------------------------------------*/
#define CSB_0 	GPIOC->BRR  = GPIO_Pin_13
#define CSB_1 	GPIOC->BSRR = GPIO_Pin_13
/*----------------------------------------
 函数功能: LDC1000初始化
 函数参数: 无
----------------------------------------*/

void LDC1000_Init(void)
{
	LDC1000_GPIO_Init();
	LDC1000_SPI_Init();
	
  LDC1000_WriteReg( LDC1000_CMD_PWRCONFIG,  0x00);	 							//0b  待机模式
  LDC1000_WriteReg(LDC1000_CMD_RPMAX,       0x0c);              //01   38
  LDC1000_WriteReg(LDC1000_CMD_RPMIN,       0x2e);					 		//02    21
	LDC1000_WriteReg(LDC1000_CMD_SENSORFREQ,  0xae);								//03   N=181 =0Xb5
	LDC1000_WriteReg(LDC1000_CMD_LDCCONFIG,   0x17);								//04   幅值 4V  响应时间 6144
	LDC1000_WriteReg(LDC1000_CMD_CLKCONFIG,   0x00);								//05   
	LDC1000_WriteReg(LDC1000_CMD_INTCONFIG,   0x00);								//0a     
	LDC1000_WriteReg(LDC1000_CMD_THRESHILSB,  0x00);								//06   默认
	LDC1000_WriteReg(LDC1000_CMD_THRESHIMSB,  0x80);							//07默认
	LDC1000_WriteReg(LDC1000_CMD_THRESLOLSB,  0xc0);     						//08默认
	LDC1000_WriteReg(LDC1000_CMD_THRESLOMSB,  0x12);					//09默认
	LDC1000_WriteReg( LDC1000_CMD_PWRCONFIG,  0x01);	 							//0b	 电源模式 开启
	
}

/*----------------------------------------
 函数功能: LDC1000 GPIO初始化 
 函数参数: IO口连接图

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

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	//记住使能AFIO时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB| RCC_APB2Periph_GPIOC , ENABLE);
	
//	//PB6->PWM_OUTPUT
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;				    
//  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;		//PB6复用推挽输出
//  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
//  	GPIO_Init(GPIOB, &GPIO_InitStructure);

	//A5->SCK  A6->MISO A7->MOSI
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	   //SPI用复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);

	//PA2->INT 改过
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
 函数功能: SPI参数初始化
 函数参数: LDC1000的时钟频率必须在4.2MHz
 		   且不能超过5MHz,不能低于4MHz,否则
		   模块不能够正常通信,所以SPI波特率
		   分频系数选择16分频.
----------------------------------------*/
void LDC1000_SPI_Init(void)
{
	SPI_InitTypeDef SPI_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);		//使能APB2外设时钟

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;	
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;			//SPI主
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//数据格式8位
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;				//时钟极性低,高都可以
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;		    //时钟相位第1,2边沿触发都可以
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler	= SPI_BaudRatePrescaler_16; //必须是16分频
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;		//MSB在前,必须
	SPI_InitStructure.SPI_CRCPolynomial = 7;				//CRC多项式校验,可有可无
	SPI_Init(SPI1,&SPI_InitStructure);

	SPI_Cmd(SPI1,ENABLE);
}
/*----------------------------------------
 函数功能: LDC1000写寄存器
 函数参数: - addr 寄存器地址
 		       - dat  数据
----------------------------------------*/
void LDC1000_WriteReg(int8_t addr, uint8_t dat)
{
	addr = addr & 0x7F;     		//MSB为0写寄存器,为1读寄存器
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
 函数功能: LDC1000读寄存器
 函数参数: - addr 寄存器地址
----------------------------------------*/
u8 SPI_LDC1000_Read(int8_t addr)
{
	u16 Temp = 0;	  
	CSB_0;		
 	ReadWriteByte(addr | 0x80);
	Temp= ReadWriteByte(0xff);
//	SPIx_ReadWriteByte(addr | 0x80);//发送读取ID命令,          //读出ID
//										         //最高位为0表示写入，最高位为1表示读出，
//										         //剩余7位为寄存器的地址,LDC1000_ID | 0x80 使最高位是1表示读出	     	 			     
//	Temp = SPIx_ReadWriteByte(0x34);	 	     //将寄存器中读出的数 存到temp中
	CSB_1;		
	
	return Temp;
	
}


//u8 SPIx_ReadWriteByte(u8 TxData)
//{		
//	u8 retry=0;				 	
//	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) //检查指定的SPI标志位设置与否:发送缓存空标志位
//		{
//		retry++;
//		if(retry>200)return 0;
//		}			  
//	SPI_I2S_SendData(SPI1, TxData); //通过外设SPIx发送一个数据
//	retry=0;
//		
//	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) //检查指定的SPI标志位设置与否:接受缓存非空标志位
//	
//		{
//		retry++;
//		if(retry>200)return 0;
//		}	 
//	
//	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET) //检查指定的SPI标志位设置与否:接受缓存非空标志位
//		SPI_I2S_SendData(SPI1, TxData);
//	return SPI_I2S_ReceiveData(SPI1); //返回通过SPIx最近接收的数据					    
//}


/*----------------------------------------
 函数功能: 读取LDC1000 len个字节
 函数参数: - addr
 		   - pBuffer
		   - len
调用实例：	 SPI_LDC1000_ReadBytes(LDC1000_CMD_PROXLSB,&proximtyData[0],2);	       //读取  距离值  存入数组中  8位一数
        SPI_LDC1000_ReadBytes(LDC1000_CMD_FREQCTRLSB,&frequencyData[0],3);
----------------------------------------*/
u8 ReadBytes(char ReadAddr,char* pBuffer,u8 NumByteToRead) //main()函数中读取Rp时会用到这个函数   
{ 
 	u8 i,readback;
	u8 txReadAddr;
	txReadAddr = ReadAddr | 0x80;                //取最高位  为0  就是写入    为1  表示读出 												    
	CSB_0;                            // 使能器件   
   ReadWriteByte(txReadAddr);              //发送读取命令     
//    SPIx_ReadWriteByte((u8)txReadAddr);   
    for(i=0;i<NumByteToRead;i++)	             //表示  读几个寄存器   若为距离 就是16位  电感就是24位
	{ 
        pBuffer[i]=ReadWriteByte(0XFF);     //循环读数  
    }
	CSB_1;                            //取消片选 
	return readback;    	      
}  


u8 ReadWriteByte(u8 TxData)
{		
	u8 retry=0;				 	
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) //检查指定的SPI标志位设置与否:发送缓存空标志位
		{
		retry++;
		if(retry>200)return 0;
		}			  
	SPI_I2S_SendData(SPI1, TxData); //通过外设SPIx发送一个数据
	retry=0;

	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET) //检查指定的SPI标志位设置与否:接受缓存非空标志位
		{
		retry++;
		if(retry>200)return 0;
		}	  						    
	return SPI_I2S_ReceiveData(SPI1); //返回通过SPIx最近接收的数据					    
}

