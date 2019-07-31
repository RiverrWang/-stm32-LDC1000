/*-----------------------------------------------------------
			STM32 SPI&SST25VF016B配置驱动程序
 说明: SPI参数配置和SST25V016B FLASH配置驱动程序,写此程序的
       时候花费了功夫不少,调试了半天调试不出来,放了一个星期
	   之后再拿起来看恍然大悟原来是CS片选电平搞反了,一个对调
	   就完全正确了,自此,SPI的配置已经基本熟悉.具体的操作时序
	   和指令请参阅SST25VF016B手册.
 时间: 2013年8月8日      BoX编写
-----------------------------------------------------------*/
#include "stm32f10x.h"
#include "spi.h"

#define CS_1 GPIO_ResetBits(GPIOC,GPIO_Pin_4)
#define CS_0 GPIO_SetBits(GPIOC,GPIO_Pin_4)
/*----------------------------------------
 函数功能: FLASH初始化
 函数参数: 无
 返 回 值: 无
----------------------------------------*/
void SPI_Flash_Init(void)
{
	SPI_Config();
}
/*----------------------------------------
 函数功能: STM32 SPI1参数配置
 函数参数: 无
 返 回 值: 无
----------------------------------------*/
void SPI_Config(void)	  //奋斗板还接了其他SPI外设所以要把无关的关掉
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef SPI_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);	//使能APB2外设时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	//记住使能AFIO时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC , ENABLE);
	//A5->SCK  A6->MISO A7->MOSI
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	  //SPI用复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	//C4->CS  FLASH的片选
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC,&GPIO_InitStructure);
	//B7->CS  触摸的片选
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	//A4->CS 以太网片选
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	//SPI1配置
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;	//2线全双工
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;			//SPI主
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//数据格式8位
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;				//时钟极性高
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;		    //时钟相位第二边沿触发
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler	= SPI_BaudRatePrescaler_8; //8分频
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;		//MSB在前
	SPI_InitStructure.SPI_CRCPolynomial = 7;				//CRC多项式校验
	SPI_Init(SPI1,&SPI_InitStructure);

	SPI_Cmd(SPI1,ENABLE);

	GPIO_SetBits(GPIOB,GPIO_Pin_7);	//禁止触摸片选
	GPIO_SetBits(GPIOA,GPIO_Pin_4);	//禁止以太网片选
	GPIO_SetBits(GPIOC,GPIO_Pin_4); //先暂时禁止FLASH片选
}
/*----------------------------------------
 函数功能: 向FLASH发送一字节数据
 函数参数: 要发送的数据dat
 返 回 值: 器件发回来的数据
----------------------------------------*/
unsigned char SPI_Flash_SendByte(unsigned char dat)
{
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  SPI_I2S_SendData(SPI1, dat);
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
  return SPI_I2S_ReceiveData(SPI1);
}
/*----------------------------------------
 函数功能: 读取FLASH一个字节
 函数参数: 空
 返 回 值: 从器件发来的一字节数据
----------------------------------------*/
unsigned char SPI_Flash_ReadByte(void)
{
	return(SPI_Flash_SendByte(0));	//记住MOSI发送MISO立即也发送
}	
/*----------------------------------------
 函数功能: 获取FLASH的厂商ID
 函数参数: 空
 返 回 值: 空
----------------------------------------*/
void Read_Flash_ID(void)
{
	CS_1;
	SPI_Flash_SendByte(0x90);  //90H或ABH都可
	SPI_Flash_SendByte(0x00);
	SPI_Flash_SendByte(0x00);
	SPI_Flash_SendByte(0x00);  //先输出厂商ID,再输出器件ID
	CS_0;
}
/*----------------------------------------
 函数功能: 读取FLASH的状态寄存器
 函数参数: 空
 返 回 值: 返回一字节寄存器值
----------------------------------------*/
unsigned char Read_Status_Reg(void)
{
	unsigned char temp;
	CS_1;
    SPI_Flash_SendByte(0x05);
	temp = SPI_Flash_ReadByte();  //读出8位状态寄存器数据
	CS_0;
	return temp;
}
/*----------------------------------------
 函数功能: 写状态寄存器
 函数参数: 无
 返 回 值: 无
----------------------------------------*/
void Write_Status_Reg(void)
{	
	CS_1;
	SPI_Flash_SendByte(0x50);  //0x50或0x06都可以
	CS_0; 
	CS_1;
	SPI_Flash_SendByte(0x01);
	SPI_Flash_SendByte(0x00);  //向状态寄存器写入0x00
	CS_0;
    Busy();
}
/*----------------------------------------
 函数功能: 器件判忙
 函数参数: 无
 返 回 值: 无
----------------------------------------*/
void Busy(void)
{
	unsigned char a=1;
	while( (a&0x01)==1) 
	a=Read_Status_Reg();	
}
/*----------------------------------------
 函数功能: 清除扇区
 函数参数:	32位地址dat
 返 回 值:	无
----------------------------------------*/
void Clear_Sector(unsigned int dat)
{
	Write_Status_Reg();
	Write_Enable();     
	CS_1;	  
	SPI_Flash_SendByte(0x20);   //4KByte擦除指令
	SPI_Flash_SendByte((dat & 0xFFFFFF)>>16);//取出dat[16..23]位
	SPI_Flash_SendByte((dat & 0xFFFF)>>8);   //取出dat[8..15]位 
	SPI_Flash_SendByte( dat & 0xFF);         //取出dat[0..7]位
	CS_0;
	Busy();
}
/*----------------------------------------
 函数功能: FLASH写入使能
 函数参数: 无
 返 回 值: 无
----------------------------------------*/
void Write_Enable(void)
{
	CS_1;
	SPI_Flash_SendByte(0x06);  //写入使能指令
	CS_0;
}
/*----------------------------------------
 函数功能: FLASH页读取
 函数参数: 32位地址addr
 		   读取出的数据存入readbuff中
		   读取的字节数BlockSize
 返 回 值: 无
----------------------------------------*/
void Flash_Page_Read(unsigned int addr, unsigned char *readbuff, unsigned int BlockSize)
{
	unsigned int i=0; 	
	CS_1;
	SPI_Flash_SendByte(0x0B); //高速读取模式50MHz
	SPI_Flash_SendByte((addr&0xFFFFFF)>>16); //取出dat[16..23]位
	SPI_Flash_SendByte((addr&0xFFFF)>>8);	 //取出dat[8..15]位
	SPI_Flash_SendByte (addr&0xFF);			 //取出dat[0..7]位
	SPI_Flash_SendByte(0);					 //接着发送Dummy字节
	while(i<BlockSize)
	{	
		readbuff[i++]=SPI_Flash_ReadByte();
	}
	CS_0;	 	
}
/*----------------------------------------
 函数功能: FLASH页写入
 函数参数: 32位地址addr
 		   需要写入的数据从readbuff读出
		   写入的字节数BlockSize
 返 回 值: 无
----------------------------------------*/
void Flash_Page_Write(unsigned int addr,unsigned char *readbuff,unsigned int BlockSize)
{
	unsigned int i=0,a2;	//AAI模式
	Clear_Sector(addr);   	//删除页		  
	Write_Status_Reg();
  	Write_Enable();	
	CS_1;    
	SPI_Flash_SendByte(0xAD);
	SPI_Flash_SendByte((addr&0xFFFFFF)>>16);
	SPI_Flash_SendByte((addr&0xFFFF)>>8);
	SPI_Flash_SendByte (addr&0xFF);
  	SPI_Flash_SendByte(readbuff[0]);  //地址后面跟两字节数据
	SPI_Flash_SendByte(readbuff[1]);
	CS_0;
	i=2;   //前面0,1字节已经发送,接下来从2开始往后发送
	while(i<BlockSize)
	{
		for(a2=120;a2>0;a2--);	 //简短的延时
		CS_1;
		SPI_Flash_SendByte(0xAD);			//AD后跟接下来的两字节数据
		SPI_Flash_SendByte(readbuff[i++]);
		SPI_Flash_SendByte(readbuff[i++]);
		CS_0;
	}
	for(a2=100;a2>0;a2--);
	Write_Forbid();		  //写入0x04来结束AAI模式
	CS_1;	
	Busy();
}
/*----------------------------------------
 函数功能: 写入禁止
 函数参数: 无
 返 回 值: 无
----------------------------------------*/
void Write_Forbid(void)
{
	CS_1;
	SPI_Flash_SendByte(0x04);  //WRDI命令
	CS_0;
	Busy();	
}


	