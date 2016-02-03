
/* vim: set tabstop=4 softtabstop=4 shiftwidth=4 expandtab: */

#ifndef MOS65XX_H
#define MOS65XX_H

#include "chipemu.h"

namespace chipemu
{

class MOS6500 : public virtual chip
{
public:

    virtual unsigned pin_count() const noexcept = 0;
    virtual void pin_write(unsigned, bool) noexcept = 0;
    virtual bool pin_read(unsigned) const noexcept = 0;

    virtual unsigned address_bus_width() const noexcept = 0;
    virtual unsigned read_address_bus() const noexcept = 0;

    virtual unsigned char read_data_bus() const noexcept = 0;
    virtual void write_data_bus(unsigned char) noexcept = 0;

    virtual unsigned char A() const noexcept = 0;
    virtual unsigned char X() const noexcept = 0;
    virtual unsigned char Y() const noexcept = 0;
    virtual unsigned char P() const noexcept = 0;
    virtual unsigned char S() const noexcept = 0;
    virtual unsigned char PCH() const noexcept = 0;
    virtual unsigned char PCL() const noexcept = 0;
    virtual unsigned PC() const noexcept = 0;

    virtual unsigned char IR() const noexcept = 0;

    virtual ~MOS6500();
};

class MOS6502 : public virtual MOS6500
{
public:

    static MOS6502 *create();

    enum pin
    {
        Vss0 =  1   ,/****   ****/   RES     = 40  ,
         RDY =  2   ,/*   ***   */   CLK2OUT = 39  ,
     CLK1OUT =  3   ,/*         */   SO      = 38  ,
         IRQ =  4   ,/*         */   CLK0IN  = 37  ,
                     /*         */
         NMI =  6   ,/*         */
        SYNC =  7   ,/*         */   RW      = 34  ,
         Vcc =  8   ,/*         */   DB0     = 33  ,
         AB0 =  9   ,/*  6502   */   DB1     = 32  ,
         AB1 = 10   ,/*         */   DB2     = 31  ,
         AB2 = 11   ,/*         */   DB3     = 30  ,
         AB3 = 12   ,/*         */   DB4     = 29  ,
         AB4 = 13   ,/*         */   DB5     = 28  ,
         AB5 = 14   ,/*         */   DB6     = 27  ,
         AB6 = 15   ,/*         */   DB7     = 26  ,
         AB7 = 16   ,/*         */   AB15    = 25  ,
         AB8 = 17   ,/*         */   AB14    = 24  ,
         AB9 = 18   ,/*         */   AB13    = 23  ,
        AB10 = 19   ,/*         */   AB12    = 22  ,
        AB11 = 20   ,/***********/   Vss1    = 21
    };

    virtual ~MOS6502();

};

class MOS6503 : public virtual MOS6500
{
public:

    static MOS6503 *create();

    enum pin
    {
        RES =  1    ,/***********/   CLK2OUT = 28  ,
        Vss =  2    ,/*         */   CLK0IN  = 27  ,
        IRQ =  3    ,/*         */   RW      = 26  ,
        NMI =  4    ,/*         */   DB0     = 25  ,
        Vcc =  5    ,/*         */   DB1     = 24  ,
        AB0 =  6    ,/*  6503   */   DB2     = 23  ,
        AB1 =  7    ,/*         */   DB3     = 22  ,
        AB2 =  8    ,/*         */   DB4     = 21  ,
        AB3 =  9    ,/*         */   DB5     = 20  ,
        AB4 = 10    ,/*         */   DB6     = 19  ,
        AB5 = 11    ,/*         */   DB7     = 18  ,
        AB6 = 12    ,/*         */   AB11    = 17  ,
        AB7 = 13    ,/*         */   AB10    = 16  ,
        AB8 = 14    ,/***********/   AB9     = 15
    };

    virtual ~MOS6503();

};

class MOS6504 : public virtual MOS6500
{
public:

    static MOS6504 *create();

    enum pin
    {
        RES =  1    ,/***********/   CLK2OUT = 28  ,
        Vss =  2    ,/*         */   CLK0IN  = 27  ,
        IRQ =  3    ,/*         */   RW   = 26  ,
        Vcc =  4    ,/*         */   DB0  = 25  ,
        AB0 =  5    ,/*         */   DB1  = 24  ,
        AB1 =  6    ,/*  6504   */   DB2  = 23  ,
        AB2 =  7    ,/*         */   DB3  = 22  ,
        AB3 =  8    ,/*         */   DB4  = 21  ,
        AB4 =  9    ,/*         */   DB5  = 20  ,
        AB5 = 10    ,/*         */   DB6  = 19  ,
        AB6 = 11    ,/*         */   DB7  = 18  ,
        AB7 = 12    ,/*         */   AB12  = 17  ,
        AB8 = 13    ,/*         */   AB11  = 16  ,
        AB9 = 14    ,/***********/   AB10  = 15
    };

    virtual ~MOS6504();

};

class MOS6505 : public virtual MOS6500
{
public:

    static MOS6505 *create();

    enum pin
    {
        RES =  1    ,/***********/   CLK2OUT = 28  ,
        Vss =  2    ,/*         */   CLK0IN  = 27  ,
        RDY =  3    ,/*         */   RW   = 26  ,
        IRQ =  4    ,/*         */   DB0  = 25  ,
        Vcc =  5    ,/*         */   DB1  = 24  ,
        AB0 =  6    ,/*  6505   */   DB2  = 23  ,
        AB1 =  7    ,/*         */   DB3  = 22  ,
        AB2 =  8    ,/*         */   DB4  = 21  ,
        AB3 =  9    ,/*         */   DB5  = 20  ,
        AB4 = 10    ,/*         */   DB6  = 19  ,
        AB5 = 11    ,/*         */   DB7  = 18  ,
        AB6 = 12    ,/*         */   AB11  = 17  ,
        AB7 = 13    ,/*         */   AB10  = 16  ,
        AB8 = 14    ,/***********/   AB9   = 15
    };

    virtual ~MOS6505();

};

class MOS6510 : public virtual MOS6500
{
public:

    static MOS6510 *create();

    enum pin
    {
       CLK1IN =  1  ,/***********/   RES  = 40  ,
          RDY =  2  ,/*         */   CLK2OUT = 39  ,
          IRQ =  3  ,/*         */   RW   = 38  ,
          NMI =  4  ,/*         */   DB0  = 37  ,
          AEC =  5  ,/*         */   DB1  = 36  ,
          Vcc =  6  ,/*         */   DB2  = 35  ,
          AB0 =  7  ,/*         */   DB3  = 34  ,
          AB1 =  8  ,/*         */   DB4  = 33  ,
          AB2 =  9  ,/*  6510   */   DB5  = 32  ,
          AB3 = 10  ,/*         */   DB6  = 31  ,
          AB4 = 11  ,/*         */   DB7  = 30  ,
          AB5 = 12  ,/*         */   P0   = 29  ,
          AB6 = 13  ,/*         */   P1   = 28  ,
          AB7 = 14  ,/*         */   P2   = 27  ,
          AB8 = 15  ,/*         */   P3   = 26  ,
          AB9 = 16  ,/*         */   P4   = 25  ,
         AB10 = 17  ,/*         */   P5   = 24  ,
         AB11 = 18  ,/*         */   AB15 = 23  ,
         AB12 = 19  ,/*         */   AB14 = 22  ,
         AB13 = 20  ,/***********/   Vss  = 21
    };

    virtual ~MOS6510();

};

class MOS6510_1 : public virtual MOS6500
{
public:

    static MOS6510_1 *create();

    enum pin
    {
          RES =  1  ,/***********/   CLK2IN = 40  ,
       CLK1IN =  2  ,/*         */   RW    = 39  ,
          IRQ =  3  ,/*         */   DB0  = 38  ,
          AEC =  5  ,/*         */   DB1  = 38  ,
          Vcc =  6  ,/*         */   DB2  = 37  ,
          AB0 =  7  ,/*         */   DB3  = 36  ,
          AB1 =  8  ,/*         */   DB4  = 35  ,
          AB2 =  9  ,/* 6510-1  */   DB5  = 34  ,
          AB3 = 10  ,/*         */   DB6  = 33  ,
          AB4 = 11  ,/*         */   DB7  = 32  ,
          AB5 = 12  ,/*         */   P0   = 31  ,
          AB6 = 13  ,/*         */   P1   = 29  ,
          AB7 = 14  ,/*         */   P2   = 28  ,
          AB8 = 15  ,/*         */   P3   = 27  ,
          AB9 = 16  ,/*         */   P4   = 26  ,
         AB10 = 17  ,/*         */   P5   = 25  ,
         AB11 = 18  ,/*         */   P6   = 24  ,
         AB12 = 19  ,/*         */   P7   = 23  ,
         AB13 = 19  ,/*         */   AB15 = 22  ,
          Vss = 20  ,/***********/   AB14 = 21
    };

    virtual ~MOS6510_1();

};

class MOS6510_2 : public virtual MOS6500
{
public:

    static MOS6510_2 *create();

    enum pin
    {
          RES =  1  ,/***********/   CLK2OUT = 40  ,
       CLK2IN =  2  ,/*         */   RW    = 39  ,
          IRQ =  3  ,/*         */   DB0  = 38  ,
          AEC =  5  ,/*         */   DB1  = 38  ,
          Vcc =  6  ,/*         */   DB2  = 37  ,
          AB0 =  7  ,/*         */   DB3  = 36  ,
          AB1 =  8  ,/*         */   DB4  = 35  ,
          AB2 =  9  ,/*         */   DB5  = 34  ,
          AB3 = 10  ,/*         */   DB6  = 33  ,
          AB4 = 11  ,/* 6510-2  */   DB7  = 32  ,
          AB5 = 12  ,/*         */   P0   = 31  ,
          AB6 = 13  ,/*         */   P1   = 29  ,
          AB7 = 14  ,/*         */   P2   = 28  ,
          AB8 = 15  ,/*         */   P3   = 27  ,
          AB9 = 16  ,/*         */   P4   = 26  ,
         AB10 = 17  ,/*         */   P5   = 25  ,
         AB11 = 18  ,/*         */   P6   = 24  ,
         AB12 = 19  ,/*         */   P7   = 23  ,
         AB13 = 19  ,/*         */   AB15 = 22  ,
          Vss = 20  ,/***********/   AB14 = 21
    };

    virtual ~MOS6510_2();

};

}

#endif
