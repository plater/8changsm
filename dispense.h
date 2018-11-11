/* 
 * File:   dispense.h
 * Author: Dave Plater
 *
 * Created on 24 March 2018, 12:49 PM
 */

#ifndef DISPENSE_H
#define	DISPENSE_H

#include "vend.h"
#include "lcd.h"

bool price_check(void);

void dispense(uint8_t chanel);

void vend_nosense(uint8_t chanel);

void vend_dispense(uint8_t chanel);

void set_motortime(uint8_t chanel);

void find_highprice(void);

void find_lowprice(void);

void create_chanmask(void);

void disable_channel(uint8_t chanel);

void init_pricestore(void);

void update_vends(uint8_t channel);

#endif	/* DISPENSE_H */

