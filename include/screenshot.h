//
// Screen Shot to bmp
//
#ifndef SCREENSHOT_H
#define SCREENSHOT_H

#include <Arduino.h>
#include <LGFX.h>
#include <Wire.h>
#include <SD.h>
#include <sys/syslimits.h>

class ScreenShot
{
public:
    static ScreenShot &instance() { static ScreenShot i; return i; }
    static void take(lgfx::LGFX_Device *display, const char *filename = nullptr);
protected:
    ScreenShot();
    ~ScreenShot();
    static const char *get_next_filename();

    struct __attribute__((__packed__)) BMP_File_Header {
        uint16_t bfType;
        uint32_t bfSize;
        uint16_t bfReserved1, bfReserved2;
        uint32_t bfOffBits;
    };

    struct __attribute__((__packed__)) BMP_Info_Header {
        uint32_t biSize;
        uint32_t biWidth;
        uint32_t biHeight;
        uint16_t biPlanes;
        uint16_t biBitCount;
        uint32_t biCompression;
        uint32_t biSizeImage;
        uint32_t biXPixPerMeter;
        uint32_t biYPixPerMeter;
        uint32_t biClrUsed;
        uint32_t biClrImportant;
    };

    static BMP_File_Header _bf;
    static BMP_Info_Header _bi;

    static char _filename[PATH_MAX];
};

#endif