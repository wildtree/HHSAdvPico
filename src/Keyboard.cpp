// Keyboard class

#include <keyboard.h>


PicoCalcKeyBoard::PicoCalcKeyBoard()
    : KeyBoard(), _i2c_inited(1), _keycheck(0) 
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

bool 
PicoCalcKeyBoard::wait_any_key()
{
    bool r = false;
    int k = _read_i2c_kbd();
    if (k > 0)
    {
        r = true;
    }
    return r;
}

bool 
PicoCalcKeyBoard::fetch_key(uint8_t &c)
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

int 
PicoCalcKeyBoard::_write_i2c_kbd() 
{
    int retval;
    unsigned char msg[2];
    msg[0] = 0x09;

    if(_i2c_inited == 0) return -1;

    retval = i2c_write_timeout_us(I2C_KBD_MOD, I2C_KBD_ADDR, msg, 1, false, 500000);
    if ( retval == PICO_ERROR_GENERIC || retval == PICO_ERROR_TIMEOUT) 
    {
        Serial.printf( "i2c write error\r\n");
        return -1;
    }
    return 0;
}

int 
PicoCalcKeyBoard::_read_i2c_kbd() 
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

#if defined(BLEKBD)

// Bluetooth keyboard handler (NimBLE)

static volatile bool connected = false;
static volatile bool nonsecure = false;

static const int MAX_KEYCODE = 96;

const char *BTKeyBoard::HID_SERVICE = "1812";
//const char *BTKeyBoard::HID_REPORT_MAP = "2A48";
const char *BTKeyBoard::HID_REPORT_DATA = "2A4D";
const uint8_t BTKeyBoard::_keymap[][MAX_KEYCODE] = {
    {    0,   0,   0,   0, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',
       'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '1', '2',
       '3', '4', '5', '6', '7', '8', '9', '0',  13,  27,   8,   9, ' ', '-', '=', '[',
       ']','\\',   0, ';','\'', '`', ',', '.', '/',   0,   0,   0,   0,   0,   0,   0,
         0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 127,   0,   0,   0,
         0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    },
    {
         0,   0,   0,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,
        13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,   0,   0,
         0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
         0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
         0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
         0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    },
    {
         0,   0,   0,   0, 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
       'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '!', '@',
       '#', '$', '%', '^', '&', '*', '(', ')',  13,  27,   8,   9, ' ', '_', '+', '{',
       '}', '|',   0, ':', '"', '~', '<', '>', '?',   0,   0,   0,   0,   0,   0,   0,
         0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 127,   0,   0,   0,
         0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    },
};

const uint32_t BTKeyBoard::scanTime = 0 * 1000; // in milliseconds (0 = forever)

typedef union {
    struct __attribute__((__packed__))
    {
        uint8_t modifiers;
        uint8_t keys[10];
    } k1;
    struct __attribute__((__packed__))
    {
        uint8_t modifiers;
        uint8_t reserved;
        uint8_t keys[6];
        uint8_t padding[3];
    } k2;
    uint8_t raw[11];
} keyboard_t;


static keyboard_t keyboardReport;
static std::queue<uint16_t> keybuf;

static void
notifyCallback(NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify)
{
#if 0
    std::string str = (isNotify == true) ? "Notification" : "Indication";
    str += " from ";
    str += pRemoteCharacteristic->getRemoteService()->getClient()->getPeerAddress().toString();
    str += ": Service = " + pRemoteCharacteristic->getRemoteService()->getUUID().toString();
    str += ", Characteristic = " + pRemoteCharacteristic->getUUID().toString();
    str += ", Handle " + pRemoteCharacteristic->getHandle();
    Serial.println(str.c_str());
    Serial.printf("Length: %d, Handle: %d\r\n", length, pRemoteCharacteristic->getHandle());
    Serial.print("\r\ndata: ");
    for (int i = 0; i < length; i++) {
        // uint8_tを頭0のstringで表示する
        Serial.printf("%02X ", pData[i]);
    }
    Serial.print("\r\n");
#endif
    // handle: 41 -- key / 51 -- media key
    if (length != 8 && length != 11) return; // not key data interface (maybe)
    keyboard_t *newKeyReport = (keyboard_t*)pData;
    int buflen = 6;
    uint8_t *buf = keyboardReport.k2.keys;
    uint8_t *input = newKeyReport->k2.keys;
    uint8_t mod = newKeyReport->k2.modifiers;
    if (length == 11)
    {
        buflen = 10;
        buf = keyboardReport.k1.keys;
        input = newKeyReport->k1.keys;
        mod = newKeyReport->k1.modifiers;
    }
    for (int i = 0 ; i < buflen ; i++)
    {
        uint8_t c = input[i];
        if (c == 0) continue;
        if (memchr(buf, c, buflen) == NULL) keybuf.push(((uint16_t)mod << 8)|c);
    }
    memcpy(&keyboardReport, pData, length);
}

void
ClientCallbacks::onConnect(NimBLEClient* pClient)
{
    Serial.println("BLE Device connected.");
    memset(&keyboardReport, 0, sizeof(keyboardReport));
    pClient->updateConnParams(120, 120, 0, 60);
    connected = true;
}

void
ClientCallbacks::onDisconnect(NimBLEClient* pClient)
{
    Serial.print(pClient->getPeerAddress().toString().c_str());
    Serial.println(" disconnected");
    connected = false;
}

bool
ClientCallbacks::onConnParamsUpdateRequest(NimBLEClient* pClient, const ble_gap_upd_params* params)
{
    return true;
}

uint32_t
ClientCallbacks::onPassKeyRequest()
{
    Serial.println("Client passkey required.");
    return 123456;
}

bool
ClientCallbacks::onConfirmPIN(uint32_t pin)
{
    Serial.print("The passkey number: ");
    Serial.println(pin);
    return true; //pin == 123456;
}

void
ClientCallbacks::onAuthenticationComplete(ble_gap_conn_desc *desc)
{
    if (!desc->sec_state.encrypted)
    {
      Serial.println("Encrypt connection failed - disconnecting");
      /** Find the client with the connection handle provided in desc */
      NimBLEDevice::getClientByID(desc->conn_handle)->disconnect();
    }
}

static NimBLEAdvertisedDevice *advDevice = nullptr;
static bool doConnect = false;


void
AdvertisedDeviceCallbacks::onResult(NimBLEAdvertisedDevice *advertisedDevice)
{
    if (advertisedDevice->haveServiceUUID() && advertisedDevice->isAdvertisingService((NimBLEUUID)BTKeyBoard::HID_SERVICE))
    {
        Serial.print("Advertised HID Device found: ");
        Serial.println(advertisedDevice->toString().c_str());

        NimBLEDevice::getScan()->stop();
        advDevice = advertisedDevice;
        nonsecure = (advDevice->getName() == "M5-Keyboard");
        doConnect = true;
        if (nonsecure)
        {
            Serial.printf("Keyboard: '%s' does not support secure connection.\r\n", advDevice->getName().c_str());
        }
    }
}

bool
BTKeyBoard::connectToServer()
{
    NimBLEClient *pClient = nullptr;
    Serial.println("Start connecting to server...");
    if (NimBLEDevice::getClientListSize())
    {
        pClient = NimBLEDevice::getClientByPeerAddress(advDevice->getAddress());
        if (pClient)
        {
            if (!pClient->connect(advDevice, false))
            {
                Serial.println("Failed to reconnect.");
                return false;
            }
            if (!nonsecure && !pClient->secureConnection())
            {
                Serial.println("Failed to establish secure connection.");
            }
            Serial.println("Reconnected.");
        }
        else
        {
            pClient = NimBLEDevice::getDisconnectedClient();
        }
    }

    if (!pClient)
    {
        if (NimBLEDevice::getClientListSize() >= NIMBLE_MAX_CONNECTIONS)
        {
            Serial.println("No more connection.");
            return false;
        }
        pClient = NimBLEDevice::createClient();
        Serial.println("A new client created.");
        pClient->setClientCallbacks(new ClientCallbacks(), false);
        pClient->setConnectionParams(12, 12, 0, 51);
        pClient->setConnectTimeout(5); // 5sec
        if (!pClient->connect(advDevice))
        {
            NimBLEDevice::deleteClient(pClient);
            Serial.println("Failed to connect.");
            return false;
        }
        if (!nonsecure && !pClient->secureConnection())
        {
            Serial.println("Failed to establish secure connection.");
            return false;
        }

    }
    if (!pClient->isConnected())
    {
        if (!pClient->connect(advDevice))
        {
            Serial.println("Failed to connect.");
            return false;
        }
        if (!nonsecure && !pClient->secureConnection())
        {
            Serial.println("Failed to establish secure connection.");
            return false;
        }
    }
    Serial.print("Connected to ");
    Serial.println(pClient->getPeerAddress().toString().c_str());
    Serial.print("RSSI: ");
    Serial.println(pClient->getRssi());

    NimBLERemoteService *pSvc = nullptr;

    if (pSvc = pClient->getService(HID_SERVICE))
    {
        std::vector<NimBLERemoteCharacteristic*> *chrvec = pSvc->getCharacteristics(true);
        for(auto &it: *chrvec)
        {
            if (it->getUUID() == NimBLEUUID(HID_REPORT_DATA))
            {
                //Serial.println(it->toString().c_str());
                if (it->canNotify() /*&& (it->getHandle() == 41||it->getHandle() == 29)*/)
                {
                    //Serial.print("    Subcribe this...");
                    if (!it->subscribe(true, notifyCallback))
                    {
                        Serial.println("Subscribe notification failed.");
                        pClient->disconnect();
                        return false;
                    }
                    //Serial.println("Subscribed.");
                }
            }
        }
    }

    Serial.println("Done.");
    return true;
}

void
BTKeyBoard::begin()
{
    NimBLEDevice::init("");
    NimBLEDevice::setSecurityAuth(true, true, true);
    //NimBLEDevice::setSecurityIOCap(BLE_HS_IO_KEYBOARD_ONLY);
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);
    NimBLEScan *pScan = NimBLEDevice::getScan();
    pScan->setAdvertisedDeviceCallbacks(new AdvertisedDeviceCallbacks());
    pScan->setInterval(45);
    pScan->setWindow(15);
    pScan->setActiveScan(true);
    //pScan->setDuplicateFilter(true);
    //Serial.printf("Scan %d millisecond(s).\r\n", scanTime);
    pScan->start(scanTime);
}

void
BTKeyBoard::update()
{
    if(doConnect)
    {
        doConnect = false;
        if (!connectToServer())
        {
            NimBLEDevice::getScan()->start(scanTime);
        }
    }
    if (!connected)
    {
        Serial.println("Start scan.");
        NimBLEDevice::getScan()->start(scanTime);
    }
    while (!keybuf.empty())
    {
        uint16_t k = keybuf.front();
        uint8_t  m = (uint8_t)(k >> 8) & 3;
        k &= 0xff;
        if (m == 3) m = 1;
        if (k < MAX_KEYCODE)
        {
            uint8_t  c = _keymap[m][k];
            if (c) _buf.push(c);
        }
        keybuf.pop();
    }
}

bool
BTKeyBoard::wait_any_key()
{
    update();
    if (_buf.empty()) return false;
    _buf.pop(); // just drop one key stroke.
    return true;
}

bool
BTKeyBoard::fetch_key(uint8_t &c)
{
    update();
    if (_buf.empty()) return false;
    c = _buf.front();
    _buf.pop();
    return true;
}

bool
BTKeyBoard::exists()
{
    return true;
}

#endif