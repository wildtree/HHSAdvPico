/*
 * ZMapData
 */

#include <zsystem.h>
#include <zmap.h>

const static String _emptyString = String();

ZMapData::ZMapData()
    : _blank(false), _m(""), _b("")
{
    _v = NULL;
    _msg = NULL;

}

ZMapData::ZMapData(const uint8_t *b)
    :_blank(false)
{
    _v = new uint8_t [vectorSize];
    _msg = new ZMapMessage [maxMapElements];
    memcpy(_v, b, vectorSize);
    int m = vectorSize + relationSize;
    int n = 0;
    String msg[maxMapElements];
    for (n = 0 ; m < fileBlockSize ; n++)
    {
        int len = ((int)(b[m++] & 0xff) << 8)|(int)(b[m++] & 0xff);
        if (len == 0) break;
        char bmsg[len+1];
        memcpy(bmsg, &b[m], len);
        bmsg[len] = 0;
        msg[n] = String(bmsg);
        m += len;
    }
    _m = msg[0];
    int j = vectorSize;
    for (int i = 0 ; i <= n ; i++)
    {
        uint8_t v = b[j++] & 0xff;
        if (v == 0) break;
        uint8_t o = b[j++] & 0xff;
        uint8_t idx = (b[j++] & 0xff) - 1;
        _msg[i] = ZMapMessage(v, o, msg[idx]);
    }
    //_msg[n + 1] = ZMapMessage();
}

ZMapData::ZMapData(const ZMapData &x)
    : _blank(x._blank), _m(x._m), _b(x._b)
{
    _v = new uint8_t [vectorSize];
    _msg = new ZMapMessage [maxMapElements];
    memcpy(_v, x._v, vectorSize);
    int i = 0;
    for (i = 0 ; ! x._msg[i].invalid() ; i++) _msg[i] = x._msg[i];
    //_msg[i] = ZMapMessage();
}

ZMapData::~ZMapData()
{
    if (_v != NULL) delete [] _v;
    if (_msg != NULL) delete [] _msg;
}

ZMapData &
ZMapData::operator = (const ZMapData &x)
{
    _blank = x._blank;
    _m = x._m;
    _b = x._b;
    if (_v == NULL) _v = new uint8_t [vectorSize];
    memcpy(_v, x._v, vectorSize);
    if (_msg == NULL) _msg = new ZMapMessage [maxMapElements];
    int i = 0;
    for (i = 0 ; ! x._msg[i].invalid() ; i++) _msg[i] = x._msg[i];
    //_msg[i] = ZMapMessage();
    return *this;
}

const String&
ZMapData::find(uint8_t v, uint8_t o) const
{
    for (int i = 0 ; ! _msg[i].invalid() ; i++)
    {
        if (_msg[i].match(v, o))
        {
            return _msg[i].getMessage();
        }
    }
    return _emptyString;
}

void
ZMapData::setBlank(const String &msg)
{
    _blank = true;
    _b = msg;
}

const String &
ZMapData::getMessage() const
{
    if (_blank) return _b;
    return _m;
}

void
ZMapData::draw(Canvas *cv)
{
    if (_blank) 
    {
        cv->cls();
        return;
    }
    _draw(cv);
}

void
ZMapData::_draw(Canvas *cv)
{
    int i = (int)(_v[0] & 0xff) * 3 + 1;
    cv->cls(TFT_BLUE);
    i = _drawOutline(cv, i, TFT_WHITE);
    int x0 = (int)(_v[i++] & 0xff);
    int y0 = (int)(_v[i++] & 0xff);
    while (x0 != 0xff || y0 != 0xff)
    {
        uint16_t c = cv->getColor((int)(_v[i++] & 0xff));
        cv->paint(x0, y0, c, TFT_WHITE);
        x0 = (int)(_v[i++] & 0xff);
        y0 = (int)(_v[i++] & 0xff);
    }
    if ((_v[i] &0xff) == 0xff && ((_v[i + 1] & 0xff) == 0xff))
    {
        i += 2;
    }
    else
    {
        i = _drawOutline(cv, i, TFT_WHITE);
    }
    if ((_v[i] &0xff) == 0xff && ((_v[i + 1] & 0xff) == 0xff))
    {
        i += 2;
    }
    else
    {
        i = _drawOutline(cv, i, TFT_BLACK);
    }
    cv->tonePaint(_v);
}

int
ZMapData::_drawOutline(Canvas *cv, int ofst, uint16_t c)
{
    int x0, y0, x1, y1;
    int p = ofst;
    x0 = (int)(_v[p++] & 0xff);
    y0 = (int)(_v[p++] & 0xff);
    for(;;)
    {
        x1 = (int)(_v[p++] & 0xff);
        y1 = (int)(_v[p++] & 0xff);
        if (y1 == 0xff)
        {
            if (x1 == 0xff) break;
            x0 = (int)(_v[p++] & 0xff);
            y0 = (int)(_v[p++] & 0xff);
            continue;
        }
        cv->line(x0, y0, x1, y1, c);
        x0 = x1;
        y0 = y1;
    }
    return p;
}

ZMapRoot::ZMapRoot(const String &file)
    : _p(1), _l(0), _v(84), _file(file), _map(nullptr)
{
}

ZMapRoot::~ZMapRoot()
{
}

ZMapData &
ZMapRoot::curMapData()
{
    if(_p != _l)
    {
        //Serial1.printf("loading map id=%d\r\n", _p);
        File f = SD.open(_file);
        //Serial1.printf("'%s' is opened\r\n", _file);
        uint8_t *buf = new uint8_t [fileBlockSize];
        //Serial1.printf("buffer is allocated: %08x\r\n", buf);
        f.seek(_p * fileBlockSize);
        //Serial1.printf("file pointer is moved to: %08x\r\n", _p * fileBlockSize);
        if (f.available())
        {
            f.readBytes((char*)buf, fileBlockSize);
            if (_map != nullptr) delete _map;
            _map = new ZMapData(buf);
            if (_p == 0 || _p == 84 || _p == 85)
            {
                _map->setBlank();
                // _map.setBlank(msg[0x4c]);
            }
            _l = _p;
        }
        //Serial1.print("data loaded. buffer to delete.\r\n");
        delete [] buf;
        f.close();
        //Serial1.print("loaded.\r\n");
    }
    return *_map;
}

ZMessage::ZMessage(const String &file)
{
    File f = SD.open(file);
    int i = 0;
    uint8_t c;
    String tmp = String();
    while (f.available())
    {
        f.readBytes((char*)&c, 1);
        if (c == 0)
        {
            _msgs[i++] = String(tmp);
            tmp = String();
            continue;
        }
        tmp += (char)c;
    }
    f.close();
    while (i < ZMessage::MAX_MESSAGE)
    {
        _msgs[i++] = String();
    }
}

ZMessage::~ZMessage()
{
}