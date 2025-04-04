#if !defined(KEYBOARD_H)
#define KEYBOARD_H

#include <Arduino.h>
//#include <Wire.h>
#include <queue>
#include <pico/stdlib.h>
#include <pico/platform.h>
#include <hardware/gpio.h>
#include <hardware/i2c.h>

class KeyBoard
{
protected:
    uint8_t _board;
public:
    KeyBoard() : _board(0) {}
    virtual ~KeyBoard() {}

    virtual bool wait_any_key() = 0;
    virtual bool fetch_key(uint8_t &c) = 0;
    virtual bool exists() = 0;

    static const int I2C_ADDR = 0x08;
    enum kbd_type_t { unknown = -1, faces = 0, cardputer = 1, ble = 2, picocalc = 3, };

    virtual inline kbd_type_t keyboard_type() = 0;
};

#define I2C_KBD_MOD i2c1
#define I2C_KBD_SDA 6
#define I2C_KBD_SCL 7

#define I2C_KBD_SPEED  400000 // if dual i2c, then the speed of keyboard i2c should be 10khz

#define I2C_KBD_ADDR 0x1F

class PicoCalcKeyBoard : public KeyBoard
{
public:
    PicoCalcKeyBoard() : KeyBoard(), _i2c_inited(1), _keycheck(0) 
    {
#if 0
        Wire1.setSCL(I2C_KBD_SCL);
        Wire1.setSDA(I2C_KBD_SDA);
        Wire1.setClock(I2C_KBD_SPEED);

        Wire1.begin();
#else
        gpio_set_function(I2C_KBD_SCL, GPIO_FUNC_I2C);
        gpio_set_function(I2C_KBD_SDA, GPIO_FUNC_I2C);
        i2c_init(I2C_KBD_MOD, I2C_KBD_SPEED);
        gpio_pull_up(I2C_KBD_SCL);
        gpio_pull_up(I2C_KBD_SDA);
#endif
    }
    ~PicoCalcKeyBoard() {}

    bool wait_any_key() override
    {
        bool r = false;
        int k = _read_i2c_kbd();
        if (k > 0)
        {
            r = true;
        }
        return r;
    }
    bool fetch_key(uint8_t &c) override
    {
        int k = _read_i2c_kbd();
        bool r = false;
        if (k > 0)
        {
            c = (uint8_t)k;
            r = true;
        }
        return r;
    }
    bool exists() override { return true; }

    inline kbd_type_t keyboard_type() override { return picocalc; }
protected:
    int _write_i2c_kbd() 
    {
        int retval;
        unsigned char msg[2];
        msg[0] = 0x09;
    
        if(_i2c_inited == 0) return -1;
    
        retval = i2c_write_timeout_us(I2C_KBD_MOD, I2C_KBD_ADDR, msg, 1, false, 500000);
        if ( retval == PICO_ERROR_GENERIC || retval == PICO_ERROR_TIMEOUT) {
            Serial.printf( "i2c write error\r\n");
            return -1;
        }
        return 0;
    }
    int _read_i2c_kbd() 
    {
        int retval;
        static int ctrlheld = 0;
        uint16_t buff = 0;
        unsigned char msg[2];
        int c  = -1;
        msg[0] = 0x09;

        if(_i2c_inited == 0) return -1;

        if(_keycheck == 0)
        {
            retval = _write_i2c_kbd();
            _keycheck = 1;
            return retval;
        }
        else 
        {
            retval = i2c_read_timeout_us(I2C_KBD_MOD, I2C_KBD_ADDR, (unsigned char *) &buff, 2, false, 500000);
            if (retval == PICO_ERROR_GENERIC || retval == PICO_ERROR_TIMEOUT) 
            {
                Serial.printf("i2c read error read\n");
                return -1;
            }
            _keycheck = 0;
        }
        if(buff != 0) 
        {
            if (buff == 0xA503) ctrlheld = 0;
            else if (buff == 0xA502) 
            {
                ctrlheld = 1;
            }
            else if((buff & 0xff) == 1) 
            {//pressed
                c = buff >> 8;
                int realc = -1;
                switch (c) 
                {
                case 0xA1:
                case 0xA2:
                case 0xA3:
                case 0xA4:
                case 0xA5:
                    realc = -1;//skip shift alt ctrl keys
                    break;
                default:
                    realc = c;
                    break;
                }
                c = realc;
                if(c >= 'a' && c <= 'z' && ctrlheld) c = c - 'a' + 1;
            }
            return c;
        }
        return -1;
    }
    uint8_t _i2c_inited;
    uint8_t _keycheck;
};

#endif