/*----------------------------------------- Define to prevent recursive inclusion ----------------------------------------------------*/
#ifndef MY_STRUCT_H_IFND
#define MY_STRUCT_H_IFND


/*-------------------------------------------------------- Includes ------------------------------------------------------------------*/

/*--------------------------------------------------------- Defines ------------------------------------------------------------------*/

#define TURN						360			   			// degrees for one full rotation
#define MICROSTEPS_ON_TURN			4000		   			// Chosen number of microsteps in driver
#define MM_ON_TURN					72						// distance in mm which stand makes on one full motor rotation TODO: Calc actual value !!!!!!
#define STEP_TO_DEGREES				(double)TURN/MICROSTEPS_ON_TURN // calc value of one step in degrees
#define KOEF_MM_TO_STEPS			(double)MICROSTEPS_ON_TURN/MM_ON_TURN
#define KOEF_STEPS_TO_MM			(double)MM_ON_TURN/MICROSTEPS_ON_TURN
#define STEPS_PER_1MM				floor((double)MICROSTEPS_ON_TURN/MM_ON_TURN)	//
#define CALIB_STEP					MM_ON_TURN
#define CALIB_CENTER				350                     //TODO: change to real value
#define POS_LIMIT					CALIB_CENTER*2

#define BIT_CLEAR					(uint8_t)0x0
#define BIT_SET						(uint8_t)0x1

#define EDGE_LOW					(uint8_t)0x0
#define EDGE_HIGH					(uint8_t)0x1

#define DIR_CLOCKWISE	           	(uint8_t) 1
#define DIR_COUNTERCLOCKWISE       	(uint8_t) 0
#define DIR_LEFT		           	(uint8_t) DIR_COUNTERCLOCKWISE
#define DIR_RIGHT		           	(uint8_t) DIR_CLOCKWISE
//#define DIR_LEFT					(uint8_t)0x0
//#define DIR_RIGHT					(uint8_t)0x1

/*------------------------------------------------------ TypeDefines -----------------------------------------------------------------*/


typedef struct
{
	uint16_t						STEPS_CNT;
	uint8_t							DIR:1;
	uint8_t							READY:1;
	uint8_t							EDGE:1;   			// signal edge status
	int32_t							CUR_POS;  			// current motor position
	int32_t							DIF_ZERO_POS;  		//difference between current zero and calibration zero
	uint8_t							CUR_SPEED;  		// current motor speed (mm/sec)
	int8_t							PREV_POS_REQ;  		// previous position request
	uint8_t							PREV_CMD_SET_ZERO; 	// previous position request

}MotorTypeDef;



typedef struct
{
	uint8_t							CALIBRATION_RQ:1;
	uint8_t							CALIBRATION_PREV_CMD:1;
	uint8_t							CALIBRATION_STATUS:1;
	uint8_t							CENTER_POS;


}StandTypeDef;



/* ------- BUTTONS structures ------------------------------------ */

typedef struct  
{    
	uint8_t							CUR_POS:1;
	uint32_t						COUNT:31;
	//uint8_t							RESERV:2;
	
}ButtonTypeDef;



typedef struct  
{    
	uint8_t							CUR_POS:1;
	uint8_t							CUR_SCALE:2;
	uint8_t							COUNT:5;
	
}ButtonScaleTypeDef;




typedef struct  
{    
	uint8_t											RFINDER_ON_OFF:1;
	uint8_t											RFINDER_IN_PROCESS:1;
	uint8_t											SaveVerif:1;
	uint8_t											SaveAckLauncherVerif:1;
	uint8_t											OpsnVerif:1;
	uint8_t											FireRq:1;
	uint8_t											CalibrationTermo:1;
	uint8_t											LauncherHorizDis:1;
	uint8_t											LauncherVertDis:1;
	uint8_t											DrumReturnRqService:1;
	uint8_t											EjectionState:1;
	uint8_t											AutotrackingOn:1;
	uint8_t											ALOS:1;

}PrevBitState_TypeDef;









/*-------------------------------------------------------- Functions -----------------------------------------------------------------*/



#endif /* MY_STRUCT_H_IFND */

