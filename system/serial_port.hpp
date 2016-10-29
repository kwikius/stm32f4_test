#ifndef SERIAL_PORT_HPP_INCLUDED
#define SERIAL_PORT_HPP_INCLUDED

#include <quan/stm32/serial_port.hpp>
#include <quan/stm32/gpio.hpp>


#if 1
struct sp_info{
   typedef quan::stm32::usart1 usart;
   typedef quan::mcu::pin<quan::stm32::gpioa,9>    txo_pin;
   typedef quan::mcu::pin<quan::stm32::gpioa,10>   rxi_pin;
};
#else
struct sp_info{
   typedef quan::stm32::uart4 usart;
   typedef quan::mcu::pin<quan::stm32::gpioa,0>    txo_pin;
   typedef quan::mcu::pin<quan::stm32::gpioa,1>   rxi_pin;
};

#endif
// large tx buffer is for eeprom write! Fix me
typedef quan::stm32::serial_port<
   sp_info::usart,3000,500,sp_info::txo_pin,sp_info::rxi_pin
> serial_port;

void panic(const char* str);


#endif // SERIAL_PORT_HPP_INCLUDED
