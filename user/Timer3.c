/****************************************************************************
* �ļ�����Timer3.c
* ���ݼ���: 
*			      ��ʱ��3��ʼ������غ�������
*			      ��ʼ������������ԭ��
*			                           1����ʹ�����ʱ�ӣ�2���������� 3��Ҫ���ж��������жϿ�����
*�ļ���ʷ��
*			�汾��	  ��������		����
*			 v0.1	 2012/11/10	   TianHei
*��ϵ��ʽ��Qq:763146170  Email��763146170@qq.com
* ˵    ����
****************************************************************************/


#include "include.h"
#include "Timer3.h"
#include "stm32f10x_tim.h"

/*
 *���� ��ʱ��3�Ļ�����ʼ��
 *���� ��
 *���� ��
*/
static __INLINE void TIM3_BaseInit(void)
{
	TIM_TimeBaseInitTypeDef  TIM3_TimeBaseStructure;

	//Ԥ��Ƶ��TIM3_PSC=72�����ڲ�ʱ�ӵķ�Ƶϵ����
	TIM3_TimeBaseStructure.TIM_Prescaler = 7200;
	//���������ϼ���ģʽ TIM3_CR1[4]=0��������ʽ��	 
	TIM3_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	//�Զ���װ�ؼĴ���TIM3_APR  ��ʱ15MS�ж�һ��
	TIM3_TimeBaseStructure.TIM_Period = 1000;
	//ʱ�ӷ�Ƶ���� TIM3_CR1[9:8]=00�����ⲿ����ʱ�ӽ��еķ�Ƶ���ã� 		     
	TIM3_TimeBaseStructure.TIM_ClockDivision = 0x0;
	//(�ظ�����������������߼���ʱ����ʹ��)
	TIM3_TimeBaseStructure.TIM_RepetitionCounter = 0x0;

	TIM_TimeBaseInit(TIM3,&TIM3_TimeBaseStructure);	//дTIM3���Ĵ�������

	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);//ʹ�ܶ�ʱ���ĸ����¼��ж�
}

/*
 *���� ��ʱ��3�ĳ�ʼ��
 *���� ��
 *���� ��
*/
void TIM3_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);//��ʹ��ʱ��

	TIM3_BaseInit();

	TIM_ClearFlag(TIM3, TIM_IT_Update);//�����жϱ�־���������

	TIM_Cmd(TIM3,ENABLE);//������ʱ��3 TIM3_CR1[0]=1;
}







