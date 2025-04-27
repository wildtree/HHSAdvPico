/*
 * ZWords -- HighHigh School Adventure Words Dictionary
 */

#include <zsystem.h>
#include <zwords.h>

ZWord::ZWord(const char b[])
   : _word(""), _id(-1)
{
    for (int i = 0 ; i < 4 ; i++)
    {
        uint8_t z = (b[i] & 0xff);
        if (z == 0) break;
        _word += (char)(z - 1);
    }
    _id = (int8_t)b[4];
}

bool
ZWord::match(const String &v) const
{
    String z = v + "    ";
    z = z.substring(0,4);
    z.toUpperCase();
    if (z.equalsIgnoreCase(_word))
    {
        return true;
    }
    return false;
}

ZWords::ZWords(const String &file)
{
    File f = SD.open(file);
    char buf[5];
    int len = 0;
    int p = 0;
    while (f.available())
    {
        int sz = f.readBytes(buf, 5);
        len += sz;
        if (len >= 0x200 || buf[0] == 0) break;
        _verbs[p++] = ZWord(buf);
    }
//    M5.Lcd.setCursor(0,0);
//    M5.Lcd.printf("%d verbs ",p);
    f.seek(0x200);
    len = 0;
    p = 0;
    while(f.available())
    {
        int sz = f.readBytes(buf, 5);
        len += sz;
        if (len >= 0x200 || buf[0] == 0) break;
        _objs[p++] = ZWord(buf);
    }
//    M5.Lcd.printf("/ %d objs ",p);
    f.close();
}

ZWords::ZWords(const ZWords &x)
{
    for (int i = 0 ; i < 100 ; i++)
    {
        if (x._verbs[i].valid()) _verbs[i] = x._verbs[i];
        if (x._objs[i].valid()) _objs[i] = x._objs[i];
    }
}

int
ZWords::findVerb(const String &s) const
{
    for (int i = 0 ; i < 100 ; i++)
    {
        if (_verbs[i].valid() && _verbs[i].match(s))
        {
            return _verbs[i].id();
        }
    }
    return -1;
}

int
ZWords::findObjs(const String &s) const
{
    for (int i = 0 ; i < 100 ; i++)
    {
        if (_objs[i].valid() && _objs[i].match(s))
        {
            return _objs[i].id();
        }
    }
    return -1;
}
