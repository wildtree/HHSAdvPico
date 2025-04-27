/*
 * ZObject Data
 */

#include <zsystem.h>
#include <zobjdata.h>

const uint16_t ZObjectData::_col[] = {
    TFT_BLACK, TFT_BLUE, TFT_RED, TFT_MAGENTA, TFT_GREEN, TFT_CYAN, TFT_YELLOW, TFT_WHITE
};

ZObjectData::ZObjectData(const uint8_t buf[])
{
    _v = new uint8_t [ZObjRoot::blockSize];
    memcpy(_v, buf, ZObjRoot::blockSize);
}

ZObjectData::ZObjectData(const ZObjectData &x)
{
    _v = new uint8_t [ZObjRoot::blockSize];
    memcpy(_v, x._v, ZObjRoot::blockSize);
}

ZObjectData::~ZObjectData()
{
    delete[] _v;
}

int
ZObjectData::_drawOutline(Canvas *cv, int ofst, uint16_t col, uint16_t ox, uint16_t oy)
{
    uint16_t x0, y0, x1, y1;
    int p = ofst;

    x0 = (uint16_t)(_v[p++] & 0xff);
    y0 = (uint16_t)(_v[p++] & 0xff);
    for(;;)
    {
        x1 = (uint16_t)(_v[p++] & 0xff);
        y1 = (uint16_t)(_v[p++] & 0xff);
        if (y1 == 0xff)
        {
            if (x1 == 0xff) break;
            x0 = (uint16_t)(_v[p++] & 0xff);
            y0 = (uint16_t)(_v[p++] & 0xff);
            continue;
        }
        cv->line(x0 + ox, y0 + oy, x1 + ox, y1 + oy, col);
        y0 = y1;
        x0 = x1;
    }
    return p;
}

void
ZObjectData::_draw(Canvas *cv, bool pre, int ofst)
{
    int o = ofst;
    uint16_t b = _col[(int)(_v[o++] & 0xff)];
    int xs = (int)(_v[o++] & 0xff) / 2;
    int ys = (int)(_v[o++] & 0xff);

    if (pre) b = 0xfc9f; // 0xce40; // OLIVE; (192,192,0) --> rgb565
    
    o = _drawOutline(cv, o, b, xs, ys);
    int x0 = (int)(_v[o++] & 0xff);
    int y0 = (int)(_v[o++] & 0xff);
    while (x0 != 0xff || y0 != 0xff)
    {
        uint16_t c = _col[(int)(_v[o++] & 0xff)];
        if (pre) c = b;
        cv->paint(xs + x0, ys + y0, c, b);
        x0 = (int)(_v[o++] & 0xff);
        y0 = (int)(_v[o++] & 0xff);
    }
}

void
ZObjectData::draw(Canvas *cv, bool offset)
{
    _draw(cv, true, (offset) ? 256:0);
    _draw(cv, false, (offset) ? 256:0);
}

ZTeacherData::~ZTeacherData()
{
}

void
ZTeacherData::draw(Canvas *cv, bool offset)
{
    const int ox = -32;
    int x0, y0, x1, y1;
    const int r1[] = { 18, 24, 2, 2, 2, 22, 9, 0xffff };
    const int r2[] = { 148,14, 126, 6, 0, 0 };

    y0 = 63;
    int i = 0;
    for (i = 0 ; i <= 172 ; i += 2)
    {
        x0 = (int)(_v[i] & 0xff);
        x1 = (int)(_v[i + 1] & 0xff);
        cv->line(ox + x0, y0, ox + x1, y0, TFT_BLUE);
        ++y0;
    }
    uint16_t c = TFT_BLACK;
    for (int j = 0 ; r1[j] != 0xffff ; j++)
    {
        c = _col[(int)(_v[i++] & 0xff)];
        //if (c == TFT_RED) c = 0xf81f; //MAROON;
        x0 = (int)(_v[i++] & 0xff);
        y0 = (int)(_v[i++] & 0xff);
        for (int k = 0 ; k <= r1[j] + 1 ; k++)
        {
            x1 = (int)(_v[i++] & 0xff);
            y1 = (int)(_v[i++] & 0xff);
            cv->line(ox + x0, y0, ox + x1, y1, c);
            x0 = x1;
            y0 = y1;
        }
        x0 = (int)(_v[i++] & 0xff);
        y0 = (int)(_v[i++] & 0xff);
        cv->paint(ox + x0, y0, c, c);
    }
    x0 = (int)(_v[i++] & 0xff);
    y0 = (int)(_v[i++] & 0xff);
    cv->paint(ox + x0, y0, c, c);
    for (int j = 120 ; j < 124 ; j++)
    {
        cv->line(ox + j, 64, ox + j + 8, 110, TFT_YELLOW);
        cv->line(ox + j + 9, 110, ox + j + 11, 126, TFT_WHITE);
    }
    cv->line(ox + 125, 111, ox + 133, 109, TFT_RED);
    cv->line(ox + 133, 109, ox + 134, 110, TFT_RED);
    cv->line(ox + 134, 110, ox + 125, 112, TFT_RED);
    cv->line(ox + 125, 112, ox + 125, 111, TFT_RED);

    cv->line(ox + 120, 65, ox + 123, 64, TFT_WHITE);
    cv->line(ox + 123, 64, ox + 121, 62, TFT_WHITE);
    cv->line(ox + 121, 62, ox + 120, 65, TFT_WHITE);

    cv->paint(ox + 122, 63, TFT_WHITE, TFT_WHITE);

    for(int k = 0 ; r2[k + 1] != 0 ; k += 2)
    {
        x0 = r2[k];
        uint16_t c = 0xfa95; // { 255, 192, 192 }
        for (int j = 0 ; j < r2[k + 1] ; j += 2)
        {
            y0 = (int)(_v[i++] & 0xff);
            y1 = (int)(_v[i++] & 0xff);
            cv->line(ox + x0, y0, ox + x0, y1, c);
            ++x0;
            cv->line(ox + x0, y0, ox + x0, y1, c);
            ++x0;
            y0 = (int)(_v[i++] & 0xff);
            y1 = (int)(_v[i++] & 0xff);
            cv->line(ox + x0, y0, ox + x0, y1, c);
            ++x0;
        }
    }
    cv->drawRect(ox + 148, 78, ox + 164, 84, TFT_BLACK);
    cv->fillRect(ox + 149, 79, ox + 163, 83, TFT_WHITE);
    cv->fillRect(ox + 155, 78, ox + 156, 84, TFT_BLACK);
    
    for(;;)
    {
        x1 = (int)(_v[i++] & 0xff);
        y1 = (int)(_v[i++] & 0xff);
        if (y1 == 0xff)
        {
            if (x1 == 0xff) break;
            x0 = (int)(_v[i++] & 0xff);
            y0 = (int)(_v[i++] & 0xff);
            continue;
        }
        cv->line(ox + x0, y0, ox + x1, y1, TFT_BLACK);
        x0 = x1;
        y0 = y1;
    }
}

void
ZObjRoot::draw(Canvas *cv, int id, bool offset)
{
    if (id != _id)
    {
        _id = id;
        if (_obj != NULL) delete _obj;
        File f = SD.open(_file);
        f.seek(blockSize * _id);
        uint8_t *buf = new uint8_t [blockSize];
        if (f.available())
        {
            f.readBytes((char*)buf, blockSize);
        }
        _obj = (_id == 14) ? new ZTeacherData(buf) : new ZObjectData(buf);
        delete[] buf;
        f.close();
    }
    _obj->draw(cv, offset);
}