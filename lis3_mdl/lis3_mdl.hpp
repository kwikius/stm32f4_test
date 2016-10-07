#ifndef LIS3_MDL_HPP_INCLUDED
#define LIS3_MDL_HPP_INCLUDED

#include <cstdint>
#include "../i2c_driver/i2c_driver.hpp"

struct lis3_mdl : i2c_driver{
   static bool read(uint8_t register_index, uint8_t * data, uint32_t len);
   static bool write(uint8_t register_index, uint8_t value);
   // read handlers
   static void on_read_start_sent();
   static void on_read_device_address_sent();
   static void on_read_reg_index_sent();
   static void on_read_repeated_start_sent();
   static void on_read_device_read_address_sent();
   static void on_read_single_byte_handler();
   // dma handler
   static void on_read_dma_transfer_complete();
      // error handler
   static void on_read_error();

   // write handlers
   static void on_write_start_sent();
   static void on_write_device_address_sent();
   static void on_write_reg_index_sent();
   static void on_write_value_sent();



  private:
    union data_ptr_type{
      uint8_t * read_ptr; // return data
      uint8_t const * write_ptr;
   };
   static data_ptr_type m_data;
   static uint32_t m_data_length;
   static uint8_t  m_register_index; //

  public:
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

   struct val{
      static constexpr uint8_t whoami = 0b00111101;
   };


};


#endif // LIS3_MDL_HPP_INCLUDED
