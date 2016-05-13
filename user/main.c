/*******************************************************************************
* File Name          : main
* Author             : luohui
* Date               : 21-08-2014
* E-mail             : luohui2320@gmail.com
* Description        : 	
											利用STM32的USB库实现 USB_keyboard功能
********************************************************************************/
#include "include.h"
#include "hw_config.h"
#include "usb_lib.h"
#include "usb_pwr.h"
#include <stdio.h>
#include <stdlib.h>

#define FLASH_RANDOM_POOL 0x0801FC00
#define FLASH_SIGN 0x01010101
#define PASSWORD_LEN 25

__IO uint8_t PrevXferComplete = 1;
INT8U Send_Buffer[8] = {0, 0, 0, 0, 0, 0, 0, 0};

unsigned char password_base[PASSWORD_LEN];
unsigned char write_buf[1024];
unsigned int  password_mask = 0x00;
unsigned int  not_enter = 1;

volatile int btn = 0;

void 				System_Init(void);
int 				flash_init(void);
uint32_t 		flash_read(uint32_t address);
void 				flash_erase(unsigned int pageAddress);
void 				flash_write(unsigned char* data, unsigned int address, unsigned int count);
void        send_key(int mod, int key);
void 				send_array(unsigned char *array, unsigned int size);
void 				get_password(int num);

void TIM3_IRQHandler(void) {
	GPIOC->ODR ^= GPIO_Pin_13;
	GPIOA->BSRR = GPIO_Pin_4;
	TIM_ClearFlag(TIM3, TIM_FLAG_Update);
}

int main(void) {
	int i;
	
	FLASH->KEYR = 0x45670123;
	FLASH->KEYR = 0xCDEF89AB;
	
  Set_System();
	System_Init();
  USB_Interrupts_Config();
  Set_USBClock();
  USB_Init();
	SysTick_Config(0xFFF);
	flash_init();
	
	if ((GPIOA->IDR & GPIO_Pin_2)  == 0) {
		GPIOA->BRR = GPIO_Pin_4;
		not_enter = 0;
	  while ((GPIOA->IDR & GPIO_Pin_2)  == 0) __NOP();	
	}
	
	TIM3_Init();
	
  while (1)
  {
    if (bDeviceState == CONFIGURED) {
			if (btn == 0) {
				if ((GPIOA->IDR & GPIO_Pin_2)  == 0) btn = 1;
				if ((GPIOA->IDR & GPIO_Pin_6)  == 0) btn = 2;
				if ((GPIOA->IDR & GPIO_Pin_8)  == 0) btn = 3;
				//if ((GPIOA->IDR & GPIO_Pin_12) == 0) btn = 4;
				if ((GPIOB->IDR & GPIO_Pin_5)  == 0) btn = 5;
				if ((GPIOB->IDR & GPIO_Pin_10) == 0) btn = 6;
				if (btn != 0) {
					TIM3->CNT = 0;
					GPIOA->BRR = GPIO_Pin_4;
					
					get_password(btn);
					send_array(password_base, PASSWORD_LEN);
					
//					switch (btn) {
//						case 1:
//							get_password(0);
//							send_array(password_base, PASSWORD_LEN);
//							break;
//						case 2:
//							get_password(0);
//							send_array(password_base, PASSWORD_LEN);
//							break;
//						case 3:
//							
//							break;
//						case 4:
//							
//							break;
//						case 5:
//							
//							break;
//						case 6:
//							
//							break;
//					}
					for (i=0; i<5000000; i++);
					btn = 0;
				}
			}
    }
  }
}

void System_Init(void) {
	GPIO_InitTypeDef xIn;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA, ENABLE);

  xIn.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_6 | GPIO_Pin_8;
  xIn.GPIO_Speed = GPIO_Speed_10MHz;
  xIn.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(GPIOA, &xIn);
	xIn.GPIO_Mode = GPIO_Mode_IPU;
  xIn.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_10;
  GPIO_Init(GPIOB, &xIn);
	
  xIn.GPIO_Pin =  GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_7 | GPIO_Pin_9 | GPIO_Pin_15;
  xIn.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA, &xIn);
	GPIOA->BRR = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_7 | GPIO_Pin_9 | GPIO_Pin_15; 
	
  xIn.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_6 | GPIO_Pin_11;
	xIn.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &xIn);
	GPIOB->BRR = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_6 | GPIO_Pin_11;
	
	xIn.GPIO_Pin = GPIO_Pin_13;
	xIn.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOC, &xIn);
}

void send_key(int mod, int key) {
	PrevXferComplete = 0;
	Send_Buffer[0] = mod;
	Send_Buffer[2] = key;
	USB_SIL_Write(EP1_IN, Send_Buffer, 8);
	SetEPTxValid(ENDP1);
	while (PrevXferComplete==0) __NOP();
}

void send_array(unsigned char *array, unsigned int size) {
	int i, j, pmask, smask;
	for (i=0; i<size; i++) {
		pmask = i % 32;
		smask = (((password_mask >> pmask) & 0x01) == 1) ? 0x02 : 0;
		
		send_key(smask, array[i]);
		send_key(0, 0);
	}
	
	if (not_enter) {
		send_key(0, 0x28);
		send_key(0, 0);
	}
}

void flash_erase(unsigned int pageAddress) {
	while (FLASH->SR & FLASH_SR_BSY);
	if (FLASH->SR & FLASH_SR_EOP) {
		FLASH->SR = FLASH_SR_EOP;
	}

	FLASH->CR |= FLASH_CR_PER;
	FLASH->AR = pageAddress;
	FLASH->CR |= FLASH_CR_STRT;
	while (!(FLASH->SR & FLASH_SR_EOP));
	FLASH->SR = FLASH_SR_EOP;
	FLASH->CR &= ~FLASH_CR_PER;
}

uint32_t flash_read(uint32_t address) {
  return (*(__IO uint32_t*) address);
}

void flash_write(unsigned char* data, unsigned int address, unsigned int count) {
	unsigned int i;

	while (FLASH->SR & FLASH_SR_BSY);
	if (FLASH->SR & FLASH_SR_EOP) {
		FLASH->SR = FLASH_SR_EOP;
	}

	FLASH->CR |= FLASH_CR_PG;

	for (i = 0; i < count; i += 2) {
		*(volatile unsigned short*)(address + i) = (((unsigned short)data[i + 1]) << 8) + data[i];
		while (!(FLASH->SR & FLASH_SR_EOP));
		FLASH->SR = FLASH_SR_EOP;
	}

	FLASH->CR &= ~(FLASH_CR_PG);
}

void get_password(int num) {
	unsigned int i, j = 0, off, fl;
	off = FLASH_RANDOM_POOL + 64 + (num * 32);
	for (i=off; i<(PASSWORD_LEN+off); i++, j++) {
		fl = flash_read(i);
		password_base[j] = (fl >> 24) & 0xFF;	
	}

	//password_base[PASSWORD_LEN-1] = 0x00;
	password_mask = flash_read(FLASH_RANDOM_POOL + (num * 4) + 32);
}

int flash_init(void) {
	int i, j, pinval, last;
	GPIO_InitTypeDef xIn;
	
	uint32_t val = flash_read(FLASH_RANDOM_POOL);
	if (val == FLASH_SIGN) return 1;
	
	flash_erase(FLASH_RANDOM_POOL);
	write_buf[0] = 0x01;
	write_buf[1] = 0x01;
	write_buf[2] = 0x01;
	write_buf[3] = 0x01;
	for (i=4; i<32; i++) write_buf[i] = 0x00;
	
  xIn.GPIO_Pin = GPIO_Pin_All;
  xIn.GPIO_Speed = GPIO_Speed_50MHz;
  xIn.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOB, &xIn);
	
	i = 32;
	while (1) {
		pinval = (GPIOB->IDR & 0xFF) ^ ((GPIOB->IDR >> 8) & 0xFF) ^ (SysTick->VAL & 0xFF) ^ (rand() & 0xFF);
		if ((pinval > 0x03) && (pinval < 0x28) && (pinval != last)) {
			write_buf[i] = pinval & 0xFF;
			last = pinval;
			i++;
			if (i >= 1024) break;
		}
		GPIOA->ODR ^= GPIO_Pin_4;
		if ((rand()/1111) == 0) for (j=0; j<(50); j++) __NOP();
	}
	
	flash_write(write_buf, 0x0801FC00, 1024);
	
	while (1) {
		GPIOA->ODR ^= GPIO_Pin_4;
		for (i=0; i<1000000; i++);
	}
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
