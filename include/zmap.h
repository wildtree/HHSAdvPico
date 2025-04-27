/*
 * Map data class
 */

#ifndef ZMAP_H
#define ZMAP_H

#include <Arduino.h>
#include <graph.h>

class ZMapMessage
{
protected:
    uint8_t _verb;
    uint8_t _obj;
    String _msg;
public:
    ZMapMessage() : _verb(0), _obj(0) {}
    ZMapMessage(uint8_t v, uint8_t o, const String &s) : _verb(v), _obj(o), _msg(s) {}
    ZMapMessage(const ZMapMessage &x) : _verb(x._verb), _obj(x._obj), _msg(x._msg) {}
    bool match(uint8_t v, uint8_t o) const { return v == _verb && o == _obj; }
    const String &getMessage() const { return _msg; }
    bool invalid() const { return _verb == 0 && _obj == 0; }
};

class ZMapData
{
protected:
    const int vectorSize = 0x400;
    const int relationSize = 0x100;
    const int messageSize = 0x500;
    const int maxMapElements = 0x100;

    uint8_t *_v;
    ZMapMessage *_msg;
    String _m, _b;
    bool _blank;

    int _drawOutline(Canvas *cv, int ofst, uint16_t c);
    void _draw(Canvas *cv);
public:
    ZMapData();
    ZMapData(const ZMapData &x);
    ZMapData(const uint8_t *b);
    ~ZMapData();
    ZMapData &operator = (const ZMapData &x);
    const bool isBlank() const { return _blank; }
    const String &find(uint8_t v, uint8_t o) const;
    void setBlank(const String &msg);
    void setBlank() { _blank = true; }
    void resetBlank() { _blank = false; }
    const String &getMessage() const;
    void draw(Canvas *cv);

    const int fileBlockSize = 0xa00;
};

class ZMapRoot
{
protected:
    const int maxRooms = 100;
    const int fileBlockSize = 0xa00;
    ZMapData *_map;
    int _p, _l, _v;
    String _file;
    bool _blank;
public:
    ZMapRoot(const String &file);
    ~ZMapRoot();

    int setCursor(int p) { return _p = p; };
    int getCursor() const { return _p; }

    bool isBlank() const { return _blank; }

    int look(int p) { _v = _p; return _p = p; }
    int back(void) { return _p = _v; }

    ZMapData &curMapData();
};

class ZMessage
{
protected:
    static const int MAX_MESSAGE = 127;
    String _msgs[ZMessage::MAX_MESSAGE];
public:
    ZMessage(const String &file);
    ~ZMessage();

    const String& getMessage(int id) const { return _msgs[id & 0x7f]; }
};

#endif /* ZMAPP_H */