
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
bool i2c_register_based_driver_base::ll_read(uint8_t register_index, uint8_t * data, uint32_t len)
{
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

bool i2c_register_based_driver_base::ll_write(uint8_t sub_address, uint8_t value)
{return false;}
// read handlers
void i2c_register_based_driver_base::on_read_start_sent()
{   
   i2c::get_sr1();
   i2c::send_data(get_device_address()); //
   i2c::set_event_handler(on_read_device_address_sent);
}

void i2c_register_based_driver_base::on_read_device_address_sent()
{   
   i2c::get_sr1();
   i2c::get_sr2();
   i2c::send_data(m_register_index);
   i2c::request_start_condition();
   i2c::set_event_handler(on_read_reg_index_sent);
}

void i2c_register_based_driver_base::on_read_reg_index_sent()
{    
   i2c::receive_data(); //clear the txe and btf flags
   i2c::set_event_handler(on_read_repeated_start_sent);
}

void i2c_register_based_driver_base::on_read_repeated_start_sent()
{   
   i2c::get_sr1();
   i2c::send_data(get_device_address() | 1);
   i2c::set_event_handler(on_read_device_read_address_sent);
}

void i2c_register_based_driver_base::on_read_device_read_address_sent()
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

void i2c_register_based_driver_base::on_read_single_byte_handler()
{   
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

// write handlers
// only one reg is written at a time for setting up
void i2c_register_based_driver_base::on_write_start_sent()
{return;}

void i2c_register_based_driver_base::on_write_device_address_sent()
{return;}

void i2c_register_based_driver_base::on_write_reg_index_sent()
{return;}

void i2c_register_based_driver_base::on_write_value_sent()
{return;}

namespace {
   constexpr quan::time_<uint32_t>::ms wait_time{500U};
}

//template <typename ID>
//bool i2c_register_based_driver<ID>::read(uint8_t register_index, uint8_t * data, uint32_t len)
//{
//     if (! get_bus(wait_time)){
//        return false;
//     }
//     set_device_name(ID::device_name);
//     set_device_address(ID::bus_address);
//     return ll_read(register_index,data,len);
//}
//
//template <typename ID>
//bool i2c_register_based_driver<ID>::write(uint8_t register_index, uint8_t value)
//{
//    return false;
//}
