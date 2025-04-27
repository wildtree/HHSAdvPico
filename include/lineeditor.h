//
// Line Editor
//

#if !defined(LINEEDITOR_H)
#define LINEEDITOR_H

#include <Arduino.h>

class LineEditor
{
protected:
    char *_buf;
    int _cursor, _tail;
    uint16_t _size;

    String toString() const; 
public:
    LineEditor(uint16_t size = 40);
    LineEditor(const LineEditor &x);
    ~LineEditor();

    operator String() const;
    const String putChar(char c);
    const String flush();
};

#endif /* LINEEDITOR_H */