
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

   constexpr uint32_t timer_freq = quan::stm32::get_raw_timer_frequency<rc_out_timer>();

   typedef quan::mcu::pin<quan::stm32::gpiob,0> rc_out5;
   typedef quan::mcu::pin<quan::stm32::gpiob,1> rc_out6;

   void setup_pin()
   {
      quan::stm32::module_enable<rc_out6::port_type>();

      // set up as mode TIM1_CH2N
      quan::stm32::apply<
         rc_out5
         ,quan::stm32::gpio::mode::af1
         ,quan::stm32::gpio::pupd::none
         ,quan::stm32::gpio::ospeed::slow
      >();

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
         cr2.ois2n = false;  // output idle state state when moe == 0
         cr2.ois3n = false;  // output idle state state when moe == 0
         cr2.ccpc = false;  
         cr2.ccds = false;
         rc_out_timer::get()->cr2.set(cr2.value);
      }

       {
        // set pwm mode on ch2 pwm mode in OCxM in TIM1_CCMRx
         quan::stm32::tim::ccmr1_t ccmr1 = 0;
         ccmr1.cc2s = 0b00; // channel 2 output
         ccmr1.oc2m = 0b110;  // PWM mode 1
         ccmr1.oc2pe = true;  // recommended setting
         rc_out_timer::get()->ccmr1.set(ccmr1.value);
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
        // set up TIM1_CH2N,TIM1_CH3N polarity high pulse
         quan::stm32::tim::ccer_t ccer = 0;
         ccer.cc2np = false; // TIM1_CH2N is positive pulse
         ccer.cc2e = false ; // enable TIM1_CH2N
         ccer.cc2ne = true  ; // enable TIM1_CH2N
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

   uint16_t pwm5 = 1500U;
   uint16_t pwm6 = 1500U;

   rc_out_timer::get()->ccr2 = pwm5;
   rc_out_timer::get()->ccr3 = pwm6;

   bool dir6 = true;
   bool dir5 = false;
   quan::time::ms now6 = millis();
   quan::time::ms now5 = millis();

   for (;; ){
      if((millis() - now6) > quan::time::ms{500}){
         now6 = millis();
         if ( dir6 == true){
             pwm6 += 100U;
             if ( pwm6 >= 2000U){
               dir6 = false;
             }
         }else{
             pwm6 -= 100U;
             if ( pwm6 <= 1000U){
               dir6 = true;
             }
         }
         rc_out_timer::get()->ccr3 = pwm6;
      }

      if((millis() - now5) > quan::time::ms{600}){
         now5 = millis();
         if ( dir5 == true){
             pwm5 += 150U;
             if ( pwm5 >= 2000U){
               dir5 = false;
             }
         }else{
             pwm5 -= 150U;
             if ( pwm5 <= 1000U){
               dir5 = true;
             }
         }
         rc_out_timer::get()->ccr2 = pwm5;
      }
   }

   return true;

}