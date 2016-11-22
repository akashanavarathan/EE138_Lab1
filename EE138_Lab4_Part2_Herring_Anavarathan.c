////////////////////////////////////////////////////////////////////////////////////// 
// Lab 4: Part 2: Motor Speed Measurement
// Scott Herring and Akash Anavarathan
//////////////////////////////////////////////////////////////////////////////////////

#include <asf.h>

void Simple_Clk_Init(void);
void enable_adc_clocks(void);
void init_adc(void);
void displayLED(int digit);
void Integer_to_Array(int resultValue);
void overallDisplay(void);
void wait(int t);
void enable_port(void);
void enable_tc_clocks(void);
void enable_tc(void);
void enable_eic(void);
unsigned int read_adc(void);
void initclks(void);
int convert_read(int read_result);

int speed_old = 0;
int speed_new = 0;
int speed = 0;
volatile signed int position_counter = 0;
PortGroup *porta = (PortGroup *)PORT;
Adc *portadc = ADC; // define a pointer to the ADC block
Tc * tc = TC4;
TcCount8 *tcpointer;
// Values used in the Integer_to_Array function to pass into array
int digit1 = 0;
int digit2 = 0;
int digit3 = 0;
int digit4 = 0;
float res_val = 0;	

volatile int result_read = 0; // Variable used to store the value from the RESULT register
volatile int new_read_result = 0;
float display_result = 0; // Variable used to store the final result for display
volatile int eic_overflow = 0;
volatile int display_array[4] = {10,10,10,10}; // Array used to hold values displayed; 10 refers to display nothing


int main (void)
{	
	tcpointer = &(tc->COUNT8);
	Simple_Clk_Init(); // Enable the Clocks for the SAMD20
	initclks();
	enable_eic();
	init_adc(); // Initialize the proper registers for the ADC
	enable_tc();
	
	int counter = 0;
	
	Port *ports = PORT_INSTS;										/* Create pointer to port group */
	PortGroup *porA = &(ports->Group[0]);							/* Assign port group A */
	PortGroup *porB = &(ports->Group[1]);							/* Assign port group B */
	
	// Set the direction of pin to be output to 7-segment LED (active low) (ABCD-EFG(dp)) and PB09 for negative/positive indicator
	porB->DIR.reg = PORT_PB00|PORT_PB01|PORT_PB02|PORT_PB03|PORT_PB04|PORT_PB05|PORT_PB06|PORT_PB07|PORT_PB09;
	porB->OUTSET.reg = PORT_PB00|PORT_PB01|PORT_PB02|PORT_PB03|PORT_PB04|PORT_PB05|PORT_PB06|PORT_PB07|PORT_PB09;
	
	porA->DIRSET.reg = PORT_PA04 | PORT_PA05 | PORT_PA06 | PORT_PA07 | PORT_PA13;  // Set as Outputs
	porA->DIRCLR.reg = PORT_PA16 | PORT_PA17 | PORT_PA18 | PORT_PA19; // Set as Inputs
	
	porA->PINCFG[19].reg = PORT_PINCFG_INEN | PORT_PINCFG_PULLEN;
	porA->PINCFG[18].reg = PORT_PINCFG_INEN | PORT_PINCFG_PULLEN;
	porA->PINCFG[17].reg = PORT_PINCFG_INEN | PORT_PINCFG_PULLEN;
	porA->PINCFG[16].reg = PORT_PINCFG_INEN | PORT_PINCFG_PULLEN;
		
	// A, B, C, and D for Seven-Segment LED
	porA->DIRSET.reg = PORT_PA04|PORT_PA05|PORT_PA06|PORT_PA07;
	porA->OUTSET.reg = PORT_PA04|PORT_PA05|PORT_PA06|PORT_PA07;
	
	porA->OUTSET.reg = PORT_PA13;
	porB->DIRCLR.reg = PORT_PB14;
	porB->PINCFG[14].bit.INEN = 1;
	porB->OUTCLR.reg = PORT_PB14;
	porB->PINCFG[14].bit.PULLEN = 1;
	
	
	while(1)
	{
			result_read = read_adc();
			new_read_result = convert_read(result_read);
			
		if((result_read < 2000) && (result_read > 2200))
		{
			tcpointer->CC[0].reg = (result_read / 17);
			tcpointer->CC[1].reg = (result_read / 17);
		}
		else if(result_read > 2048) 
		{
			tcpointer->CC[0].reg =new_read_result;
			tcpointer->CC[1].reg = 0;
		}
		else
		{
			tcpointer->CC[0].reg = 0;
			tcpointer->CC[1].reg = new_read_result;
		}
			Integer_to_Array((abs)(position_counter)); // Convert display_result into an Array
			overallDisplay(); // Will Display the 4 Digit Number + Decimal Point on the 7-segment Displays
				
	}
}
int convert_read(int read_result)
{
	float temp = 0;
	if (read_result > 0x800)
	{
		temp = (156 * read_result / 4095);
	}
	else
	{
		temp =156 -  (156 * read_result / 2048);
	}
	
	
	return (int)temp;
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

// Display LED Function Used to display the Voltage on the 7-segment Displays
void displayLED(int digit)
{
	Port *ports = PORT_INSTS;
	PortGroup *porB = &(ports->Group[1]);
	porB-> DIRSET.reg = PORT_PB06 | PORT_PB05 | PORT_PB04 | PORT_PB03 | PORT_PB02 | PORT_PB01 | PORT_PB00 | PORT_PB07|PORT_PB09;
	
	porB->OUTSET.reg = PORT_PB07|PORT_PB06|PORT_PB05|PORT_PB04|PORT_PB03|PORT_PB02|PORT_PB01|PORT_PB00 | PORT_PB07;
	
	switch(digit)
	{
		// Display 0
		case 0:
		porB-> OUTSET.reg = PORT_PB06 | PORT_PB07;
		porB-> OUTCLR.reg = PORT_PB00 | PORT_PB02 | PORT_PB01 | PORT_PB03 | PORT_PB05 | PORT_PB04;
		break;
		
		// Display 1
		case 1:
		porB-> OUTCLR.reg = PORT_PB01 | PORT_PB02;
		porB-> OUTSET.reg = PORT_PB00 | PORT_PB05 | PORT_PB06 | PORT_PB04 | PORT_PB03 | PORT_PB07;
		break;
		
		// Display 2
		case 2:
		porB-> OUTSET.reg = PORT_PB05 | PORT_PB02 | PORT_PB07;
		porB-> OUTCLR.reg = PORT_PB00 | PORT_PB01 | PORT_PB06 | PORT_PB04 | PORT_PB03;
		break;
		
		// Display 3
		case 3:
		porB-> OUTSET.reg = PORT_PB05 | PORT_PB04 | PORT_PB07;
		porB-> OUTCLR.reg = PORT_PB00 | PORT_PB01 | PORT_PB02 | PORT_PB03 | PORT_PB06;
		break;
		
		// Display 4
		case 4:
		porB-> OUTSET.reg = PORT_PB00 | PORT_PB04 | PORT_PB03 | PORT_PB07;
		porB-> OUTCLR.reg = PORT_PB01 | PORT_PB02 | PORT_PB05 | PORT_PB06;
		break;
		
		// Display 5
		case 5:
		porB-> OUTSET.reg = PORT_PB00 | PORT_PB04 | PORT_PB01 | PORT_PB07;
		porB-> OUTCLR.reg = PORT_PB00 | PORT_PB02 | PORT_PB03 | PORT_PB05 | PORT_PB06;
		break;
		
		// Display 6
		case 6:
		porB-> OUTSET.reg = PORT_PB01 | PORT_PB07;
		porB-> OUTCLR.reg = PORT_PB00 | PORT_PB02 | PORT_PB03 | PORT_PB04 | PORT_PB05 | PORT_PB06;
		break;
		
		// Display 7
		case 7:
		porB-> OUTSET.reg = PORT_PB03 | PORT_PB04 | PORT_PB05 | PORT_PB06 | PORT_PB07;
		porB-> OUTCLR.reg = PORT_PB00 | PORT_PB02 | PORT_PB01;
		break;
		
		// Display 8
		case 8:
		porB-> OUTSET.reg = PORT_PB07;
		porB-> OUTCLR.reg = PORT_PB00 | PORT_PB02 | PORT_PB01 | PORT_PB03 | PORT_PB04 | PORT_PB05 | PORT_PB06;
		break;
		
		// Display 9
		case 9:
		porB-> OUTSET.reg = PORT_PB04;
		porB-> OUTCLR.reg = PORT_PB00 | PORT_PB02 | PORT_PB01 | PORT_PB03 | PORT_PB05 | PORT_PB06;
		break;
		
		// Display Nothing
		case 10:
		porB-> OUTSET.reg = PORT_PB00 | PORT_PB02 | PORT_PB01 | PORT_PB03 | PORT_PB04 | PORT_PB05 | PORT_PB06;
		break;
		
		// Display the Decimal Point
		case 11:
		porB-> OUTCLR.reg = PORT_PB07;
		porB-> OUTSET.reg = PORT_PB00 | PORT_PB02 | PORT_PB01 | PORT_PB03 | PORT_PB05 | PORT_PB06 | PORT_PB04;
		break;
	}
	
	wait(2);
	
}

// Used to convert the Integer that's passed in, into an Array
void Integer_to_Array(int resultValue)
{
	int newCounter = 0;

	if(resultValue >= 1000 && resultValue <= 9999)
	{
		digit1 = resultValue / 1000;
		newCounter = (resultValue - (digit1*1000));
		digit2 = newCounter / 100;
		newCounter = (newCounter - (digit2*100));
		digit3 = newCounter / 10;
		newCounter = (newCounter - digit3*10);
		digit4 = newCounter;	
		
	}
	else if(resultValue < 1000 && resultValue >= 100)
	{
		digit1 = 0;
		digit2 = resultValue / 100;
		newCounter = (resultValue - (digit2*100));
		digit3 = newCounter / 10;
		newCounter = (newCounter - digit3*10);
		digit4 = newCounter;
	}
	else if(resultValue < 100 && resultValue >= 10)
	{
		digit1 = 0;
		digit2 = 0;
		digit3 = resultValue / 10;
		newCounter = (resultValue - digit3*10);
		digit4 = newCounter;
	}
	else if(resultValue < 10 && resultValue >= 0) 
	{
		digit1 = 0;
		digit2 = 0;
		digit3 = 0;
		digit4 = resultValue;
	}
	
	// Set the Array to hold the proper digits, which will displayed.
	display_array[0] = digit1;
	display_array[1] = digit2;
	display_array[2] = digit3;
	display_array[3] = digit4;

}

// Function used to Display the Overall Digits on the 7-segment display
void overallDisplay(void)
{
	Port *ports = PORT_INSTS;										
	PortGroup *porA = &(ports->Group[0]);
	PortGroup *porB = &(ports->Group[1]);
	
	porA->PINCFG[19].reg = PORT_PINCFG_INEN | PORT_PINCFG_PULLEN;
	porA->PINCFG[18].reg = PORT_PINCFG_INEN | PORT_PINCFG_PULLEN;
	porA->PINCFG[17].reg = PORT_PINCFG_INEN | PORT_PINCFG_PULLEN;
	porA->PINCFG[16].reg = PORT_PINCFG_INEN | PORT_PINCFG_PULLEN;
	
	porA-> OUTSET.reg = PORT_PA07 | PORT_PA06 | PORT_PA05 | PORT_PA04;
	porB-> DIRSET.reg = PORT_PB07|PORT_PB06|PORT_PB05|PORT_PB04|PORT_PB03|PORT_PB02|PORT_PB01|PORT_PB00|PORT_PB09;

	for(int i = 0; i < 4; i++)
	{
		porA -> OUTCLR.reg = 1u << (7-i); // Turn on 7-Segment Display
		displayLED(display_array[i]); // Display Value
		wait(2);
		porA -> OUTSET.reg = 1u << (7-i); // 7-Segment Display
		
		porB->OUTSET.reg = PORT_PB07|PORT_PB06|PORT_PB05|PORT_PB04|PORT_PB03|PORT_PB02|PORT_PB01|PORT_PB00; // Turn off specific segments
	}
	
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

/* Set correct PA pins as TC pins for PWM operation */
void enable_port(void)
{
	Port *ports = PORT_INSTS;
	PortGroup *por = &(ports->Group[0]);

	por->PINCFG[22].bit.PMUXEN = 0x1;		// set to correct pin configuration
	por->PINCFG[23].bit.PMUXEN = 0x1;		// set to correct pin configuration
	por->PMUX[11].bit.PMUXE = 0x5;			// set to correct peripheral F
	por->PMUX[11].bit.PMUXO = 0x5;			// set to correct peripheral F
	
	por->PINCFG[28].bit.PMUXEN = 0x1;
	por->PMUX[14].bit.PMUXE = 0x0; // Peripheral A
	

/* Configure the basic timer/counter to have a period of________ or a frequency of _________  */
}
void enable_tc(void)
{
	enable_port();
	
	/* Set up CTRLA */
	tcpointer->CTRLA.reg = (1u << 2); // set counter mode
	tcpointer->CTRLA.reg |= (0x6 << 8); // prescaler set to DIV64
	tcpointer->CTRLA.reg |= (1u << 12); // PRESCSYNC set to PRESC
	tcpointer->CTRLA.reg |= 0x2 << 5;
	tcpointer->PER.reg = 0x9C;
	
	/*Enable TC*/
	tcpointer->CTRLA.reg |= 1 << 1; // Enable TC4
	
	tcpointer->INTENSET.reg = 0x1; // Enable the Overflow Interrupt
	NVIC->ISER[0] = (1 << 17); // Enables the Interrupt TC4
	
}

void TC4_Handler(void)
{	
	speed_old = (((eic_overflow * 400 + position_counter) * 200) * 60) / 380;
	speed = (0.03093*(speed_old)) + ((0.9691)*speed_new);
	speed_new = speed;
	// position_counter = 0;
	//eic_overflow = 0;
	tcpointer->INTFLAG.bit.OVF = 0x1; // Clear the Overflow Interrupt
}

void enable_eic(void)
{
	EIC->CTRL.reg = 0x0; // Disables EIC
	//EIC->EVCTRL.reg = 0x1 << 8; //Enables EXTINT[8]
	EIC->CONFIG[1].bit.SENSE0 = 0x1; 	//defines the triggering of EXTINT[8]
	EIC->INTENSET.reg = 0x1 << 8; //Set EXTINT[8] and EXINT[14] as a target for interrupt
	
	while(EIC->STATUS.reg & EIC_STATUS_SYNCBUSY) {} //wait for the the EIC to finish sync
	
	EIC->CTRL.reg = 0x2;  //enable EIC
	
	NVIC_EnableIRQ(EIC_IRQn);
}

void EIC_Handler(void)
{
	Port *ports = PORT_INSTS;
	PortGroup *portB = &(ports->Group[1]);
	portB->DIRCLR.reg = PORT_PB14;

	if (EIC->INTFLAG.reg & (0x1 << 8))
	{
		if((portB->IN.reg & PORT_PB14))
		{
			position_counter--;
			
			if (position_counter < -400)
			{
				eic_overflow++;
				position_counter = 0;
			}
		}
		else// if(!(portB->IN.reg & PORT_PB14))
		{
			position_counter++;
			if (position_counter > 400)
			{
				eic_overflow++;
				position_counter = 0;
			}
		}
	}
	if (position_counter > 0)
	{
		portB->OUTSET.reg = PORT_PB14;
	}
	else
	{
		portB->OUTCLR.reg = PORT_PB14;
	}
	EIC->INTFLAG.bit.EXTINT8 = 0x1;
}

void initclks(void)
{
	PM->APBCMASK.reg = (1u << 16)|(1u << 12);
	
	PM->APBAMASK.reg |= (1u << 6);
	
	uint32_t temp = 0x17; // ID for GENCLK is 0x17
	temp |= 0<<8; // Selection Generic clock generator 0
	GCLK->CLKCTRL.reg = temp; // Setup in the CLKCTRL register
	GCLK->CLKCTRL.reg |= 0x1u << 14; // enable it.
	
	while(GCLK->STATUS.bit.SYNCBUSY){};
	
	temp = 0x15;   		// ID for ________ is __________  (see table 14-2)
	temp |= 0<<8;         			//  Selection Generic clock generator 0
	GCLK->CLKCTRL.reg = temp;   		//  Setup in the CLKCTRL register
	GCLK->CLKCTRL.reg |= 0x1u << 14;    	// enable it.
	
	while(GCLK->STATUS.bit.SYNCBUSY){};
	
	temp = 0x03;   		// ID for EIC_GCLK is 0x03  (from table 14-2)
	temp |= 0<<8;         				//  Selection Generic clock generator 0
	GCLK->CLKCTRL.reg = temp;   			//  Setup in the CLKCTRL register
	GCLK->CLKCTRL.reg |= 0x1u << 14;    // enable it.
	
	while(GCLK->STATUS.bit.SYNCBUSY){};
		
}