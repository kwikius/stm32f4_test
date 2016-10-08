#ifndef BMP_280_HPP_INCLUDED
#define BMP_280_HPP_INCLUDED

#include "../i2c_driver/i2c_register_based_driver.hpp"

struct bmp280_base {

// the registers
   struct reg{

      static constexpr uint8_t whoami = 0xD0;

   };

   // values
   struct val{
      static constexpr uint8_t whoami = 0x58;
   };

   // variants
   struct ID{
         // 111011x  
      static constexpr uint8_t  get_device_address() { return 0b11101100;}
      static constexpr const char * get_device_name() {return "BMP280";}
   };
};

// couple the bmp280 registers with the driver

struct bmp280 : bmp280_base, i2c_register_based_driver<bmp280_base::ID> 
{
};

#endif //BMP_280_HPP_INCLUDED
