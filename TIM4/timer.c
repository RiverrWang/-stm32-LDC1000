#include "timer.h"
#include "oled.h"
#include "usart1.h"
#include "ldc1000.h"
#include "bsp_GeneralTim.h" 
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK Mini STM32开发板
//PWM  驱动代码			   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2010/12/03
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 正点原子 2009-2019
//All rights reserved
////////////////////////////////////////////////////////////////////////////////// 	  

//通用定时器中断初始化
//这里时钟选择为APB1的2倍，而APB1为36M
//arr：自动重装值。
//psc：时钟预分频数
//这里使用的是定时器3!

extern char proximtyData[2];
extern int val_code;
extern int a[25];  //储存值做平均值
extern int pj_value;	//处理后的返回值值
extern int proximtyDataMAX;
extern int proximtyDataMIN;
extern unsigned long x,ProximityData,proximtyDataSUM,proximtyDataAVE,proximtyDataAVE_LAS,ProximityDataMAX;
extern int b;
extern float  distance;
extern int dingshi,miao,fen,biaoji;


int ldc_read_avr(void)   //均值滤波
{   
    char rpi=0;  //取rpi次平均值 
	 proximtyDataAVE_LAS=0;
    x = 0;  
    for (rpi=0;rpi<5;rpi++)
    {
    proximtyData[0] = SPI_LDC1000_Read(LDC1000_CMD_PROXLSB);
		x|= proximtyData[0] ; 
		proximtyData[1] = SPI_LDC1000_Read(LDC1000_CMD_PROXMSB); 
		x|= (proximtyData[1]<<8);
	
		proximtyDataSUM += x;
		
    }
    proximtyDataAVE = proximtyDataSUM/5;
     proximtyDataSUM=0;
     proximtyDataAVE_LAS=proximtyDataAVE;

    return  proximtyDataAVE; 

}
int z=12;
int Position_PID1 (int Encoder,int Target)           // 直立环
{ 	
	 float Position_KP=0.008,Position_KI=0,Position_KD=0;
	 float Bias,Pwm,Integral_bias,Last_Bias;
	 Bias=Encoder-Target;                                  //计算偏差
	 Integral_bias+=Bias;	                                 //求出偏差的积分
	
	if(Integral_bias>10000)
	{
	Integral_bias=10000;
	}
	
	if(Integral_bias<-10000)
	{
	Integral_bias=-10000;
	}
	 Pwm=Position_KP*Bias+Position_KI*Integral_bias+Position_KD*(Bias-Last_Bias);       //位置式PID控制器
	 if(Pwm<-z)
	 {
	  Pwm=-z;
	 }
	 if(Pwm>z)
	 {
	  Pwm=z;
	 }
	 Last_Bias=Bias;
	 
	 return Pwm;                                           //增量输出
}



//定时器5通道1输入捕获配置

TIM_ICInitTypeDef  TIM4_ICInitStructure;

void TIM4_Cap_Init(u16 arr,u16 psc)
{	 
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
   	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);	//使能TIM5时钟
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);  //使能GPIOA时钟
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_6;  //PA0 清除之前设置  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //PA0 输入  
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOB,GPIO_Pin_6);						 //PA0 下拉
	
	//初始化定时器5 TIM5	 
	TIM_TimeBaseStructure.TIM_Period = arr; //设定计数器自动重装值 
	TIM_TimeBaseStructure.TIM_Prescaler =psc; 	//预分频器   
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
  
	//初始化TIM5输入捕获参数
	TIM4_ICInitStructure.TIM_Channel = TIM_Channel_1; //CC1S=01 	选择输入端 IC1映射到TI1上
  	TIM4_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//上升沿捕获
  	TIM4_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //映射到TI1上
  	TIM4_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //配置输入分频,不分频 
  	TIM4_ICInitStructure.TIM_ICFilter = 0x00;//IC1F=0000 配置输入滤波器 不滤波
  	TIM_ICInit(TIM4, &TIM4_ICInitStructure);
	
	//中断分组初始化
	 NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;  //TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  //先占优先级2级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;  //从优先级0级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器 
	
	TIM_ITConfig(TIM4,TIM_IT_Update|TIM_IT_CC1,ENABLE);//允许更新中断 ,允许CC1IE捕获中断	
	
   	TIM_Cmd(TIM4,ENABLE ); 	//使能定时器5
   


}
int x1,x2,x3;

extern u16 RegArray[REG_LEN];
extern int i,val_code;
u8  TIM4CH1_CAPTURE_STA=0;	//输入捕获状态		    				
u16	TIM4CH1_CAPTURE_VAL;	//输入捕获值
extern float  distance;
//定时器5中断服务程序	 
extern int dingshi,miao,fen;
void TIM4_IRQHandler(void)
{ 
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
		
	   val_code = ldc_read_avr();
	  
			x1=Position_PID1(val_code,12300);
			x2=20+x1;
	    x3=20-x1;

	GENERAL_TIM_Mode_Config(x2 ,x3);
	
	    TIM_ClearITPendingBit(TIM4, TIM_IT_CC1|TIM_IT_Update); //清除中断标志位
	
	
//	dingshi++;       //从编码器开始计数开始计时
//	 if(dingshi>19)
//	 {
//		 dingshi=0;
//	 }
//	while(dingshi==19&&i!=0)
//	{
//		miao=miao+1;
//		//printf("Time:%d",miao);
//		dingshi=0;
//		if(miao==60)
//		{
//			fen++;
//			miao=0;
//		}
//		printf("Time:%2d:%2d",fen,miao);
//	}
//   
// 	if((TIM4CH1_CAPTURE_STA&0X80)==0)//还未成功捕获	
//	{	  
//		if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
//		 
//		{	    
//			if(TIM4CH1_CAPTURE_STA&0X40)//已经捕获到高电平了
//			{
//				if((TIM4CH1_CAPTURE_STA&0X3F)==0X3F)//高电平太长了
//				{
//					TIM4CH1_CAPTURE_STA|=0X80;//标记成功捕获了一次
//					
//					TIM4CH1_CAPTURE_VAL=0XFFFF;
//				}else TIM4CH1_CAPTURE_STA++;
//			}	 
//		}
//	if (TIM_GetITStatus(TIM4, TIM_IT_CC1) != RESET)//捕获1发生捕获事件
//		{	
//			if(TIM4CH1_CAPTURE_STA&0X40)		//捕获到一个下降沿 		
//			{	  			
//				TIM4CH1_CAPTURE_STA|=0X80;		//标记成功捕获到一次上升沿
//				TIM4CH1_CAPTURE_VAL=TIM_GetCapture1(TIM4);
//		   		TIM_OC1PolarityConfig(TIM4,TIM_ICPolarity_Rising); //CC1P=0 设置为上升沿捕获
//			}else  								//还未开始,第一次捕获上升沿
//			{
//				TIM4CH1_CAPTURE_STA=0;			//清空
//				TIM4CH1_CAPTURE_VAL=0;
//	 			TIM_SetCounter(TIM4,0);
//				TIM4CH1_CAPTURE_STA|=0X40;		//标记捕获到了上升沿
//		   		TIM_OC1PolarityConfig(TIM4,TIM_ICPolarity_Falling);		//CC1P=1 设置为下降沿捕获
//			}		    
//		}			     	    					   
// 	}
// 
//    TIM_ClearITPendingBit(TIM4, TIM_IT_CC1|TIM_IT_Update); //清除中断标志位
// 		if(TIM4CH1_CAPTURE_STA&0X80)//成功捕获到了一次上升沿
//		{ i++;
//			
//			printf("%d",i);
//			distance=(float)i/12*22;

//			printf("destance:%2fcm",distance);
//			TIM4CH1_CAPTURE_STA=0;//开启下一次捕获

			
}

