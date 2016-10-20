
/*
Copyright (C) 2016  Andy Little
modified from ardupilot/libraries/AP_InertialSensor_BMI160.cpp

 * Copyright (C) 2016  Intel Corporation. All rights reserved.
 *
 * This file is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.

 */
#include <quan/stm32/gpio.hpp>
#include "../system/serial_port.hpp"
#include "../system/led.hpp"
#include <quan/stm32/millis.hpp>
#include "bmi_160.hpp"
#include <quan/three_d/vect.hpp>

using quan::stm32::millis;

void delay(quan::time::ms const & t);


void bmi_160_soft_reset()
{
   bmi_160::init();
   bmi_160::reg_write(bmi_160::reg::cmd,bmi_160::cmd::soft_reset);
   delay(quan::time_<uint32_t>::ms{200U});
   bmi_160::reg_read(0x7F);
}

bool bmi_160_whoami_test()
{
   bool result = bmi_160::reg_read(bmi_160::reg::chip_id) == bmi_160::val::chip_id;
   delay(quan::time::ms{1});
   return result;
}

//bool bmi_160_test()
//{
//   serial_port::write("bmi_160 whoami test\n");
////   bmi_160::init();
////
////   bmi_160::reg_write(0x7E,0xB6);
////
////   auto now = millis();
////   for (;;){
////      if ( (millis() - now) > quan::time_<uint32_t>::ms{200U}){ break;}
////   }
////   bmi_160::reg_read(0x7F);
//   bmi_160_soft_reset();
//   serial_port::write("bmi_160 whoami inited\n");
//   
//   uint8_t value = bmi_160::reg_read(0);
//   bool result = false;
//   if ( value == 0xD1){
//     serial_port::write("bmi_160 whoami test succeeded\n");
//     result = true;
//   }else{
//     serial_port::write("bmi_160 whoami test failed\n");
//   }
//   serial_port::printf<30>("got %u\n",static_cast<unsigned int>(value));
//   return result;
//}

// ret true if no errors
bool bmi_160_check_no_errors()
{
  bool result = bmi_160::reg_read(bmi_160::reg::err_reg) == 0;
  if(! result){
     serial_port::write("bmi_160 hardware errors flags set\n");
  }
  return result;
}

bool bmi_160_accel_setup()
{
   bmi_160::reg_write(bmi_160::reg::cmd,bmi_160::cmd::acc_pmu_mode_normal);
   delay(quan::time::ms{5});

   bmi_160::acc_conf_bits acc_conf;

   acc_conf.us  = false;  //in normal power mode undersampling should be disabled

/*
     datasheet says ODR <= 1600
     ODR = 100/ (2^(8-odr_reg_value)
     therefore
     odr_reg_value = 8 - log2(100/ODR)
*/
   constexpr uint8_t odr_1600Hz = 12;
   constexpr uint8_t odr_800Hz  = 11;
   constexpr uint8_t odr_400Hz  = 10;
   constexpr uint8_t odr_200Hz  =  9;
   constexpr uint8_t odr_100Hz  =  8;
   constexpr uint8_t odr_50Hz   =  7;
   constexpr uint8_t odr_25Hz   =  6;

   acc_conf.odr = odr_1600Hz;   // n.b  divided by oversampling below

/*
    when .us = false 
    the bwp reg value represents filtering
*/
   constexpr uint8_t bwp_OSR1 = 0b010; //  1x oversampling
   constexpr uint8_t bwp_OSR2 = 0b001; //  2x oversampling
   constexpr uint8_t bwp_OSR4 = 0b000; //  4x oversampling

   acc_conf.bwp = bwp_OSR2;   // 2x oversampling actual odr == 800 Hz
   
   bmi_160::reg_write(bmi_160::reg::acc_conf,acc_conf.value);

//--------------------

   bmi_160::acc_range_bits acc_range;

   constexpr uint8_t range_2g  = 0b0011;
   constexpr uint8_t range_4g  = 0b0101;
   constexpr uint8_t range_8g  = 0b1000;
   constexpr uint8_t range_16g = 0b1100;

   acc_range.range = range_8g;

   bmi_160::reg_write(bmi_160::reg::acc_range,acc_range.value);

   uint8_t arr[20];
   bmi_160::read(bmi_160::reg::data_0,arr,20);

   return bmi_160_check_no_errors();
}

bool bmi_160_gyro_setup()
{
   // should this be last in fun?
   // can we setup flags then enable?
   bmi_160::reg_write(bmi_160::reg::cmd,bmi_160::cmd::gyr_pmu_mode_normal);
   delay(quan::time::ms{81});

   bmi_160::gyr_conf_bits gyr_conf;

   constexpr uint8_t odr_3200Hz = 13;
   constexpr uint8_t odr_1600Hz = 12;
   constexpr uint8_t odr_800Hz  = 11;
   constexpr uint8_t odr_400Hz  = 10;
   constexpr uint8_t odr_200Hz  =  9;
   constexpr uint8_t odr_100Hz  =  8;
   constexpr uint8_t odr_50Hz   =  7;
   constexpr uint8_t odr_25Hz   =  6;

   gyr_conf.odr = odr_3200Hz;   // n.b  divided by oversampling below

   constexpr uint8_t bwp_OSR1 = 0b010; //  1x oversampling
   constexpr uint8_t bwp_OSR2 = 0b001; //  2x oversampling
   constexpr uint8_t bwp_OSR4 = 0b000; //  4x oversampling

   gyr_conf.bwp = bwp_OSR4;   // 4x oversampling actual odr == 800 Hz

   bmi_160::reg_write(bmi_160::reg::gyr_conf,gyr_conf.value);

   bmi_160::gyr_range_bits gyr_range;

   constexpr uint8_t range_2000_deg_s  = 0b000;
   constexpr uint8_t range_1000_deg_s  = 0b001;
   constexpr uint8_t range_500_deg_s   = 0b010;
   constexpr uint8_t range_250_deg_s   = 0b011;
   constexpr uint8_t range_125_deg_s   = 0b100;

   gyr_range.range = range_2000_deg_s;

   bmi_160::reg_write(bmi_160::reg::gyr_range,gyr_range.value);

   uint8_t arr[20];
   bmi_160::read(bmi_160::reg::data_0,arr,20);

   return bmi_160_check_no_errors();
}

bool bmi_160_interrupt_setup()
{ 
   {
      bmi_160::int_map_1_bits int_map_1;
      int_map_1.drdy = true;
      if ( int_map_1.value != 0x80){
         serial_port::write("invalid reg val\n");
      }
      bmi_160::reg_write(bmi_160::reg::int_map_1,int_map_1.value);
   }
   {
      bmi_160::int_out_ctrl_bits int_out_ctrl;
      int_out_ctrl.int1_edge_ctrl = false; // level triggered
      int_out_ctrl.int1_output_en = true;  // enable output
      int_out_ctrl.int1_lvl       = false; // active low
      int_out_ctrl.int1_od        = false; // push-pull 

      if ( int_out_ctrl.value != 0x08){
         serial_port::write("invalid reg val\n");
      }
      bmi_160::reg_write(bmi_160::reg::int_out_ctrl,int_out_ctrl.value);
   }
   {
      bmi_160::int_en_1_bits int_en_1;
      int_en_1.drdy = true;
      bmi_160::reg_write(bmi_160::reg::int_en_1,int_en_1.value);
   }
   return bmi_160_check_no_errors();
}

namespace {

   typedef quan::mcu::pin<quan::stm32::gpioc,13> irq_pin;
   void setup_irq_pin()
   {
      quan::stm32::module_enable<irq_pin::port_type>();
      quan::stm32::apply<
         irq_pin
         , quan::stm32::gpio::mode::input
         , quan::stm32::gpio::pupd::pull_up
      >();
   }
}

bool bmi_160_setup()
{
   setup_irq_pin();
   bmi_160_soft_reset();

   if (! bmi_160_whoami_test()){
      serial_port::write("bmi 160 whoami failed\n");
      return false;
   }

   bmi_160_accel_setup();
   bmi_160_gyro_setup();
   bmi_160_interrupt_setup();

   return bmi_160_check_no_errors();
}

bool bmi_160_run()
{
   if( ! quan::stm32::get<irq_pin>()){
      serial_port::write("unexpected : irq pin already low\n");
      return false;
   }
   if ( bmi_160_setup()){
      auto last_print = millis();
      for (;;){ 
         auto now = millis();
         union u{
            struct{
               quan::three_d::vect<int16_t> gyr;
               quan::three_d::vect<int16_t> acc;
            }d;
            uint8_t arr[12];
            u(){ for ( auto & v: arr){ v = 0;}}
         }u;
         
         while ( quan::stm32::get<irq_pin>() ){

            if ( (millis() - now)  > quan::time::ms{500}){

               serial_port::write("no sign of bmi_160 irq\n");
             
               break;
            }
         }

         bmi_160::read(bmi_160::reg::gyro_data_lsb,u.arr,12);
         // n.b there is a delay until the irq pin goes high here
         // after the read of the registers, delay count is around 2200
         // possibly the 39 usec clock?
         // could do temporary latch the irq and detect edge?
         // this will be annoying in irq
         int irq_delay_count = 0;
         while( ! quan::stm32::get<irq_pin>()){
             if ( ++irq_delay_count > 100000){
               serial_port::write("unexpected : irq pin stayed low\n");

               bmi_160::status_bits status = bmi_160::reg_read(bmi_160::reg::status);
               serial_port::printf<100>("status = %X\n",status.value);
               if (status.drdy_acc){
                  serial_port::write("drdy acc\n");        
               }else{
                   serial_port::write("no drdy acc\n"); 
               }
               if (status.drdy_gyr){
                  serial_port::write("drdy gyr\n");        
               }else{
                   serial_port::write("no drdy gyr\n"); 
               }
               serial_port::write("fail\n");
             return false;
           }
         }
         
         if ( (millis() - last_print) > quan::time::ms{100}){
          
            last_print = millis();
            serial_port::printf<100>("delay count = %d\n", irq_delay_count);
            serial_port::printf<100>("acc = [%d,%d,%d]\ngyr = [%d,%d,%d]\n",u.d.acc.x,u.d.acc.y,u.d.acc.z,u.d.gyr.x,u.d.gyr.y,u.d.gyr.z);
         }
      }
      return true;

   }else{
      serial_port::write("bmi_160 setup failed\n");

      return false;
   }
}




