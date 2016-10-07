#ifndef STM32F4_TEST_I2C_DRIVER_I2C_DRIVER_HPP_INCLUDED
#define STM32F4_TEST_I2C_DRIVER_I2C_DRIVER_HPP_INCLUDED

#include <cstdint>
#include "../system/i2c.hpp"
#include  <quan/stm32/millis.hpp>

// common i2c driver stuff
struct i2c_driver {

   typedef quan::time_<uint32_t>::ms millis_type;

   static bool get_bus(millis_type const & wait_time);

   // the I2C address of the derived device
   // The address is in 8 bit format. Bit 0 is the read write bit, but is always 0 for this address
   // when derived acquires bus it sets this to its handler array
   // read and write addresses?
   static void set_device_address(uint8_t address) { m_device_bus_address = address;}
   static uint8_t get_device_address() { return m_device_bus_address;}
 private:
   static uint8_t            m_device_bus_address; 

   i2c_driver() = delete;
   i2c_driver(i2c_driver const & ) = delete;
   i2c_driver& operator = (i2c_driver&) = delete;

};


#endif // STM32F4_TEST_I2C_DRIVER_I2C_DRIVER_HPP_INCLUDED
