////////////////////////////////////////////////////////////////////////////////////// 
////	Lab 2 - Analog To Digital Converter 
////		-SAMPLE CODE DOES NOT WORK-
////			- insert variables into appropriate registers
////			- set up ADC pointer (similar to port_inst setup)
//////////////////////////////////////////////////////////////////////////////////////

#include <asf.h>

void Simple_Clk_Init(void);
void enable_adc_clocks(void);
void init_adc(void);
PortGroup *porta = (PortGroup *)PORT;
Adc *portadc = ADC;		// define a pointer to the ADC block

unsigned int read_adc(void)
{

	// start the conversion
	portadc->SWTRIG.reg = (1u << 1);
		
	while(portadc->INTFLAG.bit.RESRDY != 0);		//wait for conversion to be available
	
	return(portadc->RESULT.reg); 					//insert register where ADC store value
	
}

int main (void)
{	
	Simple_Clk_Init();
	enable_adc_clocks();
	init_adc();
		
	int x;
	
	while(1)
	{
			x = read_adc();		//store variable from ADC into variable "x"
	}
}

// set up generic clock for ADC
void enable_adc_clocks(void)
{
	PM->APBCMASK.reg |= (1u << 16); 			// PM_APBCMASK_______ is in the ___ position
	
	uint32_t temp = 0x17; 			// ID for ________ is__________ (see table 14-2)
	temp |= 0<<8; 					// Selection Generic clock generator 0
	GCLK->CLKCTRL.reg = temp; 			// Setup in the CLKCTRL register
	GCLK->CLKCTRL.reg |= 0x1u << 14; 		// enable it.
}

// initialize the on-board ADC system 
void init_adc(void)
{
	portadc->CTRLA.reg = (0 << 1);				//ADC block is disabled
	
	// you will need to configure 5 registers
		portadc->REFCTRL.reg = (1u << 7); 
		portadc->REFCTRL.reg |= (1u << 1);
		
		portadc->SAMPCTRL.reg = 0;
		
		portadc->AVGCTRL.reg |= (1u << 0); 
		portadc->AVGCTRL.reg |= (0 << 1);
		portadc->AVGCTRL.reg |= (1u << 2);
		portadc->AVGCTRL.reg |= (0 << 3);
		portadc->AVGCTRL.reg |= (1u << 4); 
		portadc->AVGCTRL.reg |= (0 << 5);
		portadc->AVGCTRL.reg |= (1u << 6);
		portadc->AVGCTRL.reg |= (0 << 7); //sets avg ctrl to 32
		
		portadc->CTRLB.reg = (1u << 8); // Pre-Scaler value set to divide the Clock by 128
		portadc->CTRLB.reg |= (1u << 10); 
		portadc->CTRLB.reg |= (0 << 4); // Set the Resolution to a 12-bit Result
		portadc->CTRLB.reg |= (1u << 3); // Digital Correction Logic is Enabled
		portadc->CTRLB.reg |= (1u << 2); // Enable Free Running Mode
		portadc->CTRLB.reg |= (0 << 1); // Result is right-justfied
		portadc->CTRLB.reg |= (0 << 0); // Enabling Single-Ended Mode
		
		// Set the Gain Stage to be 1/2
		portadc->INPUTCTRL.reg = (1u << 24); 
		portadc->INPUTCTRL.reg |= (1u << 25); 
		portadc->INPUTCTRL.reg |= (1u << 26);
		portadc->INPUTCTRL.reg |= (1u << 27); 
		
		portadc->INPUTCTRL.reg |= (1u << 11);
		portadc->INPUTCTRL.reg |= (1u << 12); //grounds the negative mux pins
		
		portadc->INPUTCTRL.reg |= (1u << 0);
		portadc->INPUTCTRL.reg |= (1u << 1);
		portadc->INPUTCTRL.reg |= (0 << 2);
		portadc->INPUTCTRL.reg |= (0 << 3);
		portadc->INPUTCTRL.reg |= (1 << 4);//muxpos setup AIN[19] corresponds to PA11
		
		
	// config PA11 to be owned by ADC Peripheral
		
		porta->PMUX[20].bit.PMUXE = 0x1;		//refer to pg345 data sheet
		porta->PINCFG[20].bit.PMUXEN = 0;	//shows that its an even number pin
	
	portadc->CTRLA.reg = (1 << 1);				//Enable ADC	
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