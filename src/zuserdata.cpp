/*
 * User Data
 */

#include <zsystem.h>
#include <zuserdata.h>

const String ZUserData::item_labels[] = {
    "ネクタイ",
    "制服",
    "鍵",
    "懐中電灯",
    "乾電池",
    "ビデオテープ",
    "ファイル",
    "ダイナマイト",
    "塩酸",
    "ジャッキ",
    "マッチ",
    "ペンチ",
};

ZMapLink::ZMapLink()
{
    memset(_link.l, 0, sizeof(_link));
}

ZMapLink::ZMapLink(const uint8_t b[])
{
    memcpy(_link.l, b, sizeof(_link));
}

ZMapLink::ZMapLink(const ZMapLink &x)
{
    memcpy(_link.l, x._link.l, sizeof(_link));
}

void
ZMapLink::setLink(int id, uint8_t value)
{
    _link.l[id] = value;
}

ZUserData::ZUserData()
{
    _map = new ZMapLink [links];
    _places = new uint8_t [items];
    _flags = new uint8_t [flags];
    _buf = new uint8_t [items + flags + links * sizeof(ZMapLink)];
}

ZUserData::ZUserData(const ZUserData &x)
{
    _map = new ZMapLink [links];
    _places = new uint8_t [items];
    _flags = new uint8_t [flags];
    _buf = new uint8_t [items + flags +links * sizeof(ZMapLink)];

    memcpy(_map, x._map, links * sizeof(ZMapLink));
    memcpy(_places, x._places, items);
    memcpy(_flags, x._flags, flags);
}

ZUserData::ZUserData(const String &file)
{
    _map = new ZMapLink [links];
    _places = new uint8_t [items];
    _flags = new uint8_t [flags];
    _buf = new uint8_t [items + flags + links * sizeof(ZMapLink)];
    this->init(file);
}

ZUserData::~ZUserData()
{
    delete [] _map;
    delete [] _places;
    delete [] _flags;
    delete [] _buf;
}

void
ZUserData::init(const String &file)
{
    File f = SD.open(file);
    uint8_t *buf = new uint8_t [sizeof(ZMapLink)];
    for (int i = 0 ; i < links ; i++)
    {
        if (!f.available()) break;
        f.readBytes((char*)buf, sizeof(ZMapLink));
        _map[i] = ZMapLink(buf);
    }
    if (f.available())
    {
        f.seek(0x301); // top of item places
        f.readBytes((char*)_places, items);
    }
    if (f.available())
    {
        f.seek(0x311); // top of flags
        f.readBytes((char*)_flags, flags);
    }
    delete[] buf;
    f.close();
}

void
ZUserData::unpack(const uint8_t buf[])
{
    for (int i = 0 ; i < links ; i++)
    {
        getMap(i).unpack(&buf[i * sizeof(ZMapLink)]);
    }
    memcpy(_places, &buf[links * sizeof(ZMapLink)], items);
    memcpy(_flags, &buf[items + links * sizeof(ZMapLink)], flags);
}

uint8_t *
ZUserData::pack() const
{
    for (int i = 0 ; i < links ; i++)
    {
        memcpy(&_buf[i * sizeof(ZMapLink)], _map[i].pack(), sizeof(ZMapLink));
    }
    memcpy(&_buf[links * sizeof(ZMapLink)], _places, items);
    memcpy(&_buf[items + links * sizeof(ZMapLink)], _flags, flags);

    return _buf;
}

String ZUserData::itemList() const
{
    String s;
    for (int i = 0 ; i < items ; i++)
    {
        if (getPlace(i) == 0xff) s += ZUserData::item_labels[i] + " ";
    }
    return s;
}