/* 
 * File:   hopper.h
 * Author: davepl
 *
 * Created on 21 March 2018, 12:42 PM
 */

#ifndef HOPPER_H
#define	HOPPER_H
#include "vend.h"
//#include <xc.h>
//#include "tmr4.h"
//#include "tmr2.h"

uint8_t change = 0;
uint8_t outcoins;
signed char hopercount = 3;

void hopper_test(void);
bool hopper_pay(uint8_t change);
bool switch_read(void);
bool pay_coin(void);
void Update_coinsout(uint8_t coinsout);
bool Reset_hopper(void);
void give_change(uint8_t paycash);

#endif	/* HOPPER_H */

