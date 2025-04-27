/*
 * Graphics Library
 */

#include <zsystem.h>
#include <graph.h>

const float Canvas::blueFilter[] = {
    0.0F, 0.0F, 0.1F,
    0.0F, 0.0F, 0.2F,
    0.0F, 0.0F, 0.7F,
};

const float Canvas::redFilter[] = {
    0.7F, 0.0F, 0.0F,
    0.2F, 0.0F, 0.0F,
    0.1F, 0.0F, 0.0F,
};

const float Canvas::sepiaFilter[] = {
    0.269021F, 0.527950F, 0.103030F,
    0.209238F, 0.410628F, 0.080135F,
    0.119565F, 0.234644F, 0.045791F,
};


Canvas::Canvas(lgfx::LGFX_Device *display, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
    : _display(display), _ox(x), _oy(y), _w(w), _h(h), _dx(0), _dy(0), _scale(1.0), _colorFilter(nullptr)
{
#ifdef ZRGB332
    _v = new uint8_t [_w * _h];
#else
#if defined(RP2350_PSRAM_CS)
    _v = (uint16_t*)pmalloc(_w * _h * sizeof(uint16_t));
#else
    _v =  new uint16_t [_w * _h];
#endif
#endif
    cls();
}

Canvas::~Canvas()
{
    cls();
#if defined(RP2350_PSRAM_CS)
    free(_v);
#else
    delete [] _v;
#endif
}

uint16_t
Canvas::applyFilter(uint16_t c)
{
    float b = (float)(c & 0x1f);
    float g = (float)((c >> 5) & 0x3f);
    float r = (float)((c >> 11) & 0x1f);
    uint8_t rd = (uint8_t)(_colorFilter[0] * r + _colorFilter[1] * g + _colorFilter[2] * b);
    uint8_t gd = (uint8_t)(_colorFilter[3] * r + _colorFilter[4] * g + _colorFilter[5] * b);
    uint8_t bd = (uint8_t)(_colorFilter[6] * r + _colorFilter[7] * g + _colorFilter[8] * b);
    return (uint16_t)((rd & 0x1f) << 11)|((gd & 0x3f) << 5)|(bd & 0x1f);
}

void
Canvas::pset(uint16_t x, uint16_t y, uint16_t col)
 {
    if (x >= _w || y >= _h) return;
    if (_scale == 1.0) _display->drawPixel(_ox + x, _oy + y, col);
#ifdef ZRGB332
    //Serial1.printf("col16to8(%4x) = %2x\r\n", col, col16to8(col));
    _v[x + y * _w] = col16to8(col);
#else
    _v[x + y * _w] = col;
#endif
}

void
Canvas::cls(uint16_t c)
 { 
    if (_scale == 1.0) _display->fillRect(_ox, _oy, _w, _h, c); 
    //memset(_v, c, _w * _h);
    for (int i = 0 ; i < _w * _h ; i++)
    {
#ifdef ZRGB332
        _v[i] = col16to8(c);
#else
        _v[i] = c;
#endif
    }
 }

void
Canvas::line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t col)
{
    //M5.Lcd.drawLine(_ox + x0, _oy + y0, _ox + x1, _oy + y1, col);
    int dx, ddx, dy, ddy;
    int wx, wy;
    int x, y;
    dy = y1 - y0;
    ddy = 1;
    if (dy < 0)
    {
         dy = -dy;
         ddy = -1;
    }
    wy = dy / 2;
    dx = x1 - x0;
    ddx = 1;
    if (dx < 0)
    {
        dx = -dx;
        ddx = -1;
    }
    wx = dx / 2;
    if (dx == 0 && dy == 0)
    {
        pset(x0, y0, col);
        return;
    }
    if (dy == 0)
    {
        for (x = x0 ; x != x1 ; x += ddx) pset(x, y0, col);
        pset(x1, y0, col);
        return;
    }
    if (dx == 0)
    {
        for (y = y0 ; y != y1 ; y += ddy) pset(x0, y, col);
        pset(x0, y1, col);
        return;
    }

    pset(x0, y0, col);
    if (dx > dy)
    {
        y = y0;
        for (x = x0 ; x != x1 ; x += ddx)
        {
            pset(x, y, col);
            wx -= dy;
            if (wx < 0)
            {
                wx += dx;
                y += ddy;
            }
        }
    }
    else
    {
        x = x0;
        for (y = y0 ; y != y1 ; y += ddy)
        {
            pset(x, y, col);
            wy -= dx;
            if (wy < 0)
            {
                wy += dy;
                x += ddx;
            }
        }
    }
    pset(x1, y1, col);
}

void
Canvas::paint(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc)
{
    int l, r;
    int wx;
    uint16_t c;

    std::queue<Point> q;
    c = pget(x, y);
    if (c == fc || c == bc)
    {
        return;
    }
    q.push(Point(x,y));
    while (!q.empty())
    {
        Point p = q.front();
        q.pop();
        c = pget(p.x, p.y);
        if (c == fc || c == bc) continue;
        for (l = p.x - 1 ; l >= 0 ; l--)
        {
            c = pget(l, p.y);
            if (c == fc || c == bc) break;
        }
        ++l;
        for (r = p.x + 1 ; r < _w ; r++)
        {
            c = pget(r, p.y);
            if (c == fc || c == bc) break;
        }
        --r;
        line(l, p.y, r, p.y, fc);
        for (wx = l ; wx <= r ; wx++)
        {
            int uy = p.y - 1;
            if (uy >= 0)
            {
                c = pget(wx, uy);
                if (c != fc && c != bc)
                {
                    if (wx == r)
                    {
                        q.push(Point(wx, uy));
                    }
                    else
                    {
                        c = pget(wx + 1, uy);
                        if (c == fc || c == bc) q.push(Point(wx,uy));
                    }
                }
            }
            int ly = p.y + 1;
            if (ly < _h)
            {
                c = pget(wx, ly);
                if (c != fc && c != bc)
                {
                    if (wx == r)
                    {
                        q.push(Point(wx, ly));
                    }
                    else
                    {
                        c = pget(wx + 1, ly);
                        if (c == fc || c == bc) q.push(Point(wx, ly));
                    }
                }
            }
        }
    }
}

void
Canvas::tonePaint(const uint8_t tone[], bool tiling)
{
    uint8_t pat[][3] = {
        {0x00, 0x00, 0x00},
        {0xff, 0x00, 0x00},
        {0x00, 0xff, 0x00},
        {0xff, 0xff, 0x00},
        {0x00, 0x00, 0xff},
        {0xff, 0x00, 0xff},
        {0x00, 0xff, 0xff},
        {0xff, 0xff, 0xff}
    };
    uint16_t col[] = {
        TFT_BLACK,
        TFT_BLUE,
        TFT_RED,
        TFT_MAGENTA,
        TFT_GREEN,
        TFT_CYAN,
        TFT_YELLOW,
        TFT_WHITE
    };
    int p = 0;
    int n = tone[p++];
    for (int i = 1 ; i <= n ; i++)
    {
        pat[i][0] = (tone[p++] & 0xff);
        pat[i][1] = (tone[p++] & 0xff);
        pat[i][2] = (tone[p++] & 0xff);
        uint8_t b = 0, r = 0, g = 0;
        for (int bit = 0 ; bit < 8 ; bit++)
        {
            uint8_t mask = (1 << bit);
            if (pat[i][0] & mask) ++b;
            if (pat[i][1] & mask) ++r;
            if (pat[i][2] & mask) ++g;
        }
        r = (r == 0) ? 0 : r * 4 - 1;
        g = (g == 0) ? 0 : g * 8 - 1;
        b = (b == 0) ? 0 : b * 4 - 1;
        col[i] = (r << 11) | (g << 5) | b; // RGB565 is for M5Stack
    }
    for (int wy = 0 ; wy < _h ; wy++)
    {
        for (int wx = 0 ; wx < _w ; wx++)
        {
            uint16_t c = pget(wx, wy);
            uint8_t b = c & 0x1f;
            uint8_t g = (c >> 5) & 0x3f;
            uint8_t r = (c >> 11) & 0x1f;
            int ci = ((b == 0) ? 0 : 1) + ((r == 0) ? 0 : 2) + ((g == 0) ? 0 : 4);
            uint16_t cc = col[ci];
            if (tiling)
            {
                b = ((pat[ci][0] >> (7 - wx % 8)) & 1) == 0 ? 0 : 0x1f;
                r = ((pat[ci][1] >> (7 - wx % 8)) & 1) == 0 ? 0 : 0x1f;
                g = ((pat[ci][2] >> (7 - wx % 8)) & 1) == 0 ? 0 : 0x3f;
                cc = (r << 11) | (g << 5) | b;
            }
            pset(wx, wy, cc);
        }
    }
}

void
Canvas::drawRect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t c)
{
    line(x0, y0, x1, y0, c);
    line(x1, y0, x1, y1, c);
    line(x0, y1, x1, y1, c);
    line(x0, y1, x0, y0, c);
}

void
Canvas::fillRect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t c)
{
    if (y0 > y1)
    {
        uint16_t y = y0;
        y0 = y1;
        y1 = y;
    }
    for (uint16_t y = y0 ; y <= y1 ; y++)
    {
        line(x0, y, x1, y, c);
    }
}

void
Canvas::colorFilter(void)
{
    if (_colorFilter == nullptr) return;
    for (int x = 0 ; x < _w ; x++)
    {
        for (int y = 0 ; y < _h ; y++)
        {
            pset(x, y, applyFilter(pget(x, y)));
        }
    }
}

void
Canvas::invalidate(bool force) const
{
    _display->startWrite();
    if (_scale == 1.0) {
        if (force)
        {
            _display->pushImage(_ox, _oy, _w, _h, _v);
        }
    }
    else
    {
        float affine[6] = {_scale, 0.0, (float)(_ox * _scale + _dx), 0.0, _scale, (float)(_oy * _scale + _dy)};
        _display->pushImageAffineWithAA(affine, _w, _h, _v);
    }
    _display->endWrite();  
}
