////////////////////////////////////////////////////////////////////////////////////// 
////	Lab 3 - PWM
////	Scott Herring and Akash Anavarathan
////	Set up the TC to display a Sine Wave
//////////////////////////////////////////////////////////////////////////////////////

#include <asf.h>
void Simple_Clk_Init(void);
void enable_port(void);
void enable_tc_clocks(void);
void enable_tc(void);

//setup a the correct TC pointer for the corresponding pins. (use table 5-1 as a reference)
//there are multiple TC #'s, choose the one that ties to the specified port utilized
Tc * tc = TC2;
TcCount8 *tcpointer = &(tc->COUNT8); 


int main (void)
{
	Simple_Clk_Init();

	/* Enable the timer*/
	enable_tc();
	
	while(1)
	{
	
	}
}

/* Set correct PA pins as TC pins for PWM operation */
void enable_port(void)
{
	Port *ports = PORT_INSTS;
	PortGroup *por = &(ports->Group[0]);
	
	por->PINCFG[13].bit.PMUXEN = 0x1;		// set to correct pin configuration
	por->PMUX[6].bit.PMUXO = 0x4;			// set to correct peripheral
}

/* Perform Clock configuration to source the TC 
1) ENABLE THE APBC CLOCK FOR THE CORREECT MODULE
2) WRITE THE PROPER GENERIC CLOCK SELETION ID*/
void enable_tc_clocks(void)
{
	PM->APBCMASK.reg |= (1u << 10);  	// PM_APBCMASK_______ is in the ___ position
	
	uint32_t temp= 0x14;   		// ID for ________ is __________  (see table 14-2)
	temp |= 0<<8;         			//  Selection Generic clock generator 0
	GCLK->CLKCTRL.reg=temp;   		//  Setup in the CLKCTRL register
	GCLK->CLKCTRL.reg |= 0x1u << 14;    	// enable it.
}

/* Configure the basic timer/counter to have a period of________ or a frequency of _________  */
void enable_tc(void)
{
	enable_port();
	enable_tc_clocks();
	
	/* Set up CTRLA */
	tcpointer->CTRLA.reg = (1u << 1); // Enable CTRLA
	tcpointer->CTRLA.reg |= (1u << 2); // set counter mode
	tcpointer->CTRLA.reg |= (4u << 8); // prescaler set
	tcpointer->CTRLA.reg |= (1u << 12); // PRESCSYNC set to PRESC
	tcpointer->CTRLA.reg |= (1u << 6); // NPWM Chosen in Wavegen
	tcpointer->PER.reg = 

	/* Write a suitable value to fix duty cycle and period.*/
	

	/*Enable TC*/

}

//Simple Clock Initialization
void Simple_Clk_Init(void)
{
	/* Various bits in the INTFLAG register can be set to one at startup.
	   This will ensure that these bits are cleared */
	
	SYSCTRL->INTFLAG.reg = SYSCTRL_INTFLAG_BOD33RDY | SYSCTRL_INTFLAG_BOD33DET |
			SYSCTRL_INTFLAG_DFLLRDY;
			
	system_flash_set_waitstates(0);  		//Clock_flash wait state =0

	SYSCTRL_OSC8M_Type temp = SYSCTRL->OSC8M;      	/* for OSC8M initialization  */

	temp.bit.PRESC    = 0;    			// no divide, i.e., set clock=8Mhz  (see page 170)
	temp.bit.ONDEMAND = 1;    			//  On-demand is true
	temp.bit.RUNSTDBY = 0;    			//  Standby is false
	
	SYSCTRL->OSC8M = temp;

	SYSCTRL->OSC8M.reg |= 0x1u << 1;  		// SYSCTRL_OSC8M_ENABLE bit = bit-1 (page 170)
	
	PM->CPUSEL.reg = (uint32_t)0;    		// CPU and BUS clocks Divide by 1  (see page 110)
	PM->APBASEL.reg = (uint32_t)0;     		// APBA clock 0= Divide by 1  (see page 110)
	PM->APBBSEL.reg = (uint32_t)0;     		// APBB clock 0= Divide by 1  (see page 110)
	PM->APBCSEL.reg = (uint32_t)0;     		// APBB clock 0= Divide by 1  (see page 110)

	PM->APBAMASK.reg |= 01u<<3;   			// Enable Generic clock controller clock (page 127)

	/* Software reset Generic clock to ensure it is re-initialized correctly */

	GCLK->CTRL.reg = 0x1u << 0;   			// Reset gen. clock (see page 94)
	while (GCLK->CTRL.reg & 0x1u ) {  /* Wait for reset to complete */ }
	
	// Initialization and enable generic clock #0

	*((uint8_t*)&GCLK->GENDIV.reg) = 0;  		// Select GCLK0 (page 104, Table 14-10)

	GCLK->GENDIV.reg  = 0x0100;   		 	// Divide by 1 for GCLK #0 (page 104)

	GCLK->GENCTRL.reg = 0x030600;  		 	// GCLK#0 enable, Source=6(OSC8M), IDC=1 (page 101)
}
