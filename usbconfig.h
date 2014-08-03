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
/*      This is the USB Configuration part from V-USB      */
/***********************************************************/
/*                    License is GPLv3                     */
/*  Please consult https://github.com/racerxdl/piuio_clone */
/***********************************************************/
#ifndef __usbconfig_h_included__
#define __usbconfig_h_included__

/* ---------------------------- Hardware Config ---------------------------- */

#define USB_CFG_IOPORTNAME      D               //  We use PORT D to USB Port Pins
#define USB_CFG_DMINUS_BIT      0               //  This is the USB D- line. PORTD.0
#define USB_CFG_DPLUS_BIT       1               //  This is the USB D+ line. PORTD.1
                                                //  The D+ from USB must be also connected to INT0, on ATMEGA328 is the PORTD.2
#define USB_CFG_CLOCK_KHZ       (F_CPU/1000)    //  This is the USB clock in kHz. we just use AVR Frequency / 1000;
                                                //  The good values for crystal: 12MHz, 12.8Mhz, 15MHz, 16Mhz, 16.5Mhz, 18Mhz, 20Mhz
                                                //  The F_CPU is the definition of the clock, so you dont need to change anything here
#define USB_CFG_CHECK_CRC       0               //  That makes CRC Checks at USB packets. We dont want it. Also it only works with 18MHz


/* --------------------------- Functional Range ---------------------------- */
// There is too many stuff here lol, so I will just comment a few of these and also if its not default value, I will say why I changed.
// You can look what each one does at the original usbconfig from V-USB

#define USB_CFG_HAVE_INTRIN_ENDPOINT    0       //  We dont need any additional entry points, so this will be 0
#define USB_CFG_HAVE_INTRIN_ENDPOINT3   0       //  Same as below
#define USB_CFG_EP3_NUMBER              3       //  The Entrypoint 3 number. We dont need it, so we keep default.
#define USB_CFG_IMPLEMENT_HALT          0       //  Thats for interrupting a endpoint. We dont need it
#define USB_CFG_SUPPRESS_INTR_CODE      0       
#define USB_CFG_INTR_POLL_INTERVAL      10
#define USB_CFG_IS_SELF_POWERED         0       //  PIUIO does have own power supply. But I dont like that lol, so mine is just USB powered.
#define USB_CFG_MAX_BUS_POWER           500     //  This is the value in mA, it will be divided by two (100 mean 50mA). Its just an info for PC
#define USB_CFG_IMPLEMENT_FN_WRITE      1       //  We implemented a write-from-computer function. This is basicly used when game writes the lamp data.
#define USB_CFG_IMPLEMENT_FN_READ       0       //  We dont need to implement that. The stuff that V-USB provides to us is all we need.
#define USB_CFG_IMPLEMENT_FN_WRITEOUT   0
#define USB_CFG_HAVE_FLOWCONTROL        0
#define USB_CFG_DRIVER_FLASH_PAGE       0
#define USB_CFG_LONG_TRANSFERS          0
#define USB_COUNT_SOF                   0
#define USB_CFG_CHECK_DATA_TOGGLING     0
#define USB_CFG_HAVE_MEASURE_FRAME_LENGTH   0
#define USB_USE_FAST_CRC                0       //  Doesnt make much difference for us.

/* -------------------------- Device Description --------------------------- */

#define  USB_CFG_VENDOR_ID       0x47, 0x05                             //  That is Vendor ID from PIUIO. The Cypress 0x547
#define  USB_CFG_DEVICE_ID       0x02, 0x10                             //  That is Device ID from PIUIO. The FX-USB  0x1002
#define USB_CFG_DEVICE_VERSION  0x00, 0x01                              //  USB Device version. 1.0 - Yeah lol.
#define USB_CFG_VENDOR_NAME     'H', 'A', 'C', 'K', 'I', 'T', 'U', 'P'  //  The device description. It is used to initialize an char array.  
#define USB_CFG_VENDOR_NAME_LEN 8                                       //  The size of device description.
#define USB_CFG_DEVICE_NAME     'P', 'I', 'U', 'I', 'O'                 //  Device name! yeah we can actually name it correctly instead FX-USB
#define USB_CFG_DEVICE_NAME_LEN 5                                       //  The size of device name
#define USB_CFG_DEVICE_CLASS        0xff                                //  0xFF is a vendor-specifc class. Thats what we want
#define USB_CFG_DEVICE_SUBCLASS     0
#define USB_CFG_INTERFACE_CLASS     0                                   
#define USB_CFG_INTERFACE_SUBCLASS  0
#define USB_CFG_INTERFACE_PROTOCOL  0

/* ------------------- Fine Control over USB Descriptors ------------------- */

#define USB_CFG_DESCR_PROPS_DEVICE                  0
#define USB_CFG_DESCR_PROPS_CONFIGURATION           0
#define USB_CFG_DESCR_PROPS_STRINGS                 0
#define USB_CFG_DESCR_PROPS_STRING_0                0
#define USB_CFG_DESCR_PROPS_STRING_VENDOR           0
#define USB_CFG_DESCR_PROPS_STRING_PRODUCT          0
#define USB_CFG_DESCR_PROPS_STRING_SERIAL_NUMBER    0
#define USB_CFG_DESCR_PROPS_HID                     0
#define USB_CFG_DESCR_PROPS_HID_REPORT              0
#define USB_CFG_DESCR_PROPS_UNKNOWN                 0

#endif /* __usbconfig_h_included__ */
