#ifndef STM32F4_I2C_LIB_EEPROM_HPP_INCLUDED
#define STM32F4_I2C_LIB_EEPROM_HPP_INCLUDED

#include <quan/stm32/millis.hpp>

struct eeprom{

   /*
      i2c address
      For large devices e.g 256k where more than 16 bits reprsnt the address
      multiple I2c address are used with the next bit after the read/write bit representing bit 16 of the address
   */

   // absolute time at which last write will be complete
   static quan::time_<uint32_t>::ms get_write_end_time() { return quan::time_<uint32_t>::ms{m_write_end_time};}
   // for larger devices Bit 1 must be set to bit 16 of the eeprom data address
   static constexpr uint8_t i2c_bus_address = 0b10100000; // 0xA0, D'160'

   static constexpr quan::time_<uint32_t>::ms write_cycle_time{5U};
   static constexpr uint32_t data_size = 16U * 1024U;
   static constexpr uint32_t page_size = 64U;
   static bool write_in_progress() { return quan::stm32::millis().numeric_value() < m_write_end_time;}
protected :
   static void set_new_write_end_time()
   {
     m_write_end_time = (quan::stm32::millis() + write_cycle_time).numeric_value() + 1U;
   }

private:
   static volatile uint32_t m_write_end_time;
   eeprom() = delete;
   eeprom(eeprom const &) = delete;
   eeprom& operator= (eeprom const & ) = delete;
};

#endif // STM32F4_I2C_LIB_EEPROM_HPP_INCLUDED
