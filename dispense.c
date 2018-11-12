/* 
 * File:   dispense.h
 * Author: Dave Plater
 *
 * Created on 24 March 2018, 12:49 PM
 */

#include "dispense.h"

bool price_check(void)
{
//    return 0;
    channel = 0x00;
    cash = credit_check();
    if(lowprice > cash)
    {
        return 0;
    }
    if(highprice <= cash && !venflags.hiprice)
    {
        note_disable();
        venflags.hiprice = 1;
    }
    chanmask = 0;
    char i = 7;
    while(i > 0x00)
    {//If enough cash for channel then set mask bit
        if(pricevend[i] <= cash)
        {
            chanmask = chanmask | 0x01;
        }
        chanmask = chanmask << 1;
        i--;
    }
    if(pricevend[0] <= cash)
    {
        chanmask = chanmask | 0x01;
    }
    chanmask = chanmask & ~errormask;
    venflags.error = 0;
    //Turn on button lights for enabled channels.
    //Channel 1 to 4 = RB0 to RB3
    //Channel 5 to 8 = RD4 to RD7
    LATCbits.LC2 = 1;
    LATB = chanmask & 0x0F;
    LATD = chanmask & 0xF0;
    buttons = butin();
    if((buttons & errormask) != 0x00)
    {
        if(!venflags.pricedisplay)
        {
            lcd_string(emptymsg, line1);
            venflags.pricedisplay = 1;
        }
        
        return 0;
    }
    buttons = buttons & chanmask;
    if(buttons != 0)
    {
        channel = get_channel(buttons);
        vendprice = pricevend[channel];
        uint8_t savechan = channel;
        uint8_t savebut = buttons;
        //This is a quick fix for coke + pie dispense.
        if(channel > 0x03)
        {
            buttons = 0x80;
            channel = 0x07;
        }
        dispense(channel);
        channel = savechan;
        buttons = savebut;
        uint8_t chanlinkbit = DATAEE_ReadByte(chan1linkbits + channel);
        if(chanlinkbit != 0x00 && !venflags.error)
        {
            channel = get_channel(chanlinkbit);
            dispense(channel);
        }
        credit_subtract(vendprice);
        venflags.hiprice = 0;
        cash = credit_check();
        if(cash > 0 && !venflags.nochange && !venflags.error)
        {
            
            give_change(cash);
        }
        LATAbits.LA5 = 0;
        LATCbits.LC2 = 1;
        return 1;
    }
    else
    {
        venflags.pricedisplay = 0;
        venflags.credisplay = 1;
    }
    return 0;
}

void dispense(uint8_t chanel)
{
    channel = chanel;
    uint8_t sensmask = DATAEE_ReadByte(sensorflags);
    if((buttons & sensmask) != 0)
    {
        vend_nosense(chanel);
    }
    else
    {
        vend_dispense(chanel);
    }
    if(!venflags.error)
    {
        update_vends(channel);
    }
    
}

void vend_nosense(uint8_t chanel)
{
    LATB = 0x00;
    LATD = 0x00;
    //Turn off button light common
    LATCbits.LC2 = 0;
    //Get channel bit
    chanmask = 0x01;
    chanmask = chanmask << chanel;
    mdb_unlock();
     //Turn on motor common
    LATAbits.LA5 = 1;
    LATB = chanmask & 0x0F;
    LATD = chanmask & 0xF0;
    __delay_ms(250);
    LATB = 0;
    LATD = 0;
}

void vend_dispense(uint8_t chanel)
{//Ensure all channels off
    mdb_unlock();
    LATB = 0x00;
    LATD = 0x00;
    //Turn on motor common
    LATAbits.LA5 = 1;
    //Turn off button light common
    LATCbits.LC2 = 0;
    //Get channel bit
    chanmask = 0x01;
    chanmask = chanmask << chanel;
    set_motortime(chanel);
    while(!CMOUTbits.MC1OUT && !PIR0bits.TMR0IF)
    {
        LATB = chanmask & 0x0F;
        LATD = chanmask & 0xF0;
    }
    LATB = 0;
    LATD = 0;
    TMR0_StopTimer();
    if(PIR0bits.TMR0IF)
    {//Timeout no package has dropped
        set_motortime(chanel);
        //Wait for drop.
        while(!CMOUTbits.MC1OUT && !PIR0bits.TMR0IF){}
        if(PIR0bits.TMR0IF)
        {//Now we have a no stock situation
            venflags.error = 1;
            disable_channel(chanel);
            //No vend no pay
            vendprice = 0x00;
        }
    }
    
}

void set_motortime(uint8_t chanel)
{//void TMR0_Write16bitTimer(uint16_t timerVal)
    uint8_t i = DATAEE_ReadByte(chan1time + chanel);
    TMR0_StopTimer();
    PIR0bits.TMR0IF = 0;
    //Initialize timer at one second
    uint16_t time = second1;
    time = time - (second_5 * i);
    TMR0_WriteTimer(time);
    TMR0_StartTimer();
}

void find_highprice(void)
{
    char i = 0;
    highprice = pricevend[0];
    while(i < 8)
    {
        if(highprice < pricevend[i])
        {
            highprice = pricevend[i];
        }
        i++;
    }
}

void find_lowprice(void)
{
    char i = 0;
    lowprice = pricevend[0];
    while(i < 8)
    {
        if(lowprice > pricevend[i])
        {
            lowprice = pricevend[i];
        }
        i++;
    }

}

void create_chanmask(void)
{
    
}

void init_pricestore(void)
{
    //Transfer prices to pricevend[]
    char i = 0;
    while(i < 8)
    {
        pricevend[i] = DATAEE_ReadByte(pricestore + i);
        i++;
    }

}

void disable_channel(uint8_t channel)
{
    uint8_t errormask = 0x01;
    errormask = errormask << channel;
    errormask = errormask | DATAEE_ReadByte(venderrors);
    DATAEE_WriteByte(venderrors, errormask);
}

void update_vends(uint8_t chanel)
{
    uint8_t x = DATAEE_ReadByte(vendstore + chanel);
    x++;
    DATAEE_WriteByte(vendstore + chanel, x);
}