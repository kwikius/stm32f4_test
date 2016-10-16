#ifndef BMI_160_HPP_INCLUDED
#define BMI_160_HPP_INCLUDED

#include <cstdint>
#include <quan/stm32/spi.hpp>
#include <quan/stm32/gpio.hpp>

struct bmi_160{
  typedef quan::stm32::spi1 spi1;
  typedef quan::mcu::pin<quan::stm32::gpiob,5>   spi1_mosi;
  typedef quan::mcu::pin<quan::stm32::gpiob,4>   spi1_miso;
  typedef quan::mcu::pin<quan::stm32::gpiob,3>   spi1_sck;

  #if defined STM32F4_TEST_AERFLITE_BOARD
      typedef quan::mcu::pin<quan::stm32::gpioa,15>  spi1_ncs;
  #else 
   #if defined STM32F4_TEST_QUANTRACKER_BOARD
      typedef quan::mcu::pin<quan::stm32::gpioc,12>  spi1_ncs;
   #else
    #error which board?
   #endif
  #endif
  static void init();
  static void setup_rcc();
  static void setup_spi_pins();
  static void setup_spi_regs();
  static void toggle_ncs();

   static uint8_t transfer(uint8_t data)
   {
      while (!txe()){;}
      ll_write(data);
      while ( !rxne() ){;}
      return ll_read();
   }

   static void start_spi()
   {
      spi1::get()->cr1.bb_setbit<6>(); //( SPE)
   }

   static void stop_spi()
   {
      spi1::get()->cr1.bb_clearbit<6>(); //( SPE)
   }

   static bool txe(){ return spi1::get()->sr.bb_getbit<1>();}
   static bool rxne(){ return spi1::get()->sr.bb_getbit<0>();}
   static bool busy(){ return spi1::get()->sr.bb_getbit<7>();}

   static uint8_t ll_read() { return spi1::get()->dr;}
   static void ll_write( uint8_t val){ spi1::get()->dr = val;}  

   static void transfer (const uint8_t *tx, uint8_t* rx, uint16_t len)
   {
      for ( uint16_t i = 0; i < len; ++i){
         rx[i]= transfer(tx[i]);
      }
   }

   static void cs_assert()
   {
      // 20 ns setup time
      // 6ns isntruction --> ~3 nop
      quan::stm32::clear<spi1_ncs>();
      (void)ll_read();
       asm volatile ("nop":::);
       asm volatile ("nop":::);
   }

   static void cs_release()
   {
      while (busy()) { asm volatile ("nop":::);}
      // 40 ns hold 
      // 6ns instruction --> ~6 nop
      asm volatile ("nop":::);
      asm volatile ("nop":::);
      asm volatile ("nop":::);
      asm volatile ("nop":::);
      asm volatile ("nop":::);
      asm volatile ("nop":::);
      quan::stm32::set<spi1_ncs>();
   }

   static bool transaction(const uint8_t *tx, uint8_t *rx, uint16_t len) 
   {
      cs_assert();
      transfer(tx,rx,len);
      cs_release();
      return true;
   } 

   static void reg_write( uint8_t r, uint8_t v)
   {
      uint8_t arr[2] = {r, v};
      transaction(arr,arr,2U);
   }

   static uint8_t reg_read( uint8_t r)
   {
      uint8_t arr[2] = {static_cast<uint8_t>(r | 0x80),0U};
      transaction(arr,arr,2U);
      return arr[1];
   }


};


#endif // BMI_160_HPP_INCLUDED
