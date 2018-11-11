/* 
 * File:   cctalk.h
 * Author: Dave Plater
 *
 * Created on 07 November 2017, 10:57 AM
 */

#ifndef CCTALK_H
#define	CCTALK_H

#include "vend.h"
#include <xc.h>
#include "lcd.h"
#include "mcc_generated_files/mcc.h"
#include "mdb.h"

//Address of hopper
const uint8_t hopadd = 0x03;
//My address
const uint8_t mastadd = 0x01;
//Reset hopper wait 60mS
const uint8_t cc_reset = 0x01;
//Bit0 motor overload, bit1 payment time out, bit2 blockage free attempt
//bit3 block sensor attempt, bit4 nop, bit5 sensor blocked when paying
//bit 6 nop, bit7 payment disabled
const uint8_t cc_test = 0xA3;
//0xA5 activates, not 0xA5 deactivates
const uint8_t cc_enable =0xA4;
//Returns coins paid since reset if byte 1 not 0 then
//coins to be paid and coins paid
const uint8_t cc_status = 0xA6;
//Pay 1st byte to 3rd byte = serial number from cc_serial
//Last byte coins to pay.
const uint8_t cc_pay = 0xA7;
//Returns number coins since reset
const uint8_t cc_countrq = 0xA8;
//Emergency stop returns coins left to pay
const uint8_t cc_stop = 0xAC;
//Read opto status bit 0 = low bit 1 = full 11 = nearly empty
const uint8_t cc_level = 0xEC;
//Enter pin no
const uint8_t cc_pin = 0xDA;
//Enter new pin no
const uint8_t cc_newpin = 0xDB;
//Request serial number returns lsb to msb 3 byte serial number
//To be used when paying out.
const uint8_t cc_serial = 0xF2;
//Detect address
const uint8_t cc_adpoll = 0xFD;
//simple poll to detect hopper
const uint8_t cc_poll = 0xFE;

uint8_t dest;
uint8_t nobytes;
uint8_t command;
//This stores the serial number of the hopper
//needed for the pay command
uint8_t hpserial[3];

struct
{
    unsigned isdata : 1;
    unsigned hoperr : 1;
    unsigned mode : 1;
    unsigned timeout : 1;
    unsigned spare2 : 1;
    unsigned spare3 : 1;
    unsigned spare4 : 1;
    unsigned spare5 : 1;
}cctflags;

void cctalk_adpoll(void);
void cctalk_pinenter(void);
void cctalk_init(void);
void cctalk_poll(void);
void cctalk_serial(void);
void cctalk_off(void);
void cctalk_on(void);
uint8_t cctalk_read(void);
//cctalk message destination, number of bytes, command, and 
uint8_t cctalk_comm(uint8_t dest, uint8_t nobytes, uint8_t command);

#endif	/* CCTALK_H */

