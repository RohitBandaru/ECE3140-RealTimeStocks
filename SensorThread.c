
#include "cmsis_os.h"                                           // CMSIS RTOS header file
#include "Board_LED.h"                  // ::Board Support:LED
#include "Board_Accelerometer.h"        // ::Board Support:Accelerometer
#include "Board_Magnetometer.h"         // ::Board Support:Magnetometer

ACCELEROMETER_STATE g_accel;
MAGNETOMETER_STATE g_magneto;
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
void Thread (void const *argument);                             // thread function
osThreadId tid_Thread;                                          // thread id
osThreadDef (Thread, osPriorityNormal, 1, 0);                   // thread object

int Init_Thread (void) {

  tid_Thread = osThreadCreate (osThread(Thread), NULL);
  if (!tid_Thread) return(-1);
  
  return(0);
}

void Thread (void const *argument) {

	Accelerometer_Initialize();
	Magnetometer_Initialize();
  while (1) {
		Accelerometer_GetState (&g_accel); 
		Magnetometer_GetState (&g_magneto);
    osDelay(500);
		LED_Off(2); //blue LED
		osDelay(500);
		LED_On(2);
    osThreadYield ();                                           // suspend thread
  }
}
