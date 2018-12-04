/* 
 * File:   gsm.c
 * Author: Dave Plater
 *
 * Created on September 26, 2018, 3:14 PM
 */
#include "gsm.h"

//Wait for buffer empty
void gsm_waitx(void)
{
    TX2STAbits.TXEN = 1;
    while(!PIR3bits.TX2IF)
    {
    }
}
//Wait for buffer empty
void gsm_waitr(void)
{
    while(!PIR3bits.RC2IF)
    {
        
    }
}

void gsm_transmit(uint8_t txbyte)
{
    TX2STAbits.TXEN = 1;
    EUSART2_Write(txbyte);
    while(!TX2STAbits.TRMT){}
}

void gsm_zerobuff(uint8_t* gsmsgbuf, uint16_t count )
{
    uint16_t x = 0;
    while(x < count)
    {
        gsmsgbuf[x] = 0x00;
        x++;
    }
    
}

void gsm_msg(uint8_t *msgadd)
{
    __uint24 msgbkup = msgadd;
    retrans:
    msgadd = msgbkup;
    gsmflags.retransmit = 0;
    
    for(uint8_t gsmstr = 0; msgadd[gsmstr] != 0x00; gsmstr++)
    {
        gsmbyte = msgadd[gsmstr];
        gsm_transmit(gsmbyte);
    }
    if(gsmflags.retransmit)
    {
        goto retrans;
    }
//     __delay_us(500);
}

//BAUDCONbits.ABDEN
void gsm_init(void)
{
    PIE3bits.RC2IE =1;
    gsmflags.meerror = 0;
    eusart2RxCount = 512;
    gsminit:
    
    gsm_zerobuff(gsmmsg, 0x200);
  //Make sure that 5 seconds have passed.
    while(!PIR4bits.TMR4IF)
    {
        
    }
//    gsm_msg(noecho);
//    gsm_receive(1, gsmmsg);
    //Transmit AT\r to sync baud rate. 
    gsm_txAT();
    gsm_transmit(0x0D);
    //Response AT\r\r\nOK\r\n
    gsm_receive(2, gsmmsg);
    gsm_msg(noecho);
    gsm_receive(1, gsmmsg);
    gsm_netwait();

    if(gsmflags.meerror)
    {
        LATCbits.LC3 = 0;
        __delay_ms(2000);
        gsm_on();
        goto gsminit;
    }
//At this point the gsm modem returns 0x00,0x0D,0x0A
//    gsmbyte = gsm_unsolic();
    
/*    gsm_txAT();
    gsm_transmit(0x0D);
    __delay_us(500);
    //Response AT\r\r\nOK\r\n
    gsm_receive(2);*/
    gsm_setime();
    if(gsmflags.meerror)
    {
        LATCbits.LC3 = 0;
        __delay_ms(2000);
        gsm_on();
        goto gsminit;
    }
//   INTCONbits.GIE = 1;
 //   INTCONbits.PEIE = 1;
//    while(1 == 1){}
//    gsm_getbalance();
//    gsm_msg(loctim);
//    gsm_receive(1);
    
    
    clock_display();


    gsm_msg(smstxt);
    gsm_receive(1, gsmmsg);
    
    gsm_msg(smsdel);
    gsm_receive(1, gsmmsg);
    
    gsm_getbalance();
    
    gsm_msg(smslst);
    gsm_receive(1,gsmmsg);
    
    gsm_msg(smdqry);
    gsm_receive(1, gsmmsg);
    
    gsm_msg(engqry);
    gsm_receive(2, gsmmsg);
    
    gsm_msg(netoff);
    gsm_receive(2, gsmmsg);
   
    gsm_msg(facres);
    gsm_receive(1, gsmmsg);
    asm("nop");
}

void clock_display(void)
{
    INTCONbits.PEIE = 0;
    PIE3bits.RC2IE =0;
    dispclka:
    TMR2_Initialize();
    T2CONbits.TMR2ON = 1;
    gsm_gettime();
    disp_clock();
    INTCONbits.PEIE = 0;
    gsmflags.msgavl = 0;
    while(!PIR4bits.TMR2IF)
    {
        if(PIR3bits.RC2IF)
        {
            gsm_receive(1, gsmusm);
            gsmflags.msgavl = 1;
        }
    }
    if(gsmflags.msgavl)
    {
        lcd_string(gsmusm, line1);
        parse_sms();
        asm("nop");
    }
    INTCONbits.PEIE = 0;
//    gsm_msg(smstxt);
//    gsm_receive(1);
//    gsm_msg(smslst);
 //   gsm_receive(71);
    
    goto dispclka;
}

void parse_sms(void)
{
    int diffr = memcmp(gsmusm, cmti, 0x05);
    if(diffr == 0x00)
    {
        index = (gsmusm[0x0C] & 0x0F) * 2;
        gsm_msg(smstxt);
        gsm_receive(1, gsmmsg);
        gsm_msg(smslst);
        gsm_receive(++index, gsmmsg);
        gsm_msg(setgsm);
        gsm_receive(1, gsdate);
        gsm_msg(smstxt);
        gsm_receive(1, gstime);
        gsm_msg(sendms);
        gsm_msg(pnum);
        gsm_numack();
        //if gsmflags.abrtmsg send call me instead
        gsm_msg(ackmsg);
        //Send ^Z or SUB to terminate sms
        gsm_transmit(0x1A);
        gsm_transmit(0x0D);
        gsm_receive(1, gsdate);
        asm("NOP"); //gsm_receive(1, gsdate); needs to be fixed!
        gsm_msg(smstxt);
        gsm_receive(1, gsmusm);
        gsm_msg(smsdel);
        gsm_receive(1, gsmtim);
        sms_report();
        asm("NOP");
    }
}

void start_sms(void)
{
    gsm_msg(setgsm);
    gsm_receive(1, gsmmsg);
    gsm_msg(smstxt);
    gsm_receive(1, gsmmsg);
    gsm_msg(sendms);
    gsm_msg(pnum);
    gsm_numack();
    //if gsmflags.abrtmsg send call me instead
}

void gsm_numack(void)
{
    gsmbyte = 0x00;
    uint8_t x = 4;
    gsmflags.abrtmsg = 0;
    while(x > 0)
    {
        gsmbyte = EUSARTG_Read();
        if(gsmbyte == '4')
        {
            gsmflags.abrtmsg = 1;
            break;
        }
        if(gsmbyte == '\"')
        {
            gsmflags.abrtmsg = 0;
            break;
        }
        x--;
    }
}

void sms_report(void)
{
    uint8_t i = 0;
    uint8_t z = i;
    gsm_gettime();
    
    gsm_zerobuff(gsmums, 0x200);
    gsmums[i] = 0x20;
    //Read in "Date "
    i = write_sms(i,clockdate);
    //Read in actual date.
    i = write_sms(i,gsdate);
    gsmums[i++] = 0x20;
    //Things go wrong!
    i = write_sms(i,clocktime);
    i = write_sms(i,gstime);
    gsmums[i++] = 0x20;
    gsmums[i++] = ',';
    gsmums[i++] = 0x20;
    gsmums[i++] = 'R';
    Read_NVstore(cashinv, ((uint8_t*) &pvcash), 0x02);
    Read_NVstore(cashint, ((uint8_t*) &pnvcash), 0x03);
    uint8_t *gsmval = convert_hex((__uint24) pvcash);
    i = write_sms(i,gsmval);
    gsmums[i++] = ',';
    gsmums[i++] = 0x20;
    gsmums[i++] = 'R';
    gsmval = convert_hex(pnvcash);
    i = write_sms(i,gsmval);
    gsmums[i++] = ',';
    i = write_sms(i, totalvendsm);
    //Total vends
    uint16_t vendstores = vendstore;
    for(char x = 0; x < 0x08; x++)
    {
        gsmums[i++] = (x +1) | 0x30;
        gsmums[i++] = '=';
        gsmbyte = DATAEE_ReadByte(vendstores + x);
        gsmval = convert_hex((__uint24) gsmbyte);
        i = write_sms(i,gsmval);
        gsmums[i++] = ',';
    }
    gsmums[i++] = 0x20;
    //Display change coins given
    i = write_sms(i,coinsout);
    Read_NVstore(cashoutv, ((uint8_t*) &pvcash), 0x02);
    gsmval = convert_hex((__uint24) pvcash);
    i = write_sms(i,gsmval);
    i = write_sms(i,coinvalu);
    gsmbyte = DATAEE_ReadByte(hopcoin);
    gsmums[i++] = gsmbyte | 0x30;
    
    //Null string terminator
    gsmums[i] = 0x00;
    start_sms();
    //if gsmflags.abrtmsg send call me instead
    if(gsmflags.abrtmsg)
    {
        
    }
    gsm_msg(gsmums);
    gsm_transmit(0x1A);
    gsm_transmit(0x0D);
}

uint8_t write_sms(uint8_t i, uint8_t *msgpnt)
{
    uint8_t x = 0x00;
    gsmbyte = 0x20;
    while(gsmbyte != 0x00)
    {
        gsmbyte = msgpnt[x++];
        gsmums[i++] = gsmbyte;
    }
    return --i;
}

//Take a 24 bit hex value and convert it.
// Return pointer to eight ASCII digits lsd first.
uint8_t* convert_hex(__uint24 hexnum)
{
    signed char x = 0;
    uint8_t value[8];
    uint8_t xvalue[8];
    __uint24 hexnumsave = hexnum;
    while(hexnum > 0)
    {
        xvalue[x] = hexnum % 10;
        hexnum /= 10;
        xvalue[x] = xvalue[x] | 0x30;
        x++;
    }
    //insert null string terminator
    value[x--] = 0x00;
    uint8_t y = 0;
    while(x >= 0)
    {
        value[x--] = xvalue[y++];
    }
    
        //In case hexnum is zero
    if(hexnumsave == 0)
    {
       value[0] = 0x30;
       value[1] = 0x00;
    }
    return value;

}
void parse_date_time(void)
{
    gsm_zerobuff(gsdate, 0x014);
    gsm_zerobuff(gstime, 0x014);
    uint8_t x = 8;
    uint8_t i = 0x07;
    gsmbyte = gsmmsg[27];
    gsmbyte = gsmbyte & 0x0F;
    gsmbyte = gsmbyte/4;
    uint8_t hour = (gsmmsg[18] & 0x0F) + gsmbyte;
    uint8_t hhour = gsmmsg[17] & 0x0F;
    if(hour > 9)
    {
        gsmmsg[18] = 0x30;
        gsmmsg[17]++;
        if(gsmmsg[17] > 0x32)
        {
            gsmmsg[17] = 0x30;
        }
        while(hour > 0x0A)
        {
            hour--;
            gsmmsg[18]++;
        }
    }
    else
    {
        gsmmsg[18] = hour | 0x30;
    }
    while(x < 0x10)
    {
        gsdate[i - 1] = gsmmsg[x++];
        gsdate[i--] = gsmmsg[x++];
        gsdate[--i] = gsmmsg[x++];
        i--;
    }
    i = 0;
    while(i < 0x08)
    {
        gstime[i++] = gsmmsg[x++];
    }
}

void gsm_gettime(void)
{
    gsm_msg(clockr);
    
    gsm_receive(2,gsmmsg);
    parse_date_time();
}

void gsm_getbalance(void)
{
    gsm_msg(setgsm);
    gsm_receive(1,gsmmsg);
    if(gsmflags.mtn)
    {
        gsm_msg(ussdwm);
        gsm_receive(11,gsmums);
    }
    else
    {
        gsm_msg(ussdwv);
        gsm_receive(3,gsmums);
    }
    gsm_msg(ussdwc);
    gsm_receive(1,gsmmsg);
}

void gsm_setime(void)
{
    gsm_zerobuff(gsmmsg, 0x200);
    gsm_msg(nettst);
//    gsm_receive(5,gsmmsg);
    retry:
    gsmbyte = 0;
    uint8_t x = 0;
    //Search for *
    while(gsmbyte != '*')
    {
        gsmbyte = EUSARTG_Read();
        if(gsmflags.meerror)
        {
            goto offnet;
        }
    }
    gsmbyte = EUSARTG_Read(); //P
    gsmbyte = EUSARTG_Read(); //S
    gsmbyte = EUSARTG_Read(); //U
    if(gsmbyte != 'U')
    {
        goto retry;
    }
    while(gsmbyte != ':')
    {
        gsmbyte = EUSARTG_Read();
    }
    while(gsmbyte != '\n')
    {
        gsmbyte = EUSARTG_Read();
        gsmmsg[x++] = gsmbyte;
    }
    //In case of DST message.
    __delay_ms(5);


    x = 0x03;
    gsmtim[0] = '"';
    uint8_t i = 0x01;
    while(i < 0x09)
    {
        gsmtim[i] = gsmmsg[x];
        gsmtim[i+1] = gsmmsg[x+1];
        if(gsmmsg[x+1] == ',')
        {
            gsmtim[i+1] = gsmmsg[x];
            gsmtim[i] = '0';
            x--;
        }
        gsmtim[i+2] = '/';
        i = i + 3;
        x = x + 3;
    }
    gsmtim[i-1] = ',';
    
    
    while(i < 0x11)
    {
        gsmtim[i] = gsmmsg[x];
        gsmtim[i+1] = gsmmsg[x+1];
        if(gsmmsg[x+1] == ',')
        {
            gsmtim[i+1] = gsmmsg[x];
            gsmtim[i] = '0';
            x--;
        }
        gsmtim[i+2] = ':';
        i = i + 3;
        x = x + 3;
    }
    //This should take us to time zone dec i
    i--;
    x++;
    gsmtim[i] = gsmmsg[x];
    gsmtim[i+1] = '0';
    gsmtim[i+2] = gsmmsg[x+1];
    gsmtim[i+3] = '"';
    gsmtim[i+4] = '\r';
    gsmtim[i+5] = 0x00;
    offnet:
    gsm_msg(netoff);
    gsm_receive(1, gsmmsg);
    gsm_msg(clockw);
    gsm_msg(gsmtim);
    gsm_receive(1, gsmmsg);
    if(gsmmsg[0] != '0')
    {
        LATCbits.LC3 = 0;
        __delay_ms(2000);
        asm("RESET");
    }
}
//Receive unsolicited messages for 16 seconds
//and return the number of newlines.
uint8_t gsm_unsolic(void)
{
    gsm_zerobuff(gsmums, 0x200);
    uint8_t i = 1;
    uint16_t x = 0;
    uint8_t y = 0;
    resttim:
    TMR4_Initialize();
    T4CONbits.TMR4ON = 1;
    while(!PIR4bits.TMR4IF && x < 0x200)
    {
        gsmums[x] = gsm_Read();
        if(gsmums[x] == '\n')
        {
            y++;
        }
        x++;
    }
    if(i > 0)
    {
        i--;
        goto resttim;
    }
    return y;
}

void gsm_receive(uint8_t noofline, uint8_t *messagebuf)
{
    gsm_zerobuff(messagebuf, 0x200);
    uint8_t x = 0;
    gsmbyte = 0;
    while(noofline > 0x00)
    {
        gsmbyte = EUSARTG_Read();
        if(gsmflags.meerror)
        {
            break;
        }
        messagebuf[x] = gsmbyte;
        x++;
        if(gsmbyte == 0x0A || gsmbyte == 0x00)
        {
            noofline--;
        }
    }
}

void gsm_txAT(void)
{
    gsm_transmit('A');
    gsm_transmit('T');
}

//Turn on the gsm unit and initialize the 3 second startup timer.
void gsm_on(void)
{
    LATCbits.LC3 = 1;
    TMR4_Initialize();
    T4CONbits.T4ON = 1;
}

uint8_t gsm_Read(void)
{
    while(!PIR3bits.RC2IF)
    {
        if(PIR4bits.TMR4IF)
        {
            break;
        }
    }

    
    if(1 == RC2STAbits.OERR)
    {
        // EUSART2 error - restart

        RC2STAbits.CREN = 0; 
        RC2STAbits.CREN = 1; 
    }

    return RC2REG;
}

void gsm_off(void)
{
    LATCbits.LC3 = 0;
}

void gsm_netwait(void)
{
    gsmbyte = 0;
    uint8_t x = 0;
    while(gsmbyte != 'S')
    {
        gsmbyte = EUSARTG_Read();
        gsmmsg[x++] = gsmbyte;
    }
    while(gsmbyte != '\n')
    {
        gsmbyte = EUSARTG_Read();
        gsmmsg[x++] = gsmbyte;
    }
    gsmflags.mtn = 0;
    if(gsmmsg[0] == '+')
    {
        gsmflags.mtn = 1;
    }
}

uint8_t EUSARTG_Read(void)
{
//    TRISCbits.TRISC3 = 1;
    while(!PIR3bits.RC2IF)
    {
        
    }
//    TRISCbits.TRISC3 = 0;

    
    if(RC2STAbits.OERR)
    {
        // EUSART2 error - restart

        RC2STAbits.CREN = 0; 
        RC2STAbits.CREN = 1; 
    }

    return RC2REG;
}


//PIR3bits.TX2IF -  EUSART2 Transmit Interrupt Flag bit
//1 = The EUSART2 transmit buffer, TX2REG, is empty (cleared by writing TX2REG)
//0 = The EUSART2 transmit buffer is full

//&& TX2STAbits.TXEN -  Transmit Enable bit SREN/CREN bits of RCxSTA (Register 27-2) override TXEN in Sync mode.
// 1 = Transmit enabled
// 0 = Transmit disabled

//TX2STAbits.TRMT -  Transmit Shift Register Status bit
// 1 = TSR empty TSR = Tx shift reg
// 0 = TSR full

//PIR3bits.RC2IF -  EUSART2 Receive Interrupt Flag bit
//1 = The EUSART2 receive buffer, RC2REG, is full (cleared by reading RC2REG)
//0 = The EUSART2 receive buffer is empty


