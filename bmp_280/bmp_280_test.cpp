
#include <quan/stm32/millis.hpp>
#include "../system/i2c.hpp"
#include "../system/serial_port.hpp"
#include "../system/led.hpp"
#include "bmp_280.hpp"

using quan::stm32::millis;

namespace {
   constexpr quan::time_<uint32_t>::ms max_time{500U};
}

bool bmp_280_test()
{
   serial_port::write("bmp 280 whoami test \n");
   auto  start_time = millis();
   uint8_t whoami = 0;

   unsigned count = 0;

   for ( uint8_t i = 0; i < 5; ++i){
      if ( bmp280::read(bmp280::reg::whoami, &whoami, 1U)){
         // Though read function returned, it is non blocking
         // result is only available once bus has been released
         auto read_start_time = millis();
         while (!i2c::bus_released()){
            if((millis() - read_start_time) > max_time){
               serial_port::write("whoami test i2c function failed: bus not released\n");
               // want to clean up and reset 
               // disable interrupts
               // set handlers
               // try to reset i2c bus
               if(i2c::has_errored()){
                  i2c::init();
               }
               return false;
            }
         }
         
         if ( whoami == bmp280::val::whoami){
            serial_port::printf<100>("whoami test succesful: got %u\n",static_cast<unsigned>(whoami));
            ++count;
         }else{
            serial_port::printf<100>("whoami test failed: got %u\n",static_cast<unsigned>(whoami));
         }
      }else{
        
         serial_port::write("whoami baro test i2c read function failed: read failed\n");
         if(i2c::has_errored()){
            i2c::init();
         }
         return false;
      }
   }
   auto time_taken = (millis() - start_time).numeric_value();
   serial_port::printf<100>("count = %u, time taken = %u ms\n",count,static_cast<uint32_t>(time_taken));
   return (count > 0);
}
