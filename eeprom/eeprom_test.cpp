
#include <stdarg.h>
#include <malloc.h>
#include <cstring>
#include <cstdio>
#include <quan/stm32/millis.hpp>
#include "../system/serial_port.hpp"
#include "../system/led.hpp"
#include "../system/i2c.hpp"
#include "eeprom_reader.hpp"
#include "eeprom_writer.hpp"

char const data[] =

#if 0
 "# A Simple mixer\n"
"# Provides separate aileron left and right flapperons\n"
"# and v tail mixing\n"
"\n"
"# On this aircraft servos are placed symmetrically\n"
"# so aileron servos pulse in same direction provides roll\n"
"# opposite pulses for flap\n"
"\n"
"# similarly for v tail same pulse dir to each servo for yaw\n"
"# different for elevator\n"
"\n"
"# On this plane a bit of rudder is mixed in with the ailerons\n"
"\n"
"#assign servo channels\n"
"aileron_left = 1;\n"
"v_tail_left  = 2;\n"
"throttle = 3;\n"
"aileron_right = 4;\n"
"v_tail_right = 5;\n"
"\n"
"#gains\n"
"elev_gain = 0.3;\n"
"ail_roll_gain = 0.7;\n"
"rudder_roll_gain = 0.3;\n"
"yaw_gain  = 0.5;\n"
"throttle_gain = 10.0;\n"
"flap_gain = 1.0;\n"
"\n"
"mixer()\n"
"{\n"
"   # runtime constants\n"
"   roll = input(Roll) ;\n"
"   pitch = input(Pitch);\n"
"   yaw = input(Yaw);\n"
"   flap = input(Flap);\n"
"\n"
"   # assign outputs\n"
"   output[throttle]      = input(Throttle) * throttle_gain;\n"
"   output[aileron_left]  = roll * ail_roll_gain + flap * flap_gain;\n"
"   output[aileron_right] = roll * ail_roll_gain - flap * flap_gain;\n"
"   output[v_tail_left]   = yaw * yaw_gain - pitch * elev_gain + roll * rudder_roll_gain;\n"
"   output[v_tail_right]  = yaw * yaw_gain + pitch * elev_gain + roll * rudder_roll_gain;\n"
"}\n"
"\n";

#else

"Simple Mixer \n"
"# anything global must be a constant ...\n"
"# type is deduced from the initialiser expression\n"
"# There are 3 types integer, float bool\n"
"# floats are differentiated by including a decimal point and fractional part\n"
"# bools are either true or false\n"
"# There is NO conversion between the different types\n"
"#  x = 1 ; // x is an integer\n"
"#  y = 1.0 ; // y is a float\n"
"#  z1 = x + x ; // OK z1 is an integer\n"
"#  z2 = x + y ; // Error : x and y are different types\n"
"#  b = true;   // b is a bool\n"
"#  c = b && x ;   // Error :  b and x are different types\n"
"#  d = b && !b ; // ok ( c is false)\n"
"#  w = x != 0  ; // w is bool \n"
"\n"
"pitch_gain = 500.0; # pitch gain surprisingly ( Just here to test comments)\n"
"roll_gain = 1000.0 - pitch_gain;\n"
"throttle_gain = 10.0;\n"
"pulse_offset = 1500.0;\n"
"thrt_pls_offset = 1000.0;\n"
"\n"
"# Provide names for some of the output array indices\n"
"elevon_left = 1;\n"
"elevon_right = 2;\n"
"\n"
"# An output in the mixer function is evaluated periodically\n"
"# (Possibly when the inputs connected to it change)\n"
"mixer()\n"
"{\n"
"   # in the mixer function expression can be variables as wel as constants\n"
"\n"
"   F_deflection = (input(Airspeed) * input(Airspeed))/( input(ARSPD_FBWA_MIN) * input(ARSPD_FBWA_MIN)) ;\n"
"   pitch = input(Pitch) * pitch_gain;\n"
"   roll  = input(Roll) * roll_gain;\n"
"\n"
"   # The output expressions\n"
"   output[elevon_left] = ( pitch + roll) / F_deflection + pulse_offset;\n"
"   output[elevon_right] = (pitch - roll) / F_deflection + pulse_offset;\n"
"\n"
"   # just to show literals work also for outputs\n"
"   output[3] = input(Throttle) * throttle_gain + thrt_pls_offset;\n"
"}\n"
"\n";

#endif

using quan::stm32::millis;

// TODO. However for now I solved this by initing the pins before the module
/*
https://my.st.com/public/STe2ecommunities/mcu/Lists/cortex_mx_stm32/Flat.aspx?RootFolder \
=https%3a%2f%2fmy%2est%2ecom%2fpublic%2fSTe2ecommunities%2fmcu%2fLists%2fcortex%5fmx%5fstm32%2f\
Proper%20Initialization%20of%20the%20I2C%20Peripheral&FolderCTID=0x01200200770978C69A1141439FE559EB\
459D7580009C4E14902C3CDE46A77F0FFD06506F5B&currentviews=1824
Over the years I have encountered several I2C peripherals that don't always reset correctly on power up.
When I initialize the I2C controller, or after a timeout error, I disable the I2C on the STM32, put the I2C pins in GPIO open drain mode,
and then toggle the SCK pin (at < 100KHz) until the SDA pin shows the bus is free.  Then I enable the I2C on the STM32 and start a new transaction.
  Jack Peacock
*/
void i2c_recover()
{
   // disable i2c

   // make scl open collector output
   // make sda input
   // while the sda pin is low
   // clock the SCL pin untill the SDA pin goes high

   // then chcnge pins back and reenable the i2c module

}

bool eeprom_test()
{

   // Can happen if devices active on bus
   if ( i2c::is_busy()) {
      serial_port::write("i2c bus busy at startup\n");
      while (1) {asm volatile("nop":::);}
   }

   uint32_t const address = 127;
   uint32_t const len = strlen(data) +1;

   serial_port::printf<30>("length is %d\n",len);
   if ( i2c_eeprom_writer::write(address,(uint8_t const*) data,len)){

      serial_port::write("write succeeded\n");
      char * result = (char*)malloc(len +1);
      if (result){
         if (i2c_eeprom_reader::read(address,(uint8_t*) result,len)){
            serial_port::write("read succeeded\n");
            // after the read function returns
            // we need to wait until the read has completed,
            // signified by the bus being released.
            // obviously only for single threaded!
            auto now = quan::stm32::millis();
            while(!i2c::bus_released() && ((quan::stm32::millis() - now) < quan::time::ms{1000U})){;}

            if (i2c::bus_released()){
               serial_port::write(result,len);
               serial_port::write("\n");
            }else{
               serial_port::write("read failed, bus not released\n");
               return false;
            }
            serial_port::write("done\n");
            free (result); result = nullptr;
         }else{
            serial_port::write("read failed\n");
            return false;
         }
      }else{
         serial_port::write("malloc failed\n");
         return false;
      }
   }
   return true;;
}
