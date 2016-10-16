#include "system/serial_port.hpp"
#include "system/i2c.hpp"
#include "system/led.hpp"
#include <stm32f4xx.h>

bool eeprom_test();
bool compass_test();
bool compass_test1();
bool compass_test2();

bool lis3_mdl_test();
bool bmp_280_test();
bool bmi_160_test();

bool lis3_mdl_run();

int main()
{
   led::off();
   serial_port::write ("-------------------------------------------------\n");

   //  compass_test2();
  // compass_test();
//   lis3_mdl_test();
//
//   serial_port::write ("-------------------------------------------------\n");
//
//   bmp_280_test();
//
//   serial_port::write ("-------------------------------------------------\n");
//
//   bmi_160_test();
//
//   serial_port::write ("-------------------------------------------------\n");
//
//   eeprom_test();
//
//   serial_port::write ("-------------------------------------------------\n");
//
    lis3_mdl_run();
//   compass_test1();
//   serial_port::write ("-------------------------------------------------\n");
//   compass_test2();
   serial_port::write ("-----------------Tests completed------------------\n");
   
   while (1) { asm volatile("nop":::); }

   return 0;
}

