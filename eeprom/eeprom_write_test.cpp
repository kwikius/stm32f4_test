
#include <quan/stm32/millis.hpp>
#include <quan/conversion/itoa.hpp>
#include "../system/serial_port.hpp"
#include "../system/eeprom_writer.hpp"
#include "../system/i2c.hpp"

/*
   test function
*/
char data_out[]= "LoLiHey";// the data to write n.b in dma available memory

bool eeprom_write_test()
{
    i2c_eeprom_writer::write(5U,(uint8_t const*)data_out,8);

    // at this point still busy

    auto now = quan::stm32::millis();
    typedef decltype(now) ms;
    while(i2c::is_busy() && ((quan::stm32::millis() - now) < ms{500U})){;}

    if (i2c::is_busy()){
         panic("looks like irq write hung");

         uint32_t const dma_flags = DMA1->HISR & 0x3F;

         char buffer [20];
         quan::itoasc(dma_flags,buffer,16);
         serial_port::write("dma flags = 0x");
         serial_port::write(buffer);
         serial_port::write("\n");

         uint32_t ndata = DMA1_Stream4->NDTR;
         quan::itoasc(ndata,buffer,10);
         serial_port::write("ndtr = ");
         serial_port::write(buffer);
         serial_port::write("\n");

         return false;
    }
    serial_port::write("write completed\n");
    return true;
}
