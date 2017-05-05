4-Digit NTP Clock on the K64F in Keil uVision:
Note: Uses MDK-Pro middleware; requires a license or the 7-day trial.

Project setup:

Using the software pack installer, 
-be sure version 6.5 of the MDK Middleware is installed
-be sure the ARM CMSIS, Keil ARM Compiler, and Keil Kinetis SDK are installed

Create a project using the K6x Crypto CPU version of the K64FN.

Do not add any libraries yet (click cancel).

Go to the window "Select Software Packs" under Project -> Manage
UNCHECK "Use latest versions" (necessary to force v6.5)

Set "selection" dropdowns of the 4 libraries mentioned above to Fixed,
all others to Excluded.
Select version 6.5.0 of the MDK Middleware.  Default/latest versions of the other
3 are fine.

Go to the window "Manage Runtime Environment" under Project -> Manage

Under Network:
	select CORE
	under Interface set ETH to 1
	under Service select DNS Client
	under Socket select UDP
	
Under CMSIS Driver:
	under Ethernet MAC select Ethernet MAC
	under Ethernet PHY select KSZ8081RNA

THEN click Resolve in the bottom left.

Then add:

Under Device:
	under KSDK Utility select Debug Console AND Misc
	under KSDK Configuration API select FRDM-K64F

THEN click Resolve again.

Click OK.

Under Device, Navigate to the hardware_init.c file:

Change the function hardware_init to:

void hardware_init(void) {
  /* Disable Memory Protection Unit */
  MPU->CESR &= ~MPU_CESR_VLD_MASK;

  /* enable clock for PORTs */
  CLOCK_SYS_EnablePortClock(PORTA_IDX);
  CLOCK_SYS_EnablePortClock(PORTB_IDX);
  CLOCK_SYS_EnablePortClock(PORTC_IDX);
  CLOCK_SYS_EnablePortClock(PORTD_IDX);
  CLOCK_SYS_EnablePortClock(PORTE_IDX);

  configure_enet_pins (0);

  /* Setup board clock source. */
  g_xtal0ClkFreq = 50000000U;
  g_xtalRtcClkFreq = 32768U;

  /* Init board clock */
  BOARD_ClockInit();
  dbg_uart_init();
}

This code is taken from a networking example project.

Under Device, Navigate to the RTE_Device.h file, 
and switch to the configuration wizard tab on the bottom.
	Activate ENET; Set the Mode to RMII and the Clock Source to OSCERCLK

Under CMSIS, Navigate to RTX_Conf_CM.c, 
and switch to the configuration wizard tab.

	Set the Default Thread Stack Size and the Main Thread Stack Size to 2048. 
	(They need to be nice and big to deal with all the debug printing.)

	Set the RTOS Kernel Timer Clock Frequency to 12000000.  
	NOTE: 12 Million, not 120 Million.
	This is a bit of a hack, but gives us much finer control over the timing
	of the system and was the easiest way to accomplish this.
	
	Uncheck Round Robin Switching.

The default is to get a dynamic IP using DHCP.  
A static IP and static DNS hosts can be configured under 
Network -> Net_Config_ETH_0.h.
You can also change the MAC address.  The K64F is a bit sketchy and does
not actually have a unique MAC assigned to the hardware, so be sure to do this
if using more than 1 on the same network!


Open the Options for Target window.
Configure the project to use the CMSIS-DAP debugger as usual.
Under Linker, set up as in the accelerometer tutorial. 
	Uncheck "Use Memory Layout from Target Dialog"
	Click the ... button and navigate to 
	RTE\Device\MK64FN1M0VLL12\MK64FN1M0xxx12_flash.scf
	Click OK

Add to Source Group 1 the files:
	main.c
	NTPClient_v6API.c
	segments.c

Right click Source Group 1, open the window "Options for Source Group 1"
	Go to the C/C++ Tab
	Paste into Include Paths:
	C:\Keil_v5\ARM\Pack\Keil\Kinetis_SDK_DFP\2.2.0\platform\devices\MK64F12\include
	OR
	Click the ... next to Include Paths
	Navigate to the path as above
	Click OK
	
Build and load the project.

Be sure the board is connected to a working network.  Everything should work.
Connect via serial as appropriate to view debug output!
