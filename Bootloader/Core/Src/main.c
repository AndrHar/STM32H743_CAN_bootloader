/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program
  ******************************************************************************
  * Program of bootloader via CAN-bus for STM32H743
  *
  * Flash memory can be erased only by sectors (128K bytes).
  * Bootloader is always placed at the start of the flash memory (Sector0 - 0x8000000),
  * Sector1 (0x8020000) is used for configuration data (not default board-id, etc.) This sector
  * is erasing and writing by user program.
  * User program can be written from the beginning of Sector2 (0x8040000) to the end
  * of BANK1 (Sector7). This bootloader doesn't have functions to work with BANK2.
  * After reset bootloader waits some delay for CAN-msg with defined id. If there is no can-msg
  * then user program starts.
  *
  * The protocol of loading program via CAN was used 'as it is' for windows
  * program 'CANLoader'
  *
  * Bootloader can be reset by CAN-msg with id: 0x580+BoardId. Byte0 should be 0xAA, Byte1= 0xBB
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private constants ---------------------------------------------------------*/
const uint16_t VERSION = 1.3;

/* Defines -------------------------------------------------------------------*/
#define BOOT_START_ADDRESS 					(0x8000000U)
#define APP_PROG_ADDRESS 					(0x8040000U)
#define APP_KONF_ADDRESS 					(0x8020000U)

#define FLASH_DATA_HEADER 					((uint32_t)0x0123fedc)

#define DELAY_BEFORE_JUMP_TO_USER_PROGRAM 	(2000U) // ms
#define PROG_MSG_LENGTH						(8U)

/* Variables -----------------------------------------------------------------*/

volatile uint8_t checksum = 0;

static uint8_t buff[1024];
static uint16_t i_buff = 0;

static uint16_t Status = 0;

static uint32_t delayBeforeJump = DELAY_BEFORE_JUMP_TO_USER_PROGRAM;
static uint8_t enableJump = 1;

uint8_t Error_status;

/* ----------- CAN TxMsg headers ------------------*/

extern FDCAN_TxHeaderTypeDef headerTxMsg_0x550;  //declaration in 'can.c'
extern FDCAN_TxHeaderTypeDef headerTxMsg_0x551;  //declaration in 'can.c'
extern FDCAN_TxHeaderTypeDef headerTxMsg_0x555;  //declaration in 'can.c'

static typeDefCanMessage CAN_TxMsg_0x550;
static typeDefCanMessage CAN_TxMsg_0x551;
static typeDefCanMessage CAN_TxMsg_0x555;


/* ---------- CAN RxMsg headers ------------------*/
uint32_t rxCANid[] = {0x560, 0x570, 580};

extern FDCAN_FilterTypeDef headerRxMsg_0x56x; //declaration in 'can.c'
extern FDCAN_FilterTypeDef headerRxMsg_0x57x; //declaration in 'can.c'
extern FDCAN_FilterTypeDef headerRxMsg_0x58x; //declaration in 'can.c'

static typeDefCanMessage CAN_RxMsg_0x56x;
static typeDefCanMessage CAN_RxMsg_0x57x;
static typeDefCanMessage CAN_RxMsg_0x58x;

/*----------------------------------------------------------------------------*/



/* Main ----------------------------------------------------------------------*/
int main(void)
{
	CAN_TxMsg_0x550.always_transmit = 0;
	CAN_TxMsg_0x551.always_transmit = 0;
	CAN_TxMsg_0x555.always_transmit = 0;

	/* After reset MC reads flash from the beginning (0x08000000). At this address this Bootloader is written. After execution
	 * the boot loader jump to User program, which makes a new assignation of 'Vector Table Offset Register'.
	 * When we jump to Bootloader from the User prog then reassignation of 'Vector Table Offset Register' is required
	 * */
	if (SCB->VTOR != BOOT_START_ADDRESS)
	{
		__disable_irq();
		SCB->VTOR = (uint32_t)BOOT_START_ADDRESS;
		__enable_irq();
	}

	/* FLASH_SetLatency */
	MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY, FLASH_ACR_LATENCY_4WS);
	if ( (READ_BIT(FLASH->ACR, FLASH_ACR_LATENCY)) != FLASH_ACR_LATENCY_4WS)
	{
		Error_status = FLASH_LATENCY_ERROR;
	}

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

	TimerInit(1000);  //timer for 1kHz

	InitLEDs();

	ReadAppConfigFromFlash();  // rxCANid are configured

	InitCAN1(rxCANid);

	CheckAppExist();

	TimerStart();

	/* Loop forever */
	while(1)
	{
		CheckRxMessageCAN1();
		CheckTxMessageCAN1();

		/* actions for 1 ms period */
		if (TimerGet().FLAGS.flag_1ms)
		{
			TimerResetFlag(TIMER_FL_1MS);
			Tick_1ms();
		}

		/* actions for 500 ms period */
		if (TimerGet().FLAGS.flag_500ms)
		{
			TimerResetFlag(TIMER_FL_500MS);
			Tick_500ms();
		}

		/* actions for 1 sec period */
		if (TimerGet().FLAGS.flag_1s)
		{
			TimerResetFlag(TIMER_FL_1S);
			Tick_1sec();
		}


	}
}
/* End main ------------------------------------------------------------------*/


/* ReadAppConfigFromFlash ----------------------------------------------------*/
void ReadAppConfigFromFlash(void)
{
	/* If several boards, need to be programmed, are present on same CAN-bus, then each board
	 * should have unique CAN-id for programming. To switch on such unique id, User program
	 * should write number from 0x1 to 0xF to flash address 'APP_KONF_ADDRESS'
	 */

	uint32_t config_data = 0;
	uint32_t address = APP_KONF_ADDRESS;

	config_data = flashRead(address);
	if (config_data == FLASH_DATA_HEADER)
	{
		address += 4;
		config_data = flashRead(address);
		if (config_data <= 0xF)
		{
			rxCANid[0] = (uint32_t) 0x560 + config_data;
			rxCANid[1] = (uint32_t) 0x570 + config_data;
			rxCANid[2] = (uint32_t) 0x580 + config_data;
		}

		address += 4;
		config_data = flashRead(address);
		if (config_data != 0xFFFFFFFF)
		{
			delayBeforeJump = config_data;
		}
	}
}
/* End ReadAppConfigFromFlash ------------------------------------------------*/



/* CheckAppExist -------------------------------------------------------------*/
void CheckAppExist(void)
{
	uint32_t config_data = 0;

	config_data = flashRead(APP_PROG_ADDRESS);

	/* if flash memory at user program address is empty
	 * then stay in bootloader */
	if (config_data == 0xFFFFFFFF){enableJump = 0;}
}
/* End CheckAppExist ---------------------------------------------------------*/



/* Tick_500ms ----------------------------------------------------------------*/
void Tick_500ms (void)
{
	/* simple status indication */

	if (Error_status == FLASH_RDY ){
		LED2_TOOGLE();
	}
	else{
		LED3_TOOGLE();
	}

}
/* End Tick_500ms ------------------------------------------------------------*/





/* Tick_1ms ------------------------------------------------------------------*/
void Tick_1ms (void)
{

	if (enableJump)
	{
		if (delayBeforeJump > 0) delayBeforeJump--;
	}

	if (delayBeforeJump == 0)
	{

		uint32_t appJumpAdress;
		void (*GoToApp)(void); // pointer on function

		appJumpAdress = *((volatile uint32_t*)(APP_PROG_ADDRESS + 4));
		GoToApp = (void (*)(void))appJumpAdress; // new address for function
		__disable_irq();
		SysTick->CTRL = 0x00000000;                  //disable SysTick
		__set_MSP(*((volatile uint32_t*)APP_PROG_ADDRESS)); // move stack pointer on new address
		__NOP();
		__NOP();
		//	__set_PSP(*((volatile uint32_t*)0x803E000));
		GoToApp();
	}

}

/* End Tick_1ms --------------------------------------------------------------*/


/* Tick_1sec -----------------------------------------------------------------*/
void Tick_1sec (void)
{

// actions for test

}
/* Tick_1sec -----------------------------------------------------------------*/



/* InitLEDs ------------------------------------------------------------------*/
void InitLEDs(void)
{
	/* Board NUCLEO-H743ZI */
	/* onboard leds: LED1, LED2, LED3 - PB0, PB7, PB14 */
	RCC->AHB4ENR|= RCC_AHB4ENR_GPIOBEN; 																														//enable clock for bus AHB4 (GPIO G)

	GPIOB->MODER &= ~ (GPIO_MODER_MODER0 | GPIO_MODER_MODER7 | GPIO_MODER_MODER14 ); 					// reset bits
	GPIOB->MODER |= (GPIO_MODER_MODER0_0 | GPIO_MODER_MODER7_0 | GPIO_MODER_MODER14_0 ); 				// output mode
	GPIOB->OTYPER &= ~(GPIO_OTYPER_OT_0 | GPIO_OTYPER_OT_7 | GPIO_OTYPER_OT_14 );  						// output push-pull (reset state)
	GPIOB->OSPEEDR &= ~(GPIO_OSPEEDER_OSPEEDR0 | GPIO_OSPEEDER_OSPEEDR7 | GPIO_OSPEEDER_OSPEEDR14 ); 	// low speed

	LED3_ON();
}
/* End InitLEDs --------------------------------------------------------------*/



/* CheckTxMessageCAN1 --------------------------------------------------------*/
void CheckTxMessageCAN1 (void)
{
	if (CAN_TxMsg_0x550.onetime_transmit)
	{
		CAN_TxMsg_0x550.onetime_transmit = 0;

		CAN_TxMsg_0x550.data[0] = Status;

		FDCAN_SendMessage(&headerTxMsg_0x550, CAN_TxMsg_0x550.data, TxMsg_0x550_BUF_NUMBER, CAN_MODULE1);
		Status = 0;
	}


	if (CAN_TxMsg_0x551.onetime_transmit)
	{
		FDCAN_SendMessage(&headerTxMsg_0x551, CAN_TxMsg_0x551.data, TxMsg_0x551_BUF_NUMBER, CAN_MODULE1);
		CAN_TxMsg_0x551.onetime_transmit = 0;
	}


	if (CAN_TxMsg_0x555.onetime_transmit)
	{
		FDCAN_SendMessage(&headerTxMsg_0x555, CAN_TxMsg_0x555.data, TxMsg_0x555_BUF_NUMBER, CAN_MODULE1);
		CAN_TxMsg_0x555.onetime_transmit = 0;
	}


}
/* End CheckTxMessageCAN1 ----------------------------------------------------*/



/* CheckRxMessageCAN1 -------------------------------------------------------*/
void CheckRxMessageCAN1 (void)
{
	 uint32_t reg_NewDataFlags = FDCAN1->NDAT1; // for RxBufferIndex < 32

	/* Check Msg 0x56x reception */
	if((reg_NewDataFlags & (1 << headerRxMsg_0x56x.RxBufferIndex)) != 0)
	{
		ReceiveCanMsg(headerRxMsg_0x56x.RxBufferIndex, CAN_RxMsg_0x56x.data, CAN_MODULE1);
		Actions_CAN_0x56x_received();
	}


	/* Check Msg 0x57x reception */
	if((reg_NewDataFlags & (1 << headerRxMsg_0x57x.RxBufferIndex)) != 0)
	{
		ReceiveCanMsg(headerRxMsg_0x57x.RxBufferIndex, CAN_RxMsg_0x57x.data, CAN_MODULE1);
		Actions_CAN_0x57x_received();
	}

	/* Check Msg 0x58x reception */
	if((reg_NewDataFlags & (1 << headerRxMsg_0x58x.RxBufferIndex)) != 0)
	{
		ReceiveCanMsg(headerRxMsg_0x58x.RxBufferIndex, CAN_RxMsg_0x58x.data, CAN_MODULE1);

		// check command for reset MC
		if ( (CAN_RxMsg_0x58x.data[0] == 0xAA) && (CAN_RxMsg_0x58x.data[1] == 0xBB) )
		{
			NVIC_SystemReset();
		}
	}
}
/* End CheckRxMessageCAN1 ---------------------------------------------------*/



/* Actions_CAN_0x56x_received ------------------------------------------------*/
void Actions_CAN_0x56x_received(void)
{
	static uint32_t offset;
	static uint16_t flashNotErase;
	static uint8_t sectorNbr;
	static uint32_t sectorEndAddress;
	enableJump = 0;

	switch(CAN_RxMsg_0x56x.data[0])
	{
		case 0xAA:
			offset = 0;
			flashNotErase = 0;

			sectorNbr = FLASH_SECTOR_USER_PROG;
			sectorEndAddress = ADDR_FLASH_SECTOR_2_BANK1 - 1;

			Status = 0xAA;
			CAN_TxMsg_0x550.onetime_transmit = 1;
			break;

		case 0xBB:
			CAN_TxMsg_0x555.onetime_transmit = 1;
			checksum = 0;
			i_buff = 0;
			break;

		case 0xCC:
			if (flashNotErase){break;}
			uint8_t checksum_can = CAN_RxMsg_0x56x.data[1];
			if ((uint8_t)(checksum + checksum_can) == 0)
			{
				// if WriteData occupies next sector in Flash memory then clear this sector before writing
				if ( (APP_PROG_ADDRESS + offset + sizeof(buff)) > sectorEndAddress )
				{

					if (flash_EraseSector(sectorNbr) != FLASH_RDY)
					{
						flashNotErase = 1;
						Error_status = FLASH_PGM_ERROR;
					}
					else
					{
						flashNotErase = 0;
						sectorEndAddress += FLASH_SECTOR_SIZE;
						sectorNbr++;
					}
				}


				if ( ( flashNotErase == 0) && ( (flashWrite(APP_PROG_ADDRESS + offset, ((uint32_t)buff), sizeof(buff))) == FLASH_RDY ) )
				{
					offset += sizeof(buff);
					Status = 0xB0;
				}
				else {Error_status = FLASH_PGM_ERROR;}

			}
			 else {
				//Status = 0xB1;
				flashNotErase = 1;
				offset = 0;
				checksum = 0;
				Error_status = FLASH_PGM_ERROR;
				}

			CAN_TxMsg_0x550.onetime_transmit = 1;
			break;

		case 0xDD:
				NVIC_SystemReset();
				break;

		case 0xEE:
				CAN_TxMsg_0x551.onetime_transmit = 1;
				default:
				break;
	}


}
/* End Actions_CAN_0x56x_received --------------------------------------------*/



/* Actions_CAN_0x57x_received ------------------------------------------------*/
void Actions_CAN_0x57x_received(void)
{
	enableJump = 0;

	/* Program is sending by CAN-mesage with data length = 8 bytes */
	if (i_buff <= (sizeof(buff) - PROG_MSG_LENGTH) )
	{
		for(int i = 0; i < PROG_MSG_LENGTH; i++){
			buff[i_buff+i] = CAN_RxMsg_0x57x.data[i];
			checksum += CAN_RxMsg_0x57x.data[i];
		}
		i_buff += PROG_MSG_LENGTH;
	}

}
/* End Actions_CAN_0x57x_received --------------------------------------------*/







