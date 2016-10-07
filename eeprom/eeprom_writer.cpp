
#include "../system/i2c.hpp"
#include "../system/serial_port.hpp"
#include "eeprom_writer.hpp"

volatile uint32_t eeprom::m_write_end_time=0U;

// return true if apply ok
// else leave in good state and return false
// (call i2c::is_busy() first)
// The function exits before the write is complete
// call i2c::is_busy() after returning
 // and wait til it returns false
 // device address is the 8 bit i2c address
  // the data buffer must remain available until i2c::bus_released() returns true
using quan::stm32::millis;
namespace {
constexpr quan::time_<uint32_t>::ms max_time{500U};
}
bool i2c_eeprom_writer::apply(uint32_t data_address, uint8_t const* data, uint32_t len)
{
   auto now = millis();
   while (!i2c::bus_released() || eeprom::write_in_progress()) {
      if( (millis() - now) > max_time) {
         serial_port::write("eewr apply timeout 1:");
         if (!i2c::bus_released()){
            serial_port::write("bus not released\n");
         }
         if (eeprom::write_in_progress()){
            serial_port::write("write in progress\n");
         }
         return false;
      }

   }
   now = millis();
   while (!i2c::get_bus()){
        if( (millis() - now) > max_time) {
         serial_port::write("eewr apply timeout fail get bus\n");
         return false;
      }
   }

   m_data_address[0] = static_cast<uint8_t>((data_address & 0xFF00) >> 8U);
   m_data_address[1] = static_cast<uint8_t>(data_address & 0xFF);

   i2c::set_error_handler(on_error);
   i2c::set_event_handler(on_start_sent);
   i2c::set_dma_tx_handler(on_dma_transfer_complete);
   i2c::set_dma_tx_buffer(data,len);
   i2c::enable_dma_tx_stream(false);
   i2c::clear_dma_tx_stream_flags();
   i2c::enable_dma_bit(true);
   i2c::enable_event_interrupts(true);
   i2c::request_start_condition();

   return true;
}

bool i2c_eeprom_writer::write(uint32_t start_address_in, uint8_t const* data_in, uint32_t len)
{
   if ( len == 0){
      panic("eeprom write zero length");
      return false;
   }
   if ( data_in == nullptr){
      panic("eeprom write data is null");
      return false;
   }
   // end address is one past the last address to write
   uint32_t const end_address = start_address_in + len;

   if ( end_address >= eeprom::data_size){
      panic("eeprom_write address out of range");
      return false;
   }
   uint32_t const start_page = start_address_in / eeprom::page_size;
   uint32_t const end_page =  (end_address -1) / eeprom::page_size;
   if ( start_page == end_page){
      return apply(start_address_in,data_in, len);
   }else{
      uint32_t start_address = start_address_in;
      uint8_t const* data = data_in;
      uint8_t const* const data_end = data_in + len;
      uint32_t cur_page = start_page;
      uint32_t bytes_to_write  = eeprom::page_size - (start_address_in % eeprom::page_size);
      if (! apply(start_address,data, bytes_to_write)){
         panic("eewr first page failed\n");
         return false;
      }
      data += bytes_to_write; // advance
      while (data < data_end){
         ++cur_page;
         start_address += bytes_to_write; // dest eeprom address
         if ( cur_page == end_page){
            bytes_to_write = data_end - data;
         }else{
            bytes_to_write = eeprom::page_size;
         }
         if (! apply(start_address,data, bytes_to_write)){
           panic("eewr failed2\n");
           return false;
         }
         data += bytes_to_write; //advance
      }
   }
   return true;
}

// Start condition generated event . (EV5)
// Clear by reading sr1 and
// then writing dr with address of i2c device
void i2c_eeprom_writer::on_start_sent()
{
   i2c::get_sr1();
   i2c::send_data(i2c_bus_address);
   i2c::set_event_handler(on_device_address_sent);
}

// device address sent event. EV6
// Clear by reading SR1 followed by reading SR2.
// then send first byte of data address
// want txe and point to next handler
void i2c_eeprom_writer::on_device_address_sent()
{
   i2c::get_sr1();
   i2c::get_sr2();
   i2c::send_data(m_data_address[0]);
   i2c::set_event_handler(on_data_address_hi_sent);
}

// txe on first data address
// send second byte of data address
// update to next handler
void i2c_eeprom_writer::on_data_address_hi_sent()
{
    i2c::send_data(m_data_address[1]);
    i2c::set_event_handler(on_data_address_lo_sent);
}

// txe on 2nd data address
// disble i2c events
// do final dma setup
// and point dma handler to end of dma handler
// start sending the data using dma
void i2c_eeprom_writer::on_data_address_lo_sent()
{
    i2c::enable_event_interrupts(false);
    i2c::enable_dma_tx_stream(true);
}

// dma handler called when last byte of dma data sent
// disable dma and enable i2c event irq's to get btf
// update the event handler
void i2c_eeprom_writer::on_dma_transfer_complete()
{
   i2c::enable_dma_tx_stream(false);
   i2c::enable_dma_bit(false);
   i2c::clear_dma_tx_stream_tcif();
   i2c::enable_event_interrupts(true);
   i2c::set_event_handler(on_last_byte_transfer_complete);
}

// btf at end of last byte transfer
// request stop condition and clean up
void i2c_eeprom_writer::on_last_byte_transfer_complete()
{
   i2c::enable_event_interrupts(false);
   i2c::request_stop_condition();
   i2c::set_default_handlers();
   set_new_write_end_time();
   i2c::release_bus();
}

void i2c_eeprom_writer::on_error()
{
   panic ("i2c eeprom error");
}

uint8_t          i2c_eeprom_writer::m_data_address[] = {0U,0U};

