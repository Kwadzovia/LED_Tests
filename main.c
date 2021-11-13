#include <msp430.h> 
#include "ioMapping.h"
#include "i2c_device.h"
#include <stdlib.h>
#include <stdio.h>
#define ARRAY_SIZE(array) \
    (sizeof(array) / sizeof(array[0]))
//#include <stdint.h>
//#include <stddef.h>
//#include <stdio.h>
//#include "Arrays.h"

/**
 * main.c
 */

//Variable Definition
#define YELLOW_LED 0
#define GREEN_LED 2
volatile int ledCount = 1;
volatile int countDir = 1;


void i2c_init();
unsigned int i2c_receive();
void i2c_transmit(unsigned int address,unsigned int data[],unsigned int numBytes);

static int health_read(uint8_t address);
static int health_write();
int main(void)
//Clean up eventually
{
    //------------ Setup ------------
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	//IO Initialization (PxDIR, PxREN, PxOUT, PxIES)
	P2DIR |= 0x00;
	P4DIR |= 0x00;
	P5DIR |= 0x00;
	P6DIR |= 0x00;
    PADIR |= 0x00;
    PBDIR |= 0x00;
    PCDIR |= 0x00;


    P1OUT |= 0x00;
    P2OUT |= 0x00;
    P4OUT |= 0x00;
    P5OUT |= 0x00;
    P6OUT |= 0x00;
    PAOUT |= 0x00;
    PBOUT |= 0x00;
    PCOUT |= 0x00;

    //P1DIR |= BIT6;
    //P1REN |= 0x00;
    //P1OUT |= 0x00;
    //P1IES |= BIT6;

	P3DIR |= 0x05;
	P3REN |= 0x00;
	P3OUT |= 0x00;
	P3IES |= 0x00;

	i2c_init();

    PM5CTL0 &= ~(LOCKLPM5);

	//Initialize clock (24Mhz)
	//Clear clock and dividers
	TB0CTL |= TBCLR;

	//Initialize TBxCCRn
	TB0CCR0 |= 165;
	TB0CCR1 |= ledCount;
	//Configure TBxIV, TBIDEX, TBxCCTLn
	TB0CCTL0 = CCIE;
	TB0CCTL1 = CCIE;
	//TB0CCTL1 = CCIE+OUTMOD_7;
	//Configure TBxCTL, and MC bits
	TB0CTL = TBSSEL__ACLK+MC_1;

	__bis_SR_register(GIE);


	//uint8_t temp[2] = {0x09,0x02};

	while(1){
	    //P3OUT |= 0x05;
	    health_write();
	    health_read(0x07);
	}

	return 0;
}


static int health_read(uint8_t address){
    int err;
    struct i2c_device dev;
    struct i2c_data data;
    uint8_t rx_data[1];

    //Slave Address
    dev.address = 0xAF;

    data.tx_buf = &address;
    data.tx_len = sizeof(address);
    data.rx_len = ARRAY_SIZE(rx_data);
    data.rx_buf = (uint8_t*)rx_data;

    err = i2c_transfer(&dev,&data);
    if(err == 0){
        //Do Something with data (rx_data[0])?
    }
    return err;
}

static int health_write(){
    int err;
    struct i2c_device dev;
    struct i2c_data data;
    uint8_t write_cmd[2];

    //Slave Address
    dev.address = 0xAE;

    write_cmd[0] = 0x09;
    write_cmd[1] = 0x02;

    data.tx_buf = write_cmd;
    data.tx_len = ARRAY_SIZE(write_cmd);
    data.rx_len = 0;

    err = i2c_transfer(&dev,&data);

    return err;
}

void i2c_init(){
    //The recommended eUSCI_B initialization or reconfiguration process is:
    //1. Set UCSWRST (BIS.B #UCSWRST,&UCxCTL1).
    UCB0CTLW0 |= UCSWRST;
    //2. Initialize all eUSCI_B registers with UCSWRST = 1 (including UCxCTL1).
    UCB0CTLW0 |= UCMST__MASTER + UCMODE_3 + UCSYNC_1 + UCSSEL_2; //ugcgen?
    UCB0BRW = 0x10;
    //3. Configure ports.
    P1DIR = 0x0F;
    P1OUT |= 0x00;
    //4. Clear UCSWRST using software (BIC.B #UCSWRST,&UCxCTL1).
    UCB0CTLW0 &= ~(UCSWRST);
    //5. Enable interrupts (optional).
}




//void i2c_transmit(unsigned int address,unsigned int data[], unsigned int numBytes){
//    UCB0I2CSA = address;
//    UCB0CTLW0 |= UCTR + UCTXSTT;
//    //while(UCB0CTLW0 & UCTXSTT){}
//    unsigned int i;
//    for(i = 0; i < numBytes; i++){
//        //while(!(UCB0IFG & UCTXIFG0)){};//Wait for buffer to transfer
//        UCB0TXBUF = data[i];
//    }
//    UCB0CTLW0 |= UCTXSTP;
//}
//unsigned int i2c_receive(){
//    UCB0I2CSA = 0xAF;
//    UCB0CTLW0 &= ~(UCTR);
//    UCB0CTLW0 |= UCTXSTT;
//    //while(UCB0CTLW0 & UCTXSTT){}
//
//    unsigned int data = UCB0RXBUF;
//    UCB0CTLW0 |= UCTXSTP;
//    return data;
//}

#pragma vector=TIMER0_B0_VECTOR
__interrupt void Led_Timer (void)
{
    P3OUT = 0x05;
    TB0CTL &= ~(MC_1);
    ledCount += countDir;
    if(ledCount >= 162 || ledCount <= 0){
        countDir *= -1;
    }
    TB0CCR1 = ledCount;
    TB0CTL |= (MC_1);


}

#pragma vector=TIMER0_B1_VECTOR
__interrupt void Led_Timer2 (void)
{
    TB0CCTL1 &= ~(CCIFG);
    P3OUT = 0x00;
}
