#ifndef INTERRUPTS_PRIO_HPP_INCLUDED
#define INTERRUPTS_PRIO_HPP_INCLUDED

#include <cstdint>

struct interrupt_priority{
   static constexpr uint32_t systick_timer = 15;
   static constexpr uint32_t serial_port = 14;
   static constexpr uint32_t i2c_port = 13;
};

#endif // INTERRUPTS_PRIO_HPP_INCLUDED
