/*******************************************************************************
* File Name          : main
* Author             : luohui
* Date               : 21-08-2014
* E-mail             : luohui2320@gmail.com
* Description        : 	
											利用STM32的USB库实现 USB_keyboard功能
********************************************************************************/


/* Includes ------------------------------------------------------------------*/
#include "include.h"
#include "hw_config.h"
#include "usb_lib.h"
#include "usb_pwr.h"
#include <math.h>
#include <stdio.h>
#include "USART.h"
#include "rotary_encoder.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
__IO uint8_t PrevXferComplete = 1;
INT8U key_flag = 0x41;
INT8U flag = 0;

extern INT8U rotary_detect_flag;
extern INT8S rotary_dir[4][4];
//extern INT8U rotary_dir_flag[3];
extern INT8S rotary_hdir_flag[32];
extern INT8S rotary_zdir_flag[32];
extern INT8S rotary_vdir_flag[32];

extern struct rotary_data rdata;


INT8U Send_Buffer[8] = {0, 0, 0, 0, 0, 0, 0, 0};


/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
void System_Init(void);
void Key_Handler(void);

/*******************************************************************************
* Function Name  : main.
* Description    : main routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
int main(void)
{
  Set_System();
	
  System_Init();
  
  USB_Interrupts_Config();
  
  Set_USBClock();
  
  USB_Init();
	
  while (1)
  {
    if (bDeviceState == CONFIGURED)
    {
      if(PrevXferComplete && (flag | (get_read_avalible() > 0)))
      {
				
				Key_Handler();
				
				flag = 0;
				rotary_detect_flag = 0;
				
				/* Reset the control token to inform upper layer that a transfer is ongoing */
				PrevXferComplete = 0;
  
				/* Copy mouse position info in ENDP1 Tx Packet Memory Area*/
				USB_SIL_Write(EP1_IN, Send_Buffer, 8);
  
				/* Enable endpoint for transmission */
				SetEPTxValid(ENDP1);
      }
    }
  }
}

/*
 *简述 初始化USB后的系统初始化
 *参数 无
 *返回 无
*/
void System_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	 /*按键GPIO配置*/
	/*使能端口A的时钟*/
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA
							,ENABLE);
	/* Configure rotary pins*/
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 ;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	/*设置为带上拉输入*/
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /*按键GPIO配置*/
	/*使能端口B的时钟*/
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB
							,ENABLE);
	/* Configure KEY pins*/
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0 | GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	/*设置为带上拉输入*/
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

	/*I2C引脚初始化*/
	//I2C_GPIOInit();

	
	/*定时器的初始化*/
	TIM3_Init();
}
/*
 *简述 按键值处理
 *参数 无
 *返回 无
*/
void Key_Handler(void)
{
	INT8U ridx;

	if (flag)
	{
		if((key_flag & 0x40) == 0x00) //检测Keyboard N是否按下
			Send_Buffer[2] = 0x11;
		else
			Send_Buffer[2] = 0x00; 
		
		if((key_flag & 0x01) == 0x00) //检测Keyboard P是否按下
			Send_Buffer[3] = 0x13;
		else
			Send_Buffer[3] = 0x00;
	}

	if (rotary_detect_flag)
	{
		if (get_read_avalible() < 1)
			return;
		ridx = get_current_readidx();
		/* keyboard LeftArrow and RightArrow */
		if (rotary_hdir_flag[ridx] == ROTARY_DIR_F)
			Send_Buffer[4] = 0x50;
		else if (rotary_hdir_flag[ridx] == ROTARY_DIR_B)
			Send_Buffer[4] = 0x4f;
		else
			Send_Buffer[4] = 0x00;

		/* keyboard PgDn and PgUp */
		if (rotary_zdir_flag[ridx] == ROTARY_DIR_F)
			Send_Buffer[5] = 0x4E;
		else if (rotary_zdir_flag[ridx] == ROTARY_DIR_B)
			Send_Buffer[5] = 0x4B;
		else
			Send_Buffer[5] = 0x00;

		/* keyboard Down Arrow  and Up Arrow*/
		if (rotary_vdir_flag[ridx] == ROTARY_DIR_F)
			Send_Buffer[6] = 0x51;
		else if (rotary_vdir_flag[ridx] == ROTARY_DIR_B)
			Send_Buffer[6] = 0x52;
		else
			Send_Buffer[6] = 0x00;

		rotary_hdir_flag[ridx] = ROTARY_DIR_NONE;
		rotary_zdir_flag[ridx] = ROTARY_DIR_NONE;
		rotary_vdir_flag[ridx] = ROTARY_DIR_NONE;

		inc_read_idx();
	}

}

#ifdef  USE_FULL_ASSERT
/*******************************************************************************
* Function Name  : assert_failed
* Description    : Reports the name of the source file and the source line number
*                  where the assert_param error has occurred.
* Input          : - file: pointer to the source file name
*                  - line: assert_param error line source number
* Output         : None
* Return         : None
*******************************************************************************/
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {}
}
#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
