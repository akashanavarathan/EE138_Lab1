////////////////////////////////////////////////////////////////////////////////////// 
////	Lab 2 - Analog To Digital Converter 
////		-SAMPLE CODE DOES NOT WORK-
////			- insert variables into appropriate registers
////			- set up ADC pointer (similar to port_inst setup)
//////////////////////////////////////////////////////////////////////////////////////

#include <asf.h>

void Simple_Clock_Init(void);
void enable_adc_clocks(void);
void init_adc(void);
PortGroup *porta = (PortGroup *)PORT;
PortGroup *portadc = (PortGroup *)ADC;		// define a pointer to the ADC block

unsigned int read_adc(void)
{

	// start the conversion
		// SWTRIG.reg = ;
		
	while(!//INTFLAG.bit.RESRDY);			//wait for conversion to be available
	
	return( ); 					//insert register where ADC store value
	
}

int main (void)
{	
	Simple_Clock_Init();
	enable_adc_clocks();
	init_adc();
		
	int x;
	
	while(1)
	{
			x = read_adc();			//store variable from ADC into variable "x"
	}
}

// set up generic clock for ADC
void enable_adc_clocks(void)
{
	PM->APBCMASK.reg |= (1u << 16); 			// PM_APBCMASK_______ is in the ___ position
	
	uint32_t temp = ________; 			// ID for ________ is__________ (see table 14-2)
	temp |= 0<<8; 					// Selection Generic clock generator 0
	GCLK->CLKCTRL.reg = temp; 			// Setup in the CLKCTRL register
	GCLK->CLKCTRL.reg |= 0x1u << 14; 		// enable it.
}

// initialize the on-board ADC system 
void init_adc(void)
{
	portadc->CTRLA.reg = (0 << 1);				//ADC block is disabled
	
	// you will need to configure 5 registers
		portadc->REFCTRL.reg = (1 << 7); 
		//avgctrl.reg
		//sampctrl.reg
		//ctrlb.reg
		//inputctrl.reg (muxpos, muxneg, gain)
	
	// config PA11 to be owned by ADC Peripheral
	
		//PMUX[ /*?*/ ].bit.PMUXE = ;		//refer to pg304 data sheet
		//PINCFG[ /*?*/ ].bit.PMUXEN = ;	//refer to pg304 data sheet
	
	// CTRLA.reg = ;				//Enable ADC	
}
	
//Simple Clock Initialization
void Simple_Clk_Init(void)
{
	/* Various bits in the INTFLAG register can be set to one at startup.
	   This will ensure that these bits are cleared */
	
	SYSCTRL->INTFLAG.reg = SYSCTRL_INTFLAG_BOD33RDY | SYSCTRL_INTFLAG_BOD33DET |
			SYSCTRL_INTFLAG_DFLLRDY;
			
	system_flash_set_waitstates(0);  	//Clock_flash wait state =0

	SYSCTRL_OSC8M_Type temp = SYSCTRL->OSC8M;      /* for OSC8M initialization  */

	temp.bit.PRESC    = 0;    		// no divide, i.e., set clock=8Mhz  (see page 170)
	temp.bit.ONDEMAND = 1;    		// On-demand is true
	temp.bit.RUNSTDBY = 0;    		// Standby is false
	
	SYSCTRL->OSC8M = temp;

	SYSCTRL->OSC8M.reg |= 0x1u << 1;  	// SYSCTRL_OSC8M_ENABLE bit = bit-1 (page 170)
	
	PM->CPUSEL.reg = (uint32_t)0;    	// CPU and BUS clocks Divide by 1  (see page 110)
	PM->APBASEL.reg = (uint32_t)0;     	// APBA clock 0= Divide by 1  (see page 110)
	PM->APBBSEL.reg = (uint32_t)0;     	// APBB clock 0= Divide by 1  (see page 110)
	PM->APBCSEL.reg = (uint32_t)0;     	// APBB clock 0= Divide by 1  (see page 110)

	PM->APBAMASK.reg |= 01u<<3;   		// Enable Generic clock controller clock (page 127)

	/* Software reset Generic clock to ensure it is re-initialized correctly */

	GCLK->CTRL.reg = 0x1u << 0;   		// Reset gen. clock (see page 94)
	while (GCLK->CTRL.reg & 0x1u ) {  /* Wait for reset to complete */ }
	
	// Initialization and enable generic clock #0

	*((uint8_t*)&GCLK->GENDIV.reg) = 0;  	// Select GCLK0 (page 104, Table 14-10)

	GCLK->GENDIV.reg  = 0x0100;   		// Divide by 1 for GCLK #0 (page 104)

	GCLK->GENCTRL.reg = 0x030600;  		// GCLK#0 enable, Source=6(OSC8M), IDC=1 (page 101)
}