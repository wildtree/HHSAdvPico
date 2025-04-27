/*
 * Line Editor
 */
#include <lineeditor.h>

LineEditor::LineEditor(uint16_t size)
    : _size(size), _cursor(0), _tail(0)
{
    _buf = new char[_size];
}

LineEditor::LineEditor(const LineEditor &x)
    : _size(x._size), _cursor(x._cursor), _tail(x._tail)
{
    _buf = new char [_size];
    for (int i = 0 ; i < _size ; i++) _buf[i] = x._buf[i];
}

LineEditor::~LineEditor()
{
    delete [] _buf;
}

String
LineEditor::toString() const
{
    String s;
    for(int i = 0 ; i < _tail ; i++) s += _buf[i];
    return s;
}

const String
LineEditor::flush()
{
    String s = toString();
    _cursor = _tail = 0;
    return s;
}

LineEditor::operator String() const
{
    return toString();
}

const String
LineEditor::putChar(char c)
{
    if (c == '\r')
    {
        return flush();
    }
    if (c == 0x08)
    {
        if (_cursor > 0)
        {
            for (int i = _cursor - 1 ; i < _tail ; i++) _buf[i] = _buf[i + 1];
            _cursor--;
            _tail--;
        }
        return toString();
    }
    if (c >= 0x20 && c < 0x7f)
    {
        if (_cursor < _size)
        {
            _buf[_cursor++] = c;
            if (_tail < _size) _tail++; 
        }
    }
    return toString();
}
