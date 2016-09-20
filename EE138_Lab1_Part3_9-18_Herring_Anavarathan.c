/* Lab 1 Task 3: Calculator on the GPIO Board
*  Scott Herring and Akash Anavarathan
*  EE 138: Embedded Systems
*  September 14, 2016
*  A: Add, B: Subtract, C: Equals, D: Backspace
*/

//Include header files for all drivers
#include <asf.h>

void keypad_state_machine();
void overallDisplay();
int keypad_scan();
void displayLED(int digit);
void Simple_Clk_Init(void);
void wait(int t);
int Array_to_Integer();
void Integer_to_Array(int finalCounter);

// State Definitions
#define idle 0
#define key_press_debounce 1
#define process_input 2
#define key_press_release 3
#define null_value 25
#define del 11
#define equals 100
#define add 90
#define subtract 91
#define negative 101
#define positive 102

volatile int display_array[4] = {10,10,10,10}; // Array used to hold values displayed; 10 refers to display nothing
volatile int key_press_value = 0; // Variable for storing button press value
volatile int key_press_value_last = 0;
volatile int state = 0;
int debounce_counter = 0;
int key_press_segment = 0; // Variable for which 7-segment display 
int firstValue = 0;
int secondValue = 0;
int result = 0;
int integerValue = 0;
int multiplier = 1;
int operation = 0;
volatile int display_result = 0;
int valueSign = positive;

int digit1 = 0;
int digit2 = 0;
int digit3 = 0;
int digit4 = 0;


int main(void)
{
	Simple_Clk_Init();
	
	Port *ports = PORT_INSTS;										/* Create pointer to port group */
	PortGroup *porA = &(ports->Group[0]);							/* Assign port group A */
	PortGroup *porB = &(ports->Group[1]);							/* Assign port group B */
	
	// Set the direction of pin to be output to 7-segment LED (active low) (ABCD-EFG(dp)) and PB09 for negative/positive indicator
	porB->DIR.reg = PORT_PB00|PORT_PB01|PORT_PB02|PORT_PB03|PORT_PB04|PORT_PB05|PORT_PB06|PORT_PB07|PORT_PB09;
	porB->OUTSET.reg = PORT_PB00|PORT_PB01|PORT_PB02|PORT_PB03|PORT_PB04|PORT_PB05|PORT_PB06|PORT_PB07|PORT_PB09;
	
	porA->DIRSET.reg = PORT_PA04 | PORT_PA05 | PORT_PA06 | PORT_PA07;  // Set as Outputs
	porA->DIRCLR.reg = PORT_PA16 | PORT_PA17 | PORT_PA18 | PORT_PA19; // Set as Inputs
	
	porA->PINCFG[19].reg = PORT_PINCFG_INEN | PORT_PINCFG_PULLEN;
	porA->PINCFG[18].reg = PORT_PINCFG_INEN | PORT_PINCFG_PULLEN;
	porA->PINCFG[17].reg = PORT_PINCFG_INEN | PORT_PINCFG_PULLEN;
	porA->PINCFG[16].reg = PORT_PINCFG_INEN | PORT_PINCFG_PULLEN;
		
	// A, B, C, and D for Seven-Segment LED
	porA->DIRSET.reg = PORT_PA04|PORT_PA05|PORT_PA06|PORT_PA07;
	porA->OUTSET.reg = PORT_PA04|PORT_PA05|PORT_PA06|PORT_PA07;
	
	while(1)
	{
		overallDisplay(); // Display the values in Display_Array
		keypad_state_machine(); // State Machine to Debounce
	}
}

void keypad_state_machine()
{
	Port *ports = PORT_INSTS;										
	PortGroup *porA = &(ports->Group[0]);
	
	porA->DIRSET.reg = PORT_PA04 | PORT_PA05 | PORT_PA06 | PORT_PA07; 
	
	switch(state)
	{
		case idle:
			key_press_value = keypad_scan(); // Check button press value
			if(key_press_value != null_value) // Check to see if anything is pressed
				state = key_press_debounce;
			key_press_value_last = key_press_value;
			debounce_counter = 0;
			
			break;
		
		case key_press_debounce:
			key_press_value = keypad_scan(); // Check button press value
			if(key_press_value == key_press_value_last) // See if it's the same as the old value
			{	
				debounce_counter++;
				if(debounce_counter > 10)
				{
					state = process_input; // Change state
				}
				else
				{
					state = key_press_debounce; // Else, stay in same state
				}
			}
			else
			{
				state = idle;
			}
			
			break;
		
		case process_input:
			if(key_press_value == del)			
			{
				if(key_press_segment > 0)
				{
					key_press_segment--;	
				}
				else if(key_press_segment == 0)
				{
					key_press_segment = 3;
				}
			}
			display_array[key_press_segment] = key_press_value; // Store key press value into the array
			key_press_segment++; // Increment Array Counter
			if(key_press_value == del)			
			{
				if(key_press_segment > 0)
				{
					key_press_segment--;
				}
			}
			
			if(key_press_value == equals)
			{
				secondValue = Array_to_Integer();
				secondValue = secondValue - firstValue;
				
				if(valueSign = negative)
				{
					secondValue = secondValue * -1;
				}
				
				valueSign = positive;
				integerValue = 0;
				
				if (operation == add)
				{
					result = firstValue + secondValue;
				}
				if (operation == subtract)
				{
					result = firstValue - secondValue;
				}
				
				for(int m = 0; m < 4; m++)
				{
					display_array[m] = 10;
				}
				
				key_press_segment = 0;
				firstValue = 0;
				secondValue = 0;
				
				display_result = result;
				result = 0;
				
				if(display_result != abs(display_result))
				{
					display_result = display_result * -1;
					porB->OUTCLR.reg = PORT_PB09;
				}
				
				Integer_to_Array(display_result);
				
			}
			
			
			if(key_press_segment == 4)
				key_press_segment = 0; // Reset Array Counter
			
			debounce_counter = 0;
			state = key_press_release; // Change state
			
			break;
		
		case key_press_release: 
			key_press_value = keypad_scan(); // Check Button Press Value
			while(key_press_value == null_value) // If nothing is being pressed
			{
				debounce_counter++;
				if(debounce_counter >= 10)
				{
					debounce_counter = 0;
					state = idle; // Go back to Idle State
					break;
				}
		
			}
			if(key_press_value != null_value)
				debounce_counter = 0; // Reset the Debounce Counter
			
			break;
			
			
	}	
	
}

void overallDisplay()
{
	Port *ports = PORT_INSTS;										
	PortGroup *porA = &(ports->Group[0]);
	PortGroup *porB = &(ports->Group[1]);
	
	porA-> OUTSET.reg = PORT_PA07 | PORT_PA06 | PORT_PA05 | PORT_PA04;
	porB-> DIRSET.reg = PORT_PB07|PORT_PB06|PORT_PB05|PORT_PB04|PORT_PB03|PORT_PB02|PORT_PB01|PORT_PB00;

	for(int i = 0; i < 4; i++)
	{
		porA -> OUTCLR.reg = 1u << (7-i); // Turn on 7-Segment Display
		displayLED(display_array[i]); // Display Value
		wait(2);
		porA -> OUTSET.reg = 1u << (7-i); // 7-Segment Display
		
		porB->OUTSET.reg = PORT_PB07|PORT_PB06|PORT_PB05|PORT_PB04|PORT_PB03|PORT_PB02|PORT_PB01|PORT_PB00; // Turn off specific segments
	}
	
	
}

int keypad_scan()
{
	Port *ports = PORT_INSTS;
	
	PortGroup *porA = &(ports->Group[0]);
	porA-> DIRSET.reg = PORT_PA07 | PORT_PA06 | PORT_PA05 | PORT_PA04; // Direction Set for the LEDs
	porA-> DIRCLR.reg = PORT_PA19 | PORT_PA18 | PORT_PA17 | PORT_PA16;
	
	porA-> OUTCLR.reg = PORT_PA19 | PORT_PA18 | PORT_PA17 | PORT_PA16;
	
	key_press_value = null_value; // null_value means no key has been pressed
	
	for(int row = 0; row < 4; row++)
	{
		if(row == 0)
		{
			porA->OUTCLR.reg = PORT_PA07;
			porA->OUTSET.reg = PORT_PA06 | PORT_PA05 | PORT_PA04;
			wait(2);
			if(porA->IN.reg & PORT_PA19) // 1 KEY
			{
				key_press_value = 1;
			}
			if(porA->IN.reg & PORT_PA18) // 2 KEY
			{
				key_press_value = 2;
			}
			if(porA->IN.reg & PORT_PA17) // 3 KEY
			{
				key_press_value = 3;
			}
			if(porA->IN.reg & PORT_PA16) // A KEY
			{
				operation = add;
				firstValue = Array_to_Integer();
				if(valueSign = negative)
				{
					firstValue = firstValue * -1;
				}
				porB->OUTSET.reg = PORT_PB09;
				valueSign = positive;
				key_press_value = 10;
				for(int l = 0; l < 4; l++)
				{
					display_array[l] = 10;
				}
				key_press_segment = 0;
				valueSign = positive;
			}
		}
		if(row == 1)
		{
			porA->OUTCLR.reg = PORT_PA06;
			porA->OUTSET.reg = PORT_PA07 | PORT_PA04 | PORT_PA05;
			wait(2);
			if(porA->IN.reg & PORT_PA19) // 4 KEY
			{
				key_press_value = 4;
			}
			if(porA->IN.reg & PORT_PA18) // 5 KEY
			{
				key_press_value = 5;
			}
			if(porA->IN.reg & PORT_PA17) // 6 KEY
			{
				key_press_value = 6;
			}
			if(porA->IN.reg & PORT_PA16) // B KEY
			{
				operation = subtract;
				firstValue = Array_to_Integer();
				key_press_value = 10;
				porB->OUTSET.reg = PORT_PB09;
				valueSign = positive;
				if(valueSign = negative)
				{
					firstValue = firstValue * -1;
				}
				for(int l = 0; l < 4; l++)
				{
					display_array[l] = 10;
				}
				key_press_segment = 0;
				valueSign = positive;
			}
		}
		if(row == 2)
		{
			porA->OUTCLR.reg = PORT_PA05;
			porA->OUTSET.reg = PORT_PA07 | PORT_PA06 | PORT_PA04;
			wait(2);
			if(porA->IN.reg & PORT_PA19) // 7 KEY
			{
				key_press_value = 7;
			}
			if(porA->IN.reg & PORT_PA18) // 8 KEY
			{
				key_press_value = 8;
			}
			if(porA->IN.reg & PORT_PA17) // 9 KEY
			{
				key_press_value = 9;
			}
			if(porA->IN.reg & PORT_PA16) // C KEY
			{
				key_press_value = equals;
				porB->OUTSET.reg = PORT_PB09;
				
			}
		}
		if(row == 3)
		{
			porA->OUTCLR.reg = PORT_PA04;
			porA->OUTSET.reg = PORT_PA07 | PORT_PA06 | PORT_PA05;
			wait(2);
			if(porA->IN.reg & PORT_PA19) // * KEY
			{
				key_press_value = negative;
				valueSign = negative;
				porB->OUTCLR.reg = PORT_PB09;
			}
			if(porA->IN.reg & PORT_PA18) // 0 KEY
			{
				key_press_value = 0;
			}
			if(porA->IN.reg & PORT_PA16) // D KEY
			{
				key_press_value = del;
				porB->OUTSET.reg = PORT_PB09;
			}
		}
	}	
	
	
	
	return key_press_value;	
}

void displayLED(int digit)
{
	Port *ports = PORT_INSTS;
	PortGroup *porB = &(ports->Group[1]);
	porB-> DIRSET.reg = PORT_PB06 | PORT_PB05 | PORT_PB04 | PORT_PB03 | PORT_PB02 | PORT_PB01 | PORT_PB00;
	
	porB->OUTSET.reg = PORT_PB07|PORT_PB06|PORT_PB05|PORT_PB04|PORT_PB03|PORT_PB02|PORT_PB01|PORT_PB00;
	
	switch(digit)
	{
		// Display 0
		case 0:
		porB-> OUTSET.reg = PORT_PB06;
		porB-> OUTCLR.reg = PORT_PB00 | PORT_PB02 | PORT_PB01 | PORT_PB03 | PORT_PB05 | PORT_PB04;
		break;
		
		// Display 1
		case 1:
		porB-> OUTCLR.reg = PORT_PB01 | PORT_PB02;
		porB-> OUTSET.reg = PORT_PB00 | PORT_PB05 | PORT_PB06 | PORT_PB04 | PORT_PB03;
		break;
		
		// Display 2
		case 2:
		porB-> OUTSET.reg = PORT_PB05 | PORT_PB02;
		porB-> OUTCLR.reg = PORT_PB00 | PORT_PB01 | PORT_PB06 | PORT_PB04 | PORT_PB03;
		break;
		
		// Display 3
		case 3:
		porB-> OUTSET.reg = PORT_PB05 | PORT_PB04;
		porB-> OUTCLR.reg = PORT_PB00 | PORT_PB01 | PORT_PB02 | PORT_PB03 | PORT_PB06;
		break;
		
		// Display 4
		case 4:
		porB-> OUTSET.reg = PORT_PB00 | PORT_PB04 | PORT_PB03;
		porB-> OUTCLR.reg = PORT_PB01 | PORT_PB02 | PORT_PB05 | PORT_PB06;
		break;
		
		// Display 5
		case 5:
		porB-> OUTSET.reg = PORT_PB00 | PORT_PB04| PORT_PB01;
		porB-> OUTCLR.reg = PORT_PB00 | PORT_PB02 | PORT_PB03 | PORT_PB05 | PORT_PB06;
		break;
		
		// Display 6
		case 6:
		porB-> OUTSET.reg = PORT_PB01;
		porB-> OUTCLR.reg = PORT_PB00 | PORT_PB02 | PORT_PB03 | PORT_PB04 | PORT_PB05 | PORT_PB06;
		break;
		
		// Display 7
		case 7:
		porB-> OUTSET.reg = PORT_PB03 | PORT_PB04 | PORT_PB05 | PORT_PB06;
		porB-> OUTCLR.reg = PORT_PB00 | PORT_PB02 | PORT_PB01;
		break;
		
		// Display 8
		case 8:
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
		
		// Display Nothing and indicate Deleted Value
		case 11:
		porB-> OUTSET.reg = PORT_PB00 | PORT_PB02 | PORT_PB01 | PORT_PB03 | PORT_PB04 | PORT_PB05 | PORT_PB06;
	}
	
	wait(2);
	
}

int Array_to_Integer()
{
	
	
	multiplier = 1;
	for(int k = 3; k >= 0; k--)
	{
		if(display_array[k] < 10)
		{
		integerValue += multiplier * display_array[k];
		multiplier = multiplier * 10;
		}
	}
	
	return integerValue;
}


void Integer_to_Array(int resultValue)
{
	
	int newCounter = 0;
	
	if(resultValue >= 1000)
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
	
	display_array[0] = digit1;
	display_array[1] = digit2;
	display_array[2] = digit3;
	display_array[3] = digit4;

}

// Simple clock initialization
void Simple_Clk_Init(void)
{
	/* Various bits in the INTFLAG register can be set to one at startup.
	   This will ensure that these bits are cleared */
	
	SYSCTRL->INTFLAG.reg = SYSCTRL_INTFLAG_BOD33RDY | SYSCTRL_INTFLAG_BOD33DET |
			SYSCTRL_INTFLAG_DFLLRDY;
			
	//system_flash_set_waitstates(0);  //Clock_flash wait state =0

	SYSCTRL_OSC8M_Type temp = SYSCTRL->OSC8M;      /* for OSC8M initialization  */

	temp.bit.PRESC    = 0;    // no divide, i.e., set clock=8Mhz  (see page 170)
	temp.bit.ONDEMAND = 1;    //  On-demand is true
	temp.bit.RUNSTDBY = 0;    //  Standby is false
	
	SYSCTRL->OSC8M = temp;

	SYSCTRL->OSC8M.reg |= 0x1u << 1;  //SYSCTRL_OSC8M_ENABLE bit = bit-1 (page 170)
	
	PM->CPUSEL.reg = (uint32_t)0;		// CPU and BUS clocks Divide by 1  (see page 110)
	PM->APBASEL.reg = (uint32_t)0;		// APBA clock 0= Divide by 1  (see page 110)
	PM->APBBSEL.reg = (uint32_t)0;		// APBB clock 0= Divide by 1  (see page 110)
	PM->APBCSEL.reg = (uint32_t)0;		// APBB clock 0= Divide by 1  (see page 110)

	PM->APBAMASK.reg |= 01u<<3;   // Enable Generic clock controller clock (page 127)

	/* Software reset Generic clock to ensure it is re-initialized correctly */

	GCLK->CTRL.reg = 0x1u << 0;   // Reset gen. clock (see page 94)
	while (GCLK->CTRL.reg & 0x1u ) {  /* Wait for reset to complete */ }
	
	// Initialization and enable generic clock #0
	while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY){}
	*((uint8_t*)&GCLK->GENDIV.reg) = 0;  // Select GCLK0 (page 104, Table 14-10)
	while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY){}
	GCLK->GENDIV.reg  = 0x0100;   		 // Divide by 1 for GCLK #0 (page 104)
	while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY){}
	GCLK->GENCTRL.reg = 0x030600;  		 // GCLK#0 enable, Source=6(OSC8M), IDC=1 (page 101)
}

void wait(int t)
{
	volatile int count = 0;
	while (count < t*100)
	{
		count++;
	}
}