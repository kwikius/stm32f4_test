#ifndef STM32F4_I2C_LIB_EEPROM_READER_HPP_INCLUDED
#define STM32F4_I2C_LIB_EEPROM_READER_HPP_INCLUDED

#include <cstdint>
#include "eeprom.hpp"

struct i2c_eeprom_reader : eeprom{
   static bool read(uint32_t data_address, uint8_t* data, uint32_t len);
private:
   static bool apply(uint32_t data_address, uint8_t* data, uint32_t len);
   static void on_start_sent();
   static void on_device_address_sent();
   static void on_data_address_hi_sent();
   static void on_data_address_lo_sent();
   static void on_start2_sent();
   static void on_device_read_address_sent();
   static void on_dma_transfer_complete();
   static void single_byte_receive_handler();
   static void on_error();

   // data pt and length only used for single byte reads
   static uint8_t * m_data_ptr;
   static uint32_t  m_data_length;
   static uint8_t   m_data_address[2]; // could do for dma in dma avail memmory
};

#endif // STM32F4_I2C_LIB_EEPROM_READER_HPP_INCLUDED
