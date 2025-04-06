//
// VScroll Controller
//
#ifndef VSCROLL_H
#define VSCROLL_H

#include <Arduino.h>
#include <graph.h>
#include <LovyanGFX.hpp>

class ZVScroll
{
protected:
    int _top, _bottom;
    int _tx, _ty;
    int _h;
    uint16_t *_buf;
    float _scale;
    int _x,_y;
    LGFX_Sprite *_canvas;
    lgfx::LGFX_Device *_display;
public:
    ZVScroll(lgfx::LGFX_Device *display, uint16_t top = 0, uint16_t bottom = 0);
    //ZVScroll(const ZVScroll &x);
    virtual ~ZVScroll();

    virtual int scrollLine();
    virtual void print(const String &s);
    virtual void cls(void);
    virtual void setTextColor(uint16_t c) const { _canvas->setTextColor(c /*ZSystem::getInstance().getDisplay()->color16to8(c)*/); }
    virtual void setFont(const lgfx::v1::IFont *f) const { _canvas->setFont(f); }
    virtual void invalidate() const;

    virtual void home(void) { _ty = 0; _tx = 0; }
    virtual void setScale(lgfx::LGFX_Device *display, float scale) { 
        cls();
        _scale = scale; 
        _x = (display->width() - (uint16_t)(XMax * _scale)) / 2;
        _y = (display->height() - (uint16_t)(YMax * _scale)) / 2 + _top * _scale;
        invalidate();
    }
    virtual float getScale() const { return _scale; }

    static const int YMax = SCRN_HEIGHT;
    static const int XMax = SCRN_WIDTH;
    static const int FontHeight = 16;
    static const int FontWidth = 8;
};

#endif /* VSCROLL_H */