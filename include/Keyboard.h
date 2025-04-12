#if !defined(KEYBOARD_H)
#define KEYBOARD_H

#include <Arduino.h>
//#include <Wire.h>
#include <queue>
#include <pico/stdlib.h>
#include <pico/platform.h>
#include <hardware/gpio.h>
#include <hardware/i2c.h>
#ifdef USE_TINYUSB
#include <tusb.h>
#endif


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
    enum kbd_type_t { unknown = -1, faces = 0, cardputer = 1, ble = 2, picocalc = 3, usb = 4,};

    virtual inline kbd_type_t keyboard_type() = 0;
};

#ifdef PICOCALC
#define I2C_KBD_MOD i2c1
#define I2C_KBD_SDA 6
#define I2C_KBD_SCL 7

#define I2C_KBD_SPEED  400000 // if dual i2c, then the speed of keyboard i2c should be 10khz

#define I2C_KBD_ADDR 0x1F

class PicoCalcKeyBoard : public KeyBoard
{
public:
    PicoCalcKeyBoard();
    ~PicoCalcKeyBoard() {}

    bool wait_any_key() override;
    bool fetch_key(uint8_t &c) override;
    bool exists() override { return true; }

    inline kbd_type_t keyboard_type() override { return picocalc; }
protected:
    int _write_i2c_kbd(); 
    int _read_i2c_kbd(); 
    uint8_t _i2c_inited;
    uint8_t _keycheck;
};
#endif

#ifdef USBKBD

#ifndef USE_TINYUSB
#define USE_TINYUSB
#endif
#ifndef USE_TINYUSB_HOST
#define USE_TINYUSB_HOST
#endif

class USBKeyBoard : public KeyBoard
{
protected:
    void update();
public:
    USBKeyBoard();
    ~USBKeyBoard() {}

    bool wait_any_key() override;
    bool fetch_key(uint8_t &c) override;
    bool exists() override { return true; }

    inline kbd_type_t keyboard_type() override { return usb; }
};

#endif

#if defined(BLEKBD)

#include <NimBLEDevice.h>


class  ClientCallbacks : public NimBLEClientCallbacks
{
    void onConnect(NimBLEClient* pClient) override;
    void onDisconnect(NimBLEClient* pClient) override;
    bool onConnParamsUpdateRequest(NimBLEClient* pClient, const ble_gap_upd_params* params) override;

    uint32_t onPassKeyRequest () override;
    bool onConfirmPIN(uint32_t pin) override;
    void onAuthenticationComplete(ble_gap_conn_desc *desc) override;
};

class AdvertisedDeviceCallbacks: public NimBLEAdvertisedDeviceCallbacks
{
    void onResult(NimBLEAdvertisedDevice *advertisedDevice) override;
};

class BTKeyBoard : public KeyBoard
{
protected:
    std::queue<uint8_t> _buf;
    static const uint8_t _keymap[][96];
    bool connectToServer();
    void begin();
    void update();
public:
    BTKeyBoard() : KeyBoard()
    {
        begin();
    }
    virtual ~BTKeyBoard() {}
    virtual bool wait_any_key() override;
    virtual bool fetch_key(uint8_t &c) override;
    virtual bool exists() override;
    virtual inline kbd_type_t keyboard_type() override { return ble; }

    static const char *HID_SERVICE;
    static const char *HID_REPORT_DATA;

    static const uint32_t scanTime;
};


#endif

#endif