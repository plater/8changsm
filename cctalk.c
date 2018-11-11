/* 
 * File:   cctalk.c
 * Author: Dave Plater
 *
 * Created on 07 November 2017, 10:57 AM
 */
#include "cctalk.h"

void cctalk_init(void)
{
    cctalk_on();
    cctflags.timeout = 1;
    cctalk_adpoll();
    cctalk_pinenter();
    count = 0;
    while(cctflags.timeout && count < 0x03)
    {
        __delay_ms(50);
        cctalk_poll();
        count++;
    }
    asm("nop");
    eusartmdb_Initialize();
    mdb_on();
}

void cctalk_adpoll(void)
{
    uint8_t i = cctalk_comm(0x00, 0x00, cc_adpoll);
}

void cctalk_pinenter(void)
{
    init_mdbdata(0x00);
    uint8_t i = cctalk_comm(hopadd, 0x04, cc_pin);
}

void cctalk_poll(void)
{
    uint8_t i = cctalk_comm(hopadd, 0x00, cc_poll);
}

void cctalk_serial(void)
{
    uint8_t i = 0;
    i = cctalk_comm(hopadd, 0x00, cc_serial);
    //timeout flag set if data incomplete
    if(!cctflags.timeout)
    {
        hpserial[0] = mdbdata[4];
        hpserial[1] = mdbdata[5];
        hpserial[2] = mdbdata[6];
    }
}

uint8_t cctalk_comm(uint8_t dest, uint8_t nobytes, uint8_t command)
{
    //Format destination, number data bytes, Source, command, data, checksum.
    uint8_t i = 0;
    //Turn of receive buffer due to common bus.
//    RC1STAbits.CREN = 0;
    //Make sure receive buffer is clear
    i = RC1REG;
//    dest = hopadd;
    i = 0;
    //RC1STAbits.SPEN = 0;
    //RC1STAbits.CREN = 0;
    chkbyte = dest + nobytes + command + mastadd;
    mdb_transmit(dest);
    
    mdb_transmit(nobytes);
    
    mdb_transmit(mastadd);
    
    mdb_transmit(command);
    
    while(nobytes != 0)
    {
        chkbyte = mdbdata[nobytes] + chkbyte;
        mdb_transmit(mdbdata[nobytes]);
        
        nobytes--;
    }
    chkbyte = 0 - chkbyte;
    mdb_transmit(chkbyte);
    //Now we wait for a reply.
    //Format bytes: 1=destination must = master, 2=nobytes, 3=source must = hopper,
    //4=data determined by nobytes, 5=checksum
    cctflags.timeout = 0;
    cctflags.hoperr = 0;
    //RC1STAbits.SPEN = 1;
    //RC1STAbits.CREN = 1;
    init_mdbdata(0xFF);
    RC1STAbits.SPEN = 0;
    RC1STAbits.SPEN = 1;
    chkbyte = RC1REG;
    //50mS response timeout
    TMR3_Initialize();
    while(!cctflags.timeout)
    {
        //receive master address
        dest = cctalk_read();
        
        if(cctflags.isdata)
        {
            mdbdata[i++] = dest;
            
        }
        cctflags.timeout = PIR4bits.TMR3IF;
    }
    if(mdbdata[0] == mastadd && mdbdata[3] == 0x00)
    {
        return i;
    }
    cctflags.timeout = 1;
    return 0x00;
}

uint8_t cctalk_read(void)
{
    uint8_t data = 0;
    cctflags.isdata = 0;
    while(PIR3bits.RC1IF)
    {
        data = EUSART1_Read();
        mdb_waitr();
        cctflags.isdata = 1;
    }
    return data;
}
//We aren't using a cctalk hopper this turns the normal hopper off
void cctalk_off(void)
{
    LATCbits.LATC1 = 0;
}
//We aren't using a cctalk hopper this turns the normal hopper on
void cctalk_on(void)
{
    //Select 8 bit mode for cctalk
    //An MDB reset might be required afterwards
//    RC1STA = 0x90;
//    TX1STA = 0xAC;
//    LATCbits.LATC0 = 0;
    LATCbits.LATC1 = 1;
}

