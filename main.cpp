#include "system/serial_port.hpp"
#include "system/i2c.hpp"
#include <stm32f4xx.h>

bool eeprom_test();
bool compass_test();
bool compass_test1();
bool compass_test2();

bool lis3_mdl_test();

int main()
{
   serial_port::write ("-------------------------------------------------\n");
  // compass_test();
   lis3_mdl_test();
   serial_port::write ("-------------------------------------------------\n");
   eeprom_test();
//   compass_test1();
//   serial_port::write ("-------------------------------------------------\n");
//   compass_test2();
   serial_port::write ("-----------------Test succeeded------------------\n");
   
   while (1) { asm volatile("nop":::); }

   return 0;
}

