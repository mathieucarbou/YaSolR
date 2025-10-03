#define LGFX_USE_V1
#include <LovyanGFX.hpp>

//Pour SSD1306
class LGFXoled : public lgfx::LGFX_Device {
  lgfx::Panel_SSD1306 _panel_instance_SD;  //
  lgfx::Panel_SH110x _panel_instance_SH;   //
  lgfx::Bus_I2C _bus_instance;

public:

  void LGFXoled_init(int pin_sda, int pin_scl, bool SD) {
    if (SD) { //SD1306
      {
        auto cfg = _bus_instance.config();
        // I2C
        cfg.pin_sda = pin_sda;  //18;//21;
        cfg.pin_scl = pin_scl;  //22;
        cfg.i2c_addr = 0x3C;
        _bus_instance.config(cfg);
        _panel_instance_SD.setBus(&_bus_instance);
      }

      {
        auto cfg = _panel_instance_SD.config();
        cfg.memory_width = 128;  //128 pour SSD1306 et 130 pour SH1106
        cfg.panel_width = 128;
        cfg.memory_height = 64;
        cfg.panel_height = 64;
        cfg.bus_shared = false;
        cfg.offset_rotation = 0;
        _panel_instance_SD.config(cfg);
      }
      setPanel(&_panel_instance_SD);
    } else { //SH1106
      {
        auto cfg = _bus_instance.config();
        // I2C
        cfg.pin_sda = pin_sda;  //18;//21;
        cfg.pin_scl = pin_scl;  //22;
        cfg.i2c_addr = 0x3C;
        _bus_instance.config(cfg);
        _panel_instance_SH.setBus(&_bus_instance);
      }

      {
        auto cfg = _panel_instance_SH.config();
        cfg.memory_width = 130;  //128 pour SSD1306 et 130 pour SH1106
        cfg.panel_width = 130;
        cfg.memory_height = 64;
        cfg.panel_height = 64;
        cfg.bus_shared = false;
        cfg.offset_rotation = 0;
        _panel_instance_SH.config(cfg);
      }
      setPanel(&_panel_instance_SH);
    }

  }
};