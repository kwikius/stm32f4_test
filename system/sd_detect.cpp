

#include "../system/serial_port.hpp"
#include <quan/stm32/rcc.hpp>
#include <quan/stm32/millis.hpp>

using  quan::stm32::millis;

void setup_card_detect()
{

  // do some diagnostics
   // if the 

   uint32_t const bdcr = quan::stm32::rcc::get()->bdcr.get();
   if ( bdcr & (1<<0)){
      serial_port::write("LSE is on. Needs to be off for enabling SD\n");
      
   }else{
    serial_port::write("LSE is off\n");
   }

   if ( bdcr & (1<<2)){
      serial_port::write("LSE ByPass is on\n");
   }else{
      serial_port::write("LSE ByPass is off\n");
   }
   
   typedef quan::mcu::pin<quan::stm32::gpioc,15> sdio_card_detect;

   quan::stm32::module_enable<sdio_card_detect::port_type>();
   quan::stm32::apply<
      sdio_card_detect
      ,quan::stm32::gpio::mode::input 
      ,quan::stm32::gpio::pupd::none         
      ,quan::stm32::gpio::ospeed::slow
   >();

   auto now = millis();
   for (;;){
      if ( (millis() - now) > quan::time::ms{1000}){
         now = millis();
         serial_port::write("sd detect is ");
         serial_port::write( (quan::stm32::get<sdio_card_detect>() == true?"high":"low"));
         serial_port::write("\n");
      }
   }
}