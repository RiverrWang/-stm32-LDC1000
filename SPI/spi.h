#ifndef __SPI_H
#define __SPI_H

void SPI_Config(void);
unsigned char SPI_Flash_SendByte(unsigned char dat);
unsigned char SPI_Flash_ReadByte(void);
void Read_Flash_ID(void);
unsigned char Read_Status_Reg(void);
void Write_Status_Reg(void);
void Busy(void);
void Clear_Sector(unsigned int dat);
void Write_Enable(void);
void Flash_Page_Read(unsigned int addr, unsigned char *readbuff, unsigned int BlockSize);
void Flash_Page_Write(unsigned int addr,unsigned char *readbuff,unsigned int BlockSize);
void Write_Forbid(void);
void SPI_Flash_Init(void);

#endif
