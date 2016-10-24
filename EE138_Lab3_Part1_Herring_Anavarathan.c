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
int index = 0;
int cycles = 256;
float sine_table_percent[128] = {0.5,0.52452141,0.548983805,0.573328313,0.597496346,
0.62142974,0.645070896,0.668362917,0.691249749,0.71367631,0.735588627,0.756933966,
0.777660956,0.797719715,0.817061967,0.835641164,0.853412591,0.870333478,0.886363105,
0.901462892,0.9155965,0.928729914,0.940831527,0.951872215,0.961825406,0.970667147,
0.978376158,0.984933888,0.990324553,0.994535181,0.997555637,0.999378653,0.999999841,
0.999417707,0.997633651,0.994651967,0.990479831,0.985127283,0.978607206,0.970935291,
0.962130001,0.952212527,0.941206738,0.929139121,0.916038718,0.901937056,0.886868075,
0.870868039,0.853975454,0.836230976,0.81767731,0.798359106,0.778322858,0.757616784,
0.736290719,0.714395985,0.691985276,0.669112527,0.645832783,0.622202072,0.598277264,
0.574115936,0.549776238,0.525316747,0.500796326,0.47627399,0.451808753,0.427459496,
0.403284818,0.379342899,0.355691359,0.332387119,0.309486264,0.287043908,0.265114062,
0.243749504,0.223001649,0.202920432,0.18355418,0.164949501,0.14715117,0.130202021,
0.114142845,0.099012291,0.084846772,0.07168038,0.059544802,0.048469244,0.03848036,
0.029602191,0.021856103,0.015260738,0.009831969,0.00558286,0.002523639,0.000661668,
0.000001426,0.000544506,0.002289597,0.005232501,0.009366135,0.014680552,0.02116296,
0.02879776,0.037566577,0.047448308,0.05841917,0.070452761,0.08352012,0.097589799,
0.112627937,0.128598342,0.14546258,0.163180064,0.181708154,0.20100226,0.221015947,
0.241701051,0.263007789,0.284884883,0.307279683,0.330138292,0.353405699,0.377025906,
0.400942069,0.425096628,0.449431454,0.47388798};

//setup a the correct TC pointer for the corresponding pins. (use table 5-1 as a reference)
//there are multiple TC #'s, choose the one that ties to the specified port utilized


int main (void)
{
	Tc * tc = TC2;
	TcCount8 *tcpointer = &(tc->COUNT8);
	Simple_Clk_Init();

	/* Enable the timer*/
	enable_tc();
	
	while(1)
	{
		//tcpointer->CC[1].reg = sine_table_percent[index++];
		tcpointer->CC[1].reg = 0xF0;
		if(index <= cycles)
			index = 0;
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
	Tc * tc = TC2;
	TcCount8 *tcpointer = &(tc->COUNT8);
	enable_port();
	enable_tc_clocks();
	
	/* Set up CTRLA */
	tcpointer->CTRLA.reg |= (1u << 2); // set counter mode
	tcpointer->CTRLA.reg |= (0x3 << 8); // prescaler set
	tcpointer->CTRLA.reg |= (1u << 12); // PRESCSYNC set to PRESC

	/* Write a suitable value to fix duty cycle and period.*/
	tcpointer->CTRLA.reg |= (1u << 6); // NPWM Chosen in Wavegen
	tcpointer->PER.reg = 0xFF;
	/*Enable TC*/
	tcpointer->CTRLA.reg = (1u << 1); // Enable TC2
	
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
