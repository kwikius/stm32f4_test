#ifndef STM32_TEST_I2C_REGISTER_BASED_DRIVER_HPP_INCLUDED
#define STM32_TEST_I2C_REGISTER_BASED_DRIVER_HPP_INCLUDED

#include <cstdint>
#include "../i2c_driver/i2c_driver.hpp"

 extern "C" bool is_valid_heap_memory(void * p);

/*
  ned
*/

struct i2c_register_based_driver_base : i2c_driver{

  protected :

   static bool ll_read(uint8_t register_index, uint8_t * data, uint32_t len);
   static bool ll_write(uint8_t register_index, uint8_t value);

  private:
   // read handlers
   static void on_read_start_sent();
   static void on_read_device_address_sent();
   static void on_read_reg_index_sent();
   static void on_read_repeated_start_sent();
   static void on_read_device_read_address_sent();
   static void on_read_single_byte_handler();
  // static void on_read_multi_byte_handler();
   // dma handler
   static void on_read_dma_transfer_complete();
      // error handler
   static void on_read_error();

   // write handlers
   static void on_write_start_sent();
   static void on_write_device_address_sent();
   static void on_write_reg_index_sent();
   static void on_write_value_sent();

   static void on_write_error();
   // save space but respect constness of
   // input when writing to the device
   union data_ptr_type{
      uint8_t * read_ptr; 
      uint8_t const * write_ptr;
      uint8_t  value_to_write;
   };
   static data_ptr_type m_data;
   static uint32_t m_data_length;
   static uint8_t  m_register_index; //

};

template <typename ID> 
struct i2c_register_based_driver : i2c_register_based_driver_base{
   static bool read(uint8_t register_index, uint8_t * data, uint32_t len)
   {
 
      if (! get_bus(quan::time_<uint32_t>::ms{500U})){
         return false;
      }
      set_device_name(ID::get_device_name());
      set_device_address(ID::get_device_address());
      return ll_read(register_index,data,len);
   }
   static bool write(uint8_t register_index, uint8_t value)
   {
      if (! get_bus(quan::time_<uint32_t>::ms{500U})){
         return false;
      }
      set_device_name(ID::get_device_name());
      set_device_address(ID::get_device_address());
      return ll_write(register_index,value);
   }
}; 


#endif // STM32_TEST_I2C_REGISTER_BASED_DRIVER_HPP_INCLUDED
