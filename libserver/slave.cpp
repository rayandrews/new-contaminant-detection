#include "server.hpp"

#include "slave.hpp"

#include <chrono>
#include <ctime>
#include <functional>
#include <string>

#include "logger.hpp"

NAMESPACE_BEGIN

namespace server {
Slave::Slave(const Config* config) : config_{config} {
  init();
}

Slave::~Slave() {
  stop();
}

void Slave::init() {
  using namespace std::placeholders;
  using namespace std::chrono_literals;

  modbus::logger::create<server::internal::Logger>(
      config_->base_config()->debug());

  auto&& data_table = modbus::table::create(modbus::table::initializer_t{
      modbus::block::bits::initializer_t{
          modbus::address_t{config_->plc_status().meta.starting_address},
          config_->plc_status().meta.capacity, false},
      modbus::block::bits::initializer_t{
          modbus::address_t{config_->jetson_status().meta.starting_address},
          config_->jetson_status().meta.capacity, false},
      modbus::block::registers::initializer_t{
          modbus::address_t{config_->plc_data().meta.starting_address},
          config_->plc_data().meta.capacity, 0},
      modbus::block::registers::initializer_t{
          modbus::address_t{config_->jetson_data().meta.starting_address},
          config_->jetson_data().meta.capacity, 0}});

  server_ = modbus::server::create(std::move(data_table));
  server_->bind_connect(std::bind(&Slave::on_connect, this, _1, _2));
  server_->bind_disconnect(std::bind(&Slave::on_disconnect, this, _1, _2));

  // heartbeat
  timer_.start_timer(1, 780ms, [this]() {
    modbus::address_t address{config_->jetson_status(HEARTBEAT_KEY).address};
    server_->data_table().discrete_inputs().set(
        address, !server_->data_table().discrete_inputs().get(address));
  });

  // time
  timer_.start_timer(2, 1s, [this]() {
    std::time_t t = std::time(nullptr);
    std::tm*    now = std::localtime(&t);

    modbus::address_t year_address{config_->jetson_data("year").address};
    modbus::address_t month_address{config_->jetson_data("month").address};
    modbus::address_t day_address{config_->jetson_data("day").address};
    modbus::address_t hour_address{config_->jetson_data("hour").address};
    modbus::address_t minute_address{config_->jetson_data("minute").address};
    modbus::address_t second_address{config_->jetson_data("second").address};
    server_->data_table().input_registers().set(
        year_address, static_cast<std::uint16_t>(now->tm_year + 1900));
    server_->data_table().input_registers().set(
        month_address, static_cast<std::uint16_t>(now->tm_mon + 1));
    server_->data_table().input_registers().set(
        day_address, static_cast<std::uint16_t>(now->tm_mday));
    server_->data_table().input_registers().set(
        hour_address, static_cast<std::uint16_t>(now->tm_hour));
    server_->data_table().input_registers().set(
        minute_address, static_cast<std::uint16_t>(now->tm_min));
    server_->data_table().input_registers().set(
        second_address, static_cast<std::uint16_t>(now->tm_sec));
  });
}

void Slave::on_connect(modbus::server::session_ptr_t&  session,
                       [[maybe_unused]] modbus::table& table) {
  LOG_INFO(
      "Modbus master enters [remote_addr={}, remote_port={}, local_addr={}, "
      "local_port={}]",
      session->remote_address(), session->remote_port(),
      session->local_address(), session->local_port());
}

void Slave::on_disconnect(modbus::server::session_ptr_t&  session,
                          [[maybe_unused]] modbus::table& table) {
  LOG_INFO(
      "Modbus master exits [remote_addr={}, remote_port={}, local_addr={}, "
      "local_port={}]",
      session->remote_address(), session->remote_port(),
      session->local_address(), session->local_port());
}

void Slave::run() {
  server_->run("0.0.0.0", std::to_string(config_->port()).c_str());
}

void Slave::stop() {
  server_->stop();
}
}  // namespace server

NAMESPACE_END
