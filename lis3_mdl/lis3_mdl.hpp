#ifndef LIS3_MDL_HPP_INCLUDED
#define LIS3_MDL_HPP_INCLUDED

#include "../i2c_driver/i2c_register_based_driver.hpp"

struct lis3mdl_base {

// the registers
   struct reg{
      static constexpr uint8_t offset_X_reg_L = 0x05;
      static constexpr uint8_t offset_X_reg_H = 0x06;
      static constexpr uint8_t offset_Y_reg_L = 0x07;
      static constexpr uint8_t offset_Y_reg_H = 0x08;
      static constexpr uint8_t offset_Z_reg_L = 0x09;
      static constexpr uint8_t offset_Z_reg_H = 0x0A;

      static constexpr uint8_t whoami = 0x0f;
      static constexpr uint8_t ctr1reg1 = 0x20;
      static constexpr uint8_t ctr1reg2 = 0x21;
      static constexpr uint8_t ctr1reg3 = 0x22;
      static constexpr uint8_t ctr1reg4 = 0x23;
      static constexpr uint8_t ctr1reg5 = 0x24;
   };

   // values
   struct val{
      static constexpr uint8_t whoami = 0b00111101;
   };

   // variants
   struct onboardID{
      static constexpr uint8_t  get_device_address() { return 0b00111000;}
      static constexpr const char * get_device_name() {return "LIS3MDL onboard";}
   };
};

// couple the lis3mdl resiters to the driver
template <typename ID>
struct lis3mdl : lis3mdl_base, i2c_register_based_driver<ID> {};

typedef lis3mdl<lis3mdl_base::onboardID> lis3mdl_onboard;

#endif // LIS3_MDL_HPP_INCLUDED
