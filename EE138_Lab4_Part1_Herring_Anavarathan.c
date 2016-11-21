////////////////////////////////////////////////////////////////////////////////////// 
// Lab 4: Part 1: Digital Filter
// Scott Herring and Akash Anavarathan
//////////////////////////////////////////////////////////////////////////////////////

#include <asf.h>
#include <samd20j18.h>

void Simple_Clk_Init(void);
void init_adc(void);
void enable_tc(void);
unsigned int read_adc(void);
void configure_dac(void);
void initclks(void);

int count = 0;
float y2 = 0;
float y_1 = 0;
float y = 0;
float u2 = 0;
float u1 = 0;
float u = 0;

float a2 = 1.3725;
float a1 = 0.8819;
float b2 = 1.4797;
float b1 = 0.9891;

PortGroup *porta = (PortGroup *)PORT;
Adc *portadc = (Adc*)ADC; // define a pointer to the ADC block
Tc * tc = (Tc*)TC4;
Dac *portdac = (Dac*)DAC; 
TcCount8 *tcpointer;

int read_val = 0;

int main (void)
{	
	tcpointer = &(tc->COUNT8);
	porta->DIRSET.reg = PORT_PA14;
	
	Simple_Clk_Init(); // Enable the Clocks for the SAMD20
	initclks();	
	int test = 0;
	init_adc(); // Initialize the proper registers for the ADC
	configure_dac();
	enable_tc();



	while(1)
	{
	
	}
	
		
	return 0;
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
	
	portadc->CTRLB.reg = (0X1 << 8); // Pre-Scaler value set to divide the Clock by 128
	portadc->CTRLB.reg |= (0X0 << 4); // Set the Resolution to a 12-bit Result
	portadc->CTRLB.reg |= (0u << 2); // Enable single ended mode
	
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



void initclks(void)
{
	PM->APBCMASK.reg = (1u << 16)|(1u << 18)|(1u << 12);
	
	uint32_t temp = 0x17; // ID for GENCLK is 0x17
	temp |= 0<<8; // Selection Generic clock generator 0
	GCLK->CLKCTRL.reg = temp; // Setup in the CLKCTRL register
	GCLK->CLKCTRL.reg |= 0x1u << 14; // enable it.
	
	while(GCLK->STATUS.bit.SYNCBUSY){};
	
	temp = 0x1A; 			// ID for GCLK_DAC is 0x1A
	temp |= 0<<8; 					// Selection Generic clock generator 0
	GCLK->CLKCTRL.reg = temp; 			// Setup in the CLKCTRL register
	GCLK->CLKCTRL.reg |= 0x1u << 14; 		// enable it.
	
	while(GCLK->STATUS.bit.SYNCBUSY){};
	
	temp = 0x15;   		// ID for ________ is __________  (see table 14-2)
	temp |= 0<<8;         			//  Selection Generic clock generator 0
	GCLK->CLKCTRL.reg = temp;   		//  Setup in the CLKCTRL register
	GCLK->CLKCTRL.reg |= 0x1u << 14;    	// enable it.
	

}

/* Configure the basic timer/counter to have a period of________ or a frequency of _________  */
void enable_tc(void)
{

	/* Set up CTRLA */
	tcpointer->CTRLA.reg = (1u << 2); // set counter mode
	tcpointer->CTRLA.reg |= (0x5 << 8); // prescaler set to DIV64
	tcpointer->CTRLA.reg |= (1u << 12); // PRESCSYNC set to PRESC
	tcpointer->CTRLA.reg |= 0<<5;
	tcpointer->PER.reg = 0xFA;
	
	/*Enable TC*/
	tcpointer->CTRLA.reg |= 1 << 1; // Enable TC4
	
	tcpointer->INTENSET.reg = 0x1; // Enable the Overflow Interrupt
	NVIC->ISER[0] = (1 << 17); // Enables the Interrupt TC4
}

void configure_dac(void)
{
	//set pin as output for the dac
	//Port *ports = PORT_INSTS;
	//PortGroup *por = &(ports->Group[0]);
	
	porta->PINCFG[2].bit.PMUXEN = 0x1;		// set to correct pin configuration
	porta->PMUX[1].bit.PMUXE = 0x1;			// set to correct peripheral


	while (portdac->STATUS.reg & DAC_STATUS_SYNCBUSY) {
		/* Wait until the synchronization is complete */
	}

	/* Set reference voltage with CTRLB */
	portdac->CTRLB.reg = (1u << 6);


	while (portdac->STATUS.reg & DAC_STATUS_SYNCBUSY) 
	{
		/* Wait until the synchronization is complete */
	}

	/* Enable selected output with CTRLB*/
	portdac->CTRLB.reg = (1u << 0);
	
	/* Enable the module with CTRLA */
	portdac->CTRLA.reg = (1u << 1);

	

}

void TC4_Handler(void)
{
	
		
	int read_val = 0;
	read_val = read_adc();
	u = read_val >> 2;

	y = -(1.4797*u1) + (0.9891*u2) + (1.3725*y_1) - (0.8819*y2) + u;
	
	portdac->DATA.reg = y;
	
	y2 = y_1;
	y_1 = y;
	u2 = u1;
	u1 = u;
	
	tcpointer->INTFLAG.bit.OVF = 0x1; // Clear the Overflow Interrupt
	
	
}