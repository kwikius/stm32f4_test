
#include "../system/serial_port.hpp"
#include "../system/i2c.hpp"
#include "../system/led.hpp"
#include "i2c_driver.hpp"

uint8_t     i2c_driver::m_device_bus_address = 0U; 
const char * i2c_driver::m_device_name = nullptr;

using quan::stm32::millis;

bool i2c_driver::get_bus(i2c_driver::millis_type const & wait_time)
{
   auto now = millis();
   while ( !i2c::bus_released()) {
      if( (millis() - now) > wait_time) {
         serial_port::write("i2c_driver timeout : bus not released\n");
         return false;
      }
   }
   while (!i2c::get_bus()){
      if( (millis() - now) > wait_time) {
         serial_port::write("i2c_driver timeout : failed to acquire bus\n");
         return false;
      }
   }
   return true;
}
