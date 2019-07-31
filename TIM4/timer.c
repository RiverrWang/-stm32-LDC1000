#include "timer.h"
#include "oled.h"
#include "usart1.h"
#include "ldc1000.h"
#include "bsp_GeneralTim.h" 
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK Mini STM32������
//PWM  ��������			   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2010/12/03
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ����ԭ�� 2009-2019
//All rights reserved
////////////////////////////////////////////////////////////////////////////////// 	  

//ͨ�ö�ʱ���жϳ�ʼ��
//����ʱ��ѡ��ΪAPB1��2������APB1Ϊ36M
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//����ʹ�õ��Ƕ�ʱ��3!

extern char proximtyData[2];
extern int val_code;
extern int a[25];  //����ֵ��ƽ��ֵ
extern int pj_value;	//�����ķ���ֵֵ
extern int proximtyDataMAX;
extern int proximtyDataMIN;
extern unsigned long x,ProximityData,proximtyDataSUM,proximtyDataAVE,proximtyDataAVE_LAS,ProximityDataMAX;
extern int b;
extern float  distance;
extern int dingshi,miao,fen,biaoji;


int ldc_read_avr(void)   //��ֵ�˲�
{   
    char rpi=0;  //ȡrpi��ƽ��ֵ 
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
int Position_PID1 (int Encoder,int Target)           // ֱ����
{ 	
	 float Position_KP=0.008,Position_KI=0,Position_KD=0;
	 float Bias,Pwm,Integral_bias,Last_Bias;
	 Bias=Encoder-Target;                                  //����ƫ��
	 Integral_bias+=Bias;	                                 //���ƫ��Ļ���
	
	if(Integral_bias>10000)
	{
	Integral_bias=10000;
	}
	
	if(Integral_bias<-10000)
	{
	Integral_bias=-10000;
	}
	 Pwm=Position_KP*Bias+Position_KI*Integral_bias+Position_KD*(Bias-Last_Bias);       //λ��ʽPID������
	 if(Pwm<-z)
	 {
	  Pwm=-z;
	 }
	 if(Pwm>z)
	 {
	  Pwm=z;
	 }
	 Last_Bias=Bias;
	 
	 return Pwm;                                           //�������
}



//��ʱ��5ͨ��1���벶������

TIM_ICInitTypeDef  TIM4_ICInitStructure;

void TIM4_Cap_Init(u16 arr,u16 psc)
{	 
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
   	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);	//ʹ��TIM5ʱ��
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);  //ʹ��GPIOAʱ��
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_6;  //PA0 ���֮ǰ����  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //PA0 ����  
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOB,GPIO_Pin_6);						 //PA0 ����
	
	//��ʼ����ʱ��5 TIM5	 
	TIM_TimeBaseStructure.TIM_Period = arr; //�趨�������Զ���װֵ 
	TIM_TimeBaseStructure.TIM_Prescaler =psc; 	//Ԥ��Ƶ��   
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
  
	//��ʼ��TIM5���벶�����
	TIM4_ICInitStructure.TIM_Channel = TIM_Channel_1; //CC1S=01 	ѡ������� IC1ӳ�䵽TI1��
  	TIM4_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//�����ز���
  	TIM4_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //ӳ�䵽TI1��
  	TIM4_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //���������Ƶ,����Ƶ 
  	TIM4_ICInitStructure.TIM_ICFilter = 0x00;//IC1F=0000 ���������˲��� ���˲�
  	TIM_ICInit(TIM4, &TIM4_ICInitStructure);
	
	//�жϷ����ʼ��
	 NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;  //TIM3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  //��ռ���ȼ�2��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;  //�����ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);  //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ��� 
	
	TIM_ITConfig(TIM4,TIM_IT_Update|TIM_IT_CC1,ENABLE);//��������ж� ,����CC1IE�����ж�	
	
   	TIM_Cmd(TIM4,ENABLE ); 	//ʹ�ܶ�ʱ��5
   


}
int x1,x2,x3;

extern u16 RegArray[REG_LEN];
extern int i,val_code;
u8  TIM4CH1_CAPTURE_STA=0;	//���벶��״̬		    				
u16	TIM4CH1_CAPTURE_VAL;	//���벶��ֵ
extern float  distance;
//��ʱ��5�жϷ������	 
extern int dingshi,miao,fen;
void TIM4_IRQHandler(void)
{ 
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
		
	   val_code = ldc_read_avr();
	  
			x1=Position_PID1(val_code,12300);
			x2=20+x1;
	    x3=20-x1;

	GENERAL_TIM_Mode_Config(x2 ,x3);
	
	    TIM_ClearITPendingBit(TIM4, TIM_IT_CC1|TIM_IT_Update); //����жϱ�־λ
	
	
//	dingshi++;       //�ӱ�������ʼ������ʼ��ʱ
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
// 	if((TIM4CH1_CAPTURE_STA&0X80)==0)//��δ�ɹ�����	
//	{	  
//		if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
//		 
//		{	    
//			if(TIM4CH1_CAPTURE_STA&0X40)//�Ѿ����񵽸ߵ�ƽ��
//			{
//				if((TIM4CH1_CAPTURE_STA&0X3F)==0X3F)//�ߵ�ƽ̫����
//				{
//					TIM4CH1_CAPTURE_STA|=0X80;//��ǳɹ�������һ��
//					
//					TIM4CH1_CAPTURE_VAL=0XFFFF;
//				}else TIM4CH1_CAPTURE_STA++;
//			}	 
//		}
//	if (TIM_GetITStatus(TIM4, TIM_IT_CC1) != RESET)//����1���������¼�
//		{	
//			if(TIM4CH1_CAPTURE_STA&0X40)		//����һ���½��� 		
//			{	  			
//				TIM4CH1_CAPTURE_STA|=0X80;		//��ǳɹ�����һ��������
//				TIM4CH1_CAPTURE_VAL=TIM_GetCapture1(TIM4);
//		   		TIM_OC1PolarityConfig(TIM4,TIM_ICPolarity_Rising); //CC1P=0 ����Ϊ�����ز���
//			}else  								//��δ��ʼ,��һ�β���������
//			{
//				TIM4CH1_CAPTURE_STA=0;			//���
//				TIM4CH1_CAPTURE_VAL=0;
//	 			TIM_SetCounter(TIM4,0);
//				TIM4CH1_CAPTURE_STA|=0X40;		//��ǲ�����������
//		   		TIM_OC1PolarityConfig(TIM4,TIM_ICPolarity_Falling);		//CC1P=1 ����Ϊ�½��ز���
//			}		    
//		}			     	    					   
// 	}
// 
//    TIM_ClearITPendingBit(TIM4, TIM_IT_CC1|TIM_IT_Update); //����жϱ�־λ
// 		if(TIM4CH1_CAPTURE_STA&0X80)//�ɹ�������һ��������
//		{ i++;
//			
//			printf("%d",i);
//			distance=(float)i/12*22;

//			printf("destance:%2fcm",distance);
//			TIM4CH1_CAPTURE_STA=0;//������һ�β���

			
}

