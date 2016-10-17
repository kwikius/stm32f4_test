#ifndef STM32F4_TEST_I2C_DRIVER_I2C_DRIVER_HPP_INCLUDED
#define STM32F4_TEST_I2C_DRIVER_I2C_DRIVER_HPP_INCLUDED

#include <cstdint>
#include "../system/i2c.hpp"
#include  <quan/stm32/millis.hpp>

// common i2c driver stuff
struct i2c_driver {

   typedef quan::time_<uint32_t>::ms millis_type;
   static const char * get_device_name()    { return m_device_name;}
   static uint8_t      get_device_address() { return m_device_bus_address;}
 protected:
   static bool get_bus(millis_type const & wait_time);
   // the I2C address of the derived device
   // N.B. only call these  after the bus has been acquired
   static void set_device_name( const char * name) { m_device_name = name;}
   // The address is in 8 bit format. Bit 0 is the read/write bit, but is always 0 for this address
   static void set_device_address(uint8_t address) { m_device_bus_address = address;}

   static bool install_device(const char* name, uint8_t address)
   {
      if (! get_bus(quan::time_<uint32_t>::ms{500U})){
         return false;
      }
      set_device_name(name);
      set_device_address(address);
      return true;
   }

 private:
   static uint8_t            m_device_bus_address; 
   static const char*        m_device_name;
   i2c_driver() = delete;
   i2c_driver(i2c_driver const & ) = delete;
   i2c_driver& operator = (i2c_driver&) = delete;

};

#endif // STM32F4_TEST_I2C_DRIVER_I2C_DRIVER_HPP_INCLUDED
