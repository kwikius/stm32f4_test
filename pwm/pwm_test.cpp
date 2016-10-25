
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

   constexpr uint16_t max_pulsewidth = 2000U;
   constexpr uint16_t min_pulsewidth = 1000U;

   typedef quan::stm32::tim1 rc_out_5_6_timer;

   typedef quan::stm32::tim4 rc_out_timer;

   // use PWM on all channels
   typedef quan::mcu::pin<quan::stm32::gpiob,6> rc_out_ch1;
   typedef quan::mcu::pin<quan::stm32::gpiob,7> rc_out_ch2;
   typedef quan::mcu::pin<quan::stm32::gpiob,8> rc_out_ch3;
   typedef quan::mcu::pin<quan::stm32::gpiob,9> rc_out_ch4;

   typedef quan::mcu::pin<quan::stm32::gpiob,0> rc_out_ch5;
   typedef quan::mcu::pin<quan::stm32::gpiob,1> rc_out_ch6;


   void setup_rc_out_1_to_4_pin()
   {
      quan::stm32::apply<
         rc_out_ch1,
         quan::stm32::gpio::mode::af2  
        ,quan::stm32::gpio::pupd::none
        ,quan::stm32::gpio::ospeed::slow
      >();

      quan::stm32::apply<
         rc_out_ch2,
         quan::stm32::gpio::mode::af2  
        ,quan::stm32::gpio::pupd::none
        ,quan::stm32::gpio::ospeed::slow
      >();

      quan::stm32::apply<
         rc_out_ch3,
         quan::stm32::gpio::mode::af2  
        ,quan::stm32::gpio::pupd::none
        ,quan::stm32::gpio::ospeed::slow
      >();

      quan::stm32::apply<
         rc_out_ch4,
         quan::stm32::gpio::mode::af2  
        ,quan::stm32::gpio::pupd::none
        ,quan::stm32::gpio::ospeed::slow
      >();

   }

   void setup_rc_out_1_to_4()
   {
      setup_rc_out_1_to_4_pin();

  
      constexpr uint32_t timer_freq = quan::stm32::get_raw_timer_frequency<rc_out_timer>();

      quan::stm32::module_enable<rc_out_timer>();

      // set all channels to pwm mode
      // period = 20 ms == 20,000 us
      // set 1 usec tick
      {
         // set all channels to PWM mode 1
         quan::stm32::tim::ccmr1_t ccmr1 = 0;
         ccmr1.cc1s = 0b00;  // output
         ccmr1.oc1m = 0b110; // PWM mode 1
         ccmr1.cc2s = 0b00;  // output
         ccmr1.oc2m = 0b110; // PWM mode 1
         rc_out_timer::get()->ccmr1.set(ccmr1.value);
      }
      {
         quan::stm32::tim::ccmr2_t ccmr2 = 0;
         ccmr2.cc3s = 0b00;  // output
         ccmr2.oc3m = 0b110; // PWM mode 1
         ccmr2.cc4s = 0b00;  // output
         ccmr2.oc4m = 0b110; // PWM mode 1
         rc_out_timer::get()->ccmr2.set(ccmr2.value);
      }
      {
         // default disabled
         quan::stm32::tim::ccer_t ccer = 0;
//         ccer.cc1p =  false;
//         ccer.cc1np = false;
          ccer.cc1e =  true; // enable ch1 ?
//         ccer.cc2p =  false;
//         ccer.cc2np = false;
           ccer.cc2e =  true; // enable ch2
//         ccer.cc3p =  false;
//         ccer.cc3np = false;
           ccer.cc3e =  true; // enable ch3
//         ccer.cc4p =  false;
//         ccer.cc4np = false;
           ccer.cc4e =  true; // enable ch4
         rc_out_timer::get()->ccer.set(ccer.value);
      }
      // set all ccr regs to center except throttle set low
      rc_out_timer::get()->ccr1 = 1500;
      rc_out_timer::get()->ccr2 = 1500;
      rc_out_timer::get()->ccr3 = 1000;
      rc_out_timer::get()->ccr4 = 1500;
      // set the ocpe (preload) bits in ccmr1 , ccmr2 
      rc_out_timer::get()->ccmr1 |= ((1 << 3) | (1 << 11));
      rc_out_timer::get()->ccmr2 |= ((1 << 3) | (1 << 11));

      rc_out_timer::get()->psc = (timer_freq / 1000000 )-1;
      rc_out_timer::get()->arr = 20000 -1;
      rc_out_timer::get()->cnt = 0x0;
      {
         quan::stm32::tim::cr1_t cr1 = 0;
         cr1.arpe = true ;// auto preload
         rc_out_timer::get()->cr1.set(cr1.value);
      }
      rc_out_timer::get()->sr = 0;
   }

   void setup_rc_out_5_6_pin()
   {
      // set up as mode TIM1_CH2N
      quan::stm32::apply<
         rc_out_ch5
         ,quan::stm32::gpio::mode::af1
         ,quan::stm32::gpio::pupd::none
         ,quan::stm32::gpio::ospeed::slow
      >();

      // set up as mode TIM1_CH3N
      quan::stm32::apply<
         rc_out_ch6
         ,quan::stm32::gpio::mode::af1
         ,quan::stm32::gpio::pupd::none
         ,quan::stm32::gpio::ospeed::slow
      >();
   }

   
   void pwm_setup_rc_out_5_6()
   {
      setup_rc_out_5_6_pin();

      quan::stm32::module_enable<rc_out_5_6_timer>();
      constexpr uint32_t timer_5_6_freq = quan::stm32::get_raw_timer_frequency<rc_out_5_6_timer>();
      // set the granularity to 1 us, period to 50 Hz
      rc_out_5_6_timer::get()->psc = (timer_5_6_freq / 1000000 )-1;
      rc_out_5_6_timer::get()->rcr = 0;
      rc_out_5_6_timer::get()->arr = 20000 -1;  // 50 Hz
      rc_out_5_6_timer::get()->cnt = 0x0;
    
      {
         quan::stm32::tim::cr1_t cr1 = 0;  
         cr1.urs = true;   // Only counter overflow/underflow generates an update interrupt or DMA request if enabled
         cr1.arpe = true ; // recommended for these TIM1/8 with PWM 
         rc_out_5_6_timer::get()->cr1.set(cr1.value);
      }

      {
         quan::stm32::tim::cr2_t cr2 = 0; 
         cr2.ois2n = false;  // output idle state state when moe == 0
         cr2.ois3n = false;  // output idle state state when moe == 0
         cr2.ccpc = false;  
         cr2.ccds = false;
         rc_out_5_6_timer::get()->cr2.set(cr2.value);
      }

       {
        // set pwm mode on ch2 pwm mode in OCxM in TIM1_CCMRx
         quan::stm32::tim::ccmr1_t ccmr1 = 0;
         ccmr1.cc2s = 0b00; // channel 2 output
         ccmr1.oc2m = 0b110;  // PWM mode 1
         ccmr1.oc2pe = true;  // recommended setting
         rc_out_5_6_timer::get()->ccmr1.set(ccmr1.value);
      }

      {
        // set pwm mode on ch3 pwm mode in OCxM in TIM1_CCMRx
         quan::stm32::tim::ccmr2_t ccmr2 = 0;
         ccmr2.cc3s = 0b00; // channel 3 output
         ccmr2.oc3m = 0b110;  // PWM mode 1
         ccmr2.oc3pe = true;  // recommended setting
         rc_out_5_6_timer::get()->ccmr2.set(ccmr2.value);
      }
      rc_out_5_6_timer::get()->ccr2 = 1500;
      rc_out_5_6_timer::get()->ccr3 = 1500;

      {
        // set up TIM1_CH2N,TIM1_CH3N polarity high pulse
         quan::stm32::tim::ccer_t ccer = 0;
         ccer.cc2np = false; // TIM1_CH2N is positive pulse
         ccer.cc2e = false ; // enable TIM1_CH2N
         ccer.cc2ne = true  ; // enable TIM1_CH2N
         ccer.cc3np = false; // TIM1_CH3N is positive pulse
         ccer.cc3e = false ; // enable TIM1_CH3N
         ccer.cc3ne = true  ; // enable TIM1_CH3N
         rc_out_5_6_timer::get()->ccer.set(ccer.value);
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
         rc_out_5_6_timer::get()->bdtr.set(bdtr.value);
      }
      rc_out_5_6_timer::get()->sr = 0;
   }

    void pwm_setup()
   {
      quan::stm32::module_enable<quan::stm32::gpiob>();
      setup_rc_out_1_to_4();
      pwm_setup_rc_out_5_6();
   }

   void pwm_start()
   {
      rc_out_timer::get()->cr1.bb_setbit<0>(); // (CEN)
      rc_out_5_6_timer::get()->cr1.bb_setbit<0>(); // (CEN)
   }

// channel 0 to 5
   void write_rc_out(uint8_t ch, uint16_t value)
   {
      if ( ch < 4){
         volatile uint32_t * ccrs = &rc_out_timer::get()->ccr1;
         ccrs[ch] = quan::constrain(value,min_pulsewidth, max_pulsewidth);
      }else{
         if ( ch < 6){
            volatile uint32_t * ccrs = &rc_out_5_6_timer::get()->ccr2;
            ccrs[ch - 4] = quan::constrain(value,min_pulsewidth, max_pulsewidth);
         }
      }
   }

   struct pwm_params_t{
      bool dir;
      quan::time::ms last_update;
      uint16_t value;
      quan::time::ms const period;
      uint16_t const incr;
   };

  pwm_params_t pwm_params[6] ={
      {true, quan::time::ms{0}, 1500, quan::time::ms{100},6}
      ,{true, quan::time::ms{0},1500,  quan::time::ms{133},10}
      ,{true, quan::time::ms{0},1500,  quan::time::ms{265},20}
      ,{true, quan::time::ms{0}, 1500, quan::time::ms{436},50}
      ,{true, quan::time::ms{0},1500,  quan::time::ms{900},10}
      ,{true, quan::time::ms{0},1500,  quan::time::ms{134},20}
  };

}

bool pwm_test()
{
   serial_port::write("starting pwm test\n");

   pwm_setup();

   pwm_start();

   for(;;){
      for (uint8_t i=0; i < 6; ++i){
         pwm_params_t& pwm = pwm_params[i];
         if ( (millis() - pwm.last_update) > pwm.period ){
            pwm.last_update = millis();
            if (pwm.dir){
               if (pwm.value < max_pulsewidth){
                  write_rc_out(i,pwm.value);
                  pwm.value += pwm.incr;
               }else{
                  pwm.value = max_pulsewidth;
                  write_rc_out(i,pwm.value);
                  pwm.dir = false;
               }
            }else{ // false dir
                if (pwm.value > min_pulsewidth){
                  write_rc_out(i,pwm.value);
                  pwm.value -= pwm.incr;
               }else{
                  pwm.value = min_pulsewidth;
                  write_rc_out(i,pwm.value);
                  pwm.dir = true;
               }
            }
         }
      }
   }
   return true;

}