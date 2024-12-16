#include "tm4c123gh6pm.h"
#include "PLL.h"
#include <stdint.h>
#include <stdio.h>
#include "string.h"

void SysLoad(unsigned long period)
{
	NVIC_ST_RELOAD_R = period-1;  // number of counts to wait
  NVIC_ST_CURRENT_R = 0;       // any value written to CURRENT clears
  while((NVIC_ST_CTRL_R&0x00010000)==0){} // wait for count flag
}

void SysFun(void)
{
	NVIC_ST_CTRL_R = 0;               // disable SysTick during setup
	NVIC_ST_CURRENT_R = 0;
  NVIC_ST_CTRL_R = 0x00000005;          
} 

void PortB_Init(void)
{
  volatile unsigned long delay;
  SYSCTL_RCGCGPIO_R |= 0x02;     // activate clock for Port B
  delay = SYSCTL_RCGCGPIO_R;     // allow time for clock to stabilize
  GPIO_PORTB_DIR_R |= 0x74;     // make PB1 and PB4 PB5 PB6 outputs
  GPIO_PORTB_AFSEL_R &= ~0x76;   // disable alternate function on PB1 PB2 PB4 PB5 PB6
  GPIO_PORTB_DEN_R |= 0x76;      // enable digital I/O on PB1 PB2 PB4 PB5 PB6
	GPIO_PORTB_PCTL_R &= ~0x0FFF0FF0; // bits for PB1 PB2 PB4 PB5 PB6
  GPIO_PORTB_AMSEL_R &= ~0x76;   // disable analog functionality on PB1 PB2 PB4 PB5 PB6
}

int i, door_open;
// MAIN: Mandatory for a C Program to be executable
int main(void)
{
	PLL_Init();
	PortB_Init();
	SysFun();
  while(1)
	{
		while(1)
    {
      while((GPIO_PORTB_DATA_R & 0x02) == 0);	// Wait for PIR input on PB1
			SysLoad(80000); // Wait for debounce time (e.g. 1ms at 80MHz)
			GPIO_PORTB_DATA_R &= ~(0x10);
			door_open = 1;
			SysLoad(400000);
			
			GPIO_PORTB_DATA_R |= (0x20);
			SysLoad(4000000);
			GPIO_PORTB_DATA_R |= (0x40);
			SysLoad(4000000);
			for (i = 56000; i <= 120000; i = i + 1000)
			{
				GPIO_PORTB_DATA_R = 0x04;
				SysLoad(i);
				GPIO_PORTB_DATA_R = 0x00;
				SysLoad(1600000-i);
			}
			break;
		}
		
		while(1)
		{
			if((GPIO_PORTB_DATA_R & 0x02) == 0)	// Nobody, close the door
			{
				GPIO_PORTB_DATA_R |= (0x40);
				GPIO_PORTB_DATA_R |= (0x10);
				SysLoad(4000000);
				for (i = 120000; i >= 56000 ; i = i - 1000)
				{
					GPIO_PORTB_DATA_R = 0x04;
					SysLoad(i);
					GPIO_PORTB_DATA_R = 0x00;
					SysLoad(1600000-i);
				}
				
				door_open = 0;
				GPIO_PORTB_DATA_R |= (0x10);
				SysLoad(400000);
				break;
			}
			SysLoad(10000); // wait for 125us at 80MHz
		}
		while((GPIO_PORTB_DATA_R & 0x02) == 0)// Wait for PIR input on PB1
		SysLoad(80000); // Wait for debounce time (e.g. 1ms at 80MHz)
  }
}
