/* 
 * File:   vend.h
 * Author: Dave Plater
 *
 * Created on 16 October 2017, 12:38 PM
 */

#ifndef VEND_H
#define	VEND_H

#include <xc.h>
#include "lcd.h"
#include "mdb.h"
#include "cctalk.h"
#include "hopper.h"
#include "dispense.h"
#include "mcc_generated_files/mcc.h"
#include "mcc_generated_files/pin_manager.h"
//This location is the value of cash to be added or
//subtracted from credit memory
volatile uint8_t credit;
volatile uint8_t cash;
volatile uint8_t buttons;
volatile uint8_t vendprice;
volatile uint8_t sensorval;
volatile uint8_t vcash[2];
volatile uint8_t nvcash[3];
volatile uint8_t tvends[8];
volatile uint8_t vcashout[2];
volatile uint16_t pvcash;
volatile uint24_t pnvcash;
volatile uint8_t senspos;
volatile uint8_t dummy[8];
volatile uint8_t pricevend[8];
volatile uint8_t highprice;
volatile uint8_t lowprice;
volatile uint8_t chanmask;
volatile uint8_t errormask;
volatile uint8_t channel;

//Timer values for vend motor timing
const uint16_t second1 = 0xE1BA ;
//Subtract this to increase by 1/2 second
const uint16_t second_5 = 0x0F23 ;
//Subtract this to increase by 1 second
const uint16_t secondadd = 0x1E46 ;

//NVRAM address location definitions
//This location holds the value of cash in hand
const uint16_t credmem = 0x00;
//Clearable total cash uint16_t
const uint16_t  cashinv = 0x02;
//Total vends, clearable 8 x uint8_t
const uint16_t vendstore = 0x04;
//Non clearable total cash uint_24_t
const uint16_t cashint = 0x0C;
//Prices per vend 8 x uint8_t
const uint16_t pricestore = 0x0F;
//Vend error flags bits 0 to 7 uint8_t
const uint16_t venderrors = 0x17;
//Note enable bits 0 to 4 uint8_t
const uint16_t notebits = 0x18;
//Opto vend sensor value, 0 = Least sensitive
// 0x1F = Most sensitive.
const uint16_t sensval = 0x19;
//Total cash change given 2 bytes
const uint16_t cashoutv = 0x1A;
//Value of coin dispensed as change
const uint16_t hopcoin = 0x1C;
//8 x channel link flag bytes each one byte
const uint16_t chan1linkbits = 0x1D;
const uint16_t chan2linkbits = 0x1E;
const uint16_t chan3linkbits = 0x1F;
const uint16_t chan4linkbits = 0x20;
const uint16_t chan5linkbits = 0x21;
const uint16_t chan6linkbits = 0x22;
const uint16_t chan7linkbits = 0x23;
const uint16_t chan8linkbits = 0x24;
//Flags to disable sensor check storage
const uint16_t sensorflags = 0x25;
//Vend motor drive time for each channel.
//Contains number of seconds.
const uint16_t chan1time = 0x26;
const uint16_t chan2time = 0x27;
const uint16_t chan3time = 0x28;
const uint16_t chan4time = 0x29;
const uint16_t chan5time = 0x2A;
const uint16_t chan6time = 0x2B;
const uint16_t chan7time = 0x2C;
const uint16_t chan8time = 0x2D;
//No sensor pulse time.
const uint16_t nosentime = 0x2E;
//Hopper error disable byte
const uint16_t hoperror = 0x2F;
//Flags to disable sensor check on vend if set
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
} senschk;

//To access bit structures ((uint8_t*) &struct_name) for 16 bit structures uint16_t

//Channel error flags NV stored in venderrors
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
} venderr;

//These bit structures enable the channels to be linked ie. vend pie and coke.
//If one of these bits is set in chanlinkx then a vend on channel x will also
//vend on the channel designated by chanlinkx.chany y being the other channel.
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
} chanlink1 @ 0x003;

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
} chanlink2 @ 0x004;

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
} chanlink3 @ 0x005;

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
} chanlink4 @ 0x006;

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
} chanlink5 @ 0x007;

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
} chanlink6 @ 0x008;

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
} chanlink7 @ 0x009;

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
} chanlink8 @ 0x00A;

/*vend flags bit 0=still credit bit 1=error bit 2=display insert coin
  bit 3=no change bit4=credit display bit5=Buy another?*/
struct
{
   unsigned iscredit : 1;
   unsigned error : 1;
   unsigned inscoin : 1;
   unsigned nochange : 1;
   unsigned credisplay : 1;
   unsigned initialrun : 1;
   unsigned noterr : 1;
   unsigned service : 1;
   unsigned priceset : 1;
   unsigned setprice : 1;
   unsigned vendset : 1;
   unsigned chanlink : 1;
   unsigned mottime : 1;
   unsigned nosense : 1;
   unsigned settime : 1;
   unsigned sensno : 1;
   unsigned linkchan : 1;
   unsigned swclosed : 1;
   unsigned pricedisplay : 1;
   unsigned audit : 1;
   unsigned hiprice : 1;
   unsigned spare22 : 1;
   unsigned spare23 : 1;
   unsigned spare24 : 1;
} venflags;
//Write multiple EEprom bytes to ram array
void Write_NVstore(uint16_t storeadd, uint8_t *storemem, uint8_t storesize);
//Read multiple EEProm to ram array.
void Read_NVstore(uint16_t storeadd, uint8_t *storemem, uint8_t storesize);
//added to credit memory
void credit_add(uint8_t credit);
//subtracted from credit memory
void credit_subtract(uint8_t credit);
//Initialize flags
void vend_init(void);
//Initialize from NVRam
void Init_vendmem(void);

void Update_cashaudit(uint8_t addcash);

void Update_Hopcoin(uint8_t newval, uint8_t direction);

//Return credit in memory
uint8_t credit_check(void);

void setup(void);
// Only return when button released
uint8_t butindb(void);
//Read buttons return 1 for pressed button.
uint8_t butin(void);
//Audit display erasable total cash, vends and hopper coins out, clear totals
// also display unclearable total cash
void Audit(void);
//Set the value of dispensed coin
void Hopper_coin(void);

void Clear_Totals(void);

void Sensor_set(void);

void Clear_cred(void);

uint8_t Read_Service(void);

void price_set(void);
//Convert button bit to channel number 1 to 8
uint8_t get_channel(uint8_t buttons);

void set_price(uint8_t buttons);

void Vend_setup(void);

void Vend_settime(void);

void Vend_timeset(uint8_t channel);

void Chan_link(void);

void Link_chan(uint8_t channel);

void Sens_off(void);

void off_sens(uint8_t channel);

void Reset_settings(void);

#endif	/* VEND_H */

