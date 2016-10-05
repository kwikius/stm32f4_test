

#include <quan/stm32/millis.hpp>
#include "serial_port.hpp"
#include "interrupt_priority.hpp"
#include "i2c.hpp"
#include "led.hpp"
#include <cstdlib>


namespace {

   void setup_systick()
   {
      SysTick_Config(SystemCoreClock / 1000);
      NVIC_SetPriority(SysTick_IRQn,15);
   }

   void setup_serial_port()
   {
      serial_port::init();
      serial_port::set_baudrate<115200,true>();
      serial_port::set_irq_priority(interrupt_priority::serial_port);
   }

   void startup_delay()
   {
      auto elapsed = quan::stm32::millis();
      while ( (quan::stm32::millis() - elapsed) < quan::time_<uint32_t>::ms{500U}) {;}
   }

}

extern "C" void setup()
{
    setup_systick();
    setup_serial_port();
    led::setup();

     // allow a delay for board capacitors to charge
    startup_delay();
    i2c::init();
}

volatile uint32_t quan::stm32::detail::systick_tick::current = 0U;

extern "C" void Systick_Handler() __attribute__ ((interrupt ("IRQ")));
extern "C" void SysTick_Handler()
{
   ++quan::stm32::detail::systick_tick::current;
}

#if 1
extern "C" void __cxa_pure_virtual()
{
     while (1);
}

void *__dso_handle;

//extern "C" void   vPortFree( void *pv );
//extern "C" void * pvPortMalloc(size_t n);

void operator delete (void* pv)
{ 
   if (pv) {
      ::free(pv);
   } 
}
void* operator new (size_t n_in){ 
   size_t const n = (n_in > 1) ? n_in : 1;
   void * result = ::malloc(n );
   if ( result != nullptr){
      memset(result,0,n);
   }
   return result;
}


// should prrob init to 0 for ardupilot
void * operator new[](size_t n_in)
{
   size_t const n = (n_in > 1) ? n_in : 1;
   void * result = ::malloc(n );
   if ( result != nullptr){
      memset(result,0,n);
   }
   return result;
}

void operator delete[](void * ptr)
{
    if (ptr) ::free(ptr);
}

// from Ardupilot/libraries/AP_Common/c++.cpp
__extension__ typedef int __guard __attribute__((mode (__DI__)));

int __cxa_guard_acquire(__guard *g)
{
    return !*(char *)(g);
};

void __cxa_guard_release (__guard *g)
{
    *(char *)g = 1;
};

void __cxa_guard_abort (__guard *) 
{
};
#endif

