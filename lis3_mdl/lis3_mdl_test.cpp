
#include <quan/stm32/millis.hpp>
#include "../system/i2c.hpp"
#include "../system/serial_port.hpp"
#include "../system/led.hpp"
#include "lis3_mdl.hpp"

using quan::stm32::millis;

namespace {
   constexpr quan::time_<uint32_t>::ms max_time{500U};
}

bool lis3_mdl_test()
{
   serial_port::write("lis3_mdl whoami test \n");
   uint8_t whoami = 0;

   unsigned count = 0;

   for ( uint8_t i = 0; i < 5; ++i){
      if ( lis3_mdl::read(lis3_mdl::reg::whoami, &whoami, 1)){
         // Though read function returned, it is non blocking
         // result is only available once bus released

         // actually need to have a separate compass_bus_released , since, once threade
         // bus may be taken by something else
         // if compass is only accessed in one thread this may not be a problem
         // do timout
         auto start_time = millis();
         
         while (!i2c::bus_released()){
            if((millis() - start_time) > max_time){
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
         
         if ( whoami == lis3_mdl::val::whoami){
            serial_port::printf<100>("whoami test succesful: got %u\n",static_cast<unsigned>(whoami));
            ++count;
         }else{
            serial_port::printf<100>("whoami test failed: got %u\n",static_cast<unsigned>(whoami));
         }
      }else{
        
         serial_port::write("whoami compass test i2c read function failed: read failed\n");
         if(i2c::has_errored()){
            i2c::init();
         }
         return false;
      }
   }
   serial_port::printf<100>("count = %u\n",count);
   return (count > 0);
}
