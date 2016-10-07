#include "register_based_driver.hpp"

/*
   device address is setup, handlers are setup
*/
bool register_based_driver::read_register (uint8_t reg, uint8_t * result, i2c_driver::millis_type const & wait_time)
{
   if (! get_bus(wait_time)) {
      return false;
   }
   set_register_index(reg);
   set_result_ptr(result);
   set_dat_transfer_size(1);
   start();
   
}

void register_based_driver::on_device_address_sent()
{
   i2c::get_sr1();
   i2c::get_sr2();
   i2c::send_data(get_register_index());
   i2c::set_event_handler(m_irq_handler_array[++m_irq_handler_array_idx]);
}

void register_based_driver::on_write_register_sent()
{
   i2c::send_data(*m_data_ptr);
   i2c::set_event_handler(m_irq_handler_array[++m_irq_handler_array_idx]);
}





