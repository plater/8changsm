/* 
 * File:   mdb.h
 * Author: davepl
 *
 * Created on November 5, 2017, 4:51 PM
 */

#ifndef MDB_H
#define	MDB_H
#include "vend.h"
#include <xc.h>
#include "lcd.h"
#include "mcc_generated_files/mcc.h"

//Next are the MDB address commands
//Reset bill accepter
const uint8_t note_reset = 0x30;
//Setup request setup data and look for response.
const uint8_t note_setup = 0x31;
//Set security mode
const uint8_t note_security = 0x32;
//Poll the note reader and look for response.
const uint8_t note_poll = 0x33;
//Enable or disable note  channels, more data to be sent
const uint8_t note_type = 0x34;
//Send escrow accept or reject.
const uint8_t note_escrow = 0x35;
//Stacker commands
const uint8_t note_stacker = 0x36;
//Further feature setup commands
const uint8_t note_features = 0x33;
//These are received from note reader.
const uint8_t motr_error = 0x01;
const uint8_t sens_error = 0x02;
const uint8_t just_busy = 0x03;
const uint8_t rom_error = 0x04;
const uint8_t jammed_err = 0x05;
const uint8_t just_reset = 0x06;
const uint8_t note_removed = 0x07;
const uint8_t cashbox_err = 0x08;
const uint8_t acept_disabled = 0x09;
const uint8_t invalid_escrow = 0x0A;
const uint8_t note_rejected = 0x0B;
const uint8_t theft_attempt = 0x0C;

//32 bytes allowed location 32 should always be zero
uint8_t mdbdata[31];
uint8_t slvadd;
uint8_t mcount = 0;
uint8_t notebyte;
uint8_t chkbyte;
uint16_t dbcount=0;
uint8_t notenum = 0xFF;

//const uint8_t note_reset = 0x30;
//MDB flags isdata noteer mode = 1 master address or end of slave transmission.
struct
{
    unsigned isdata : 1;
    unsigned noteerr : 1;
    unsigned mode : 1;
    unsigned timeout : 1;
    //This flag is set when note reader is deliberately disabled
    unsigned vending : 1;
    unsigned noteset : 1;
    unsigned endis : 1;
    unsigned spare5 : 1;
}mdbflags;

//Notes enabled or disabled
//Initialize all enabled
struct 
{
    unsigned R10 : 1;
    unsigned R20 : 1;
    unsigned R50 : 1;
    unsigned R100 : 1;
    unsigned R200 : 1;
    unsigned spare : 1;
    unsigned spare1 : 1;
    unsigned spare2 : 1;
}noteen;
volatile uint8_t noteen_byte;

//Chanel disable flags
struct
{
    unsigned chan1 : 1;
    unsigned chan2 : 1;
    unsigned chan3 : 1;
    unsigned chan4 : 1;
    unsigned chan5 : 1;
    unsigned chan6 : 1;
    unsigned chan7 : 1;
    unsigned chan8 : 1;
 } errorflags;

 uint8_t mdbtxbyte;

//MDB and CCTALK defines

void mdb_init(void);
 
void init_mdbdata(uint8_t);

uint8_t mdb_reset(void);

void set_notes(void);

void save_notes(void);

void enable_notes(uint8_t notenum);

void note_disable(void);

void mdb_noten(void);

void mdb_security(void);

uint8_t mdb_poll(void);

uint8_t mdb_comm(uint8_t slvadd, uint8_t mcount);

void mdb_on(void);

void mdb_ron(void);

uint8_t wait_ack(void);

void mdb_test(void);

void mdb_waitx(void);

void mdb_waitr(void);

void mdb_unlock(void);

void mdb_transmit(uint8_t txbyte);

#endif	/* MDB_H */

