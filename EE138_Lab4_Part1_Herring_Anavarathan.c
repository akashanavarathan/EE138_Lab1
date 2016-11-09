////////////////////////////////////////////////////////////////////////////////////// 
// Lab 4: Part 1: Digital Filter
// Scott Herring and Akash Anavarathan
//////////////////////////////////////////////////////////////////////////////////////

#include <asf.h>

void Simple_Clk_Init(void);
void enable_adc_clocks(void);
void init_adc(void);
void wait(int t);
void enable_port(void);
void enable_tc_clocks(void);
void enable_tc(void);
unsigned int read_adc(void);
void configure_dac(void);
void configure_dac_clock(void);

PortGroup *porta = (PortGroup *)PORT;
Adc *portadc = ADC; // define a pointer to the ADC block
Tc * tc = TC4;
TcCount8 *tcpointer;

int result_read = 0; // Variable used to store the value from the RESULT register
float display_result = 0; // Variable used to store the final result for display

int main (void)
{	
	tcpointer = &(tc->COUNT8);
	int counter = 0;
	
	Simple_Clk_Init(); // Enable the Clocks for the SAMD20
	enable_tc();
	enable_adc_clocks(); // Specifically Enable the Clocks for the ADC
	init_adc(); // Initialize the proper registers for the ADC
	configure_dac_clock();
	configure_dac();
	
	Port *ports = PORT_INSTS;										/* Create pointer to port group */
	PortGroup *porA = &(ports->Group[0]);							/* Assign port group A */
	PortGroup *porB = &(ports->Group[1]);							/* Assign port group B */
	
	while(1)
	{
			result_read = read_adc();		//store variable from ADC into variable "x"
			counter++;
			
			if (counter == 80)
			{
				
			display_result = ((result_read * 3.3) / 4095); // Will convert the voltage into a range from 0 - 3.3V
			display_result = (display_result * 1000); // Will multiply and round so it's somewhere between 0 and 3300
			counter = 0;
			}
			
			if(result_read == 2048)
			{
				tcpointer->CC[0].reg = (int)(result_read / 17);
				tcpointer->CC[1].reg = (int)(result_read / 17);
			}
			else if(result_read > 2048 || (result_read < 2048 && result_read > 40))
			{
				tcpointer->CC[0].reg = (int)(result_read / 17);
				tcpointer->CC[1].reg = 255 - (int)(result_read / 17);
			}
			else
			{
				tcpointer->CC[0].reg = (int)(result_read / 17);
				tcpointer->CC[1].reg = 240 - (int)(result_read / 17);
			}
			
			Integer_to_Array((int)display_result); // Convert display_result into an Array
			
			overallDisplay(); // Will Display the 4 Digit Number + Decimal Point on the 7-segment Displays
				
	}
}

unsigned int read_adc(void)
{

	// start the conversion
	portadc->SWTRIG.reg = (0x1u << 1);
	
	while(!(portadc->INTFLAG.bit.RESRDY))
	{} // wait for conversion to be available
	
	return(portadc->RESULT.reg); // insert register where ADC store value
	
}

// initialize the on-board ADC system 
void init_adc(void)
{
	portadc->CTRLA.reg = (0 << 1); //ADC block is disabled

	portadc->REFCTRL.reg = (1u << 7); 
	portadc->REFCTRL.reg |= (1u << 1);
	
	portadc->SAMPCTRL.reg = 0;
	
	// AVG Control is set to 32, meaning it'll take that many samples
	portadc->AVGCTRL.reg |= 0X3; 
	portadc->AVGCTRL.reg |= 0X3<<4; 
	
	portadc->CTRLB.reg = (0X5 << 8); // Pre-Scaler value set to divide the Clock by 128
	portadc->CTRLB.reg |= (0X0 << 4); // Set the Resolution to a 12-bit Result
	portadc->CTRLB.reg |= (1u << 2); // Enable Free Running Mode
	portadc->CTRLB.reg |= (0 << 0); // Enabling Single-Ended Mode
	
	// Set the Gain Stage to be 1/2
	portadc->INPUTCTRL.reg = 0XF<<24; 
	
	// MUXNEG is Grounded
	portadc->INPUTCTRL.reg |= (0x18 << 8);
	
	// MUXPOS is set to AIN[19] which is PA11
	portadc->INPUTCTRL.reg |= (0x13 << 0);

	
	
	// config PA11 to be owned by ADC Peripheral
	porta->PMUX[5].bit.PMUXO = 0x1; 
	porta->PINCFG[11].bit.PMUXEN = 0X1; 

	portadc->CTRLA.reg = (1 << 1); // Enable ADC	
}

//Simple Clock Initialization
void Simple_Clk_Init(void)
{
	/* Various bits in the INTFLAG register can be set to one at startup.
	   This will ensure that these bits are cleared */
	
	SYSCTRL->INTFLAG.reg = SYSCTRL_INTFLAG_BOD33RDY | SYSCTRL_INTFLAG_BOD33DET |
			SYSCTRL_INTFLAG_DFLLRDY;
			
	//system_flash_set_waitstates(0);  	//Clock_flash wait state =0

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

void wait(int t)
{
	volatile int count = 0;
	while (count < t*100)
	{
		count++;
	}
}

// set up generic clock for ADC
void enable_adc_clocks(void)
{
	PM->APBCMASK.reg |= (1u << 16);
	
	uint32_t temp = 0x17; // ID for GENCLK is 0x17
	temp |= 0<<8; // Selection Generic clock generator 0
	GCLK->CLKCTRL.reg |= temp; // Setup in the CLKCTRL register
	GCLK->CLKCTRL.reg |= 0x1u << 14; // enable it.
}

/* Perform Clock configuration to source the TC 
1) ENABLE THE APBC CLOCK FOR THE CORREECT MODULE
2) WRITE THE PROPER GENERIC CLOCK SELETION ID*/
void enable_tc_clocks(void)
{
	PM->APBCMASK.reg |= (1u << 12);  	// PM_APBCMASK_______ is in the ___ position
	
	uint32_t temp2 = 0x15;   		// ID for ________ is __________  (see table 14-2)
	temp2 |= 0<<8;         			//  Selection Generic clock generator 0
	GCLK->CLKCTRL.reg |= temp2;   		//  Setup in the CLKCTRL register
	GCLK->CLKCTRL.reg |= 0x1u << 14;    	// enable it.
}

/* Configure the basic timer/counter to have a period of________ or a frequency of _________  */
void enable_tc(void)
{
	int check;
	enable_port();
	enable_tc_clocks();
	
	/* Set up CTRLA */
	tcpointer->CTRLA.reg = (1u << 2); // set counter mode
	tcpointer->CTRLA.reg |= (0x0 << 8); // prescaler set
	tcpointer->CTRLA.reg |= (1u << 12); // PRESCSYNC set to PRESC

	/* Write a suitable value to fix duty cycle and period.*/
	tcpointer->CTRLA.reg |= (1u << 6); // NPWM Chosen in Wavegen
	check = tcpointer->STATUS.bit.SYNCBUSY;
	tcpointer->COUNT.reg = 0;
	tcpointer->PER.reg = 0xFE;
	
	/*Enable TC*/
	tcpointer->CTRLA.reg |= 1 << 1; // Enable TC4
	
}

void configure_dac_clock(void)
{
	PM->APBCMASK.reg |= (1u << 18);		// PM_APBCMASK DAC is in the 18 position
	
	uint32_t temp = 0x1A; 			// ID for GCLK_DAC is 0x1A
	temp |= 0<<8; 					// Selection Generic clock generator 0
	GCLK->CLKCTRL.reg |= temp; 			// Setup in the CLKCTRL register
	GCLK->CLKCTRL.reg |= 0x1u << 14; 		// enable it.
}

void configure_dac(void)
{
	//set pin as output for the dac
	Port *ports = PORT_INSTS;
	PortGroup *por = &(ports->Group[0]);
	
	por->PINCFG[2].bit.PMUXEN = 0x1;		// set to correct pin configuration
	por->PMUX[1].bit.PMUXE = 0x1;			// set to correct peripheral


	while (portdac->STATUS.reg & DAC_STATUS_SYNCBUSY) {
		/* Wait until the synchronization is complete */
	}

	/* Set reference voltage with CTRLB */
	portdac->CTRLB.reg = (1u << 6);


	while (portdac->STATUS.reg & DAC_STATUS_SYNCBUSY) 
	{
		/* Wait until the synchronization is complete */
	}

	/* Enable the module with CTRLA */
	portdac->CTRLA.reg = (1u << 1);

	/* Enable selected output with CTRLB*/
	portdac->CTRLB.reg = (1u << 0);

}