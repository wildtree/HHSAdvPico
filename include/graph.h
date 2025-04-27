/*
 * Graphic Library
 */

#ifndef GRAPH_H
#define GRAPH_H

#include <Arduino.h>
#include <LovyanGFX.hpp>
#include <queue>

#ifdef PICOCALC
#define SCRN_WIDTH (320)
#define SCRN_HEIGHT (320)
#elif defined(LCD35)
#define SCRN_WIDTH (320)
#define SCRN_HEIGHT (480)
#elif defined(LCD28)
#define SCRN_WIDTH (320)
#define SCRN_HEIGHT (240)
#endif

class Canvas
{
protected:
    lgfx::LGFX_Device *_display;
    uint16_t _ox, _oy;
    uint16_t _w, _h, _dx, _dy;
    const uint16_t _col[8] = {
        TFT_BLACK, TFT_BLUE, TFT_RED, TFT_MAGENTA, TFT_GREEN, TFT_CYAN, TFT_YELLOW, TFT_WHITE
    };
#ifdef ZRGB332
    uint8_t *_v;
    inline uint16_t col8to16(uint8_t c) const {
        return (((((uint16_t)(c & 0xe0) >> 5) * 31) / 7) << 11)
             | (((((uint16_t)(c & 0x1c) >> 2) * 63) / 7) << 5)
             | (((uint16_t)(c & 3) * 31) / 3);
    }
    inline uint8_t col16to8(uint16_t c) const {
        return ((uint8_t)((uint16_t)(((c >> 11) & 0x1f) * 7) / 31) << 5)
            |  ((uint8_t)((uint16_t)(((c >> 5) & 0x3f) * 7) / 63) << 2)
            |  (uint8_t)((uint16_t)((c & 0x1f) * 3) / 31);
    }
#else
    uint16_t *_v;
#endif
    float *_colorFilter;
    float _scale;
    uint16_t applyFilter(uint16_t);
public:
    Canvas(lgfx::LGFX_Device *display, uint16_t x = 0, uint16_t y = 0, uint16_t w = SCRN_WIDTH, uint16_t h = SCRN_HEIGHT);
    virtual ~Canvas();

    virtual void cls(uint16_t c = TFT_BLACK);
    void line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t col);
    virtual void pset(uint16_t x, uint16_t y, uint16_t col);
    inline uint16_t pget(uint16_t x, uint16_t y) const { 
#ifdef ZRGB332
        //Serial.printf("col8to16(%2x) = %4x\r\n", _v[x + y * _w], col8to16(_v[x = y * _w]));
        return col8to16(_v[x + y * _w]);
#else
        return _v[x + y * _w]; 
#endif
    }
    void paint(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc);
    void tonePaint(const uint8_t tone[], bool tiling = false);
    uint16_t getColor(int c) const { return _col[c]; }
    void drawRect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t c);
    void fillRect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t c);

    void colorFilter(void);
    void setColorFilter(const float filter[]) { _colorFilter = (float*)filter; }
    void resetColorFilter() { setColorFilter(nullptr); }

    virtual void invalidate(bool force = false) const;

    float getScale() const { return _scale; }
    void setScale(lgfx::LGFX_Device *display, float scale) { 
        _scale = scale; 
        _dx = (display->width() - (uint16_t)(320.0 * _scale)) / 2;
        _dy = (display->height() - (uint16_t)(240.0 * _scale)) / 2;
    }

    static const float blueFilter[], redFilter[], sepiaFilter[];
};

class Point
{
public:
    uint16_t x, y;
    Point(uint16_t x_ = 0, uint16_t y_ = 0) : x(x_), y(y_) {}
    Point(const Point &p) : x(p.x), y(p.y) {}
};

#endif /* GRAPH_H */