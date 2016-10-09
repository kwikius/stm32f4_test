#include "bmi_160.hpp"

#include <quan/stm32/tim.hpp>
#include <quan/stm32/tim/temp_reg.hpp>
#include <quan/stm32/get_raw_timer_frequency.hpp>
#include <quan/stm32/rcc.hpp>
#include <quan/three_d/vect.hpp>
#include <quan/stm32/millis.hpp>
#include <quan/stm32/rcc.hpp>
#include <quan/stm32/f4/exti/set_exti.hpp>
#include <quan/stm32/f4/syscfg/module_enable_disable.hpp>
#include <quan/stm32/push_pop_fp.hpp>

#include "../system/serial_port.hpp"

void bmi_160::init()
{
   setup_rcc();
   setup_spi_pins();
   setup_spi_regs();
   start_spi();
}

void bmi_160::setup_rcc()
{
   serial_port::write("bmi_160 rcc\n");
   quan::stm32::rcc::get()->apb2enr.bb_setbit<12>(); 
   quan::stm32::rcc::get()->apb2rstr.bb_setbit<12>();
   quan::stm32::rcc::get()->apb2rstr.bb_clearbit<12>();
   serial_port::write("bmi_160 rcc done \n");
}

void bmi_160::setup_spi_pins()
{
   serial_port::write("bmi_160 pin setup\n");
   //spi1
   quan::stm32::module_enable<quan::stm32::gpioa>();
   quan::stm32::module_enable<quan::stm32::gpiob>();
   quan::stm32::module_enable<quan::stm32::gpioc>();

   quan::stm32::apply<
      bmi_160::spi1_mosi
      ,quan::stm32::gpio::mode::af5  
      ,quan::stm32::gpio::pupd::none
      ,quan::stm32::gpio::ospeed::medium_fast
   >();

   quan::stm32::apply<
      spi1_miso
      ,quan::stm32::gpio::mode::af5  
      ,quan::stm32::gpio::pupd::none
   >();

   quan::stm32::apply<
      spi1_sck
      ,quan::stm32::gpio::mode::af5  
      ,quan::stm32::gpio::pupd::none
      ,quan::stm32::gpio::ospeed::medium_fast
   >();


   quan::stm32::apply<
      spi1_ncs
      ,quan::stm32::gpio::mode::output
      ,quan::stm32::gpio::pupd::none
      ,quan::stm32::gpio::ospeed::medium_fast
      ,quan::stm32::gpio::ostate::high
   >();
   serial_port::write("bmi_160 pin setup done\n");

   serial_port::write("bmi_160 pin toggle\n");
   toggle_ncs();   
   serial_port::write("bmi_160 pin toggle done\n");
}

void bmi_160::toggle_ncs()
{
   // toggle ncs low --> high to make put bmi_160 in SPI mode
   for ( uint32_t i = 0; i < 1000; ++i){
      asm volatile("nop":::);
   }
   quan::stm32::clear<spi1_ncs>();
   for ( uint32_t i = 0; i < 1000; ++i){
      asm volatile("nop":::);
   }
   quan::stm32::set<spi1_ncs>();
}

void bmi_160::setup_spi_regs()
{
    serial_port::write("bmi_160 pin setup spi regs\n");
   // max mbi_160 clock is 10 MHz
   // max avail stm32 clock in spec = 5.25 MHz --> spi_brr = (4 << 3)  , could try 10.5 MHz  --> (3 << 3)
   static constexpr uint16_t spi_brr = (4 << 3);

   bmi_160::spi1::get()->cr1 = 
      ( 1 << 0)      // (CPHA)
      | ( 1 << 1)      // (CPOL)
      | ( 1 << 2)      // (MSTR)
      |  spi_brr  // (BRR)
      |  (1 << 8)      // (SSI)
      |  (1 << 9)      // (SSM)
   ;
   serial_port::write("bmi_160 pin setup spi regs done\n");
}


