#include "inc/tm4c123gh6pm.h"
#include <stdio.h>
#include <stdint.h>
#include "driverlib/rom.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_timer.h"
#include "driverlib/systick.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/ssi.h"
#include "ST7735.h"



// pc4 s1
// pc6 s3
// pd6 s2
// pc5 s0


uint32_t time;
uint32_t z=0;
int32_t outValue=0;
volatile uint8_t echowait=0;
volatile uint32_t pulse,Redfreq=0;
const double temp = 1.0/50.0; //since clock is running at 50MHz,one clockpulse corresponds to (1/50)us
unsigned char N;

void drawNum(unsigned int X);
void inputInt();
void Captureinit();
void drawChar(int16_t x, int16_t y, unsigned char c,
                   uint16_t colour, uint16_t bg, uint8_t size);

void drawNum(unsigned int X){
    while(X!=0){
        N=X%10;
        X=X/10;
        N=N+48;
        drawChar(75-z*20, 25,N,ST7735_BLACK,ST7735_WHITE , 2);
        z++;
          }
    z=0;
    SysCtlDelay(2000000);
}




int main(void)
    {
    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);//clock at 50MHz

    SysCtlDelay(10);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    SysCtlDelay(10);

    GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE,GPIO_PIN_4);
    GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE,GPIO_PIN_5);
    GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE,GPIO_PIN_6);
    GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE,GPIO_PIN_6);
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE,GPIO_PIN_4);

/****************Setting sensor to calculate the red value****************/
    GPIOPinWrite(GPIO_PORTC_BASE,GPIO_PIN_6, 0x00);//S3 low
    GPIOPinWrite(GPIO_PORTD_BASE,GPIO_PIN_6, 0x00);//S2 low

    GPIOPinWrite(GPIO_PORTC_BASE,GPIO_PIN_4, 0xFF);//S1 high
    GPIOPinWrite(GPIO_PORTC_BASE,GPIO_PIN_5, 0xFF);//S0 high


    //Interrupt setup
    GPIOIntDisable(GPIO_PORTF_BASE, GPIO_PIN_4);                    // Disable interrupt for PF4 (in case it was enabled)
    GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_4);                      // Clear pending interrupts for PF4
    GPIOIntRegister(GPIO_PORTF_BASE,inputInt);                      //Initialise register
    GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_4,GPIO_BOTH_EDGES);    //Configure it for both edges
    GPIOIntEnable(GPIO_PORTF_BASE,GPIO_PIN_4);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);
    SysCtlDelay(SysCtlClockGet()/3);;
    TimerConfigure(TIMER2_BASE, TIMER_CFG_PERIODIC_UP);
    TimerEnable(TIMER2_BASE,TIMER_A);

    LcdInit();
    Captureinit();

    //
    // Loop forever.
    //
    /* pulse gives the number of clock cycles and one clock cycle corresponds to
     * 0.02us.Inorder to get the time,we need to multiply pulse with 0.02us.
     * To display the frequency, we need to convert
     */

    fillScreen(ST7735_BLACK);
    while (1)
    {
        Redfreq= 500000/(pulse*temp);
        drawNum(Redfreq);
        SysCtlDelay(20000);
    }

}//end of main();

void inputInt(void)
{
          //Clear interrupt flag. Since we only enabled on this is enough
          GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_4);

          /*
            If it's a rising edge then set he timer to 0
            It's in periodic mode so it was in some random value
          */
          if ( GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4) == GPIO_PIN_4){
            HWREG(TIMER2_BASE + TIMER_O_TAV) = 0; //Loads value 0 into the timer.
            TimerEnable(TIMER2_BASE,TIMER_A);
            echowait=1;
          }
          /*
            If it's a falling edge that was detected, then get the value of the counter
          */
          else{
            pulse = TimerValueGet(TIMER2_BASE,TIMER_A); //record value
            TimerDisable(TIMER2_BASE,TIMER_A);
            echowait=0;

          }
}//end of Inputint function

void Captureinit(){
  /*
    Set the timer to be periodic.
  */
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);
  SysCtlDelay(SysCtlClockGet()/3);;
  TimerConfigure(TIMER2_BASE, TIMER_CFG_PERIODIC_UP);
  TimerEnable(TIMER2_BASE,TIMER_A);
}//end of Captureinit function
