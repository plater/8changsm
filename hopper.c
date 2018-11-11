/* 
 * File:   hopper.c
 * Author: Dave Plater
 *
 * Created on 21 March 2018 12:40
 * alternative to cctalk hopper.
 */
#include "hopper.h"

void hopper_test(void)
{
     if(!hopper_pay(0x05))
    {//Success
        asm("nop");
    }
    else
    {//Error
        asm("nop");
    }
}
void give_change(uint8_t paycash)
{
    cash = paycash;
    credit_subtract(cash);
    uint8_t i = DATAEE_ReadByte(hopcoin);
    change = paycash / i;
    change = hopper_pay(change);
    if(change > 0)
    {
        change = change * i;
        credit_add(change);
    }
}
//PIR4bits.TMR4IF TMR4_Initialize = 5 sec wait for coin
//PIR2bits.TMR2IF TMR2_Initialize = 1 sec coin exit
//LATCbits.LATC1 = 1 is no coin
//Read PORTBbits.PORTB4 zero is closed switch.
//cashoutv is total coins out eeprom storage
uint8_t hopper_pay(uint8_t change)
{
    cctalk_on();
    outcoins = 0x00;
    while(change > 0)
    {
        venflags.nochange = pay_coin();
        if(venflags.nochange)
        {
            cctalk_off();
            Update_coinsout(outcoins);
            venflags.nochange = 1;
            DATAEE_WriteByte(hoperror, 0x01);
            return change;
        }
        outcoins++;
        change--;
    }
    cctalk_off();
    Update_coinsout(outcoins);
    return change;
}

bool Reset_hopper(void)
{
    hopercount--;
    cctalk_off();
    __delay_ms(2000);
    TMR4_Initialize();
    if(hopercount > 0)
    {
        cctalk_on();
        return 0;
    }
    else
    {
        return 1;
    }
}

 bool pay_coin(void)
{//Pickup and exit one coin, return 0 for successful cycle.
    //cctalk_on();
    hopercount = 3;
    if(switch_read())
    {//Switch closed start exit timer 1 second
        TMR2_Initialize();
        while(switch_read() && !PIR4bits.TMR2IF){}
       // cctalk_off();
        if(PIR4bits.TMR2IF)
        {//Timeout occurred - error
            
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {//Switch open start pick up timer 10 second
        cctalk_on();
        TMR4_Initialize();
        // && !PIR4bits.TMR4IF
        while(!switch_read())
        {
            if(PIR4bits.TMR4IF)
            {
                if(Reset_hopper())
                {
                    cctalk_off();
                    return 1;
                }
                
            }
        }
        cctalk_off();
        TMR2_Initialize();
        cctalk_on();
        while(switch_read() && !PIR4bits.TMR2IF){}
        cctalk_off();
        if(PIR4bits.TMR2IF)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    
   
}

bool switch_read(void)
{
    if(PORTBbits.RB4 == venflags.swclosed)
    {//Switch changed state
        if(venflags.swclosed)
        {//switch transition from closed to open
            __delay_ms(5);
        }
        else
        {//Switch transition from opened to closed
            __delay_ms(20);
        }
    }
    if(PORTBbits.RB4)
    {//Switch is open
        venflags.swclosed = 0;
        return 0;
    }
    else
    {//Switch is closed
        venflags.swclosed = 1;
        return 1;
    }
}

void Update_coinsout(uint8_t coinsout)
{
    Read_NVstore(cashoutv, ((uint8_t*) &pvcash), 0x02);
    pvcash = pvcash + (uint16_t)coinsout;
    Write_NVstore(cashoutv, ((uint8_t*) &pvcash), 0x02);
}