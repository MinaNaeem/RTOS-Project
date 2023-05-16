#include <stdio.h>
#include "FreeRTOS.h"       // FreeRTOS library
#include "task.h"           // Task related definitions
#include "queue.h"          // Queue related definitions
#include "semphr.h"         // Semaphore related definitions
#include "tm4c123gh6pm.h"   // Tiva C Series TM4C123GH6PM microcontroller header file
#include <stdbool.h>
#include <stdint.h>

#define LED_RED     0x02  // Red LED pin
#define LED_BLUE		0x04  
#define LED_GREEN   0x08  // Green LED pin

#define DriverUp    (1U << 0)		//Driver Up button
#define DriverDown    (1U << 4)		//Driver Down button

#define PassengerUp    (1U << 0)		//Passenger Up button
#define PassengerDown  (1U << 4)		//Passenger Down button

#define EmergencyButton  (1U << 0)		//Driver Up button

#define Lock        (1U << 0)    // Latching switch 


#define UpperLimit  (1U << 5)   //UpperLimit is on PB5
#define LowerLimit  (1U << 6)		//LowerLimit is on PB6

volatile uint32_t counter = 0; 

volatile bool isUp = false;
volatile bool isLow = false;



volatile bool Locked = false;


volatile uint32_t param = 0;
volatile uint32_t param2 = 0;
SemaphoreHandle_t xSemaphore1;   
SemaphoreHandle_t xSemaphore2; 
SemaphoreHandle_t xSemaphore3;   


void portB_init(void)
{
	
		GPIOB-> LOCK = 0x4C4F434B;
		GPIOB-> CR |= 0xFF;
	// initialize the 2 interrupts of the driver buttons and the 2 limit switches
	  GPIOB->DIR &= ~(DriverUp | DriverDown | UpperLimit | LowerLimit );
    GPIOB->DEN |= DriverUp | DriverDown| UpperLimit | LowerLimit ;
    GPIOB->PUR |= DriverUp | DriverDown | UpperLimit | LowerLimit;
 // Enable interrupts on Port B
    GPIOB->IM |= DriverUp | DriverDown;

// Set interrupt sense to be edge-triggered
		GPIOB->IS &= ~(DriverUp | DriverDown);
		GPIOB->IEV &= ~(DriverUp | DriverDown);



		NVIC_EnableIRQ(GPIOB_IRQn);
		NVIC_SetPriority(GPIOB_IRQn, 3);
}
void portF_init(void)
{
		GPIOF-> LOCK = 0x4C4F434B;
		GPIOF-> CR |= 0xFF;
	//enable the leds
		GPIOF->AFSEL &= ~(LED_BLUE | LED_GREEN | LED_RED);
		GPIOF->AMSEL  &= ~(LED_BLUE | LED_GREEN | LED_RED);
		GPIOF -> DIR |= LED_BLUE | LED_GREEN | LED_RED;
		GPIOF-> DEN |=  LED_BLUE | LED_GREEN | LED_RED;
		GPIOF -> DATA &= ~(LED_BLUE | LED_GREEN | LED_RED);
	
	  GPIOF->DIR &= ~(PassengerUp | PassengerDown);
    GPIOF->DEN |= PassengerUp | PassengerDown ;
    GPIOF->PUR |= PassengerUp | PassengerDown;
 // Enable interrupts on Port B
    GPIOF->IM |= PassengerUp | PassengerDown;

// Set interrupt sense to be edge-triggered
		GPIOF->IS &= ~(PassengerUp | PassengerDown);
		GPIOF->IEV &= ~(PassengerUp | PassengerDown);

		NVIC_EnableIRQ(GPIOF_IRQn);
		NVIC_SetPriority(GPIOF_IRQn, 4);
}
void portE_init(void)
{
		GPIOE-> LOCK = 0x4C4F434B;
		GPIOE-> CR |= 0xFF;
	  GPIOE->DIR &= ~(Lock );
    GPIOE->DEN |= Lock ;
    GPIOE->PUR |= Lock;

    GPIOE->IM |= Lock;



		GPIOE->IBE |= Lock;
		GPIOE->IEV |= Lock;

		NVIC_EnableIRQ(GPIOE_IRQn);
		NVIC_SetPriority(GPIOE_IRQn, 2);
}
void portD_init(void)
{
	
		GPIOD-> LOCK = 0x4C4F434B;
		GPIOD ->CR |= 0xFF;
	  GPIOD->DIR &= ~0x01;
    GPIOD->DEN |= 0x01 ;
    GPIOD->PUR |= 0x01;
 
    GPIOD->IM |= 0x01;

// Set interrupt sense to be edge-triggered
		GPIOF->IS &= ~(0x01);
		GPIOF->IEV &= ~0x01;

		NVIC_EnableIRQ(GPIOD_IRQn);
		NVIC_SetPriority(GPIOD_IRQn, 1);




}




void vLimits(void* pvParameters)
{
    while (1)
    {
		
				if((GPIOE -> DATA & 0x01) == 1 )
			{
					Locked = false ;
			}else Locked = true;
			if ((GPIOB -> DATA & UpperLimit) == 0)
			{	
				isUp = true;
				isLow = false;
			}
			if ((GPIOB -> DATA & LowerLimit) == 0)
			{	
				isUp = false;
				isLow = true;
			}
			if ((GPIOB -> DATA & UpperLimit) != 0 & (GPIOB -> DATA & LowerLimit) != 0)
			{	
				isUp = false;
				isLow = false;
			}
		
   }
}



void vDriverButtons(void* param) // Task 1 code
{
				

    xSemaphoreTake(xSemaphore1, 0);
    while (1)
    {
        // Wait for the semaphore to be released
        xSemaphoreTake(xSemaphore1, portMAX_DELAY);
				NVIC_DisableIRQ(GPIOB_IRQn);
				uint32_t button_id = (uint32_t) param; 
			  uint32_t flag=2;//flag 1 means up 0 down
        uint32_t i2;
        counter = 0;
        while ((GPIOB -> DATA & DriverUp)== 0 | (GPIOB -> DATA & DriverDown)== 0 | counter < 1 )
        {
					if ((GPIOB -> DATA & DriverUp)== 0){flag=1;}
					if ((GPIOB -> DATA & DriverDown)== 0){flag=0;}
            counter++;
					
					    if(counter > 0x34FFF)
						{//using pulling go up or down
            if (flag == 1)  //UP
            {
                if ((GPIOB->DATA & 0x20) == false)   
                { 
                    GPIOF -> DATA |= LED_BLUE;
                    GPIOF -> DATA &= ~LED_GREEN;
                    GPIOF -> DATA &= ~LED_RED;
									break;
                }
                else
                {
                    while((GPIOB-> DATA & 0x20))
                    { 
                        GPIOF -> DATA |= LED_GREEN;
                        GPIOF -> DATA &= ~LED_BLUE;
                        GPIOF -> DATA &= ~LED_RED;
												if ((GPIOB -> DATA & DriverUp)== 1) break;
                        // motor configure to go up in some direction;
                    }
                    GPIOF -> DATA |= LED_BLUE;
                    GPIOF -> DATA &= ~LED_GREEN;
                    GPIOF -> DATA &= ~LED_RED;
										break;
                    isUp = true ;
                    isLow = false;
                }
            }
            else if(flag == 0)     // Down
            {
                if ((GPIOB->DATA & 0x40) == false)   
                {
                    GPIOF -> DATA |= LED_BLUE;
                    GPIOF -> DATA &= ~LED_GREEN;
                    GPIOF -> DATA &= ~LED_RED;
                }			
                else
                {
                    while((GPIOB->DATA & 0x40)&((GPIOB -> DATA & (1U<<4))== 0 ))
                    {
                        GPIOF -> DATA |= LED_RED;
                        GPIOF -> DATA &= ~LED_GREEN;
                        GPIOF -> DATA &= ~LED_BLUE;
                        
									
								        
                    }
                    GPIOF -> DATA |= LED_BLUE;
                    GPIOF -> DATA &= ~LED_GREEN;
                    GPIOF -> DATA &= ~LED_RED;
                    isLow = true ;
                    isUp = false;
                }
            }
        }
        }
    
       if(counter < 0x34FFF & counter > 100)
        {//if button is held for a short period
            if (flag==1)  //UP
            {
                if ((GPIOB->DATA & 0x20) == false)   
                { 
                    GPIOF -> DATA |= LED_BLUE;
                    GPIOF -> DATA &= ~LED_GREEN;
                    GPIOF -> DATA &= ~LED_RED;
                }
                else
                {
                    while((GPIOB-> DATA & 0x20))
                    { 
                        GPIOF -> DATA |= LED_GREEN;
                        GPIOF -> DATA &= ~LED_BLUE;
                        GPIOF -> DATA &= ~LED_RED;	
                        // motor configure to go up in some direction
                    }
                    GPIOF -> DATA |= LED_BLUE;
                    GPIOF -> DATA &= ~LED_GREEN;
                    GPIOF -> DATA &= ~LED_RED;
                    isUp = true ;
                    isLow = false;
                }
            }
            else if(flag==0)     // Down
            {
                if ((GPIOB->DATA & 0x40) == false)   
                {
                    GPIOF -> DATA |= LED_BLUE;
                    GPIOF -> DATA &= ~LED_GREEN;
                    GPIOF -> DATA &= ~LED_RED;
                }			
                else
                {
                    while((GPIOB->DATA & 0x40))
                    {
                        GPIOF -> DATA |= LED_RED;
                        GPIOF -> DATA &= ~LED_GREEN;
                        GPIOF -> DATA &= ~LED_BLUE;
                        // motor configure to go to the other direction
                    }
                    GPIOF -> DATA |= LED_BLUE;
                    GPIOF -> DATA &= ~LED_GREEN;
                    GPIOF -> DATA &= ~LED_RED;
                    isLow = true ;
                    isUp = false;
                }
            }
        }
        else
        {//in task but button not used
            GPIOF -> DATA |= LED_BLUE;
            GPIOF -> DATA &= ~LED_GREEN;
            GPIOF -> DATA &= ~LED_RED; 
        } 
				NVIC_EnableIRQ(GPIOB_IRQn);
				NVIC_SetPriority(GPIOB_IRQn, 3);
    }
}




void vPassengerButtons(void* param2) 
{
			xSemaphoreTake(xSemaphore2, 0);
			
    while (1)
    {
        // Wait for the semaphore to be released
      xSemaphoreTake(xSemaphore2, portMAX_DELAY);	
			uint32_t button_id2 = (uint32_t) param2; 
			NVIC_DisableIRQ(GPIOF_IRQn);
			if (Locked == true);
			else 
			{
				uint32_t flag=2;//flag 1 means up 0 down
        uint32_t i2;
        counter = 0;
        while ((GPIOF -> DATA & PassengerUp)== 0 | (GPIOF -> DATA & PassengerDown)== 0 | counter < 1 )
        {
					if ((GPIOF -> DATA & PassengerUp)== 0){flag=1;}
					if ((GPIOF -> DATA & PassengerDown)== 0){flag=0;}
            counter++;
					
					    if(counter > 0x3FFFF)
						{//using pulling go up or down
            if (flag == 1)  //UP
            {
                if ((GPIOB->DATA & 0x20) == false)   
                { 
                    GPIOF -> DATA |= LED_BLUE;
                    GPIOF -> DATA &= ~LED_GREEN;
                    GPIOF -> DATA &= ~LED_RED;
									break;
                }
                else
                {
                    while((GPIOB-> DATA & 0x20))
                    { 
                        GPIOF -> DATA |= LED_GREEN;
                        GPIOF -> DATA &= ~LED_BLUE;
                        GPIOF -> DATA &= ~LED_RED;
												if ((GPIOF -> DATA & PassengerUp)== 1) break;
                        // motor configure to go up in some direction
                    }
                    GPIOF -> DATA |= LED_BLUE;
                    GPIOF -> DATA &= ~LED_GREEN;
                    GPIOF -> DATA &= ~LED_RED;
										break;
                    isUp = true ;
                    isLow = false;
                }
            }
            else if(flag == 0)     // Down
            {
                if ((GPIOB->DATA & 0x40) == false)   
                {
                    GPIOF -> DATA |= LED_BLUE;
                    GPIOF -> DATA &= ~LED_GREEN;
                    GPIOF -> DATA &= ~LED_RED;
                }			
                else
                {
                    while((GPIOB->DATA & 0x40))
                    {
                        GPIOF -> DATA |= LED_RED;
                        GPIOF -> DATA &= ~LED_GREEN;
                        GPIOF -> DATA &= ~LED_BLUE;
                        // motor configure to go to the other direction
											if ((GPIOF -> DATA & PassengerDown)== 1) break;
                    }
                    GPIOF -> DATA |= LED_BLUE;
                    GPIOF -> DATA &= ~LED_GREEN;
                    GPIOF -> DATA &= ~LED_RED;
                    isLow = true ;
                    isUp = false;
                }
            }
        }
        }
    
       if(counter < 0x34FFF & counter > 100)
        {//if button is held for a short period
            if (flag==1)  //UP
            {
                if ((GPIOB->DATA & 0x20) == false)   
                { 
                    GPIOF -> DATA |= LED_BLUE;
                    GPIOF -> DATA &= ~LED_GREEN;
                    GPIOF -> DATA &= ~LED_RED;
                }
                else
                {
                    while((GPIOB-> DATA & 0x20))
                    { 
                        GPIOF -> DATA |= LED_GREEN;
                        GPIOF -> DATA &= ~LED_BLUE;
                        GPIOF -> DATA &= ~LED_RED;	
                        // motor configure to go up in some direction
                    }
                    GPIOF -> DATA |= LED_BLUE;
                    GPIOF -> DATA &= ~LED_GREEN;
                    GPIOF -> DATA &= ~LED_RED;
                    isUp = true ;
                    isLow = false;
                }
            }
            else if(flag==0)     // Down
            {
                if ((GPIOB->DATA & 0x40) == false)   
                {
                    GPIOF -> DATA |= LED_BLUE;
                    GPIOF -> DATA &= ~LED_GREEN;
                    GPIOF -> DATA &= ~LED_RED;
                }			
                else
                {
                    while((GPIOB->DATA & 0x40))
                    {
                        GPIOF -> DATA |= LED_RED;
                        GPIOF -> DATA &= ~LED_GREEN;
                        GPIOF -> DATA &= ~LED_BLUE;
                        // motor configure to go to the other direction
                    }
                    GPIOF -> DATA |= LED_BLUE;
                    GPIOF -> DATA &= ~LED_GREEN;
                    GPIOF -> DATA &= ~LED_RED;
                    isLow = true ;
                    isUp = false;
                }
            }
        }
        else
        {//in task but button not used
            GPIOF -> DATA |= LED_BLUE;
            GPIOF -> DATA &= ~LED_GREEN;
            GPIOF -> DATA &= ~LED_RED; 
        } 
		
	
			}
		
      NVIC_EnableIRQ(GPIOF_IRQn);
			NVIC_SetPriority(GPIOF_IRQn, 4);
    }
}
void vEmergency(void* pvParameters) 
{
			xSemaphoreTake(xSemaphore3, 0);
  while(1){  
	for(int i=0;i<0x34fff;i++){
        // Wait for the semaphore to be released
        xSemaphoreTake(xSemaphore3, portMAX_DELAY);
		                GPIOF -> DATA |= LED_BLUE;
                    GPIOF -> DATA &= ~LED_GREEN;
                    GPIOF -> DATA &= ~LED_RED;
	}}
}


int main()  // Main function
{
    // Enable the GPIO ports
    SYSCTL->RCGCGPIO |= (1U << 0 )|(1U << 1) | (1U << 2) | (1U << 3) |(1U << 4) | (1U << 5) ;
		portB_init();
		portF_init();
		portE_init();
		portD_init();
		
	 
  	
						

    
			__enable_irq();
    // Create a semaphore
    xSemaphore1 = xSemaphoreCreateBinary();
	xSemaphore2 = xSemaphoreCreateBinary();
	xSemaphore3 = xSemaphoreCreateBinary();
	
	if( xSemaphore1 != NULL & xSemaphore2 != NULL & xSemaphore3 != NULL)
		{
		
		xTaskCreate(vLimits, "Reading Inputs", 400, NULL, 1, NULL);
        xTaskCreate(vDriverButtons, "Driver Task", 400,(void*) &param, 3, NULL);
        xTaskCreate(vPassengerButtons, "Passenger Task", 400,(void*) &param2, 2, NULL);
		xTaskCreate(vEmergency, "Overload", 400, NULL, 4, NULL);


			
			
    // Start the FreeRTOS scheduler
    vTaskStartScheduler();
		}
 
		for(;;) ;
    // We should never get here
 
}

void GPIOB_Handler(void)    // Port B interrupt handler function
{ 
				NVIC_EnableIRQ(GPIOF_IRQn);
				NVIC_SetPriority(GPIOF_IRQn, 4);
		counter = 0;
		portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

   
    // Check which button was pressed
    if (GPIOB->MIS & DriverUp| GPIOB->MIS & DriverDown)  //Driver up or down button
    {
       
				
			if ((GPIOB -> DATA & UpperLimit) == 0)
			{	
				isUp = true;
				isLow = false;
			}
			else if ((GPIOB -> DATA & LowerLimit) == 0)
			{	
				isUp = false;
				isLow = true;
			}
			else
			{	
				isUp = false;
				isLow = false;
			}
			if (GPIOB->MIS & DriverUp) *(&param) = 0x3;
			else *(&param) = 0x5;
      
			
    }
xSemaphoreGiveFromISR(xSemaphore1, &xHigherPriorityTaskWoken );
		
	
	portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );		
		GPIOB->ICR = 0x11;      
		GPIOF->ICR = 0x11;	
     uint32_t i;
		i= GPIOB->ICR ;
		i= GPIOF->ICR ;

}		
void GPIOD_Handler(void)
 {
		// Clear the interrupt flag
	  NVIC_EnableIRQ(GPIOF_IRQn);
					NVIC_SetPriority(GPIOF_IRQn, 4);
		NVIC_EnableIRQ(GPIOB_IRQn);
					NVIC_SetPriority(GPIOB_IRQn, 3);
	 NVIC_EnableIRQ(GPIOE_IRQn);
					NVIC_SetPriority(GPIOE_IRQn, 2);
		GPIOD->ICR = 0x11;      
			
     uint32_t i;
		i= GPIOD->ICR ;

 
		portBASE_TYPE xHigherPriorityTaskWoken3 = pdFALSE;

		xSemaphoreGiveFromISR(xSemaphore3, &xHigherPriorityTaskWoken3 );	
            


	portEND_SWITCHING_ISR( xHigherPriorityTaskWoken3 );
 
 
 }
void GPIOF_Handler(void)
{
	// Clear the interrupt flag

	
		portBASE_TYPE xHigherPriorityTaskWoken2 = pdFALSE;

   if (GPIOF->MIS & 0x11 )   //Passenger up or down
    {
			
			if ((GPIOB -> DATA & UpperLimit) == 0)
			{	
				isUp = true;
				isLow = false;
			}
			else if ((GPIOB -> DATA & LowerLimit) == 0)
			{	
				isUp = false;
				isLow = true;
			}
			else
			{	
				isUp = false;
				isLow = false;
			}
			
			if (GPIOF->MIS & PassengerUp) *(&param2) = 0x9;
			else *(&param2) = 0x15;
     
    }
		xSemaphoreGiveFromISR(xSemaphore2, &xHigherPriorityTaskWoken2 );	
              

	/* Giving the semaphore may have unblocked a task - if it did and the
	unblocked task has a priority equal to or above the currently executing
	task then xHigherPriorityTaskWoken will have been set to pdTRUE and
	portEND_SWITCHING_ISR() will force a context switch to the newly unblocked
	higher priority task.
	NOTE: The syntax for forcing a context switch within an ISR varies between
	FreeRTOS ports. The portEND_SWITCHING_ISR() macro is provided as part of
	the Corte M3 port layer for this purpose. taskYIELD() must never be called
	from an ISR! */
	portEND_SWITCHING_ISR( xHigherPriorityTaskWoken2 );
		GPIOF->ICR = 0x11;      		
     uint32_t i;
		i= GPIOF->ICR ;
}	
		
void GPIOE_Handler(void)
{
	// Clear the interrupt flag
	      NVIC_EnableIRQ(GPIOF_IRQn);
					NVIC_SetPriority(GPIOF_IRQn, 4);
		NVIC_EnableIRQ(GPIOB_IRQn);
					NVIC_SetPriority(GPIOB_IRQn, 3);
		      	if (GPIOE -> MIS & 0x01 )    // Lock button is pressed
    {
				if (GPIOE -> IEV & 0x01) 
			{
						Locked = false;
				
      }
        else 
				{  // Falling edge occurred
            Locked = true; 
					
        }
			}
				
		GPIOE->ICR = 0x11;
	
     uint32_t i;
		i= GPIOE->ICR ;


	
}