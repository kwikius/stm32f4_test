
#include <quan/time.hpp>
#include <quan/frequency.hpp>
#include <quan/stm32/gpio.hpp>
#include <quan/stm32/f4/i2c/module_enable_disable.hpp>
#include <quan/stm32/sys_freq.hpp>
#include <quan/stm32/i2c/detail/get_irq_number.hpp>
#include <quan/stm32/millis.hpp>
#include "i2c.hpp"
#include "led.hpp"

#include "interrupt_priority.hpp"
#include "serial_port.hpp"

namespace {
   typedef quan::mcu::pin<quan::stm32::gpioa,8> scl_pin;
   typedef quan::mcu::pin<quan::stm32::gpioc,9> sda_pin;
}

// token representing wthat i2c bus has been acquired
volatile bool i2c::m_bus_taken_token = false;
volatile bool i2c::m_errored = false;

void (* volatile i2c::pfn_event_handler)()  = i2c::default_event_handler;
void (* volatile i2c::pfn_error_handler)()  = i2c::default_error_handler;
void (* volatile i2c::pfn_dma_tx_handler)()    = i2c::default_dma_tx_handler;
void (* volatile i2c::pfn_dma_rx_handler)()    = i2c::default_dma_rx_handler;


/*
  set up the i2c bus
*/
void i2c::init()
{
   quan::stm32::module_enable<scl_pin::port_type>();
   quan::stm32::module_enable<sda_pin::port_type>();
   // TODO add check they are valid pins
   quan::stm32::apply<
      scl_pin
      ,quan::stm32::gpio::mode::af4 // all i2c pins are this af mode on F4
      ,quan::stm32::gpio::otype::open_drain
      ,quan::stm32::gpio::pupd::none         //  Use external pullup 5V tolerant pins
      ,quan::stm32::gpio::ospeed::slow
   >();

   quan::stm32::apply<
      sda_pin
      ,quan::stm32::gpio::mode::af4 // all i2c pins are this af mode on F4
      ,quan::stm32::gpio::otype::open_drain
      ,quan::stm32::gpio::pupd::none          //  Use external pullup 5V tolerant pins
      ,quan::stm32::gpio::ospeed::slow
   >();


   uint32_t count = 0;
   while ( count < 100){
      asm volatile ("nop":::);
      ++count;
   }

   quan::stm32::module_enable<i2c_type>();
   quan::stm32::module_reset<i2c_type>();

   // set the bus  speed
   // all i2c buses are on apb1?
   uint32_t constexpr apb1_freq = quan::stm32::get_bus_frequency<quan::stm32::detail::apb1>();
   static_assert(apb1_freq == 42000000,"unexpected freq");
   // set clock speed for 42 MHz APB1 bus
   uint32_t constexpr freq = apb1_freq / 1000000;
   static_assert(apb1_freq % 1000000 == 0, "invalid freq");
   uint32_t const temp_cr2 = i2c_type::get()->cr2.get() & ~0b111111;
   i2c_type::get()->cr2.set( temp_cr2 | freq);

   //set clock for slow freq
   quan::frequency_<int32_t>::Hz constexpr i2c_freq_slow{100000}; // 100 kHz
   uint32_t ccr_reg_val = apb1_freq / (i2c_freq_slow.numeric_value() * 2);

   uint32_t const temp_ccr = i2c_type::get()->ccr.get() & ~0xFFF;
   i2c_type::get()->ccr.set(temp_ccr | ccr_reg_val);

   constexpr quan::time::ns max_scl_risetime{1000};
   uint32_t constexpr trise_reg_val
      = static_cast<uint32_t>(max_scl_risetime * quan::frequency::Hz{apb1_freq} + 1.f);
   uint32_t const temp_trise = i2c_type::get()->trise.get() & ~0b111111;
   i2c_type::get()->trise.set(temp_trise | trise_reg_val);

   NVIC_EnableIRQ(quan::stm32::i2c::detail::get_event_irq_number<i2c_type>::value);
   NVIC_EnableIRQ(quan::stm32::i2c::detail::get_error_irq_number<i2c_type>::value);

   NVIC_SetPriority(quan::stm32::i2c::detail::get_event_irq_number<i2c_type>::value,interrupt_priority::i2c_port);

   setup_tx_dma();
   setup_rx_dma();

   m_errored = false;
   release_bus(); 
   peripheral_enable(true);
}

// Set up transmit dma
void i2c::setup_tx_dma()
{
   // i2c3 tx on DMA1_Stream4.CH3
   quan::stm32::rcc::get()->ahb1enr |= (1U << 21U); // DMA stream 1
//   quan::stm32::rcc::get()->ahb1rstr |= (1U << 21U);
//   quan::stm32::rcc::get()->ahb1rstr &= ~(1U << 21U);
   for ( uint8_t i = 0; i < 20; ++i){
      asm volatile ("nop" : : :);
   }
   DMA_Stream_TypeDef* dma_stream = DMA1_Stream4;
   constexpr uint32_t  dma_channel = 3;
   constexpr uint32_t  dma_priority = 0b00; // low
   constexpr uint32_t  msize = 0b00; // 8 bit mem loc
   constexpr uint32_t  psize = 0b00; // 8 bit periph loc
   dma_stream->CR = (dma_stream->CR & ~(0b111 << 25U)) | ( dma_channel << 25U); //(CHSEL) select channel
   dma_stream->CR = (dma_stream->CR & ~(0b11 << 16U)) | (dma_priority << 16U); // (PL) priority
   dma_stream->CR = (dma_stream->CR & ~(0b11 << 13U)) | (msize << 13U); // (MSIZE) 8 bit memory transfer
   dma_stream->CR = (dma_stream->CR & ~(0b11 << 11U)) | (psize << 11U); // (PSIZE) 8 bit transfer
   dma_stream->CR |= (1 << 10);// (MINC)
   dma_stream->CR &= ~(1 << 9);// (PINC)
   dma_stream->CR = (dma_stream->CR & ~(0b11 << 6U)) | (0b01 << 6U) ; // (DIR ) memory to peripheral
   dma_stream->CR |= ( 1 << 4) ; // (TCIE)
   dma_stream->PAR = (uint32_t)&I2C3->DR;  // periph addr
   NVIC_SetPriority(DMA1_Stream4_IRQn,15);  // low prio
   NVIC_EnableIRQ(DMA1_Stream4_IRQn);
}

// Set up receive dma
void i2c::setup_rx_dma()
{
   // dma1 module enable done in tx dma

   DMA_Stream_TypeDef* dma_stream = DMA1_Stream2;
   constexpr uint32_t  dma_channel = 3;
   constexpr uint32_t  dma_priority = 0b00; // low
   constexpr uint32_t  msize = 0b00; // 8 bit mem loc
   constexpr uint32_t  psize = 0b00; // 8 bit periph loc
   dma_stream->CR = (dma_stream->CR & ~(0b111 << 25U)) | ( dma_channel << 25U); //(CHSEL) select channel
   dma_stream->CR = (dma_stream->CR & ~(0b11 << 16U)) | (dma_priority << 16U); // (PL) priority
   dma_stream->CR = (dma_stream->CR & ~(0b11 << 13U)) | (msize << 13U); // (MSIZE) 8 bit memory transfer
   dma_stream->CR = (dma_stream->CR & ~(0b11 << 11U)) | (psize << 11U); // (PSIZE) 8 bit transfer
   dma_stream->CR |= (1 << 10);// (MINC)
   dma_stream->CR &= ~(1 << 9);// (PINC)
   dma_stream->CR = (dma_stream->CR & ~(0b11 << 6U))  ; // (DIR ) peripheral to memory
   dma_stream->CR |= ( 1 << 4) ; // (TCIE)
   dma_stream->PAR = (uint32_t)&I2C3->DR;  // periph addr
   NVIC_SetPriority(DMA1_Stream2_IRQn,15);  // low prio
   NVIC_EnableIRQ(DMA1_Stream2_IRQn);
}

const char* i2c::get_error_string()
{
   return "error todo";
}

void i2c::default_event_handler()
{
    panic("i2c event def called");
}


// TODO. However for now I solved this by initing the pins before the module
/*
https://my.st.com/public/STe2ecommunities/mcu/Lists/cortex_mx_stm32/Flat.aspx?RootFolder \
=https%3a%2f%2fmy%2est%2ecom%2fpublic%2fSTe2ecommunities%2fmcu%2fLists%2fcortex%5fmx%5fstm32%2f\
Proper%20Initialization%20of%20the%20I2C%20Peripheral&FolderCTID=0x01200200770978C69A1141439FE559EB\
459D7580009C4E14902C3CDE46A77F0FFD06506F5B&currentviews=1824
Over the years I have encountered several I2C peripherals that don't always reset correctly on power up.
When I initialize the I2C controller, or after a timeout error, I disable the I2C on the STM32, put the I2C pins in GPIO open drain mode,
and then toggle the SCK pin (at < 100KHz) until the SDA pin shows the bus is free.  Then I enable the I2C on the STM32 and start a new transaction.
  Jack Peacock
*/

namespace {

   void delay_10usec()
   {
       // around 10 usec
       uint32_t count = 500;
       while (count > 0){
          -- count;
          asm volatile ("nop":::);
       }
   }

   bool clear_i2c_bus()
   {
      quan::stm32::apply<
         scl_pin
         ,quan::stm32::gpio::mode::output 
         ,quan::stm32::gpio::otype::open_drain
         ,quan::stm32::gpio::pupd::none         //  Use external pullup 5V tolerant pins
         ,quan::stm32::gpio::ospeed::slow
         ,quan::stm32::gpio::ostate::high
      >();

      quan::stm32::apply<
         sda_pin
         ,quan::stm32::gpio::mode::input 
         ,quan::stm32::gpio::otype::open_drain
         ,quan::stm32::gpio::pupd::none          //  Use external pullup 5V tolerant pins
         ,quan::stm32::gpio::ospeed::slow
      >();

      // sanity check
      // if bus not cleared in 100 clocks probably terminal
      int clock_count = 100;

      while (!quan::stm32::get<sda_pin>() && (clock_count > 0)){

         quan::stm32::clear<scl_pin>();
         delay_10usec();
         quan::stm32::set<scl_pin>();
         delay_10usec();
         -- clock_count;
      }

      if ( clock_count > 0){
         return true;
      }else{
         panic("couldnt clear i2c bus");
         return false;
      }
   }

}

void i2c::default_error_handler()
{
   
   serial_port::write("i2c error handler called : ");

   NVIC_DisableIRQ(DMA1_Stream4_IRQn);
   NVIC_DisableIRQ(DMA1_Stream2_IRQn);
   enable_error_interrupts(false);
   enable_event_interrupts(false);
   enable_buffer_interrupts(false);
   // disable dma
   enable_dma_tx_stream(false);
   enable_dma_rx_stream(false);

   uint32_t const flags = get_sr1();
   bool flagged = false;
   // sr1 bit 8 Bus error
   if ( flags & ( 1 << 8)){ // (BERR)
     flagged = true;
     serial_port::write("bus error");
   }
   if ( flags & ( 1 << 9)){  // (ARLO)
     flagged = true; 
     serial_port::write("arbitration lost");
   }
   if ( flags & ( 1 << 10)){ // (AF)
     flagged = true;
     serial_port::write("acknowledge failure");
   }
   if ( flagged == false){
     serial_port::write("unknown error");
   }
   serial_port::write("\n");
   
   clear_dma_tx_stream_flags();
   clear_dma_rx_stream_flags();

   quan::stm32::module_reset<i2c_type>();
   quan::stm32::module_disable<i2c_type>();

   clear_i2c_bus();
  
   led::on();
   m_errored = true;
}

void i2c::default_dma_tx_handler()
{
    panic("i2c dma tx def called");
}

void i2c::default_dma_rx_handler()
{
    panic("i2c dma rx def called");
}

// These irq handlers are aliased to the std stm32 irq handlers
template <> __attribute__ ((interrupt ("IRQ"))) void DMA_IRQ_Handler<1,4>()
{
     i2c::pfn_dma_tx_handler();
}

template <> __attribute__ ((interrupt ("IRQ"))) void DMA_IRQ_Handler<1,2>()
{
     i2c::pfn_dma_rx_handler();
}

// alias unmangled: void ::DMA_IRQ_Handler<1,4>() ;
extern "C" void DMA1_Stream4_IRQHandler() __attribute__ ((alias ("_Z15DMA_IRQ_HandlerILi1ELi4EEvv")));
// alias unmangled: void ::DMA_IRQ_Handler<1,2>() ;
extern "C" void DMA1_Stream2_IRQHandler() __attribute__ ((alias ("_Z15DMA_IRQ_HandlerILi1ELi2EEvv")));

extern "C" void I2C3_EV_IRQHandler() __attribute__ ((interrupt ("IRQ")));
extern "C" void I2C3_EV_IRQHandler()
{
   i2c::pfn_event_handler();
}

extern "C" void I2C3_ER_IRQHandler() __attribute__ ((interrupt ("IRQ")));
extern "C" void I2C3_ER_IRQHandler()
{
   i2c::pfn_error_handler();
}
