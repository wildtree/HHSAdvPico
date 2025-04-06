#include <LGFX.h>

#if defined(LCD28)
namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  bool Light::init( uint8_t brightness )
  {

    gpio_init( _cfg.pin_bl );
    gpio_set_dir( _cfg.pin_bl, GPIO_OUT );
    gpio_put( _cfg.pin_bl, 1 );

    setBrightness(brightness);

    return true;
  }

  void Light::setBrightness( uint8_t brightness )
  {
    if (_cfg.invert) brightness = ~brightness;
    // uint32_t duty = brightness + (brightness >> 7);
    if (brightness == 0)
        digitalWrite(_cfg.pin_bl, LOW);
    else
        digitalWrite(_cfg.pin_bl, HIGH);
  }

//----------------------------------------------------------------------------
 }
}
#endif

LGFX::LGFX()
{
#if defined(PICOCALC)
    {
        auto cfg = _bus_instance.config();
        cfg.spi_host = 1; // SPI1 for pico/pico2
        cfg.spi_mode = 0; // SPI mode 0
        cfg.freq_write = 50000000; // SPI write frequency
        cfg.freq_read = 25000000; // SPI read frequency

        cfg.pin_sclk = 10; // SCLK pin
        cfg.pin_mosi = 11; // MOSI pin
        cfg.pin_miso = 12; // MISO pin
        cfg.pin_dc   = 14; // DC pin

        _bus_instance.config(cfg); // Apply bus configuration
        _panel_instance.setBus(&_bus_instance); // Set bus to panel instance
    }
    {
        auto cfg = _panel_instance.config();
        cfg.pin_cs = 13; // CS pin
        cfg.pin_rst = 15; // RST pin
        cfg.pin_busy = -1; // No busy pin

        cfg.memory_width = 320; // Screen width
        cfg.memory_height = 480; // Screen height
        cfg.panel_width = 320; // Panel width
        cfg.panel_height = 320; // Panel height

        cfg.offset_x = 0; // X offset
        cfg.offset_y = 0; // Y offset
        cfg.offset_rotation = 6; // Rotation offset

        cfg.dummy_read_pixel = 8; // Dummy read pixel
        cfg.dummy_read_bits = 1; // Dummy read bits

        cfg.readable = true; // Enable readable mode

        cfg.invert = true; // inversion
        cfg.rgb_order = false; // RGB order

        // cfg.bus_shared = true; // Bus shared

        _panel_instance.config(cfg); // Apply panel configuration
    }
#if 0
    {
        auto cfg = _light_instance.config();
        cfg.pin_bl = 4; // Backlight pin
        cfg.invert = false; // No inversion
        cfg.freq = 44100; // Frequency
        cfg.pwm_channel = 0; // PWM channel

        _light_instance.config(cfg); // Apply backlight configuration
        _panel_instance.setLight(&_light_instance); // Set backlight to panel instance
    }
#endif
    setPanel(&_panel_instance); // Set panel to device
#elif defined(LCD28)
    {
        auto cfg = _bus_instance.config();
        cfg.spi_host = 1;
        cfg.spi_mode = 0;
        cfg.freq_write = 40000000;
        cfg.freq_read = 6000000;

        cfg.pin_sclk = 10;
        cfg.pin_mosi = 11;
        cfg.pin_miso = 12;
        cfg.pin_dc = 8;

        _bus_instance.config(cfg);
        _panel_instance.setBus(&_bus_instance);
    }
    {
        auto cfg = _panel_instance.config();
        cfg.pin_cs = 9;
        cfg.pin_rst = 15;
        cfg.pin_busy = -1;

        cfg.memory_width = 240;
        cfg.memory_height = 320;
        cfg.panel_width = 240;
        cfg.panel_height = 320;

        cfg.offset_x = 0;
        cfg.offset_y = 0;
        cfg.offset_rotation = 3;

        cfg.dummy_read_bits = 8;
        cfg.dummy_read_pixel = 1;

        cfg.readable = true;

        cfg.invert = true;
        cfg.rgb_order = false;

        cfg.bus_shared = true;

        _panel_instance.config(cfg);
    }
    {
        auto cfg = _light_instance.config();

        cfg.pin_bl = 13;
        cfg.invert = false;
        //cfg.freq = 44100;

        _light_instance.config(cfg);
        _panel_instance.setLight(&_light_instance);
    }
#if 0
    {
        auto cfg = _touch_instance.config();

        cfg.pin_cs = 16;
        cfg.pin_int = 17;
        cfg.pin_sclk = 10;
        cfg.pin_mosi = 11;
        cfg.pin_miso = 12;
        cfg.bus_shared = true;
        cfg.i2c_addr = 0x38; // I2C address for touch controller
        cfg.i2c_port = 0; // I2C port number

        cfg.freq = 2500000;

        _touch_instance.config(cfg);
        _panel_instance.setTouch(&_touch_instance);
    }
#endif
    setPanel(&_panel_instance);
#endif
}
