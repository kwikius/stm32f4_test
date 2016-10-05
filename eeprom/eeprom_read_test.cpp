
#include <quan/stm32/millis.hpp>
#include <quan/conversion/itoa.hpp>
#include "../system/serial_port.hpp"
#include "../system/i2c.hpp"
#include "eeprom_reader.hpp"
/*
   test function
*/
constexpr uint16_t numbytes = 8U;
char data_in[numbytes] = {"-------"};  // the data to write n.b in dma available memory

bool eeprom_read_test()
{
   bool result = i2c_eeprom_reader::read(5U,(uint8_t*)data_in,8);

   auto now = quan::stm32::millis();
   typedef decltype(now) ms;

   while(!i2c::bus_released() && ((quan::stm32::millis() - now) < ms{500U})){;}

   if (i2c::bus_released()){
      serial_port::write("read got ");
      serial_port::write(data_in,numbytes);
      serial_port::write("\n");
   // may not be ascii ...
      for ( uint8_t i = 0; i < numbytes; ++i){
         char buf[20];
         quan::itoasc(static_cast<uint32_t>(data_in[i]),buf,10);
         serial_port::write(buf);
         serial_port::write("\n");
      }
      // bus may still be busy waiting for stop etc
      now = quan::stm32::millis();
      while(!i2c::bus_free() && ((quan::stm32::millis() - now) < ms{500U})){;}

      if ( !i2c::bus_free()){
         panic ("looks like i2c didnt get freed");
         return false;
      }
      return result;
   }else{  // bus not released

      panic("looks like irq read hung");

      uint32_t const dma_flags = DMA1->LISR & 0x3D0000;

      char buffer [20];
      quan::itoasc(dma_flags,buffer,16);
      serial_port::write("dma flags = 0x");
      serial_port::write(buffer);
      serial_port::write("\n");

      uint32_t ndata = DMA1_Stream2->NDTR;
      quan::itoasc(ndata,buffer,10);
      serial_port::write("ndtr = ");
      serial_port::write(buffer);
      serial_port::write("\n");
      return false;
   }

}
