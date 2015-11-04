/*
 * Working_Code.c
 *
 *  Created on: Oct 25, 2015
 *      Author: sle
 */

 /*
  * Includes are placed here. Since it is Keil,
	* we can only use one file, so only include
	* is LPC17xx.h
  */
		// Header for lpc17xx based devices
		#include "lpc17xx.h"

 /*
  * Defines are placed here.
  */
		// Time between steps, now obsolete
		#define MAX_TIME 75000
		// How many steps every time dose is called
		#define STEPS_PER_DOSE 50

 /*
  * User-defined typedefs and enums are placed here.
  */
		// bool typedef
		typedef enum {false, true} bool;

 /*
  * Globals are placed here.
  */
		// Stepper Motor Positioning Variables
		uint32_t StepperMotor_CurrentPosition;
		uint32_t StepperMotor_GlobalPosition;

		uint32_t i = 0;
		uint32_t wait = MAX_TIME;

		// Counter to keep track of how much has been injected
		uint32_t BasalDose_DoseAmountCounter;

		// Counter to keep track of how much has been injected
		uint32_t BolusDose_DoseAmountCounter;

 /*
  * Function prototypes are placed here.
  */

		// Stepper Motor Function Prototypes
		void StepperMotor_Initiate(void);

		void StepperMotor_StepForward(void);
		void StepperMotor_StepBackward(void);

		// Basal Dose Function Prototypes
		void BasalDose_DoseTimingInitiate(void);
		void BasalDose_DoseEnableDisable(void);

		void BasalDose_DoseAmountInitiate(void);
		void BasalDose_DoseInject(void);

		// Bolus Dose Function Prototypes
		void BolusDose_DoseInitiate(void);
		void BolusDose_DoseInject(void);



	// Control Function Prototypes
	bool Control_IsSyringeEmpty(void);
	void Control_RetractSyringe(void);

	int main(void)
	{
		SystemInit();

		StepperMotor_Initiate();
		BasalDose_DoseTimingInitiate();
		BolusDose_DoseInitiate();

		while(1);
	}

	void StepperMotor_Initiate(void)
	{
		LPC_GPIO0->FIODIR |= (0x0000000F);
		LPC_GPIO0->FIOPIN &= ~(0x0000000F);

		StepperMotor_CurrentPosition = 0;
		StepperMotor_GlobalPosition = 0;
		BasalDose_DoseAmountCounter = 0;
	}

	void StepperMotor_StepForward(void)
	{
		BasalDose_DoseAmountCounter++;

		switch(StepperMotor_CurrentPosition)
		{
			case 0:
				LPC_GPIO0->FIOPIN = (0x00000003);
				StepperMotor_CurrentPosition += 1;
				break;
			case 1:
				LPC_GPIO0->FIOPIN = (0x00000002);
				StepperMotor_CurrentPosition += 1;
				break;
			case 2:
				LPC_GPIO0->FIOPIN = (0x00000006);
				StepperMotor_CurrentPosition += 1;
				break;
			case 3:
				LPC_GPIO0->FIOPIN = (0x00000004);
				StepperMotor_CurrentPosition += 1;
				break;
			case 4:
				LPC_GPIO0->FIOPIN = (0x0000000C);
				StepperMotor_CurrentPosition += 1;
				break;
			case 5:
				LPC_GPIO0->FIOPIN = (0x00000008);
				StepperMotor_CurrentPosition += 1;
				break;
			case 6:
				LPC_GPIO0->FIOPIN = (0x00000009);
				StepperMotor_CurrentPosition += 1;
				break;
			case 7:
				LPC_GPIO0->FIOPIN = (0x00000001);
				StepperMotor_CurrentPosition = 0;
				break;
		}

		StepperMotor_GlobalPosition += 1;

		if(BasalDose_DoseAmountCounter >= STEPS_PER_DOSE)
		{
			NVIC_DisableIRQ(TIMER1_IRQn);
			BasalDose_DoseAmountCounter = 0;
		}
	}

	void StepperMotor_StepBackward(void)
	{
		switch(StepperMotor_CurrentPosition)
		{
			case 0:
				LPC_GPIO0->FIOPIN = (0x00000009);
				StepperMotor_CurrentPosition = 7;
				break;
			case 1:
				LPC_GPIO0->FIOPIN = (0x00000001);
				StepperMotor_CurrentPosition--;
				break;
			case 2:
				LPC_GPIO0->FIOPIN = (0x00000003);
				StepperMotor_CurrentPosition--;
				break;
			case 3:
				LPC_GPIO0->FIOPIN = (0x00000002);
				StepperMotor_CurrentPosition--;
				break;
			case 4:
				LPC_GPIO0->FIOPIN = (0x00000006);
				StepperMotor_CurrentPosition--;
				break;
			case 5:
				LPC_GPIO0->FIOPIN = (0x00000004);
				StepperMotor_CurrentPosition--;
				break;
			case 6:
				LPC_GPIO0->FIOPIN = (0x0000000C);
				StepperMotor_CurrentPosition--;
				break;
			case 7:
				LPC_GPIO0->FIOPIN = (0x00000008);
				StepperMotor_CurrentPosition--;
				break;
		}
		StepperMotor_GlobalPosition -= 1;
	}

	void BasalDose_DoseTimingInitiate(void)
	{
		LPC_GPIO1->FIODIR |= (0xF0000000);
		LPC_GPIO1->FIOPIN &=~(0xF0000000);

		LPC_SC->PCONP |= 1 << 1; //Power up Timer0
		LPC_SC->PCLKSEL0 |= 1 << 2; // Clock select
		LPC_TIM0->PR = 0x02; // Pre-scalar
		LPC_TIM0->MR0 = 1 << 27; // Match number
		LPC_TIM0->MCR |= 1 << 0; // Interrupt and reset
		LPC_TIM0->MCR |= 1 << 1; // Reset timer on Match 0.
		LPC_TIM0->TCR |= 1 << 1; // Manually Reset Timer0 ( forced )
		LPC_TIM0->TCR &=~(1 << 1); // stop resetting the timer.
		NVIC_EnableIRQ(TIMER0_IRQn); // Enable Timer0 IRQ
		LPC_TIM0->TCR |= 1 << 0; // Reset Timer0

		BasalDose_DoseAmountInitiate();
	}

	void BasalDose_DoseEnable(void)
	{
		LPC_TIM0->IR |= 1 << 0; // Clear MR0 interrupt flag
		LPC_GPIO1->FIOPIN ^= 1 << 29; // Toggle the LED

		NVIC_EnableIRQ(TIMER1_IRQn);
	}

	void BasalDose_DoseAmountInitiate(void)
	{
		LPC_SC->PCONP |= 1 << 2; //Power up Timer1
		LPC_SC->PCLKSEL0 |= 1 << 2; // Clock select
		LPC_TIM1->PR = 0x02; // Pre-scalar
		LPC_TIM1->MR0 = 1 << 17; // Match number
		LPC_TIM1->MCR |= 1 << 0; // Interrupt and reset
		LPC_TIM1->MCR |= 1 << 1; // Reset timer on Match 0.
		LPC_TIM1->TCR |= 1 << 1; // Manually Reset Timer1	( forced )
		LPC_TIM1->TCR &=~(1 << 1); // stop resetting the timer.
		LPC_TIM1->TCR |= 1 << 0; // Reset Timer0
	}

	void BasalDose_DoseInject(void)
	{
		LPC_TIM1->IR |= 1 << 0; // Clear MR0 interrupt flag
		LPC_GPIO1->FIOPIN ^= 1 << 28; // Toggle the LED

		StepperMotor_StepForward();
	}

	void BolusDose_DoseInitiate(void)
	{

		LPC_PINCON->PINSEL4 &=~ (3<<20); // P2.10 is GPIO
		LPC_GPIO2->FIODIR &=~ (1<<10); // P2.10 in input

		LPC_GPIOINT->IO2IntEnF |= (1 << 10); // Enabling falling edge to P2.10

		NVIC_EnableIRQ(EINT3_IRQn);
	}

	void TIMER0_IRQHandler(void)
	{
		if((LPC_TIM0->IR & 0x01) == 0x01) // if MR0 interrupt
    {
			BasalDose_DoseEnable();
    }
	}

	void TIMER1_IRQHandler(void)
	{
		if((LPC_TIM1->IR & 0x01) == 0x01) // if MR0 interrupt
    {
			BasalDose_DoseInject();
			/*
			if(!Control_IsSyringeEmpty())
			{
				BasalDose_DoseInject();
			}
			else
			{
				Control_RetractSyringe();
			}
			*/
    }
	}

	void EINT3_IRQHandler(void)
	{
			LPC_GPIOINT->IO2IntClr |= (1<<10); // Clear the status
			LPC_GPIO1->FIOPIN ^= 1 << 31; // Toggle the LED

			NVIC_EnableIRQ(TIMER1_IRQn);
	}

	void Control_RetractSyringe()
	{
		LPC_TIM1->IR |= 1 << 0; // Clear MR0 interrupt flag
		LPC_GPIO1->FIOPIN ^= 1 << 28; // Toggle the LED

		StepperMotor_StepBackward();
	}

	bool Control_IsSyringeEmpty(void)
	{
		if(StepperMotor_GlobalPosition + 50 >= 200)
			return true;
		else
			return false;
	}