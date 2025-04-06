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
    lgfx::Panel_ST7789 _panel_instance;
    lgfx::Bus_SPI _bus_instance;
    lgfx::Light _light_instance;
    //lgfx::Touch_XPT2046 _touch_instance;
#endif
};

#endif // LGFX_h