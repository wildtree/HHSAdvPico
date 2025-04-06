// Pico Calc Display module device
#if !defined(LGFX_h)
#define LGFX_h
#include <LovyanGFX.hpp> // Include LovyanGFX library

// PicoCalc
//#define Pico_LCD_SCK 10 //
//#define Pico_LCD_TX  11 // MOSI
//#define Pico_LCD_RX  12 // MISO
//#define Pico_LCD_CS  13 //
//#define Pico_LCD_DC  14
//#define Pico_LCD_RST 15

// Pico ResTouch LCD 2.8
//#define Rst_LCD_SCK 10 //
//#define Rst_LCD_TX  11 // MOSI
//#define Rst_LCD_RX  12 // MISO
//#define Rst_LCD_CS  9  //
//#define Rst_LCD_DC  8
//#define Rst_LCD_RST 15
//#define Rst_LCD_BL  13

#if defined(LCD28)
namespace lgfx
{
    inline namespace v1
    {
        //----------------------------------------------------------------------------

        struct Panel_ST7789_LCD28  : public Panel_LCD
        {
            Panel_ST7789_LCD28(void)
            {
                _cfg.panel_height = _cfg.memory_height = 320;
                _cfg.dummy_read_pixel = 16;
            }

        protected:

            const uint8_t* getInitCommands(uint8_t listno) const override {

                static constexpr uint8_t list0[] = {
                    0x11, 0+CMD_INIT_DELAY, 100,
                    0x36, 1, 0x00,
                    0x3A, 1, 0x55,
                    0xB2, 5, 0x0C, 0x0C, 0x00, 0x33, 0x33,
                    0xb7, 1, 0x35,
                    0xbb, 1, 0x28,
                    0xc0, 1, 0x3c,
                    0xc2, 1, 0x01,
                    0xc3, 1, 0x0b,
                    0xc4, 1, 0x20,
                    0xc6, 1, 0x0f,
                    0xd0, 1, 0xa7,
                    0xd0, 2, 0xa4, 0xa1,
                    0xe0, 14, 0xd0, 0x01, 0x08 ,0x0f, 0x11, 0x2a, 0x36, 0x55, 0x44, 0x3a, 0x0b, 0x06, 0x11, 0x20,
                    0xe1, 14, 0xd0, 0x02, 0x07, 0x0a, 0x0b, 0x18, 0x34, 0x43, 0x4a, 0x2b, 0x1b, 0x1c, 0x22, 0x1f,
                    0x55, 1, 0xb0,
                    0x29, 0,
                    0xFF,0xFF, // end
                };
                switch (listno) {
                    case 0: return list0;
                    default: return nullptr;
                }
            }
        };

        //----------------------------------------------------------------------------
        class Light : public ILight
        {
        public:
          struct config_t
          {
            uint32_t freq = 1200;
            int16_t pin_bl = -1;
            uint8_t pwm_channel = 0;
            bool invert = false;
          };
      
          const config_t& config(void) const { return _cfg; }
      
          void config(const config_t &cfg) { _cfg = cfg; }
      
          bool init(uint8_t brightness) override;
          void setBrightness(uint8_t brightness) override;
      
        private:
          config_t _cfg;
          uint8_t _slice_num;
        };
    }
}
#endif

class LGFX : public lgfx::LGFX_Device
{
public:
    LGFX();
protected:
#if defined(PICOCALC)
    lgfx::Panel_ILI9488 _panel_instance; // ILI9488 panel
    lgfx::Bus_SPI _bus_instance; // SPI bus instance
    //lgfx::Light_PWM _light_instance; // Backlight instance
    //lgfx::Touch_FT6X36 _touch_instance; // Touch instanceL
#elif defined(LCD28)
    lgfx::Panel_ST7789_LCD28 _panel_instance;
    lgfx::Bus_SPI _bus_instance;
    lgfx::Light _light_instance;
    //lgfx::Touch_XPT2046 _touch_instance;
#endif
};

#endif // LGFX_h