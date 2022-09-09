/***********************************************************/
/*   ____ ___ _   _ ___ ___     ____ _                     */
/*  |  _ \_ _| | | |_ _/ _ \   / ___| | ___  _ __   ___    */
/*  | |_) | || | | || | | | | | |   | |/ _ \| '_ \ / _ \   */
/*  |  __/| || |_| || | |_| | | |___| | (_) | | | |  __/   */
/*  |_|  |___|\___/|___\___/   \____|_|\___/|_| |_|\___|   */
/*                                                         */
/*  By: Lucas Teske                                        */
/*  Arduino Version by: Mattia Pini                        */
/***********************************************************/
/*     Basicly this is an PIUIO Clone with an ATMEGA328    */
/*     It uses V-USB for the USB Interface and few chips   */
/***********************************************************/
/*           This is main code from PIUIO Clone            */
/***********************************************************/
/*                    License is GPLv3                     */
/*  Please consult https://github.com/racerxdl/piuio_clone */
/***********************************************************/
//#include "usbconfig.h"
#include <usbdrv.h>

//use PORT E for usb connection(MODIFY usbconfig.h)
//use PORT F for cabinet and pad P1((Cabinet=(left=PF0,center=PF1,right,=PF2),Pad=(DOWN=PF3,LEFT=PF4,UP=PF6,RIGHT=PF7))
//use PORT K for cabinet and pad P2((Cabinet=(left=PK0,center=PK1,right,=PK2),Pad=(DOWN=PK3,LEFT=PK4,UP=PK6,RIGHT=PK7))
//use PORT G for Coin->PG2,Service->PG1,Menu->PG0
//use PORT L for Pads lights
//use PORT C for Cabinet lights

#define GETBIT(port,_bit) ((port) & (0x01 << (_bit)))     //    Get Byte bit
#define SETBIT(port,_bit) ((port) |= (0x01 << (_bit)))    //    Set Byte bit
#define CLRBIT(port,_bit) ((port) &= ~(0x01 << (_bit)))   //    Clr Byte bit

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
    usbRequest_t *rq = (usbRequest_t *)data;
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
    //on Arduino Mega we don't use muxer nor output latch

      //HERE WE FILTER THE BITS FROM THE GAME, openITG
    unsigned char neon_bit = Output[1] & 0b00000100;
    unsigned char cabinet_buttons = Output[1] & 0b00011000;
    unsigned char halo = (Output[3] & 0b00000111) | ((Output[2] & 0b10000000)>> 4 );
    halo |= neon_bit << 2;
    halo |= cabinet_buttons << 2;
  
    //first 4 bits are for player 1 , other 4 bits for player 2
    unsigned char pads_lights = Output[0] & 0b00111100;
    pads_lights = pads_lights << 2;
    pads_lights |= (Output[2] & 0b00111100) >> 2; 
                                                                      //    Okay, so now we can set the output buffer, just in case the PC asks now the inputs
    Input[0] = PINF;
    Input[1] = PINK;
    InputData[0] = ~Input[0];
    InputData[1] = ~PING;                                                   //    Andamiro uses unsigned short here also
    InputData[2] = ~Input[1];
    InputData[3] = ~PING;     
}


void setup() {
      unsigned char i;
    //Set port as input
    DDRF = 0;    //P1
    DDRK = 0;    //P2  
    DDRG = 0b11111000; //Operator Menu
    //Set port as output
    DDRC = 255;
    DDRL = 255;
    PORTC = 0;
    PORTL = 0;
    for(i=0;i<8;i++)
        InputData[i] = 0xFF;
    usbInit();
    usbDeviceDisconnect();                      // enforce re-enumeration
    for(i = 0; i<250; i++) {                    // wait 500 ms
        delayMicroseconds(100);
    }
    usbDeviceConnect();
    sei();

}

void loop() {
        usbPoll();
        pollInputOutput();
}
