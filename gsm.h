/* 
 * File:   gsm.h
 * Author: Dave Plater
 *
 * Created on September 26, 2018, 3:14 PM
 */

#ifndef GSM_H
#define	GSM_H

#ifdef	__cplusplus
extern "C" {
#endif
#include <string.h>
#include "vend.h"
#include <xc.h>
#include "lcd.h"
#include "mcc_generated_files/mcc.h"

//Gsm related memory
uint8_t gsmbyte = 0;
uint8_t gsmmsg[512];
uint8_t gsmums[512];
uint8_t gsmusm[256];
uint8_t gsmtim[23];
uint8_t gsdate[10];
uint8_t gstime[10];
uint8_t noofline;

//GSM messages
const uint8_t smsdel[] = "AT+CMGDA=\"DEL ALL\"\r";
const uint8_t smslst[] = "AT+CMGL=\"ALL\"\r";
const uint8_t smscap[] = "AT+CMGL=?\r";
const uint8_t smstxt[] = "AT+CMGF=1\r";
const uint8_t engqry[] = "AT+AT+CEER\r";
const uint8_t engfmt[] = "AT+CENG=?";
const uint8_t engwrt[] = "AT+CENG?";
const uint8_t noecho[] = "ATE0;V0\r";
const uint8_t usdtst[] = "AT+CUSD=?\r";
const uint8_t ussdrd[] = "AT+CUSD?\r";
const uint8_t ussdwe[] = "AT+CUSD=1\r";
const uint8_t ussdwc[] = "AT+CUSD=2\r";
const uint8_t ussdwv[] = "AT+CUSD=1,\"*135*500#\"\r";
const uint8_t ussdwm[] = "AT+CUSD=1,\"*136#\"\r";
const uint8_t ussdw2[] = "AT+CUSD=1,\"*136#\",2\r";
const uint8_t setgsm[] = "AT+CSCS=\"GSM\"\r";
const uint8_t airtim[] = "AT+CUSD =1\r";
const uint8_t prodid[] = "ATI\r";
const uint8_t dconfg[] = "AT&V\r";
const uint8_t ownnum[] = "AT+CNUM\r";
const uint8_t clockr[] = "AT+CCLK?\r";
const uint8_t clockw[] = "AT+CCLK=";
const uint8_t pwrsta[] = "AT+CBC\r";
const uint8_t loctim[] = "AT+CIPGSMLOC=2\r";
const uint8_t nettst[] = "AT+CLTS=1\r";
const uint8_t netoff[] = "AT+CLTS=0\r";
const uint8_t baudra[] = "AT+IPR=19200\r";
const uint8_t facres[] = "AT&F0\r";

struct
{
    unsigned retransmit : 1;
    unsigned meerror : 1;
    unsigned mtn : 1;
    unsigned msgavl : 1;
    unsigned spare4 : 1;
    unsigned spare5 : 1;
    unsigned spare6 : 1;
    unsigned spare7 : 1;
} gsmflags;

void gsm_zerobuff(uint8_t* gsmsgbuf, uint16_t count );

void gsm_waitx(void);

void gsm_waitr(void);

void gsm_transmit(uint8_t txbyte);

void gsm_gettime(void);

void gsm_getbalance(void);

void gsm_setime(void);

void parse_date_time(void);

uint8_t gsm_unsolic(void);

void clock_display(void);

void gsm_receive(uint8_t noofline, uint8_t *messagebuf);

void gsm_msg(uint8_t *msgadd);

void gsm_init(void);

void gsm_txAT(void);

void gsm_on(void);

uint8_t gsm_Read(void);

void gsm_off(void);

void gsm_netwait(void);

uint8_t EUSARTG_Read(void);

#ifdef	__cplusplus
}
#endif

#endif	/* GSM_H */

