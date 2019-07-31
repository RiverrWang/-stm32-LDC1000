#ifndef __TIMER_H
#define __TIMER_H
#include "sys.h"

void TIM3_Int_Init(u16 arr,u16 psc);
void TIM3_PWM_Init(u16 arr,u16 psc);
void TIM4_Cap_Init(u16 arr,u16 psc);
#define ADVANCE_IC1PWM_CHANNEL        TIM_Channel_1
#define ADVANCE_TIM           		    TIM4
#define ADVANCE_IC2PWM_CHANNEL        TIM_Channel_2
#endif
