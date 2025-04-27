/*
 * ZObject Data
 */

#ifndef ZOBJDATA_H
#define ZOBJDATA_H

#include <SD.h>
#include <graph.h>

class ZObjectData
{
protected:
    static const uint16_t _col[]; 
    uint8_t *_v;
    int _drawOutline(Canvas *cv, int ofst, uint16_t col, uint16_t ox, uint16_t oy);
    void _draw(Canvas *cv, bool pre, int ofst);
public:
    ZObjectData(const uint8_t buf[]);
    ZObjectData(const ZObjectData &x);
    virtual ~ZObjectData();

    virtual void draw(Canvas *cv, bool offset = false);
};

class ZTeacherData : public ZObjectData
{
public:
    ZTeacherData(const uint8_t buf[]) : ZObjectData(buf) {}
    ZTeacherData(const ZTeacherData &x) : ZObjectData((ZObjectData&)x) {}
    virtual ~ZTeacherData();
    virtual void draw(Canvas *cv, bool offset = false);
};

class ZObjRoot
{
protected:
    ZObjectData *_obj;
    String _file;
    int _id;
public:
    ZObjRoot(const String &file) : _file(file), _obj(NULL), _id(-1) {}
    void draw(Canvas *cv, int id, bool offset = false);

    static const int blockSize = 0x200;
};

#endif /* ZOBJDATA_H */