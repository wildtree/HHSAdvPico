//
// EndRoll.cpp
//

#include "endroll.h"

#include <vector>
#include <sstream>


void 
EndRoll::scrollLine()
{
    uint8_t buf[_canvas->width()];
    for (int y = 1 ; y < _canvas->height() ; y++)
    {
        _canvas->readRect(0, y, _canvas->width(), 1, buf);
        _canvas->pushImage(0, y - 1, _canvas->width(), 1, buf);
    }
    _canvas->drawLine(0, _canvas->height() - 1, _canvas->width(), _canvas->height() - 1, _canvas->color16to8(TFT_BLACK));
}

void 
EndRoll::run(lgfx::v1::LGFX_Device *display) 
{
    if (_message.empty()) return; // No message to scroll
    std::stringstream messageStream(_message); // Convert _message to a stream
    _lh = 12; //display->fontHeight();
    _string = new LGFX_Sprite(display);
    _string->setColorDepth(8);
    _string->createSprite(display->width(), _lh);
    _string->setTextColor(_string->color16to8(TFT_CYAN), _string->color16to8(TFT_BLACK));
    _string->setFont(&fonts::lgfxJapanGothic_12);
    _string->setTextDatum(lgfx::v1::textdatum::textdatum_t::top_center);
    _canvas = new LGFX_Sprite(display);
    _canvas->setColorDepth(8);
    _canvas->createSprite(display->width(), 160);
    _canvas->fillRect(0, 0, _canvas->width(), _canvas->height(), display->color16to8(TFT_BLACK));
    _canvas->pushSprite(0, 0);
    std::string line;
    uint8_t buf[_canvas->width()];
    while (std::getline(messageStream, line, '\n')) 
    {
        _string->fillRect(0, 0, _string->width(), _string->height(), display->color16to8(TFT_BLACK));
        _string->drawString(line.c_str(), _string->width() / 2, 0);
        for (int y = 0 ; y < _string->height() ; y++)
        {
            scrollLine();
            _string->readRect(0, y, _string->width(), 1, buf);
            _canvas->pushImage(0, _canvas->height() - 1, _canvas->width(), 1, buf);
            _canvas->pushSprite(0, 0);
        }
    }
    
    for (int y = 0 ; y < _canvas->height() ; y++)
    {
        scrollLine();
        _canvas->pushSprite(0, 0);
    }
    delete _canvas;
    delete _string;
}