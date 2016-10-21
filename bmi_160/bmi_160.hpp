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

   struct reg{
      static constexpr uint8_t  chip_id          = 0x00;
      static constexpr uint8_t  err_reg          = 0x02;  // error flags
      static constexpr uint8_t  pmu_status       = 0x03;  // power mode
      static constexpr uint8_t  data_0           = 0x04;
      /* data 0 to data 19 ........... */
      /*
          data_0 = mag.x.lsb            =0x04
          data_1 = mag.x.msb            =0x05
          data_2 = mag.y.lsb            =0x06
          data_3 = mag.y.msb            =0x07
          data_4 = mag.z.lsb            =0x08
          data_5 = mag.z.msb            =0x09

          data_6 = rhall.lsb  // hall resistance of mag  =0x0A
          data_7 = rhall.msb  // hall resistance of mag  =0x0B
      */
         static constexpr uint8_t  gyro_data_lsb        = 0x0C;
/*
          data_8 = gyr.x.lsb        =0x0C
          data_9 = gyr.x.msb        =0x0D
          data_10 = gyr.y.lsb       =0x0E
          data_11 = gyr.y.msb       =0x0F
          data_12 = gyr.z.lsb       =0x10
          data_13 = gyr.z.msb       =0x11
*/
          static constexpr uint8_t  acc_data_lsb        = 0x12;
/*
          data_14 = acc.x.lsb     =0x12
          data_15 = acc.x.msb     =0x13
          data_16 = acc.y.lsb     =0x14
          data_17 = acc.y.msb     =0x15
          data_18 = acc.z.lsb     =0x16
          data_19 = acc.z.msb     =0x17
      */ 
      static constexpr uint8_t  data_19          = 0x17; 
      static constexpr uint8_t  sensortime_0     = 0x18;
      static constexpr uint8_t  sensortime_1     = 0x19;
      static constexpr uint8_t  sensortime_2     = 0x1A;
      static constexpr uint8_t  status           = 0x1B;
      static constexpr uint8_t  int_status_0     = 0x1C;
      static constexpr uint8_t  int_status_1     = 0x1D;
      static constexpr uint8_t  int_status_2     = 0x1E;
      static constexpr uint8_t  int_status_3     = 0x1F;
      static constexpr uint8_t  temperature_0    = 0x20;
      static constexpr uint8_t  temperature_1    = 0x21;
      static constexpr uint8_t  fifo_length_0    = 0x22;
      static constexpr uint8_t  fifo_length_1    = 0x23;
      static constexpr uint8_t  fifo_data        = 0x24;
      static constexpr uint8_t  acc_conf         = 0x40;
      static constexpr uint8_t  acc_range        = 0x41;
      static constexpr uint8_t  gyr_conf         = 0x42;
      static constexpr uint8_t  gyr_range        = 0x43;
      static constexpr uint8_t  mag_conf         = 0x44;
      static constexpr uint8_t  fifo_downs       = 0x45;
      static constexpr uint8_t  fifo_config_0    = 0x46;
      static constexpr uint8_t  fifo_config_1    = 0x47;
      static constexpr uint8_t  mag_if_0         = 0x4B; 
      static constexpr uint8_t  mag_if_1         = 0x4C;
      static constexpr uint8_t  mag_if_2         = 0x4D; 
      static constexpr uint8_t  mag_if_3         = 0x4E;
      static constexpr uint8_t  mag_if_4         = 0x4F;
      static constexpr uint8_t  int_en_0         = 0x50;
      static constexpr uint8_t  int_en_1         = 0x51;
      static constexpr uint8_t  int_en_2         = 0x52;
      static constexpr uint8_t  int_out_ctrl     = 0x53;
      static constexpr uint8_t  int_latch        = 0x54;
      static constexpr uint8_t  int_map_0        = 0x55;
      static constexpr uint8_t  int_map_1        = 0x56;
      static constexpr uint8_t  int_map_2        = 0x57; 
      static constexpr uint8_t  int_data_0       = 0x58;
      static constexpr uint8_t  int_data_1       = 0x59;
      static constexpr uint8_t  int_lowhigh_0    = 0x5A;
      static constexpr uint8_t  int_lowhigh_1    = 0x5B;
      static constexpr uint8_t  int_lowhigh_2    = 0x5C;
      static constexpr uint8_t  int_lowhigh_3    = 0x5D;  
      static constexpr uint8_t  int_lowhigh_4    = 0x5E;
      static constexpr uint8_t  int_motion_0     = 0x5F;
      static constexpr uint8_t  int_motion_1     = 0x60;
      static constexpr uint8_t  int_motion_2     = 0x61;
      static constexpr uint8_t  int_motion_3     = 0x62;
      static constexpr uint8_t  int_tap_0        = 0x63;
      static constexpr uint8_t  int_tap_1        = 0x64;
      static constexpr uint8_t  int_orient_0     = 0x65; 
      static constexpr uint8_t  int_orient_1     = 0x66;
      static constexpr uint8_t  int_flat_0       = 0x67;
      static constexpr uint8_t  int_flat_1       = 0x68;
      static constexpr uint8_t  foc_conf         = 0x69;
      static constexpr uint8_t  conf             = 0x6A;
      static constexpr uint8_t  if_conf          = 0x6B;
      static constexpr uint8_t  pmu_trigger      = 0x6C;
      static constexpr uint8_t  self_test        = 0x6D;
      static constexpr uint8_t  nv_conf          = 0x70;
      static constexpr uint8_t  offset_0         = 0x71;
      static constexpr uint8_t  offset_1         = 0x72;
      static constexpr uint8_t  offset_2         = 0x73;
      static constexpr uint8_t  offset_3         = 0x74;
      static constexpr uint8_t  offset_4         = 0x75;
      static constexpr uint8_t  offset_5         = 0x76;
      static constexpr uint8_t  offset_6         = 0x77; 
      static constexpr uint8_t  step_cnt_0       = 0x78;
      static constexpr uint8_t  step_cnt_1       = 0x79;
      static constexpr uint8_t  step_conf_0      = 0x7A;
      static constexpr uint8_t  step_conf_1      = 0x7B;
      static constexpr uint8_t  cmd              = 0x7E;
   };

   struct cmd{
      static constexpr uint8_t soft_reset = 0xB6;

      static constexpr uint8_t acc_pmu_mode_suspend   = 0b00010000;
      static constexpr uint8_t acc_pmu_mode_normal    = 0b00010001;
      static constexpr uint8_t acc_pmu_mode_lo_power  = 0b00010010;

      static constexpr uint8_t gyr_pmu_mode_suspend   = 0b00010100;
      static constexpr uint8_t gyr_pmu_mode_normal    = 0b00010101;
      static constexpr uint8_t gyr_pmu_mode_lo_power  = 0b00010110;
   };

   struct val{
      static constexpr uint8_t chip_id = 0xD1;
   };

   union err_reg_bits{
      struct {
         const bool    fatal_err       : 1;
         const uint8_t err_code        : 4;
         const bool    i2c_fail_err    : 1;
         const bool    drop_cmd_err    : 1;
         const bool    mag_drdy_err    : 1;
      };
      uint8_t value;
      err_reg_bits(uint8_t val = 0U) :value{val}{}
   } __attribute__ ((packed));

   union pmu_status_bits{
      struct {
        const uint8_t mag              : 2;
        const uint8_t gyrs             : 2;
        const uint8_t acc              : 2;
        const uint8_t                  : 2;
      };
      uint8_t value;
      pmu_status_bits(uint8_t val = 0U) :value{val}{}
   }__attribute__ ((packed));

   union status_bits{
      struct {
         const bool                    : 1;
         const bool gyr_self_test_ok   : 1;
         const bool mag_man_op         : 1;
         const bool foc_rdy            : 1;
         const bool nvm_rdy            : 1;
         const bool drdy_mag           : 1;    // mag data ready
         const bool drdy_gyr           : 1;    // gyro data ready
         const bool drdy_acc           : 1;   // accel data ready
      };
      uint8_t value;
      status_bits(uint8_t val) :value{val}{}
   }__attribute__ ((packed));

   union int_status_1_bits{
      struct{
         const uint8_t                 : 4;  // are flags but ignore for now
         const bool  drdy              : 1;  // data rdy which?
         const bool                    : 3;  // ignore for now
      };
      uint8_t value;
      int_status_1_bits(uint8_t val = 0U) :value{val}{}
   }__attribute__ ((packed));

   union acc_conf_bits{
      struct{
         uint8_t  odr                  : 4;  // acc output data rate in Hz = 100/ (2 ^ ( 8 - val(acc_odr)))
         uint8_t  bwp                  : 3;  // acc bandwidth filter config or averaging and undersampling
         bool     us                   : 1;  // acc undersampling
      };
      uint8_t value;
      acc_conf_bits(uint8_t val = 0U) :value{val}{}
   }__attribute__ ((packed));

   union acc_range_bits {
      struct{
         uint8_t range                 : 4;  // 0b0011  +-2g , 0b0101 +- 4g, 0b1000 +- 8g , 0b1100 +- 16g
         uint8_t  const                : 4;
      };
      uint8_t value;
      acc_range_bits(uint8_t val = 0U) :value{val}{}
   }__attribute__ ((packed));

   union gyr_conf_bits{
      struct{
         uint8_t  odr                  : 4;  // gyr output data rate in Hz = 100/ (2 ^ ( 8 - val(gyr_odr)))
         uint8_t  bwp                  : 2;  // gyr bandwidth filter config or averaging and undersampling
         uint8_t  const                : 2; 
      };
      uint8_t value;
      gyr_conf_bits(uint8_t val = 0U) :value{val}{}
   }__attribute__ ((packed));

   union gyr_range_bits {
      struct{
         uint8_t range             : 3; // 000 +-2000, 001 +-1000, 010 +- 500, 011 +-250, 100 +- 125 ( deg/s)
         uint8_t  const                : 5;
      };
      uint8_t value;
      gyr_range_bits(uint8_t val = 0U) :value{val}{}
   }__attribute__ ((packed));

   union int_en_1_bits {
      struct{
         const uint8_t                 : 4;  // ignore for now
         bool  drdy                    : 1;
         const uint8_t                 : 3;  // ignore for now
      };
      uint8_t value;
      int_en_1_bits(uint8_t val = 0U) :value{val}{}
   }__attribute__ ((packed));

   union int_out_ctrl_bits {
      struct{
         bool int1_edge_ctrl          : 1; // true for edge, false for level
         bool int1_lvl                : 1; // true for active high, false for active low
         bool int1_od                 : 1; // true for open drain, false for push-pull
         bool int1_output_en          : 1; // true to enable int1
         uint8_t const                : 4; // int2 settings, ignore for now
      };
      uint8_t value;
      int_out_ctrl_bits(uint8_t val = 0U) :value{val}{}
   }__attribute__ ((packed));

   union int_latch_bits {
      struct{
         uint8_t int_latch                 : 4; // 0b0000 non-latched, 0b1111 latched, otherwise timed variosuly
         bool int1_input_en                : 1; // true for int1 input enabled
         bool int2_input_en                : 1; // true for int2 input enabled
         const uint8_t                     : 2;
      };
      uint8_t value;
      int_latch_bits(uint8_t val = 0U) :value{val}{}
   }__attribute__ ((packed));

   union int_map_1_bits {
      struct{
         uint8_t const                 : 7; 
         bool    drdy                  : 1;  // true to map data rdy to int1 pin
      }; 
      uint8_t value;
      int_map_1_bits(uint8_t val = 0U) :value{val}{}
   }__attribute__ ((packed));

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
      while ( rxe() ){;}
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
   static bool rxe() { return spi1::get()->sr.bb_getbit<0>() == false;}
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

   static void read(uint8_t reg, uint8_t* data, uint16_t len)
   {
      cs_assert();
      transfer(reg | 0x80);
      transfer(data,data,len);
      cs_release();
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
