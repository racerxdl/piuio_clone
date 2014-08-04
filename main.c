/***********************************************************/
/*   ____ ___ _   _ ___ ___     ____ _                     */
/*  |  _ \_ _| | | |_ _/ _ \   / ___| | ___  _ __   ___    */
/*  | |_) | || | | || | | | | | |   | |/ _ \| '_ \ / _ \   */
/*  |  __/| || |_| || | |_| | | |___| | (_) | | | |  __/   */
/*  |_|  |___|\___/|___\___/   \____|_|\___/|_| |_|\___|   */
/*                                                         */
/*  By: Lucas Teske                                        */
/***********************************************************/
/*     Basicly this is an PIUIO Clone with an ATMEGA328    */
/*     It uses V-USB for the USB Interface and few chips   */
/***********************************************************/
/*           This is main code from PIUIO Clone            */
/*  This is the simple branch. This is for users that want */
/*           just to connect to your DIY pads              */
/*       The lights arent available on this one.           */
/***********************************************************/
/*                    License is GPLv3                     */
/*  Please consult https://github.com/racerxdl/piuio_clone */
/***********************************************************/

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h> 
#include <util/delay.h> 

#include <avr/pgmspace.h>   
#include "usbdrv.h"
#include "usbconfig.h"

//    Some Macros to help

#define GETBIT(port,bit) ((port) & (0x01 << (bit)))     //    Get Byte bit
#define SETBIT(port,bit) ((port) |= (0x01 << (bit)))    //    Set Byte bit
#define CLRBIT(port,bit) ((port) &= ~(0x01 << (bit)))   //    Clr Byte bit

//    PIUIO Bytes
// This is for Byte1 of the player short
#define BTN_TEST      1
#define BTN_SERVICE   6

// This is for Byte0 of the player short
#define SENSOR_LU     0
#define SENSOR_RU     1
#define SENSOR_CN     2
#define SENSOR_LD     3
#define SENSOR_RD     4

//    Clone Port Map

//  Player 1 Maps
#define GET_LU_P1    GETBIT(PINB,0)         //  Player 1 Upper Left at PORTB0
#define GET_RU_P1    GETBIT(PINB,1)         //  Player 1 Upper Right at PORTB1
#define GET_CN_P1    GETBIT(PINB,2)         //  Player 1 Center at PORTB2
#define GET_LD_P1    GETBIT(PINB,3)         //  Player 1 Down Left at PORTB3
#define GET_RD_P1    GETBIT(PINB,4)         //  Player 1 Down Right at PORTB4
#define GET_TEST     GETBIT(PINB,5)         //  Test Button at PORTB5

//  Player 2 Maps
#define GET_LU_P2    GETBIT(PINC,0)         //  Player 2 Upper Left at PORTC0
#define GET_RU_P2    GETBIT(PINC,1)         //  Player 2 Upper Right at PORTC1
#define GET_CN_P2    GETBIT(PINC,2)         //  Player 2 Center at PORTC2
#define GET_LD_P2    GETBIT(PINC,3)         //  Player 2 Down Left at PORTC3
#define GET_RD_P2    GETBIT(PINC,4)         //  Player 2 Down Right at PORTC4
#define GET_SERVICE  GETBIT(PINC,5)         //  Service Button at PORTC5


//    Some Vars to help
static unsigned char LampData[8];       //    The LampData buffer received
static unsigned char InputData[8];      //    The InputData buffer to send
static unsigned char datareceived = 0;  //    How many bytes we received
static unsigned char dataLength = 0;    //    Total to receive

//static unsigned char Input[2];          //    The actual 16 bits Input data
static unsigned char Output[2];         //    The actual 16 bits Output data

USB_PUBLIC uchar usbFunctionWrite(uchar *data, uchar len) {
    //    This function will be only triggered when game writes to the lamps output.
    //    I will keep this part of code, but we wont use to write to outputs. Because lack of IO
    unsigned char i;              
    for(i = 0; datareceived < 8 && i < len; i++, datareceived++)
               LampData[datareceived] = data[i];    
    if(datareceived == dataLength)    {    //    Time to set OUTPUT
        Output[0] = LampData[0];           //    The AM use unsigned short for those. 
        Output[1] = LampData[2];           //    So we just skip one byte
                                           //    The other bytes are just 0xFF junk
    }    
    return (datareceived == dataLength);   // 1 if we received it all, 0 if not
}

USB_PUBLIC uchar usbFunctionSetup(uchar data[8]) {
    usbRequest_t *rq = (void *)data;
    if(rq->bRequest == 0xAE)    {                               //    Access Game IO
        switch(rq->bmRequestType)    {
            case 0x40:                                          //    Writing data to outputs
                datareceived = 0;
                dataLength = (unsigned char)rq->wLength.word;
                return USB_NO_MSG;                              //    Just tell we want a callback to usbFunctionWrite
            break;
            case 0xC0:                                          //    Reading input data
                usbMsgPtr = InputData;                          //    Just point to the buffer, and 
                return 8;                                       //    saying to send 8 bytes to the PC
            break;
        }
    }
    return 0;                                                   //    Ops, it cant get here
}

void pollInputOutput()    {
    //  This will get the inputs.
    //  PIUIO actually sends inverted data.
    
    InputData[0] = ~( ((~GET_LU_P1) << SENSOR_LU) | ((~GET_RU_P1) << SENSOR_RU) | ((~GET_CN_P1) << SENSOR_CN) |  ((~GET_LD_P1) << SENSOR_LD) | ((~GET_RD_P1) << SENSOR_RD) );
    InputData[1] = ~( ((GET_TEST)  << BTN_TEST  ) | ((GET_SERVICE) << BTN_SERVICE) );
    InputData[2] = ~( ((~GET_LU_P2) << SENSOR_LU) | ((~GET_RU_P2) << SENSOR_RU) | ((~GET_CN_P2) << SENSOR_CN) |  ((~GET_LD_P2) << SENSOR_LD) | ((~GET_RD_P2) << SENSOR_RD) );
    InputData[3] = 0xFF;
}

int main() {
    unsigned char i;
    wdt_enable(WDTO_1S);

    DDRB = 0;
    DDRC = 0;
    
    for(i=0;i<8;i++)
        InputData[i] = 0xFF;
        
    usbInit();
    usbDeviceDisconnect();                      // enforce re-enumeration
    for(i = 0; i<250; i++) {                    // wait 500 ms
        wdt_reset();                            // keep the watchdog happy
        _delay_ms(1);
    }
    usbDeviceConnect();
    sei();                                      // Enable interrupts after re-enumeration
    while(1) {
        wdt_reset();                            // keep the watchdog happy
        usbPoll();
        pollInputOutput();
    }

    return 0;
}
