
#include <quan/stm32/millis.hpp>
#include "../system/i2c.hpp"
#include "../system/serial_port.hpp"
#include "compass.hpp"
#include "../system/led.hpp"

using quan::stm32::millis;
namespace {
   bool read_reg ( uint8_t reg, uint8_t * result)
   {
      if ( compass_reader::read(reg, result, 1) ){
         while (!i2c::bus_released()){;}
         return true;
      }else{
         return false;
      }
   }

   constexpr quan::time_<uint32_t>::ms max_time{500U};
}

bool compass_test1()
{
   serial_port::write("compass WHOAMI test with separate set sub address and read transactions\n");

   uint8_t whoami = 0;
   unsigned count = 0;

   for ( uint8_t i = 0; i < 5; ++i){
   if(compass_sub_address_setter::write( compass::reg::whoami)){
       // wait for not busy
      if ( compass_reader1::read(&whoami) ){
      // wait for not busy
         while (!i2c::bus_released()){;}
         if ( whoami == compass::val::whoami){
            serial_port::printf<100>("whoami test successful: got %u\n",static_cast<unsigned>(whoami));
             ++ count;
         }else{
            serial_port::printf<100>("whoami test failed: got %u\n",static_cast<unsigned>(whoami));
         }
      }else{
         serial_port::write("compass read failed\n");
         return false;
      }
   }else{
      
       serial_port::write("compass sub address set failed\n");
       return false;
    }
  }
  serial_port::printf<100>("count = %u\n",count);
  return (count  > 0);
}

// Do read with repeated start
bool compass_test()
{

   serial_port::write("compass WHOAMI test with repeated start\n");
   uint8_t whoami = 0;

   unsigned count = 0;

   for ( uint8_t i = 0; i < 5; ++i){
      if ( compass_reader::read(compass::reg::whoami, &whoami, 1)){
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
             // try to resey i2c bus
             if(i2c::has_errored()){
                  i2c::init();
             }
             return false;
           }
         }
         
         if ( whoami == compass::val::whoami){
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

/*
 startup

*/

bool compass_test2()
{
   serial_port::write("compass write test\n");
   uint8_t result = 0xff;
   if ( read_reg(compass::reg::ctr1reg2,&result)){
      if ( result != 0){
         serial_port::write("compass reg read not expected value\n");
         return false;
      }
      // set up ctrl reg 2
      constexpr uint8_t value = 0b0110000;
      if ( ! compass_writer::write(compass::reg::ctr1reg2,value)){
          serial_port::write("compass reg write failed\n");
          return false;
      }
      read_reg(compass::reg::ctr1reg2,&result);
      if (result != value){
        serial_port::printf<100>("compass reg not same result, got %u\n",static_cast<unsigned int>(result));
        return false;
      }else{
          serial_port::printf<100>("write succeeded, got %u\n",static_cast<unsigned int>(result));
      }
      
   }else{
     serial_port::write("compass read reg failed\n");
   }
  // read controlreg2 , default should be 0
  // startup should be
  // check a bit is not set
  // set a bit and write the control reg
  // read the control reg again and confirm the value is the new value
  return true;
}

