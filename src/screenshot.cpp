#include <screenshot.h>


ScreenShot::BMP_File_Header ScreenShot::_bf;
ScreenShot::BMP_Info_Header ScreenShot::_bi;

char ScreenShot::_filename[PATH_MAX] = "/screenshot_0000.bmp";


ScreenShot::ScreenShot()
{

}

ScreenShot::~ScreenShot()
{

}

const char *
ScreenShot::get_next_filename()
{
    char num[5];
    strncpy(num, strchr(_filename, '_') + 1, 4);
    int n = strtol(num, nullptr, 0);
    while (SD.exists(_filename))
    {
      if (++n > 9999) n = 0;
      snprintf(num, 5, "%04d", n);
      strncpy(strchr(_filename, '_') + 1, num, 4);
    }
    return _filename;
}

void
ScreenShot::take(lgfx::LGFX_Device *display, const char *filename)
{
    if (filename == nullptr)
    {
        filename = get_next_filename();
    }
    uint16_t colorDepth = 16; // 16bit
    _bf = {
        ('M'<<8)|'B',
        (uint32_t)(display->width() * display->height() * (colorDepth / 8)),
        0,
        0,
        14 + 40,
    };
    _bi = {
        40,
        (uint32_t)display->width(),
        (uint32_t)display->height(),
        1,
        colorDepth,
        3, // bit field type
        (uint32_t)(display->width() * display->height() * (colorDepth / 8)),
        7874,
        7874,
        0,
        0,
    };
    uint16_t lineBuf[display->width()];
    uint8_t b = display->getBrightness();
    display->setBrightness(20);
    File fp = SD.open(filename, FILE_WRITE);
    if (fp)
    {
        fp.write((uint8_t*)&_bf, sizeof(_bf));
        fp.write((uint8_t*)&_bi, sizeof(_bi));
        uint32_t mask[3] = {0x0000f800, 0x000007e0, 0x0000001f,}; // rgb 565
        fp.write((uint8_t*)mask, sizeof(mask));
        for (int y = display->height() - 1 ; y >= 0 ; y--)
        {
            for (int x = 0 ; x < display->width() ; x++)
            {
                lineBuf[x] = display->readPixel(x, y);
            }
            fp.write((uint8_t*)lineBuf, sizeof(lineBuf));
        }
        fp.close();
    }
    display->setBrightness(b);
}