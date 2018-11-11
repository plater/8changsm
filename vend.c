/* 
 * File:   vend.c
 * Author: Dave Plater
 *
 * Created on 16 October 2017, 12:38 PM
 */
#include "vend.h"
//Read values stored in NVRam and initialize link and time flags.
void Init_vendmem(void)
{
    //Initialize sensor check on/off flags
    ((uint8_t*) &senschk)[0] = DATAEE_ReadByte(sensorflags);
    //Initialize 8 x 8 channel link flags
    uint16_t chanlinkbits = chan1linkbits;
    uint8_t *chanlink = ((uint8_t*) &chanlink1);
    char i = 0;
    while(i < 8)
    {
       chanlink[i] = DATAEE_ReadByte(chanlinkbits + i);
       i++;
    }
    //Transfer prices to pricevend[]
    i = 0;
    while(i < 8)
    {
        pricevend[i] = DATAEE_ReadByte(pricestore + i);
        i++;
    }
}


//Initialize vend flags
void vend_init(void)
{
    //Initialize 8 vend error flags
    ((uint8_t*) &venderr)[0] = DATAEE_ReadByte(venderrors);
    venflags.hiprice = 0;
    //Initialize no change flag
    if(DATAEE_ReadByte(hoperror) != 0)
    {
        venflags.nochange = 1;
    }
    else
    {
        venflags.nochange = 0;
    }
    //Initialize channel links and vend inhibit flags
    Init_vendmem();
    find_highprice();
    find_lowprice();
    //Retrieve and set DAC for sensor comparator
    sensorval = DATAEE_ReadByte(sensval);
    DAC1_SetOutput(sensorval);
    errormask = DATAEE_ReadByte(venderrors);
    //Initialize cctalk for cctalk hopper if used
    //lcd_string(inithop, line1);
    //cctalk_init();
    //Turn on MDB
    lcd_string(initnote, line1);
    mdb_init();
    
    venflags.pricedisplay = 0;
    venflags.service = 0;
    venflags.initialrun = 1;
    if(credit_check() == 0)
    {
        venflags.iscredit = 0;
        venflags.inscoin = 1;
        venflags.credisplay = 0;
        mdbflags.noteerr = 0;
    }
    else
    {
        venflags.iscredit = 1;
        venflags.inscoin = 0;
        venflags.credisplay = 1;
        mdbflags.noteerr = 0;
    }
    //Turn off vend motor common.
    LATAbits.LA5 = 0;
    //Turn on button light common.
    LATCbits.LC2 = 1;
        
}
//Transfer bytes in storemem to storeadd
void Write_NVstore(uint16_t storeadd, uint8_t *storemem, uint8_t storesize)
{
     uint8_t i = 0;
     while(i < storesize)
     {
         DATAEE_WriteByte(storeadd + i, storemem[i]);
         i++;
     }
}
//Transfer bytes at storeadd to storemem
void Read_NVstore(uint16_t storeadd, uint8_t *storemem, uint8_t storesize)
{
    uint8_t i = 0;
    while(i < storesize)
    {
        storemem[i] = DATAEE_ReadByte(storeadd + i);
        i++;
    }
}

void Update_cashaudit(uint8_t addcash)
{
    Read_NVstore(cashinv, ((uint8_t*) &pvcash), 0x02);
    Read_NVstore(cashint, ((uint8_t*) &pnvcash), 0x03);
    //Add to clearable total cash
    pvcash = pvcash + (uint16_t)addcash;
    //Add to unclearable cash total
    pnvcash = pnvcash + (uint24_t)addcash;
    //Save updated values.
    Write_NVstore(cashinv, ((uint8_t*) &pvcash), 0x02);
    Write_NVstore(cashint, ((uint8_t*) &pnvcash), 0x03);
    asm("nop");
}


void credit_add(uint8_t credit)
{
    cash = DATAEE_ReadByte(credmem);
    cash = cash + credit;
    DATAEE_WriteByte(credmem, cash);
    if(cash != 0)
    {
        venflags.iscredit = 1;
        venflags.inscoin = 0;
        venflags.credisplay = 1;

    }
    Update_cashaudit(credit);
}

void credit_subtract(uint8_t credit)
{
    cash = DATAEE_ReadByte(credmem);
    cash = (cash - credit) & 0xFF;
    DATAEE_WriteByte(credmem, cash);
    if(cash == 0)
    {
        venflags.iscredit = 0;
        venflags.inscoin = 1;
        venflags.credisplay = 0;
    }
}

uint8_t credit_check(void)
{
    cash = DATAEE_ReadByte(credmem);
    return(cash);
}

uint8_t butindb(void)
{
    if(butin() == 0)
    {
        return 0x00;
    }
    uint8_t butt = butin();
    uint8_t butval = butt;
    while(butval != 0x00)
    {
        butval = butin();
    }
    __delay_ms(500);
    //Return when button released
    return butt;
}

uint8_t butin(void)
{
    LATA7 = 0;
    LATA6 = 1;
    __delay_ms(15);
    //Read buttons 5 to 8
    uint8_t butons = ((PORTA >> 1) & 0x0F) << 4;
    LATA7 = 1;
    LATA6 = 0;
    __delay_ms(15);
    //Read buttons 1 to 4
    butons = ((PORTA >> 1) & 0x0F) | butons;
    LATA7 = 1;
    LATA6 = 1;
    return ~butons;
}

void setup(void)
{
    
}

void Audit(void)
{
    DAC1CON1 = 0x01;
    Read_NVstore(cashinv, ((uint8_t*) &pvcash), 0x02);
    Read_NVstore(cashint, ((uint8_t*) &pnvcash), 0x03);
    lcd_string(cashinmsg, line1);
    displ_hex((uint24_t)pvcash);
    lcd_string(totalmsg, line2);
    displ_hex(pnvcash);
    lcd_string(vendispmsg, line3);
    venflags.audit = 1;
    while(venflags.audit)
    {
        buttons = butin();
        if(buttons != 0x00)
        {
            buttons = butin();
            lcd_string(clrline, line4);
            channel = get_channel(buttons);
            uint8_t vends = DATAEE_ReadByte(vendstore + channel);
            lcd_string(totalvendsm, line4);
            displ_hex((uint24_t)vends);
            while(butin() != 0x00){}
            lcd_string(clrline, line4);
        }
        if(!CMP1_GetOutputStatus())
        {
            DAC1CON1 = sensorval;
            venflags.audit = 0;
            lcd_string(servmsg, line1);
        }
    }
}

void Clear_Totals(void)
{
    uint16_t eeaddress = cashinv;
    while(eeaddress < 0x0C)
    {
        DATAEE_WriteByte(eeaddress, 0x00);
        eeaddress++;
    }
    asm("RESET");
}

void Hopper_coin(void)
{
    lcd_string(hoppcoin, line1);
    mdbflags.endis = 1;
    senspos = bflag + 2;
    uint8_t coinval = DATAEE_ReadByte(hopcoin);
    lcd_dispadd(senspos);
    displ_hex((uint24_t)coinval);
    while(mdbflags.endis)
    {
        buttons = butindb();
        switch(buttons)
        {
            case 0x01 :  Update_Hopcoin(coinval, 0x01);
            break;
            case 0x02 :  Update_Hopcoin(coinval, 0x02);
            break;
            case 0x80 :  mdbflags.endis = 0;
            lcd_string(servmsg, line1);
            buttons = 0x00;
            break;
        }
        coinval = DATAEE_ReadByte(hopcoin);
    }
}

void Update_Hopcoin(uint8_t newval, uint8_t direction)
{
    //Hopper coin can be R1, R2 or R5
    switch(newval)
    {
        case 0x01 : if(direction == 0x01) //Increase value to R2
        {
            newval++;
        }
        else
        {
            newval = 0x05;
        }
        break;
        case 0x02 : if(direction == 0x01)
        {
            newval = 0x05;
        }
        else
        {
            newval--;
        }
        break;
        case 0x05 : if(direction == 0x01)
        {
            //wrap around
            newval = 0x01;
        }
        else
        {
            newval = 0x02;
        }
        break;
        default : newval = 0x02;
    }
    lcd_string(clrmsg, senspos);
    DATAEE_WriteByte(hopcoin, newval);
    lcd_dispadd(senspos);
    displ_hex((uint24_t)newval);
}

uint8_t Read_Service(void)
{
    DAC1CON1 = 0x01;
    __delay_us(50);
    uint8_t senval = CMOUT;
    DAC1CON1 = sensorval;
    return senval;
}

void Sensor_set(void)
{
    mdbflags.endis = 1;
    //Display sensor is
    lcd_string(sensmsg, line1);
    //Save the position for on or off
    senspos = bflag + 2;
    //Display sensitivity on line 2
    lcd_string(dacmsg, line2);
    //Save the position for the sensitivity display
    uint8_t dacpos = bflag + 2;
    displ_sens(senspos, dacpos);
    while(mdbflags.endis)
    {
        buttons = butindb();
        switch(buttons)
        {
            case 0x01 : DAC1CON1 = (sensorval++) & 0x1F;
            displ_sens(senspos, dacpos);
            break;
            case 0x02 : DAC1CON1 = (sensorval--) & 0x1F;
            displ_sens(senspos, dacpos);
            break;
            case 0x80 : mdbflags.endis = 0;
            lcd_string(servmsg, line1);
            break;
        }
        if(PIR4bits.TMR3IF)
        {
            displ_sens(senspos, dacpos);
        }
    }
}

void Clear_cred(void)
{
    DATAEE_WriteByte(credmem, 0x00);
    DATAEE_WriteByte(venderrors, 0x00);
    DATAEE_WriteByte(hoperror, 0x00);
    lcd_string(creditclr, line1);
    __delay_ms(1000);
}
//Press button 6 in main menu
//button 1 increase button 2 decrease
void price_set(void)
{
    venflags.priceset = 1;
    lcd_string(pricesetmsg, line1);
    lcd_string(pricesetm2, line2);
    while(venflags.priceset)
    {
        buttons = butindb();
        if(buttons != 0x00)
        {
            set_price(buttons);
        }
        if(Read_Service() == 0x00)
        {
            venflags.priceset = 0;
        }
    }
    lcd_string(servmsg, line1);
}

uint8_t get_channel(uint8_t butons)
{
    //Convert button bit to channel number 1 to 8
    channel = 0x00;
    while(butons != 0)
    {
        channel++;
        butons = butons >> 1;
    }
    channel--;
    return channel;
}

void set_price(uint8_t buttons)
{
    venflags.setprice = 1;
    uint8_t channel = get_channel(buttons);
    //Get existing price
    vendprice = DATAEE_ReadByte(pricestore + channel);
    displ_price(vendprice);
    while(venflags.setprice)
    {
        buttons = butindb();
        //button 1 increase, button 2 decrease, button 8 exit.
        switch(buttons)
        {
            case 0x01 : vendprice++;
            DATAEE_WriteByte(pricestore + channel, vendprice);
            displ_price(vendprice);
            break;
            case 0x02 : vendprice--;
            DATAEE_WriteByte(pricestore + channel, vendprice);
            displ_price(vendprice);
            break;
            case 0x80 : venflags.setprice = 0;
            lcd_string(pricesetmsg, line1);
            lcd_string(pricesetm2, line2);

            break;
        }
            
    }
    
}
//Time to drive vend motor, sensor disable and link channels
//"Push 1 Vend Time,   2 ChanLink 3 SensOff4 ClrTot 5 FacReset 8 exit"
void Vend_setup(void)
{
    venflags.vendset = 1;
    lcd_string(vendsetup, line1);
    while(venflags.vendset)
    {
        buttons = butindb();
        switch(buttons)
        {
            case 0x01 : Vend_settime();
            lcd_string(vendsetup, line1);
            break;
            case 0x02 : Chan_link();
            lcd_string(vendsetup, line1);
            break;
            case 0x04 : Sens_off();
            lcd_string(vendsetup, line1);
            break;
            case 0x08 : Clear_Totals();
            break;
            case 0x10 : Reset_settings();
            lcd_string(vendsetup, line1);
            break;
            case 0x80 : venflags.vendset = 0;
            //Reinitialize changed flags
            Init_vendmem();
            lcd_string(servmsg, line1);
            break;
        }
    }
}

void Vend_settime(void)
{
    //setimemsg[] = "Push channel button Push service to exit"
    lcd_string(setimemsg, line1);
    buttons = butindb();
    venflags.settime = 1;
    
    while(venflags.settime)
    {
        buttons = butindb();
        if(buttons != 0x00)
        {
            Vend_timeset(get_channel(buttons));
        }
        if(Read_Service() == 0x00)
        {
            venflags.settime = 0;
        }
        
    }
}

// vendtimem[] = "Push 1 = +.5 second Push 2 = -.5 second "
void Vend_timeset(uint8_t channel)
{
    venflags.mottime = 1;
    lcd_string(vendtimem, line1);
    uint8_t chantime = DATAEE_ReadByte(chan1time + channel);
    displ_time(chantime, channel);
    while(venflags.mottime)
    {
        buttons = butindb();
        switch(buttons)
        {
            case 0x01 : chantime++;
            DATAEE_WriteByte(chan1time + channel, chantime);
            displ_time(chantime, channel);
            break;
            case 0x02 : chantime--;
            DATAEE_WriteByte(chan1time + channel, chantime);
            displ_time(chantime, channel);
            break;
            case 0x80 : venflags.mottime = 0;
            lcd_string(setimemsg, line1);
            break;
        }
    }
    
}

void Chan_link(void)
{
    lcd_string(chanlinkm, line1);
    venflags.chanlink = 1;
    while(venflags.chanlink)
    {
        buttons = butindb();
        if(buttons != 0x00)
        {
            Link_chan(get_channel(buttons));
            __delay_ms(1000);
        }
        if(Read_Service() == 0x00)
        {
            venflags.chanlink = 0;
        }
        
    }
    
}

void Link_chan(uint8_t channel)
{
    venflags.linkchan = 1;
    uint8_t chanbit = buttons;
    uint8_t linkflags = DATAEE_ReadByte(chan1linkbits + channel);
    displ_lflags(channel, linkflags);
    while(venflags.linkchan)
    {
        buttons = butindb();
        if(buttons != 0x00)
        {
            if(buttons && linkflags)
            {
                //linkflag set so unset it
                linkflags = linkflags & ~buttons;
            }
            else
            {
                //linkflag not set so set it
                linkflags = linkflags | buttons;
            }
            DATAEE_WriteByte(chan1linkbits + channel, linkflags);
            displ_lflags(channel, linkflags);
        }
        while(Read_Service() == 0x00)
        {
            venflags.linkchan = 0;
            lcd_string(chanlinkm, line1);
        }
    }
    
}


void Sens_off(void)
{
    __delay_ms(250);
    buttons = butindb();
    while(0x04 == buttons)
    {
        buttons = butindb();
    }
    venflags.nosense = 1;
    lcd_string(setimemsg, line1);
    buttons = butindb();
    while(venflags.nosense)
    {
        buttons = butindb();
        if(buttons != 0x00)
        {
            off_sens(get_channel(buttons));
        }
        if(Read_Service() == 0x00)
        {
            venflags.nosense = 0;
        }
        
    }
    
}
//"Hold channel button or Push service to  exit"
void off_sens(uint8_t channel)
{
    uint8_t chanbit = buttons;
    venflags.sensno = 1;
    uint8_t senseflags = DATAEE_ReadByte(sensorflags);
    displ_sflags(senseflags, channel, chanbit);
    //Push 1 = On, 2 = Off, 8 = Exit
    while(venflags.sensno)
    {
        buttons = butindb();
        switch(buttons)
        {
            //clear bit for sensed vend
            case 0x01 : senseflags = senseflags & ~chanbit;
            DATAEE_WriteByte(sensorflags, senseflags);
            displ_sflags(senseflags, channel, chanbit);
            break;
            //set bit to 1 for no sensor
            case 0x02 : senseflags = senseflags | chanbit;
            DATAEE_WriteByte(sensorflags, senseflags);
            displ_sflags(senseflags, channel, chanbit);
            break;
            case 0x80 : venflags.sensno = 0;
            lcd_string(setimemsg, line1);
            break;
        }
    }
}

void Reset_settings(void)
{
    lcd_string(chanresetmsg, line1);
    uint16_t chanlinkbits = chan1linkbits;
    while(chanlinkbits < chan1time)
    {
        //Set to zero all link bits and sensor disable bits
        DATAEE_WriteByte(chanlinkbits, 0x00);
        chanlinkbits++;
    }
    uint16_t chantimebits = chan1time;
    while(chantimebits < nosentime)
    {
        //Set all motors to 4 seconds
        DATAEE_WriteByte(chantimebits, 0x08);
        chantimebits++;
    }
   DATAEE_WriteByte(sensorflags, 0x05);
    __delay_ms(2000);
}


