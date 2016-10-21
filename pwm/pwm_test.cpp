
#include <stm32f4xx.h>
#include <quan/stm32/tim.hpp>
#include <quan/stm32/gpio.hpp>
#include <quan/stm32/tim/temp_reg.hpp>
#include <quan/stm32/get_raw_timer_frequency.hpp>
#include <quan/stm32/millis.hpp>
#include "../system/serial_port.hpp"
#include <quan/constrain.hpp>
#include <quan/min.hpp>

using quan::stm32::millis;

namespace {

   typedef quan::stm32::tim1 rc_out_timer;

   typedef quan::mcu::pin<quan::stm32::gpiob,1> rc_out6;

   constexpr uint32_t timer_freq = quan::stm32::get_raw_timer_frequency<rc_out_timer>();

   void setup_pin()
   {
      quan::stm32::module_enable<rc_out6::port_type>();

      // set up as mode TIM1_CH3N
      quan::stm32::apply<
         rc_out6
         ,quan::stm32::gpio::mode::af1
         ,quan::stm32::gpio::pupd::none
         ,quan::stm32::gpio::ospeed::slow
      >();
   }

   bool pwm_setup()
   {
      quan::stm32::module_enable<rc_out_timer>();
      // set the granularity to 1 us, period to 50 Hz
      rc_out_timer::get()->psc = (timer_freq / 1000000 )-1;
      rc_out_timer::get()->rcr = 0;
      rc_out_timer::get()->arr = 20000 -1;  // 50 Hz
      rc_out_timer::get()->cnt = 0x0;
    
      {
         quan::stm32::tim::cr1_t cr1 = 0;  
         cr1.urs = true;   // Only counter overflow/underflow generates an update interrupt or DMA request if enabled
         cr1.arpe = true ; // recommended for these TIM1/8 with PWM 
         rc_out_timer::get()->cr1.set(cr1.value);
      }

      {
         quan::stm32::tim::cr2_t cr2 = 0; 
         cr2.ois3n = false;  // output idle state state when moe == 0
         cr2.ccpc = false;  
         cr2.ccds = false;
         rc_out_timer::get()->cr2.set(cr2.value);
      }

      {
        // set pwm mode on ch3 pwm mode in OCxM in TIM1_CCMRx
         quan::stm32::tim::ccmr2_t ccmr2 = 0;
         ccmr2.cc3s = 0b00; // channel 3 output
         ccmr2.oc3m = 0b110;  // PWM mode 1
         ccmr2.oc3pe = true;  // recommended setting
         rc_out_timer::get()->ccmr2.set(ccmr2.value);
      }

      {
        // set up TIM1_CH3N polarity high pulse
         quan::stm32::tim::ccer_t ccer = 0;
         ccer.cc3np = false; // TIM1_CH3N is positive pulse
         ccer.cc3e = false ; // enable TIM1_CH3N
         ccer.cc3ne = true  ; // enable TIM1_CH3N
         rc_out_timer::get()->ccer.set(ccer.value);
      }
 
      {
         quan::stm32::tim::bdtr_t bdtr = 0;
         bdtr.moe = true;  // main output enable
         bdtr.aoe = true;  // automatic output enable
         bdtr.ossi = true; // offs stet select, enable the outputs in idle
         bdtr.ossr = true; // enable the outputs in run mode
         bdtr.bke = false;   // disable braek inputs
         bdtr.bkp = false;   // break polarity is dont care
         bdtr.dtg = 0;
         rc_out_timer::get()->bdtr.set(bdtr.value);
      }
      
     
      //  set TIMx_CR1.ARPE
// BDTR.MOE, BDTR.OSSI, BDTR.OSSR, OIS1, OIS1N and CC1E bits
      return true;
   }
}

   // output pwm on PB1

bool pwm_test()
{
   serial_port::write("starting pwm test\n");
   setup_pin();

   pwm_setup();

   rc_out_timer::get()->sr = 0;
  

   rc_out_timer::get()->cr1.bb_setbit<0>(); // (CEN)

 
   uint16_t pwm = 1500U;
   rc_out_timer::get()->ccr3 = pwm;
   bool dir = true;
   auto now = millis();
   for (;; ){
      asm volatile ("nop":::);
      if((millis() - now) > quan::time::ms{2000}){
         now = millis();
         if ( dir == true){
             pwm += 100U;
             if ( pwm >= 2000U){
               dir = false;
             }
         }else{
             pwm -= 100U;
             if ( pwm <= 1000U){
               dir = true;
             }
         }
         rc_out_timer::get()->ccr3 = pwm;
      }
   }

   return true;

}