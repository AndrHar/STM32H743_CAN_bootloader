/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program
  ******************************************************************************
  * Simple led blinking program as example of using with bootloader via CAN-bus
  *
  * 1) Assignation of 'Vector Table Offset Register' should be done at the beginning
  *    of the user program.
  *
  * 2) Change memory areas address in 'STM32H743ZITX_FLASH.ld':
  *    FLASH (rx)     : ORIGIN = 0x08040000, LENGTH = 1792K
  *
  *    If Keil: 'Options for target'-> 'Linker' -> 'R/O Base': 0x08040000
  *
  * 3) Set checkbox for creating output binary file:
  *
  *    Project-> Properties -> C/C++ Build Settings ->
  *    -> Tool Settings -> MCU Post build outputs -> Convert to binary file (-O binary)
  *
  *    Download output binary file to MC by 'CANLoader.exe'
  *
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"


/* Defines -------------------------------------------------------------------*/

#define BOARD_ID 			(0x1U)

#define APP_PROG_ADDRESS 	(0x8040000U)
#define APP_KONF_ADDRESS 	(0x8020000U)

#define FLASH_DATA_HEADER 	((uint32_t)0x0123fedc)

/* Variables -----------------------------------------------------------------*/

uint8_t Error_status = FLASH_RDY;

/* ----------- CAN TxMsg headers ------------------*/

extern FDCAN_TxHeaderTypeDef headerTxMsg_0x510;  //declaration in 'can.c'
static typeDefCanMessage CAN_TxMsg_0x510;


/* ---------- CAN RxMsg headers ------------------*/
uint32_t rxCANid[] = {0x580 + BOARD_ID};

extern FDCAN_FilterTypeDef headerRxMsg_0x58x; //declaration in 'can.c'
static typeDefCanMessage CAN_RxMsg_0x58x;


/* Functions -----------------------------------------------------------------*/

/* Main ----------------------------------------------------------------------*/
int main(void)
{

	/* After execution, bootloader transfers control to this program.
	 * New assignation of 'Vector Table Offset Register' should be done.
	 * */
	__disable_irq();
	SCB->VTOR = (uint32_t)APP_PROG_ADDRESS;
	__enable_irq();


	/* Set the power supply configuration */
	MODIFY_REG(PWR->CR3, (PWR_CR3_SCUEN | PWR_CR3_LDOEN | PWR_CR3_BYPASS), PWR_CR3_LDOEN);
	/* PWR_SetRegulVoltageScaling */
	MODIFY_REG(PWR->D3CR, PWR_D3CR_VOS, (PWR_D3CR_VOS_0 | PWR_D3CR_VOS_1));


	RCC_Init();

	// check if clock install is ok
	if (SystemCoreClock != 72000000)	//if SystemCoreClock not equal 72Mhz reset system
	{
		NVIC_SystemReset();
	}

	InitLEDs();

	if (BOARD_ID != 0)
	{
		uint32_t config_data[] = {0,0};
		config_data[0] = flashRead(APP_KONF_ADDRESS);
		config_data[1] = flashRead(APP_KONF_ADDRESS+4);

		if ( (config_data[0] != FLASH_DATA_HEADER) || (config_data[1] != BOARD_ID) )
		{
			//LED3_ON();
			Error_status = WriteBoardIdToFlash();
		}

	}

	InitCAN1(rxCANid);

	TimerInit(1000);  //timer for 1kHz
	TimerStart();


	/* Loop forever */
	while(1)
	{
		CheckRxMessageCAN1();

		/* actions for 1 sec period */
		if (TimerGet().FLAGS.flag_1s)
		{
			TimerResetFlag(TIMER_FL_1S);
			Tick_1sec();
		}

	}
}
/* End main ------------------------------------------------------------------*/


/* CheckRxMessageCAN1 --------------------------------------------------------*/
void CheckRxMessageCAN1 (void)
{
	 uint32_t reg_NewDataFlags = FDCAN1->NDAT1; // for RxBufferIndex < 32

	/* Check Msg 0x59x reception */
	if((reg_NewDataFlags & (1 << headerRxMsg_0x58x.RxBufferIndex)) != 0)
	{
		ReceiveCanMsg(headerRxMsg_0x58x.RxBufferIndex, CAN_RxMsg_0x58x.data, CAN_MODULE1);

		/* example of jumping back to bootloader */
		if ( (CAN_RxMsg_0x58x.data[0] == 0xAA) &&  (CAN_RxMsg_0x58x.data[1] == 0xBB))
		{
			// MC always starts from bootloader after reset
			NVIC_SystemReset();
		}

		/* new bootloader delay value can be written to flash-memory */
		if ( (CAN_RxMsg_0x58x.data[0] == 0xCC) &&  (CAN_RxMsg_0x58x.data[1] == 0xDD))
		{
			uint32_t delay;

			delay = (CAN_RxMsg_0x58x.data[2] << 8) + CAN_RxMsg_0x58x.data[3];
			ChangeBootloaderDelay(delay);
		}


	}


}
/* End CheckRxMessageCAN1 ----------------------------------------------------*/



/* WriteBoardIdToFlash -------------------------------------------------------*/
enum FLASH_STATUS WriteBoardIdToFlash(void)
{
	uint32_t buff[2];
	buff[0] = FLASH_DATA_HEADER;
	buff[1] = BOARD_ID;

	if ( flashUnlock() != FLASH_RDY ){ return FLASH_LOCK_ERROR;}


	if (flash_EraseSector(FLASH_SECTOR_CONFIG_DATA) == FLASH_RDY)
	{
		if (flashWrite(APP_KONF_ADDRESS, ((uint32_t)buff), sizeof(buff)) != FLASH_RDY)
		{
			return FLASH_PGM_ERROR;
		}

	}
	else {return FLASH_PGM_ERROR;}

	if ( flashLock() != FLASH_RDY ){ return FLASH_LOCK_ERROR;}

	return FLASH_RDY;

}
/* End WriteBoardIdToFlash ---------------------------------------------------*/



/* ChangeBootloaderDelay -----------------------------------------------------*/
enum FLASH_STATUS ChangeBootloaderDelay(uint32_t delay)
{
	uint32_t buff[3];

	buff[0] = FLASH_DATA_HEADER;
	buff[1] = BOARD_ID;
	buff[2] = delay;

	if ( flashUnlock() != FLASH_RDY ){ return FLASH_LOCK_ERROR;}

	if (flash_EraseSector(FLASH_SECTOR_CONFIG_DATA) == FLASH_RDY)
	{
		if (flashWrite(APP_KONF_ADDRESS, ((uint32_t)buff), sizeof(buff)) != FLASH_RDY)
		{
			return FLASH_PGM_ERROR;
		}

	}
	else {return FLASH_PGM_ERROR;}

	if ( flashLock() != FLASH_RDY ){ return FLASH_LOCK_ERROR;}

	return FLASH_RDY;
}
/* End ChangeBootloaderDelay -------------------------------------------------*/




/* Tick_1ms ------------------------------------------------------------------*/
void Tick_1ms (void)
{
	//
}
/* End Tick_1ms --------------------------------------------------------------*/


/* Tick_1sec -----------------------------------------------------------------*/
void Tick_1sec (void)
{

	LED1_TOOGLE();

	if (Error_status != FLASH_RDY)
	{
		LED3_ON();
	}

	CAN_TxMsg_0x510.data[0] = 0x01;
	FDCAN_SendMessage(&headerTxMsg_0x510, CAN_TxMsg_0x510.data, TxMsg_0x510_BUF_NUMBER, CAN_MODULE1);

}
/* Tick_1sec -----------------------------------------------------------------*/





/* InitLEDs ------------------------------------------------------------------*/
void InitLEDs(void)
{

	/* onboard leds: LED1, LED2, LED3 - PB0, PB7, PB14 */
	RCC->AHB4ENR|= RCC_AHB4ENR_GPIOBEN; 																														//enable clock for bus AHB4 (GPIO G)

	GPIOB->MODER &= ~ (GPIO_MODER_MODER0 | GPIO_MODER_MODER7 | GPIO_MODER_MODER14 ); 					// reset bits
	GPIOB->MODER |= (GPIO_MODER_MODER0_0 | GPIO_MODER_MODER7_0 | GPIO_MODER_MODER14_0 ); 				// output mode
	GPIOB->OTYPER &= ~(GPIO_OTYPER_OT_0 | GPIO_OTYPER_OT_7 | GPIO_OTYPER_OT_14 );  						// output push-pull (reset state)
	GPIOB->OSPEEDR &= ~(GPIO_OSPEEDER_OSPEEDR0 | GPIO_OSPEEDER_OSPEEDR7 | GPIO_OSPEEDER_OSPEEDR14 ); 	// low speed

	LED1_ON();
	LED2_OFF();
	LED3_OFF();
}
/* End InitLEDs --------------------------------------------------------------*/











