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

//    Some Vars to help
static unsigned char LampData[8];       //    The LampData buffer received
static unsigned char InputData[8];      //    The InputData buffer to send
static unsigned char datareceived = 0;  //    How many bytes we received
static unsigned char dataLength = 0;    //    Total to receive

static unsigned char Input[2];          //    The actual 16 bits Input data
static unsigned char Output[2];         //    The actual 16 bits Output data

USB_PUBLIC uchar usbFunctionWrite(uchar *data, uchar len) {
    //    This function will be only triggered when game writes to the lamps output.
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
    //    This will set outputs and get inputs
    //    The board will use 4067 muxer that is 16-to-1 muxer.
    //    PORTC is the Muxer Selector.
    //    PORTB0 is the Muxer Output
    unsigned char inputn, tmp1,tmp2;    
    SETBIT(PORTB,3);                                                        //    Disable the latches input
    for(inputn=0;inputn<16;inputn++)    {
        PORTC = inputn;                                                     //    Sets the muxer position
        tmp1 = GETBIT(PINB,0);                                              //    Gets the input
        if(tmp1 > 0)
            SETBIT(Input[(inputn/16)*2],inputn%8);                          //    Sets if input = 1
        else
            CLRBIT(Input[(inputn/16)*2],inputn%8);                          //    Clears if input = 0
    }
    //    The board will use 4099 as output latch. Two are used to get 16 bit output.
    //    PORTC is the address, PORTB1 is Data for First byte, PORTB2 is Data for Second
    //    The Latch input enables are in PORTB3
    //    So we will set two bits per loop cycle
    CLRBIT(PORTB,3);                                                        //    Enable the latches input
    for(inputn=0;inputn<8;inputn++)    {
        PORTC = inputn;                                                     //    Sets the address
        tmp1 = GETBIT(Output[0],inputn);                                    //    Gets the output data
        tmp2 = GETBIT(Output[1],inputn);
        if(tmp1 > 0) {SETBIT(PORTB,1);} else {CLRBIT(PORTB,1);};            //    Sets/Clear the output at latches
        if(tmp2 > 0) {SETBIT(PORTB,2);} else {CLRBIT(PORTB,2);}; 
        
    }
    SETBIT(PORTB,3);                                                        //    Disable the latches input, just in case.
                                                                            //    Okay, so now we can set the output buffer, just in case the PC asks now the inputs

    InputData[0] = ~Input[0];                                               //    Andamiro uses unsigned short here also
    InputData[2] = ~Input[1];
}

int main() {
    unsigned char i;
    wdt_enable(WDTO_1S);

    DDRC = 255;
    PORTC = 0;
    DDRB = 0b00001110;
    PORTB = 0;
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
