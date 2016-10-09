
#include "../system/serial_port.hpp"
#include "../system/led.hpp"
#include <quan/stm32/millis.hpp>
#include "bmi_160.hpp"

using quan::stm32::millis;

bool bmi_160_test()
{
   serial_port::write("bmi_160 whoami test\n");
   bmi_160::init();

   bmi_160::reg_write(0x7E,0xB6);

   auto now = millis();
   for (;;){
      if ( (millis() - now) > quan::time_<uint32_t>::ms{200U}){ break;}
   }
   bmi_160::reg_read(0x7F);
   serial_port::write("bmi_160 whoami inited\n");
   
   uint8_t value = bmi_160::reg_read(0);
   bool result = false;
   if ( value == 0xD1){
     serial_port::write("bmi_160 whoami test succeeded\n");
     result = true;
   }else{
     serial_port::write("bmi_160 whoami test failed\n");
   }
   serial_port::printf<30>("got %u\n",static_cast<unsigned int>(value));
   return result;
}

