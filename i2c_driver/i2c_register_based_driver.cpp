
#include "../system/serial_port.hpp"
#include "../system/i2c.hpp"
#include "../system/led.hpp"
#include "i2c_register_based_driver.hpp"

i2c_register_based_driver_base::data_ptr_type i2c_register_based_driver_base::m_data{}; // return data
uint32_t i2c_register_based_driver_base::m_data_length = 0U;
uint8_t  i2c_register_based_driver_base::m_register_index = 0U; //

using quan::stm32::millis;

/*
This function starts the read and then  returns
 The read data ptr must remain valid
 After the function returns
 wait for bus to be released
 which signals that the read has been successful
 see the lis3_mdl_test example.
*/
#define QUAN_I2C_DEBUG

#if defined QUAN_I2C_DEBUG
namespace {

   uint32_t i2c_registers [ 20];
   uint32_t reg_index = 0;
   
}

void show_i2c_registers()
{
   if ( reg_index > 20U){reg_index = 20U;}
   for (uint32_t i = 0; i < reg_index; ++i){
      serial_port::printf<50>("sr1 %d = 0x%X \n",i, i2c_registers[i]);
   }

}
#endif
bool i2c_register_based_driver_base::ll_read(uint8_t register_index, uint8_t * data, uint32_t len)
{
   m_data.read_ptr = data;
   if ( len == 1){
      m_register_index = register_index;
   }else{
     m_register_index = register_index | 0x80;
   }
   m_data_length = len;

   i2c::set_error_handler(on_read_error);
   i2c::set_event_handler(on_read_start_sent); // first handler

   i2c::enable_error_interrupts(true);
   i2c::enable_event_interrupts(true);
   i2c::enable_buffer_interrupts(false);
   i2c::enable_ack_bit(true);

#if defined QUAN_I2C_DEBUG
   reg_index = 0;
#endif

   if (m_data_length > 1 ){
     
//      i2c::enable_dma_bit(true);
//      i2c::enable_dma_last_bit(true);
//      i2c::set_dma_rx_buffer(data,m_data_length);
//      i2c::clear_dma_rx_stream_flags();
//      i2c::set_dma_rx_handler(on_read_dma_transfer_complete);
//      i2c::enable_dma_rx_stream(true);
   }

   i2c::request_start_condition();

   return true;
}

// read handlers
void i2c_register_based_driver_base::on_read_start_sent()
{   // sb set
  #if defined QUAN_I2C_DEBUG
   i2c_registers[reg_index++] = i2c::get_sr1();
#else
   i2c::get_sr1();
#endif
   i2c::send_data(get_device_address()); //
   i2c::set_event_handler(on_read_device_address_sent);
}

void i2c_register_based_driver_base::on_read_device_address_sent()
{   // txe , addr
 #if defined QUAN_I2C_DEBUG
   i2c_registers[reg_index++] = i2c::get_sr1();
#else
   i2c::get_sr1();
#endif
   i2c::get_sr2();
   i2c::send_data(m_register_index);
   i2c::request_start_condition();
   i2c::set_event_handler(on_read_reg_index_sent);
}

void i2c_register_based_driver_base::on_read_reg_index_sent()
{  // txe btf 
 #if defined QUAN_I2C_DEBUG
   i2c_registers[reg_index++] = i2c::get_sr1();
#endif 
   i2c::receive_data(); //clear the txe and btf flags
   i2c::set_event_handler(on_read_repeated_start_sent);
}

void i2c_register_based_driver_base::on_read_repeated_start_sent()
{   // sb
 #if defined QUAN_I2C_DEBUG
   i2c_registers[reg_index++] = i2c::get_sr1();
#else
   i2c::get_sr1();
#endif
   i2c::send_data(get_device_address() | 1);
   i2c::set_event_handler(on_read_device_read_address_sent);
}

void i2c_register_based_driver_base::on_read_device_read_address_sent()
{   // addr
 #if defined QUAN_I2C_DEBUG
   i2c_registers[reg_index++] = i2c::get_sr1();
#else
   i2c::get_sr1();
#endif
   i2c::get_sr2();
   if ( m_data_length > 1){ // into dma
     //  led::on();
    //  i2c::enable_event_interrupts(false);
      // since we transferred control to dma
      // then only the dma handler is  now required
      i2c::set_event_handler(on_read_multi_byte_handler);
   }else{ // dma doesnt work for single byte read
      i2c::enable_ack_bit(false);
      i2c::request_stop_condition();
      i2c::enable_buffer_interrupts(true); // enable rxne
      i2c::set_event_handler(on_read_single_byte_handler);
   }
}

void i2c_register_based_driver_base::on_read_multi_byte_handler()
{   
 #if defined QUAN_I2C_DEBUG
   i2c_registers[reg_index++] = i2c::get_sr1();
#endif
   *m_data.read_ptr = i2c::receive_data();
   ++m_data.read_ptr;
   --m_data_length;
   if ( m_data_length == 1){
      i2c::enable_ack_bit(false);
      i2c::request_stop_condition();
      i2c::enable_buffer_interrupts(true); // enable rxne
      i2c::set_event_handler(on_read_single_byte_handler);
   }
}

void i2c_register_based_driver_base::on_read_single_byte_handler()
{   
 #if defined QUAN_I2C_DEBUG
   i2c_registers[reg_index++] = i2c::get_sr1();
#endif
   *m_data.read_ptr = i2c::receive_data();
   i2c::enable_buffer_interrupts(false);
   i2c::enable_event_interrupts(false);
   i2c::set_default_handlers();
   i2c::release_bus();
}

// dma handler
void i2c_register_based_driver_base::on_read_dma_transfer_complete()
{ 
   
   i2c::enable_dma_rx_stream(false);
   i2c::enable_dma_bit(false);
   i2c::enable_dma_last_bit(false);
   i2c::clear_dma_rx_stream_tcif();
   i2c::request_stop_condition();
   i2c::set_default_handlers();
   i2c::release_bus();
}

void i2c_register_based_driver_base::on_read_error()
{
   if (get_device_name() != nullptr){
      panic (get_device_name());
   }else{
      panic("unknown device");
   }
   serial_port::write(" : read error");
   i2c::default_error_handler();
}

void i2c_register_based_driver_base::on_write_error()
{
   if (get_device_name() != nullptr){
      panic (get_device_name());
   }else{
      panic("unknown device");
   }
   serial_port::write(" : write error");
   i2c::default_error_handler();
}


bool i2c_register_based_driver_base::ll_write(uint8_t register_index, uint8_t value)
{
   m_data.value_to_write = value;
   m_register_index = register_index;
   m_data_length = 1U;

   i2c::set_error_handler(on_write_error);
   i2c::set_event_handler(on_write_start_sent);

   i2c::enable_error_interrupts(true);
   i2c::enable_event_interrupts(true);
   i2c::enable_buffer_interrupts(false);
   i2c::enable_ack_bit(true);

   i2c::request_start_condition();
   return true;
}

// write handlers
// only one reg is written at a time for setting up
void i2c_register_based_driver_base::on_write_start_sent()
{  
   i2c::get_sr1();
   i2c::send_data(get_device_address());
   i2c::set_event_handler(on_write_device_address_sent);
}

void i2c_register_based_driver_base::on_write_device_address_sent()
{   
   i2c::get_sr1();
   i2c::get_sr2();
   i2c::send_data(m_register_index);
   i2c::set_event_handler(on_write_reg_index_sent); // on_write_reg_index_sent
}

void i2c_register_based_driver_base::on_write_reg_index_sent()
{  
   i2c::send_data(m_data.value_to_write);
   i2c::set_event_handler(on_write_value_sent);
}

void i2c_register_based_driver_base::on_write_value_sent()
{   
   i2c::enable_event_interrupts(false);
   i2c::request_stop_condition();
   i2c::set_default_handlers();
   i2c::release_bus();
}

namespace {
   constexpr quan::time_<uint32_t>::ms wait_time{500U};
}
