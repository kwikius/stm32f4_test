
#include <quan/stm32/millis.hpp>
#include "../system/serial_port.hpp"
#include "../system/i2c.hpp"
#include "../system/led.hpp"
#include "compass.hpp"

using quan::stm32::millis;
namespace {
   constexpr quan::time_<uint32_t>::ms max_time{500U};
}



uint8_t* compass_reader::m_data_ptr = nullptr; // return data
uint32_t compass_reader::m_data_length = 0U;
uint8_t  compass_reader::m_sub_address = 0U;
uint8_t  compass_sub_address_setter::m_sub_address = 0U;
uint8_t * compass_reader1::m_data_ptr = nullptr;

uint8_t  compass_writer::m_sub_address = 0U;
uint8_t  compass_writer::m_value = 0U;

bool compass_reader1::read(uint8_t * data)
{
   auto now = millis();
   while ( !i2c::bus_released()) {
      if( (millis() - now) > max_time) {
         serial_port::write("compass read timeout :bus not released\n");
         return false;
      }
   }
   now = millis();
   while (!i2c::get_bus()){
      if( (millis() - now) > max_time) {
         serial_port::write("compass read timeout: failed get bus\n");
         return false;
      }
   }
   m_data_ptr = data;
   i2c::set_error_handler(on_error);
   i2c::set_event_handler(on_start_sent);

   i2c::enable_error_interrupts(true);
   i2c::enable_event_interrupts(true);
   i2c::enable_buffer_interrupts(false);
   i2c::enable_ack_bit(true);

   // set up hyandlers
   //i2c::event_handlers [] = {on_device_address_sent, on_read

   i2c::request_start_condition();


   return true;
}

// Start condition generated event . (EV5)
void compass_reader1::on_start_sent()
{
   i2c::get_sr1();
   i2c::send_data(i2c_bus_address | 1);
   i2c::set_event_handler(on_device_address_sent);
}
// device address sent event. EV6
void compass_reader1::on_device_address_sent()
{
   i2c::enable_ack_bit(false);
   i2c::enable_buffer_interrupts(false);
   i2c::get_sr1();
   i2c::get_sr2();
   i2c::enable_buffer_interrupts(true);
   i2c::enable_buffer_interrupts(true);
   i2c::request_stop_condition();
   i2c::set_event_handler(on_read);
}

void compass_reader1::on_read()
{
   *m_data_ptr = i2c::receive_data();
   i2c::enable_buffer_interrupts(false);
   i2c::enable_event_interrupts(false);
   i2c::set_default_handlers();
   i2c::release_bus();
}

//---------------------------------------

bool compass_sub_address_setter::write(uint8_t sub_address)
{

   auto now = millis();
   while ( !i2c::bus_released()) {
      if( (millis() - now) > max_time) {
         serial_port::write("compass read timeout :bus not released\n");
         return false;
      }
   }
   now = millis();
   while (!i2c::get_bus()){
      if( (millis() - now) > max_time) {
         serial_port::write("compass read timeout: failed get bus\n");
         return false;
      }
   }

   m_sub_address = sub_address;

   i2c::set_error_handler(on_error);
   i2c::set_event_handler(on_start_sent);

   i2c::enable_error_interrupts(true);
   i2c::enable_event_interrupts(true);
   i2c::enable_buffer_interrupts(false);
   i2c::enable_ack_bit(true);

   i2c::request_start_condition();
   return true;
}

// Start condition generated event . (EV5)
void compass_sub_address_setter::on_start_sent()
{
   i2c::get_sr1();
   i2c::send_data(i2c_bus_address);
   i2c::set_event_handler(on_device_address_sent);
}

// device address sent event. EV6
// Clear by reading SR1 followed by reading SR2.
// then send sub_address
// want txe and point to next handler
void compass_sub_address_setter::on_device_address_sent()
{
   i2c::get_sr1();
   i2c::get_sr2();
   i2c::send_data(m_sub_address);
   i2c::set_event_handler(on_sub_address_sent);
}

// btf
void compass_sub_address_setter::on_sub_address_sent()
{
   i2c::enable_event_interrupts(false);
   i2c::request_stop_condition();
   i2c::set_default_handlers();
   i2c::release_bus();
}

//-------------------------------------------------------------
bool compass_writer::write(uint8_t sub_address, uint8_t value)
{
   auto now = millis();
   while ( !i2c::bus_released()) {
      if( (millis() - now) > max_time) {
         serial_port::write("compass read timeout :bus not released\n");
         return false;
      }
   }
   now = millis();
   while (!i2c::get_bus()){
      if( (millis() - now) > max_time) {
         serial_port::write("compass read timeout: failed get bus\n");
         return false;
      }
   }
   m_sub_address = sub_address;
   m_value = value;

   i2c::set_error_handler(on_error);
   i2c::set_event_handler(on_start_sent);

   i2c::enable_error_interrupts(true);
   i2c::enable_event_interrupts(true);
   i2c::enable_buffer_interrupts(false);
   i2c::enable_ack_bit(true);

   i2c::request_start_condition();
   return true;

}

void compass_writer::on_start_sent()
{
   i2c::get_sr1();
   i2c::send_data(i2c_bus_address);
   i2c::set_event_handler(on_device_address_sent);
}

void compass_writer::on_device_address_sent()
{
   i2c::get_sr1();
   i2c::get_sr2();
   i2c::send_data(m_sub_address);
   i2c::set_event_handler(on_sub_address_sent);
}

//btf
void compass_writer::on_sub_address_sent()
{
   i2c::send_data(m_value);
   i2c::set_event_handler(on_value_sent);
}

// btf
void compass_writer::on_value_sent()
{
   i2c::enable_event_interrupts(false);
   i2c::request_stop_condition();
   i2c::set_default_handlers();
   i2c::release_bus();
}

//-------------------------------------------------------------------------------------

bool compass_reader::read(uint8_t sub_address, uint8_t * data, uint32_t data_length)
{
   auto now = millis();
   while ( !i2c::bus_released()) {
      if( (millis() - now) > max_time) {
         serial_port::write("compass read timeout :bus not released\n");
         return false;
      }
   }
   now = millis();
   while (!i2c::get_bus()){
      if( (millis() - now) > max_time) {
         serial_port::write("compass read timeout: failed get bus\n");
         return false;
      }
   }

   m_data_ptr = data;
   m_sub_address = sub_address;
   m_data_length = data_length;

   i2c::set_error_handler(on_error);
   i2c::set_event_handler(on_start_sent);

   i2c::enable_error_interrupts(true);
   i2c::enable_event_interrupts(true);
   i2c::enable_buffer_interrupts(false);
   i2c::enable_ack_bit(true);

   if (data_length > 1 ){
      i2c::enable_dma_bit(true);
      i2c::enable_dma_last_bit(true);
      i2c::set_dma_rx_buffer(data,data_length);
      i2c::clear_dma_rx_stream_flags();
      i2c::set_dma_rx_handler(on_dma_transfer_complete);
      i2c::enable_dma_rx_stream(true);
   }

   i2c::request_start_condition();

   return true;

}

// Start condition generated event . (EV5)
// Clear by reading sr1 and
// then writing dr with address of i2c device
void compass_reader::on_start_sent()
{
   i2c::get_sr1();
   i2c::send_data(i2c_bus_address); //
   i2c::set_event_handler(on_device_address_sent);

}

// device address sent event. EV6
// Clear by reading SR1 followed by reading SR2.
// then send reg address
void compass_reader::on_device_address_sent()
{
   i2c::get_sr1();
   i2c::get_sr2();
   i2c::send_data(m_sub_address);
   i2c::request_start_condition();
   i2c::set_event_handler(on_sub_address_sent);

}

void compass_reader::on_sub_address_sent()
{
    i2c::receive_data(); //clear the txe and btf flags
    i2c::set_event_handler(on_start2_sent);
}

// repeated start
// read
void compass_reader::on_start2_sent()
{
   i2c::get_sr1();
   i2c::send_data(i2c_bus_address | 1);
   i2c::set_event_handler(on_device_read_address_sent);
}

// read addr
void compass_reader::on_device_read_address_sent()
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
       i2c::set_event_handler(single_byte_receive_handler);
    }
}

void compass_reader::single_byte_receive_handler()
{
   *m_data_ptr = i2c::receive_data();
   i2c::enable_buffer_interrupts(false);
   i2c::enable_event_interrupts(false);
   i2c::set_default_handlers();
   i2c::release_bus();
}

// dma  handler
void compass_reader::on_dma_transfer_complete()
{
   i2c::enable_dma_rx_stream(false);
   i2c::enable_dma_bit(false);
   i2c::enable_dma_last_bit(false);
   i2c::clear_dma_rx_stream_tcif();
   i2c::request_stop_condition();
   i2c::set_default_handlers();
   i2c::release_bus();
}

// error handler
// provide info as to who caused the error
// then call default
void compass::on_error()
{
   panic ("i2c compass error");
   i2c::default_error_handler();
}
