/*----------------------------------------- Define to prevent recursive inclusion ----------------------------------------------------*/
#ifndef CALIBRATION_H_IFND
#define CALIBRATION_H_IFND


/*-------------------------------------------------------- Includes ------------------------------------------------------------------*/
#include "main.h"



/*--------------------------------------------------------- Defines ------------------------------------------------------------------*/

#define CALIB_ST_DONE					0
#define CALIB_ST_START					1
#define CALIB_ST_MOVE_TO_CENTER			2
#define CALIB_ST_WAIT_CENTER			3
#define CALIB_ST_DELAY					4

/*------------------------------------------------------ TypeDefines -----------------------------------------------------------------*/

typedef struct
{
	uint8_t				STATE;


} CalibCycleTypeDef;





/*-------------------------------------------------------- Functions -----------------------------------------------------------------*/

//void CalibCycle (StandTypeDef *Stand);
void CalibCycle (void);
void CalibInit (StandTypeDef *StandData);

#endif /* MY_STRUCT_H_IFND */
