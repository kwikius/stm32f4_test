#ifndef STM32F4_I2C_LIB_COMPASS_HPP_INCLUDED
#define STM32F4_I2C_LIB_COMPASS_HPP_INCLUDED

#include <cstdint>
/*
 basically write  a byte to set register address
   The msb of the sub address is 1 to auto incr or 0 not to  auto incr

 to write data just send a byte

  To read
   Do a write with the sub address to read, and auto incr if reading more than 1 reg
   then do repeated start to start redaing the data
*/
/*
LIS3MDL
*/
struct compass{

   static constexpr uint8_t i2c_bus_address = 0b00111000;
   // registers addresses
   struct reg{

      static constexpr uint8_t offset_X_reg_L = 0x05;
      static constexpr uint8_t offset_X_reg_H = 0x06;
      static constexpr uint8_t offset_Y_reg_L = 0x07;
      static constexpr uint8_t offset_Y_reg_H = 0x08;
      static constexpr uint8_t offset_Z_reg_L = 0x09;
      static constexpr uint8_t offset_Z_reg_H = 0x0A;

      static constexpr uint8_t whoami = 0x0f;
      static constexpr uint8_t ctr1reg1 = 0x20;
      static constexpr uint8_t ctr1reg2 = 0x21;
      static constexpr uint8_t ctr1reg3 = 0x22;
      static constexpr uint8_t ctr1reg4 = 0x23;
      static constexpr uint8_t ctr1reg5 = 0x24;

   };

   // device constant values
   struct val{
      static constexpr uint8_t whoami = 0b00111101;
   };

   static void on_error();
};

struct compass_reader : compass{

   static bool read(uint8_t sub_address, uint8_t * data, uint32_t len);

private:

   static void on_start_sent();
   static void on_device_address_sent();
   static void on_device_read_address_sent();
   static void on_sub_address_sent();
   static void on_start2_sent();
   static void on_device_read_sent();
   static void on_dma_transfer_complete();
   static void single_byte_receive_handler();

//------------data----

   static uint8_t* m_data_ptr; // return data
   static uint32_t m_data_length;
   static uint8_t  m_sub_address; //
};

struct compass_sub_address_setter : compass{

   static bool write(uint8_t sub_address);

private:

   static void on_start_sent();
   static void on_device_address_sent();
   static void on_sub_address_sent();

   static uint8_t  m_sub_address;
};

struct compass_reader1 : compass{
   static bool read(uint8_t * data);
private:
   static void on_start_sent();
   static void on_device_address_sent();
   static void on_read();

   static uint8_t * m_data_ptr;

};

// write a register, first byte is always the reg address
struct compass_writer : compass {
   static bool write(uint8_t sub_address, uint8_t value);
private:
   static void on_start_sent();
   static void on_device_address_sent();
   static void on_sub_address_sent();
   static void on_value_sent();

   static uint8_t  m_sub_address;
   static uint8_t  m_value;

};

#endif // STM32F4_I2C_LIB_COMPASS_HPP_INCLUDED
