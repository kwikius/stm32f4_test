
#include "../system/serial_port.hpp"
#include "../system/i2c.hpp"
#include "../system/led.hpp"
#include "lis3_mdl.hpp"

   lis3_mdl::data_ptr_type lis3_mdl::m_data{}; // return data
   uint32_t lis3_mdl::m_data_length = 0U;
   uint8_t  lis3_mdl::m_register_index = 0U; //

using quan::stm32::millis;
namespace {
   constexpr quan::time_<uint32_t>::ms wait_time{500U};
}


bool lis3_mdl::read(uint8_t register_index, uint8_t * data, uint32_t len)
{
   if (! get_bus(wait_time)){
      return false;
   }

   set_device_address(0b00111000);
   m_data.read_ptr = data;
   m_register_index = register_index;
   m_data_length = len;

   i2c::set_error_handler(on_read_error);
   i2c::set_event_handler(on_read_start_sent); // first handler

   i2c::enable_error_interrupts(true);
   i2c::enable_event_interrupts(true);
   i2c::enable_buffer_interrupts(false);
   i2c::enable_ack_bit(true);

   if (m_data_length > 1 ){
      i2c::enable_dma_bit(true);
      i2c::enable_dma_last_bit(true);
      i2c::set_dma_rx_buffer(data,m_data_length);
      i2c::clear_dma_rx_stream_flags();
      i2c::set_dma_rx_handler(on_read_dma_transfer_complete);
      i2c::enable_dma_rx_stream(true);
   }

   i2c::request_start_condition();

   return true;
}

// TODO-----------------------

bool lis3_mdl::write(uint8_t sub_address, uint8_t value)
{return false;}
// read handlers
void lis3_mdl::on_read_start_sent()
{   
   i2c::get_sr1();
   i2c::send_data(get_device_address()); //
   i2c::set_event_handler(on_read_device_address_sent);
}

void lis3_mdl::on_read_device_address_sent()
{   
   i2c::get_sr1();
   i2c::get_sr2();
   i2c::send_data(m_register_index);
   i2c::request_start_condition();
   i2c::set_event_handler(on_read_reg_index_sent);
}

void lis3_mdl::on_read_reg_index_sent()
{    
   i2c::receive_data(); //clear the txe and btf flags
   i2c::set_event_handler(on_read_repeated_start_sent);
}

void lis3_mdl::on_read_repeated_start_sent()
{   
   i2c::get_sr1();
   i2c::send_data(get_device_address() | 1);
   i2c::set_event_handler(on_read_device_read_address_sent);
}

void lis3_mdl::on_read_device_read_address_sent()
{   
   i2c::get_sr1();
   i2c::get_sr2();
   if ( m_data_length > 1){ // into dma
      i2c::enable_event_interrupts(false);
      // since we transferred control to dma
      // then only the dma handler is  now required
      i2c::set_event_handler(i2c::default_event_handler);
   }else{ // dma doesnt work for single byte read
      i2c::enable_ack_bit(false);
      i2c::request_stop_condition();
      i2c::enable_buffer_interrupts(true); // enable rxne
      i2c::set_event_handler(on_read_single_byte_handler);
   }
}

void lis3_mdl::on_read_single_byte_handler()
{   
   *m_data.read_ptr = i2c::receive_data();
   i2c::enable_buffer_interrupts(false);
   i2c::enable_event_interrupts(false);
   i2c::set_default_handlers();
   i2c::release_bus();
}

// dma handler
void lis3_mdl::on_read_dma_transfer_complete()
{ 
   i2c::enable_dma_rx_stream(false);
   i2c::enable_dma_bit(false);
   i2c::enable_dma_last_bit(false);
   i2c::clear_dma_rx_stream_tcif();
   i2c::request_stop_condition();
   i2c::set_default_handlers();
   i2c::release_bus();
}

void lis3_mdl::on_read_error()
{
   panic ("LIS3MDL Compass : read error");
   i2c::default_error_handler();
}

// write handlers
// only one reg is read I think
void lis3_mdl::on_write_start_sent()
{return;}

void lis3_mdl::on_write_device_address_sent()
{return;}

void lis3_mdl::on_write_reg_index_sent()
{return;}

void lis3_mdl::on_write_value_sent()
{return;}
