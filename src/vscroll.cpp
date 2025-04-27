/*
 * VScroll Contoller
 */

#include <zsystem.h>
#include <vscroll.h>

// Hardware Scroll does not work for screen orientation 0 (320x240 mode)
// Instead of it, software scroll is implemented by this module.
// Constructer
ZVScroll::ZVScroll(lgfx::LGFX_Device *display, uint16_t top, uint16_t bottom)
    :_display(display), _top(top), _bottom(bottom), _ty(0), _tx(0), _scale(1.0), _x(0), _y((int)top)
{
    _h = YMax - _top - _bottom;
    _buf = new uint16_t [XMax];
    _canvas = new LGFX_Sprite(_display);
    _canvas->setColorDepth(8);
    _canvas->createSprite(XMax, _h);
    float sx = 1.0;
    if (_display->width() < XMax)
    {
        sx = (float)_display->width();
        sx /= (float)XMax;
    }
    float sy = 1.0;
    if (_display->height() < YMax)
    {
        sy = (float)_display->height();
        sy /= (float)YMax;
    }
    _scale = (sx < sy) ? sx : sy;
    _x = (uint16_t)(_display->width() - (uint16_t)(XMax * _scale)) / 2;
    _y = (uint16_t)(_display->height() - (uint16_t)(YMax * _scale)) / 2 + _top * _scale;
    cls();
}

// Destructer
ZVScroll::~ZVScroll() {
    if (_buf) delete[] _buf;
    if (_canvas) delete _canvas;
}

int
ZVScroll::scrollLine()
{
    int ly = _ty + FontHeight;
    if (ly >= _h)
    {
        _display->startWrite();
        for (int y = 0 ; y < _h - FontHeight ; y++)
        {
            _canvas->readRect(0, y + FontHeight, XMax, 1, _buf);
            _canvas->pushImage(0, y, XMax, 1, _buf);
        }
        _display->endWrite();
        ly = _ty;
    }
    //Serial1.printf("clear line to be written: (%d)\n", _y);
    _canvas->fillRect(0, ly, XMax, FontHeight, _display->color16to8(TFT_BLACK));
    invalidate();
    _ty = ly;
    _tx = 0;
    return _ty;
}

void
ZVScroll::print(const String &s)
{
    //Serial1.printf("String: '%s' (length = %d)\n", s.c_str(), s.length());
    String t = s;
    while (t.indexOf("−") > 0)
    {
        t.replace("−", "～");
    }
    for (int i = 0 ; i < t.length() ; i++)
    {
        uint8_t c = t[i];
        //Serial1.printf("idx = %d (code = %#02x) (%d,%d)\n", i, c, _tx, _ty);
        if (isascii(c))
        {
            if (_tx >= XMax - FontWidth)
            {
                scrollLine();
            }
            _canvas->setFont(&fonts::AsciiFont8x16);
            _tx += _canvas->drawChar(t[i], _tx, _ty, 2);
        }
        else if (c >= 0x80 && c <= 0xbf) // UTF-8 letters (2nd or later byte letters)
        {
            _canvas->print(t[i]);
        }
        else
        {
            if (_tx >= XMax - FontWidth)
            {
                scrollLine();
            }
            _canvas->setFont(&fonts::lgfxJapanGothic_16);
            _canvas->setCursor(_tx, _ty);
            _canvas->print(t[i]);
            _tx += FontWidth * 2;
        }
    }
    invalidate();
    _canvas->setFont(&fonts::AsciiFont8x16);
}

void
ZVScroll::cls(void)
{
    _tx = 0;
    _ty = 0;
    _canvas->fillRect(0, 0, XMax, _h, _display->color16to8(TFT_BLACK));
    invalidate();
}

void 
ZVScroll::invalidate() const 
{
    _display->startWrite();
    if (_scale == 1.0)
    {
        _canvas->pushSprite(0, _top);
    }
    else
    {
        float affine[] = {_scale, 0.0, (float)_x, 0.0, _scale, (float)_y};
        //Serial1.printf("Vscroll: (x,y,top) = (%d,%d,%d)\r\n", _x, _y, _top);
        _canvas->pushAffineWithAA(affine);
    }
    _display->endWrite();
}
