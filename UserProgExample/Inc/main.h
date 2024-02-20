/*----------------------------------------------------------------------------*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef MAIN_H_IFND
#define MAIN_H_IFND


/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx.h"
#include "rcc.h"
#include "timer.h"
#include "flash.h"
#include "can.h"

/* Defines -------------------------------------------------------------------*/

// leds definations ------------------------------------------------
#define LED1_ON()              		(GPIOB->ODR |=  GPIO_ODR_ODR_0)
#define LED1_OFF()              	(GPIOB->ODR &= ~GPIO_ODR_ODR_0)
#define LED1_TOOGLE()              	(GPIOB->ODR ^=  GPIO_ODR_ODR_0)
#define LED2_ON()              		(GPIOB->ODR |=  GPIO_ODR_ODR_7)
#define LED2_OFF()              	(GPIOB->ODR &= ~GPIO_ODR_ODR_7)
#define LED2_TOOGLE()              	(GPIOB->ODR ^=  GPIO_ODR_ODR_7)
#define LED3_ON()              		(GPIOB->ODR |=  GPIO_ODR_ODR_14)
#define LED3_OFF()              	(GPIOB->ODR &= ~GPIO_ODR_ODR_14)
#define LED3_TOOGLE()              	(GPIOB->ODR ^=  GPIO_ODR_ODR_14)



/* Functions -----------------------------------------------------------------*/

void CheckRxMessageCAN1 (void);
enum FLASH_STATUS ChangeBootloaderDelay(uint32_t delay);

void InitLEDs(void);
enum FLASH_STATUS WriteBoardIdToFlash(void);

void Tick_1ms(void);
void Tick_1sec(void);


#endif /* MAIN_H_IFND */
