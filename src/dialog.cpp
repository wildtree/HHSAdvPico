//
// Dialog class
//

#include <zsystem.h>
#include <dialog.h>
#include <screenshot.h>

Button::Button()
    :_x(-1), _y(-1), _w(-1), _h(-1), _label(""), _enabled(false), _key(0)
{
    _canvas = nullptr;
}

Button::Button(int x, int y, int w, int h, const String &label, uint8_t key, LGFX_Sprite *canvas)
    : _x(x), _y(y), _w(w), _h(h), _label(label), _enabled(true), _key(key), _canvas(canvas)
{
    if (_label.isEmpty()) _enabled = false;
}

Button::~Button()
{

}

bool
Button::is_pressed(uint8_t c) const
{
    if (!is_enabled()) return false;
    if (c != 0 && c == _key) return true;
    return false;
}

void
Button::draw(bool is_pressed) const
{
    static uint16_t col;
    if (_enabled)
    {
        if (is_pressed)
        {
            _canvas->fillRoundRect(_x, _y, _w, _h, 8, _canvas->color16to8(TFT_BLACK));
            col = TFT_WHITE;
        }
        else
        {
            _canvas->drawRoundRect(_x, _y, _w, _h, 8, _canvas->color16to8(TFT_BLACK));
            col = TFT_BLACK;
        }
    }
    else
    {
        _canvas->fillRoundRect(_x, _y, _w, _h, 8, _canvas->color16to8(TFT_LIGHTGRAY));
        col = TFT_DARKGRAY;
    }
    String s = _label;
    if (s != String((char)_key))
    {
        s = String((char)_key) + String(". ") + _label;
    }
    auto text_datum = _canvas->getTextDatum();
    auto font = _canvas->getFont();
    _canvas->setTextDatum(middle_center);
    _canvas->setFont(&fonts::lgfxJapanGothic_16);
    _canvas->setTextColor(_canvas->color16to8(col));
    _canvas->drawString(s, _x + _w / 2, _y + _h / 2);
    _canvas->setFont(font);
    _canvas->setTextDatum(text_datum);
}

Dialog::Dialog(lgfx::LGFX_Device *display)
   : _display(display), _x(8), _y(8), _w(304), _h(144),_title("dialog"),_result(-1)
{
    _canvas = new LGFX_Sprite(_display);
    _canvas->setColorDepth(1);
    _canvas->createSprite(_w, _h);
    _btnA = new Button(  9,120,90,20,"A",'1', _canvas);
    _btnB = new Button(107,120,90,20,"B",'2', _canvas);
    _btnC = new Button(205,120,90,20,"C",'3', _canvas);
    float sx = (_display->width() < 320.0) ? (float)_display->width() / 320.0 : 1.0;
    float sy = (_display->height() < 240.0) ? (float)_display->height() / 240.0 : 1.0;
    _scale = (sx < sy) ? sx : sy;
    _dx = (uint16_t)(_display->width() - (uint16_t)(320.0 * _scale)) / 2;
    _dy = (uint16_t)(_display->height() - (uint16_t)(240.0 * _scale)) / 2;
}

Dialog::~Dialog()
{
    dismiss();
    delete _btnA;
    delete _btnB;
    delete _btnC;
    delete _canvas;
}


void
Dialog::_print() const
{
    int x = 8;
    int y = 36;

    auto font = _canvas->getFont();
    _canvas->setTextColor(_canvas->color16to8(TFT_BLACK));
    for(int i = 0 ; i < _message.length() ; i++)
    {
        uint8_t c = _message[i];
        if (isascii(c)||c >= 0xc0)
        {
            if (x >= _w - FontWidth)
            {
                x = FontWidth;
                y += _canvas->fontHeight(_canvas->getFont());
            }
            _canvas->setCursor(x, y);
            if (isascii(c))
            {
                _canvas->setFont(&fonts::AsciiFont8x16);
                x += FontWidth;
            }
            else
            {
                _canvas->setFont(&fonts::lgfxJapanGothic_16);
                x += FontWidth * 2;
            }
        }
        _canvas->print((char)c);
    }
}

void
Dialog::setScale(float scale)
{
    _scale = scale;
    _dx = (uint16_t)(_display->width() - (uint16_t)(320.0 * _scale)) / 2;
    _dy = (uint16_t)(_display->height() - (uint16_t)(240.0 * _scale)) / 2;
}

int
Dialog::draw(void)
{
    //_canvas->fillRect(0,0,_w,_h, _canvas->color16to8(WHITE));
    _canvas->fillSprite(_canvas->color16to8(TFT_WHITE));
    _canvas->fillRect(0,0,_w,20, _canvas->color16to8(TFT_BLACK));
    _canvas->drawRect(0,0,_w,20, _canvas->color16to8(TFT_WHITE));
    auto font = _canvas->getFont();
    _canvas->setFont(&fonts::lgfxJapanGothic_16);
    auto text_datum = _canvas->getTextDatum();
    _canvas->setTextDatum(middle_center);
    _canvas->setTextColor(_canvas->color16to8(TFT_WHITE));
    _canvas->drawString(_title, _w / 2, 8);
    _canvas->setTextDatum(text_datum);
 
    _print(); // put message
    _canvas->setFont(font);

    _btnA->draw();
    _btnB->draw();
    _btnC->draw();

    int b =(_btnA->is_enabled() ? 1 : 0) +
           (_btnB->is_enabled() ? 2 : 0) +
           (_btnC->is_enabled() ? 4 : 0);

    invalidate();
    while(true)
    {
        uint8_t c = 0;
        ZSystem::getInstance().getKeyboard()->fetch_key(c);
        if (c == '\x13')
            {
                ScreenShot::instance().take(ZSystem::getInstance().getDisplay());
                continue;
            }
        if (_btnA->is_pressed(c)||(b == 1 && (c == ' ' || c == 0x0d || c == 0x0a)))
        {
            _btnA->draw(true);
            _result = 1;
            invalidate();
            break;
        }
        if (_btnB->is_pressed(c)||(b == 2 && (c == ' ' || c == 0x0d || c == 0x0a)))
        {
            _btnB->draw(true);
            _result = 2;
            invalidate();
            break;
        }
        if (_btnC->is_pressed(c)||(b == 4 && (c == ' ' || c == 0x0d || c == 0x0a)))
        {
            _btnC->draw(true);
            _result = 3;
            invalidate();
            break;
        }
    }
    _canvas->fillSprite(_canvas->color16to8(TFT_BLACK));
    invalidate();
    return _result;
}

void
Dialog::dismiss()
{

}

void
Dialog::button(const String &labelA, const String &labelB, const String &labelC)
{
    _btnA->setLabel(labelA);
    _btnB->setLabel(labelB);
    _btnC->setLabel(labelC);

    if (labelA.isEmpty()) _btnA->disable();
    if (labelB.isEmpty()) _btnB->disable();
    if (labelC.isEmpty()) _btnC->disable();
}

void
Dialog::invalidate() const
{
    _display->startWrite();
    //Serial1.printf("(x,y,w,h,dx,dy) = (%d,%d,%d,%d,%d,%d)\r\n",_x,_y,_w,_h,_dx,_dy);
    if (_scale == 1.0)
    {
        _canvas->pushSprite(_x,_y);
    }
    else
    {
        float affine[] = {_scale, 0.0, (float)(_dx + _x * _scale), 0.0, _scale, (float)(_dy + _y * _scale)};
        _canvas->pushAffineWithAA(affine);
    }
    _display->endWrite();
}
