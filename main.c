/**
  Generated Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This is the main file generated using PIC10 / PIC12 / PIC16 / PIC18 MCUs 

  Description:
    This header file provides implementations for driver APIs for all modules selected in the GUI.
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs  - 1.45
        Device            :  PIC18F47K40
        Driver Version    :  2.00
    The generated drivers are tested against the following:
        Compiler          :  XC8 1.35
        MPLAB             :  MPLAB X 3.40
*/

/*
    (c) 2016 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/

#include "mcc_generated_files/mcc.h"
#include "lcd.h"
#include "vend.h"
#include "mdb.h"
#include "cctalk.h"
#include "dispense.h"
#include "gsm.h"
void enter_service(void);

/*
                         Main application
 */
void main(void)
{
    // Initialize the device
    SYSTEM_Initialize();
//Initialise gsm unit and start 5 second timer
    gsm_off();
    // Initialize lcd display
    lcd_init();
    gsm_on();
    gsm_init();
    vend_init();

    
    while(1 == 1)
    {
        asm("nop");
        DAC1CON1 = 0x01;
        if (!CMP1_GetOutputStatus())
        {
            venflags.service = 1;
            
        }
        DAC1CON1 = sensorval;
        
        if(venflags.service == 1)
        {
            enter_service();
        }
        
        if(venflags.inscoin == 1)
        {
            lcd_string(inscoin, line1);
            venflags.inscoin = 0;
            if(venflags.nochange)
            {
                displ_nochange();
            }
        }
        if(venflags.initialrun == 1)
        {
            venflags.initialrun = 0;
            mdbflags.noteerr == 0;
        }
        if(venflags.credisplay == 1)
        {
            displ_credit();
            if(venflags.nochange)
            {
                displ_nochange();
            }
        }
        
        if(PIR4bits.TMR3IF)
        {
            credit = mdb_poll();
            if(credit != 0x00)
            {
                if(mdbflags.noteerr == 1)
                {
                    switch(credit)
                    {
                        //Just reset
                        case 0x06 : mdb_init();
                        mdbflags.noteerr == 0;
                        credit = 0;
                        break;
                        //Accept disabled
                        case 0x09:
                            if(mdbflags.vending)
                            {
                                credit = 0;
                            }
                            else
                            {
                                mdb_init();
                                credit = 0;
                            }
                        break;
                        case 0x10:mdbflags.noteerr == 0;
                        credit = 0;
                        break;
                        case 0x0C : mdbflags.noteerr == 0;
                        credit = 0;
                        break;
                        default: displ_noteer();
                    }
                }
                else
                {
                    credit_add(credit);
                    venflags.iscredit = 1;
                }
            }
            else
            {
                if(mdbflags.noteerr == 1)
                {
                    displ_noteer();
                }
                
                
           }
        }
        if(venflags.iscredit)
        {
            if(price_check())
            {
                asm("RESET");
            }
        }
        else
        {
            if(butin() != 0)
            {
                
                buttons = butin();
                if(((buttons & ~errormask) == 0x00) && !venflags.pricedisplay)
                {
                    lcd_string(emptymsg, line1);
                    venflags.pricedisplay = 1;
                }
                if(!venflags.pricedisplay)
                {
                    uint8_t channel = get_channel(buttons);
                    vendprice = DATAEE_ReadByte(pricestore + channel);
                    displ_price(vendprice);
                    venflags.pricedisplay = 1;
               }
            }
            else
            {
                if(venflags.pricedisplay)
                {
                    venflags.pricedisplay = 0;
                    venflags.inscoin = 1;
                }
            }
                
                
        }
        
        
        
    }


}

/*Service main menu:
 button 1 = Audit menu
 button 2 = Hopper coin value
 button 3 = Sensor setup ie. DAC value for comparator
 button 4 = Notes accepted
 button 5 = Clear Credit and exit
 button 6 = Price set
 button 7 = Vend Setup
 button 8 = Exit
 
 */

void enter_service(void)
{
    mdb_unlock();
    lcd_string(servmsg, line1);
    
    
    while(venflags.service)
    {
        buttons = butindb();
        switch(buttons)
        {
            case 0x01 : Audit();
            break;
            case 0x02 : Sensor_set();
            break;
            case 0x04 : Hopper_coin();
            break;
            case 0x08 : set_notes();
            break;
            case 0x10 : Clear_cred();
            venflags.service = 0;
            asm("RESET");
            case 0x20 : price_set();
            break;
            case 0x40 : Vend_setup();
            break;
            case 0x80 : venflags.service = 0;
            asm("RESET");
            break;
            case 0x03 : hopper_test();
        }
    }
}
/**
 End of File
*/