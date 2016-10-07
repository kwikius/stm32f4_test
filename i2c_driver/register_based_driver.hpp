#ifndef QUAN_STM32_F4_TEST_I2C_DRIVER_REGISTER_BASED_DRIVER_HPP_INCLUDED
#define QUAN_STM32_F4_TEST_I2C_DRIVER_REGISTER_BASED_DRIVER_HPP_INCLUDED

#include "i2c_driver.hpp"

/*
   many devices are nased on state in registers
   regsiters can be written and read form
   some devices have auto increment
   when a register is read
*/
struct i2c_register_based_driver : i2c_driver{

   static void read(uint8_t reg_index, uint8_t & result);
   static void write(uint8_t reg_index, uint8_t & result);


   static void set_register_index(uint8_t index) { m_reg_index = index;}
   static void get_register_index(return m_register_index;}

   static void on_device_address_sent();
private :
   static uint8_t m_register_index; // the index of the register to read or write from
   static uint8_t * m_data_ptr;
   static pfn_irq_handler  m_irq_handlers[];
};

#endif // QUAN_STM32_F4_TEST_I2C_DRIVER_REGISTER_BASED_DRIVER_HPP_INCLUDED
