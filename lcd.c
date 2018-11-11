/*This file contains all the functions for the LCD display
 Author:   Dave Plater
 Date:     2017/10/06*/
#include "lcd.h"
#include "vend.h"
#include "mdb.h"
#include "hopper.h"
#include "gsm.h"
/* Initialize display E=RE0 R/W=RE1 RS=RE2 D4 to D7 = RD0 to RD3
   All bits initialized as digital outputs */
//Initialize LCD display
void lcd_init()
{
    LATE = 0x00;
    //write 0011 first
    LATD = 0x03;
    for(count = 3; count > 0; count-- )
    {
        lcd_clockE();
        __delay_ms(5);
        
    }
    __delay_us(50);
    LATD = 0x02;
    lcd_clockE();
    
    //4 bit interface two line 5x8 character
    lcd_write(dispfunc3);
    //Display off
    lcd_write(dispon);
    //Entry mode Increment address and no shift
    lcd_write(dispnormal);
    //Display clear
    lcd_write(dispclr);
    //Display characters from 0 to >
    lcd_test();
    __delay_ms(250);
    asm("nop");
}

void displ_sens(uint8_t smsgpos, uint8_t dmsgpos)
{
    if(CMP1_GetOutputStatus())
    {
        lcd_string(offmsg, smsgpos);
    }
    else
    {
        lcd_string(onmsg, smsgpos);
    }
    DATAEE_WriteByte(sensval, sensorval);
    lcd_string(clrmsg, dmsgpos);
    lcd_dispadd(dmsgpos);
    displ_hex((uint8_t) DAC1CON1);
    
}

void displ_lflags(uint8_t chanel, uint8_t linkflgs)
{
    lcd_string(chanmsg, line1);
    lcd_writeC(++chanel | 0x30);
    uint8_t lchan = 1;
    //uint8_t 
    lcd_string(linkmsg, line2);
    lcd_dispadd(line3);
    while(lchan < 0x09)
    {
        if(linkflgs & 0x01)
        {
            lcd_writeC(lchan | 0x30);
            lcd_writeC(',');
            lcd_writeC(' ');
        }
        lchan++;
        linkflgs = linkflgs >> 1;
    }
}

void displ_sflags(uint8_t senseflags, uint8_t chanel, uint8_t chanbit)
{
    
    lcd_string(chanmsg, line1);
    lcd_writeC(++chanel | 0x30);
    lcd_string(sensmsg, line2);
    lcd_string(sensoffmsg, line3);
    
    if((senseflags & chanbit) != 0)
    {
        lcd_string(disabled, line2 + 10);
    }
    else
    {
        lcd_string(enabled, line2 + 10);
    }

}

void displ_time(uint8_t motortime, uint8_t channel)
{
    lcd_string(clrline, line3);
    lcd_string(clrline, line4);
    lcd_string(chanmsg, line3);
    lcd_writeC(channel++ | 0x30);
    lcd_string(timemsg, line4);
    displ_hex((uint8_t) motortime);
}

uint8_t displ_note(uint8_t *notestring)
{
    lcd_write(dispclr);
    lcd_string(notestring, line2);
    return bflag+2;
}

void displ_nendis(uint8_t note2disp)
{
    uint8_t x = note2disp & noteen_byte;
    if(x != 0x00)
    {
        lcd_string(enabled, dspposition);
    }
    else
    {
        lcd_string(disabled, dspposition);
    }
}

void displ_credit(void)
{
    lcd_string(credits, line1);
    uint8_t cash = credit_check();
    displ_hex((uint8_t) cash);
    venflags.credisplay = 0;
}

void displ_code(uint8_t ercode)
{
    if((ercode >> 4) < 0x0A)
    {
        lcd_writeC((ercode >> 4) | 0x30);
    }
    else
    {
        switch(ercode >> 4)
        {
            case 0x0A : lcd_writeC(0x41);
            break;
            case 0x0B : lcd_writeC(0x42);
            break;
            case 0x0C : lcd_writeC(0x43);
            break;
            case 0x0D : lcd_writeC(0x44);
            break;
            case 0x0E : lcd_writeC(0x45);
            break;
            case 0x0F : lcd_writeC(0x46);
        }
    }
    if((ercode & 0x0F) < 0x0A)
    {
        lcd_writeC((ercode & 0x0F) | 0x30);
    }
    else
    {
        switch(ercode & 0x0F)
        {
            case 0x0A : lcd_writeC(0x41);
            break;
            case 0x0B : lcd_writeC(0x42);
            break;
            case 0x0C : lcd_writeC(0x43);
            break;
            case 0x0D : lcd_writeC(0x44);
            break;
            case 0x0E : lcd_writeC(0x45);
            break;
            case 0x0F : lcd_writeC(0x46);
        }
        
    }
}

void displ_noteer(void)
{
    lcd_string(noteerr, line1);
    ercode = mdbdata[0];
    displ_code(ercode);
    mdbflags.noteerr = 0;
    venflags.noterr = 0;
}

void displ_hex(uint24_t hexnum)
{
    signed char i = 0;
    uint8_t value[8];
    uint24_t hexnumsave = hexnum;
    while(hexnum > 0)
    {
        value[i] = hexnum % 10;
        hexnum /= 10;
        i = i + 1;
    }
    
    while(i > 0)
    {
        i--;
        lcd_writeC(value[i] | 0x30);
    }
    //In case hexnum is zero
    if(hexnumsave == 0)
    {
       lcd_writeC(0x30);
    }
}

//Write string to display
void lcd_string(uint8_t *lcdstring, uint8_t lcdline)
{
    if(lcdline == line1)
    {
        lcd_write(dispclr);
    }
    else
    {
        //MSB high for address
        lcd_dispadd(lcdline | 0x80);
    }
    for(stradd = 0; lcdstring[stradd] != 0; stradd++)
    {
        lcdata = lcdstring[stradd];
        lcd_writeC(lcdstring[stradd]);
    }
}

void displ_price(uint8_t vendprice)
{
    lcd_write(dispclr);
    lcd_dispadd(line2 + 4);
    lcd_writeC((uint8_t)'R');
    displ_hex(vendprice);
}

void displ_nochange(void)
{
    lcd_dispadd(line3);
    lcd_string(nochange, line3);
}

void disp_clock(void)
{
    lcd_write(dispclr);
    lcd_string(clockdate, line2);
    lcd_string(gsdate, line2 + 5);
    lcd_string(clocktime, line3);
    lcd_string(gstime, line3 + 5);
}

//Write character to LCD
void lcd_writeC(uint8_t lcdata)
{
    
    //Set RS High for character write
    asm("BSF LATE, 2");
    lcd_write(lcdata);
    //Check if end of line is reached
    //and start a new one.
    //line1 0 line2 40 line3 14 line4 54
    switch(bflag)
    {
        case 0x13:lcd_dispadd(line2);
        break;
        case 0x53:lcd_dispadd(line3);
        break;
        case 0x27:lcd_dispadd(line4);
    }
    asm("BCF LATE, 2");
}

void lcd_dispadd(uint8_t lcdaddress)
{
        uint8_t lcdatasave = lcdata;
        asm("BCF LATE, 2");
        //Change digit address to 0x40
        lcd_write(lcdaddress | 0x80);
        asm("BSF LATE, 2");
        lcdata = lcdatasave;

}
//Write 4 bit data to display from 8 bits
//Calling program sets up LATE
void lcd_write(uint8_t lcdata)
{
    uint8_t savelatd = LATD;
//Output upper nibble
    
    bflag = read_busy();
    while(bflag & 0x80)
    {
      bflag = read_busy();
    }
    LATD = (lcdata >> 4) & 0x0F;
//Output lower nibble
    lcd_clockE();
    
    LATD = lcdata & 0x0F;
    lcd_clockE();
    LATD = savelatd;
    
}

void lcd_test(void)
{
    uint8_t character = 0x30;
    while(character < 0x80)
    {
        lcd_writeC(character);
        character = character+ 1;
    }
}

//Read busy flag at DB7
int8_t read_busy(void)
{
    uint8_t savelate = LATE;
    uint8_t savelatd = LATD;
    asm("BCF LATE, 2"); //RS low
    asm("BSF LATE, 1"); //set R/W high
    TRISD = (TRISD | 0x0F); //Lower nibble to input
    lcd_clockH(); //Clock E high
    bflag = (PORTD << 4);//Lower nibble to upper nibble
    lcd_clockL(); //Set E low
    lcd_clockH();
    bflag = bflag | (PORTD & 0x0F);// Or the last lower nibble in
    asm("BCF LATE, 0"); //Set E low
    TRISD = (TRISD & 0xF0); //Lower nibble to output
    asm("BCF LATE, 1");
    LATD = savelatd;
    LATE = savelate;
    return(bflag); //Return the result
    
}

//Perform write or read to display toggling E clocking data
//All other registers TRISD LATD TRISE and LATE must be setup first
//Depending on the length of the cable delays may need tuning
void lcd_clockH(void)
{
    asm("BSF LATE, 0"); //Set E high
    __delay_us(1);
}
//Perform write or read to display toggling E clocking data
//All other registers TRISD LATD TRISE and LATE must be setup first
//Depending on the length of the cable delays may need tuning
void lcd_clockL(void)
{
    asm("BCF LATE, 0"); //Set E low
    __delay_us(1);
}
//Perform write or read to display toggling E clocking data
//All other registers TRISD LATD TRISE and LATE must be setup first
//Depending on the length of the cable delays may need tuning
void lcd_clockE(void)
{
     asm("BSF LATE, 0"); //Set E high
    __delay_us(1);
     asm("BCF LATE, 0"); //Set E low
    __delay_us(1);
}
