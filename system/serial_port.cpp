
#include "serial_port.hpp"
#include <quan/stm32/usart/irq_handler.hpp>

#if 1
extern "C" void USART1_IRQHandler() __attribute__ ((interrupt ("IRQ")));
extern "C" void USART1_IRQHandler()
{
   static_assert(
      std::is_same<
         serial_port::usart_type,quan::stm32::usart1
      >::value
   ,"invalid usart for serial_port irq");

   quan::stm32::usart::irq_handler<serial_port>();
}
#else
extern "C" void UART4_IRQHandler() __attribute__ ((interrupt ("IRQ")));
extern "C" void UART4_IRQHandler()
{
   static_assert(
      std::is_same<
         serial_port::usart_type,quan::stm32::uart4
      >::value
   ,"invalid usart for serial_port irq");

   quan::stm32::usart::irq_handler<serial_port>();
}

#endif

void panic(const char* str)
{
   serial_port::write("PANIC : ");
   serial_port::write(str);
   serial_port::put('\n');
}



