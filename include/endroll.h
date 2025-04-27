//
// endroll.h
//

#ifndef __ENDROLL_H__
#define __ENDROLL_H__
#include <Arduino.h>
#include <LGFX.h>

class EndRoll
{
public:
    EndRoll(const std::string &msg = std::string("")) : _message(msg) {}

    void setMessage(const std::string& msg) {
        _message = msg;
    }

    void run(lgfx::v1::LGFX_Device *display);
protected:
    std::string _message;
    uint16_t _lh;
    LGFX_Sprite *_canvas, *_string;

    void scrollLine();
};

#endif // __ENDROLL_H__