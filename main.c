/*---------------------------------------------------------------------------- 
  * CMSIS-RTOS 'main' function template 
  *---------------------------------------------------------------------------*/ 
 
 
 #define osObjectsPublic                     // define objects in main module 
 #include "osObjects.h"                      // RTOS object definitions 
 #include "Board_LED.h"                  // ::Board Support:LED 
 #include "rl_net.h"                     // Keil.MDK-Pro::Network:CORE 
 
 
 void dhcp_client_notify(uint32_t if_num, dhcpClientOption opt, const uint8_t *val, uint32_t len) {
	if (opt == dhcpClientIPaddress) {
		LED_Off(0); //green LED
		LED_On(0);
	}
}
 
extern void Init_Thread(void); 
 
extern void hardware_init (void); 



int main (void) { 
  osKernelInitialize ();                    // initialize CMSIS-RTOS 
 	hardware_init(); 
	LED_Initialize(); 
   // initialize peripherals here 
 
 
   // create 'thread' functions that start executing, 
  // example: tid_name = osThreadCreate (osThread(name), NULL); 
 	Init_Thread(); 
 	net_initialize(); 
   osKernelStart ();                         // start thread execution  
 	while(1){ 
 		net_main(); 
 		osThreadYield(); 
 		/*osDelay(500); 
 		LED_On(1); //red LED 
 		osDelay(500); 
 		LED_Off(1);*/ 
 	} 
 } 
