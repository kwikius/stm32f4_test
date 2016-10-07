
#include "../system/led.hpp"
#include "eeprom_reader.hpp"
#include "../system/i2c.hpp"
#include "../system/serial_port.hpp"

using quan::stm32::millis;
namespace {
constexpr quan::time_<uint32_t>::ms max_time{500U};
}

// return true if apply ok
// else leave in good state and return false
// (call i2c::is_busy() first)
bool i2c_eeprom_reader::apply(uint32_t data_address, uint8_t* data, uint32_t len)
{
   if ( len == 1){
     serial_port::write("read single byte\n");
   }else{
     serial_port::write("read multi byte\n");
   }
   auto now = millis();
   while ( !i2c::bus_released() || eeprom::write_in_progress()) {
      if( (millis() - now) > max_time) {
         serial_port::write("eerd apply timeout 1:");
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
         serial_port::write("eerd apply timeout fail get bus\n");
         return false;
      }
   }

   m_data_ptr = data;
   m_data_length = len;

   m_data_address[0] = static_cast<uint8_t>((data_address & 0xFF00) >> 8U);
   m_data_address[1] = static_cast<uint8_t>(data_address & 0xFF);

   i2c::set_error_handler(on_error);
   i2c::set_event_handler(on_start_sent);
   // clear flags ?
   i2c::enable_error_interrupts(true);
   i2c::enable_event_interrupts(true);
   i2c::enable_buffer_interrupts(false);
   i2c::enable_ack_bit(true);

   if (len > 1 ){
      i2c::enable_dma_bit(true);
      i2c::enable_dma_last_bit(true);
      i2c::set_dma_rx_buffer(data,len);
      i2c::clear_dma_rx_stream_flags();
      i2c::set_dma_rx_handler(on_dma_transfer_complete);
      i2c::enable_dma_rx_stream(true);
   }

   i2c::request_start_condition();

   return true;

}

bool i2c_eeprom_reader::read(uint32_t start_address_in,uint8_t  * data_out,uint32_t len)
{
   if ( len == 0){
      panic("eeprom write zero length");
      return false;
   }
   if ( data_out == nullptr){
      panic("eeprom read data ptr is null");
      return false;
   }
   // one past the last address to write
   uint32_t const end_address = start_address_in + len;

   if ( end_address >= eeprom::data_size){
      panic("eeprom read address out of range");
      return false;
   }
   uint32_t const start_page = start_address_in / eeprom::page_size;
   uint32_t const end_page =  (end_address -1) / eeprom::page_size;

   if ( start_page == end_page){
      return apply(start_address_in,data_out, len);
   }else{
      uint32_t start_address = start_address_in;
      uint8_t * data = data_out;
      uint8_t * const data_end = data_out + len;
      uint32_t cur_page = start_page;
      uint32_t bytes_to_read  = eeprom::page_size - (start_address_in % eeprom::page_size);
      if (!apply(start_address,data, bytes_to_read)){
         return false;
      }
      data += bytes_to_read; // advance
      while (data < data_end){
         ++cur_page;
         start_address += bytes_to_read; // dest eeprom address
         if ( cur_page == end_page){
            bytes_to_read = data_end - data;
         }else{
            bytes_to_read = eeprom::page_size;
         }
         if (!apply(start_address,data,bytes_to_read)){
            return false;
         }
         data += bytes_to_read; //advance
      }
   }
   return true;
}

// Start condition generated event . (EV5)
// Clear by reading sr1 and
// then writing dr with address of i2c device
void i2c_eeprom_reader::on_start_sent()
{  // flags sr1.sb
   i2c::get_sr1();
   i2c::send_data(i2c_bus_address);
   i2c::set_event_handler(on_device_address_sent);
}

// device address sent event. EV6
// Clear by reading SR1 followed by reading SR2.
// then send first byte of data address
void i2c_eeprom_reader::on_device_address_sent()
{  // flags sr2.[tra:busy:msl] sr1.[addr:txe]
   i2c::get_sr1();
   i2c::get_sr2();

   i2c::send_data(m_data_address[0]);
   i2c::set_event_handler(on_data_address_hi_sent);
}

void i2c_eeprom_reader::on_data_address_hi_sent()
{
    i2c::send_data(m_data_address[1]);
    i2c::request_start_condition();
    i2c::set_event_handler(on_data_address_lo_sent);
}

// btf on 2nd data address
// restart
void i2c_eeprom_reader::on_data_address_lo_sent()
{
   i2c::receive_data(); //clear the txe and btf flags
   i2c::set_event_handler(on_start2_sent);
}

void i2c_eeprom_reader::on_start2_sent()
{
   // flags sr1.sb
   i2c::get_sr1();
   i2c::send_data(i2c_bus_address | 1); // send eeprom read address
   i2c::set_event_handler(on_device_read_address_sent);
}

// addr sent
void i2c_eeprom_reader::on_device_read_address_sent()
{
    // flags sr2.[busy:msl] sr1.addr
    i2c::get_sr1();
    i2c::get_sr2();
    if ( m_data_length > 1){ // into dma
      i2c::enable_event_interrupts(false);
      i2c::set_event_handler(i2c::default_event_handler);
    }else{ // dma doesnt work for single byte read
       i2c::enable_ack_bit(false);
       i2c::request_stop_condition();
       i2c::enable_buffer_interrupts(true); // enable rxne
       i2c::set_event_handler(single_byte_receive_handler);
    }
}
void i2c_eeprom_reader::single_byte_receive_handler()
{
       *m_data_ptr = i2c::receive_data();
        i2c::enable_buffer_interrupts(false);
        i2c::enable_event_interrupts(false);
        i2c::set_default_handlers();
        i2c::release_bus();
}

// when last byte of dma data received
// cleanup
void i2c_eeprom_reader::on_dma_transfer_complete()
{
   //led::on();
   i2c::enable_dma_rx_stream(false);
   i2c::enable_dma_bit(false);
   i2c::enable_dma_last_bit(false);
   i2c::clear_dma_rx_stream_tcif();
   i2c::request_stop_condition();
   i2c::set_default_handlers();
   i2c::release_bus();
}

void i2c_eeprom_reader::on_error()
{
   panic ("i2c error");
}

uint8_t* i2c_eeprom_reader::m_data_ptr = nullptr;
uint32_t i2c_eeprom_reader::m_data_length = 0;
uint8_t  i2c_eeprom_reader::m_data_address[] = {0U,0U};

