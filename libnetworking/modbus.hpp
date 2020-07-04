#ifndef LIB_NETWORKING_MODBUS_HPP_
#define LIB_NETWORKING_MODBUS_HPP_

/** @file modbus.hpp
 *  @brief Modbus implementation
 *
 * Modbus Implementation
 */

#include <array>
#include <cstdint>
#include <variant>

#include <boost/system/error_code.hpp>

#include <libcore/core.hpp>

NAMESPACE_BEGIN

namespace networking {
// forward declarations
class Modbus;
struct ModbusResponse;
struct ModbusError;

namespace modbus {
/**
 *  \author Ray Andrew
 *  \ingroup Networking
 *  @brief is a strongly typed enum class representing type of message of Modbus
 */
// enum class message : std::uint8_t { indication, confirmation };

/**
 *  \author Ray Andrew
 *  \ingroup Networking
 *  @brief is a strongly typed enum class representing the function of Modbus
 */
enum class function : std::uint8_t {
  read_coils = 0x01,
  read_discrete_inputs = 0x02,
  read_holding_registers = 0x03,
  read_input_registers = 0x04,
  write_single_coil = 0x05,
  write_single_register = 0x06,
  read_exception_status = 0x07,
  diagnostics = 0x08,
  write_multiple_coils = 0x0F,
  write_multiple_registers = 0x10,
  read_file_record = 0x14,
  write_file_record = 0x15,
  mask_write_register = 0x16,
  read_write_multiple_registers = 0x17,
  read_fifo_queue = 0x18,
  encapsulated_interface_transport = 0x2B
};

/**
 *  \author Ray Andrew
 *  \ingroup Networking
 *  @brief is a strongly typed enum class representing the exception of Modbus
 * and internal lib
 */
enum class exception : std::uint8_t {
  min_exception = 0x00, /**< helper for checking modbus::eception value */

  /** modbus exception */
  illegal_function = 0x01,
  illegal_data_address,
  illegal_data_value,
  server_device_failure,
  acknowledge,
  server_device_busy,
  negative_acknowledge,
  memory_parity_error,
  undef,
  gateway_path_unavailable,
  gateway_target_device_failed_to_respond,

  /** Internal exception */
  bad_data,      /*<< data is not sent properly, bad request, bad response */
  bad_data_size, /*<< bad data size provided (can be out of bound, buffer size
                    is lesser than expected, etc) */
  connection_problem, /*<< connection problem because because of timed
                         out */
  bad_exception,      /*<< unknown exception */

  /** helper */
  no_exception, /**<< No exception status */
  max_exception /**<< helper for checking modbus::eception value */
};
}  // namespace modbus

/**
 * @brief Modbus Slave implementation.
 *        This is a virtual class wrapper that implement Modbus Slave
 * specification
 *
 * Read Modbus_Application_Protocol_Specification_V1_1b3.pdf for more info
 *
 * @author Ray Andrew
 * @date   July 2020
 */
class Modbus : public StackObj {
 public:
  /**
   * Open connection to Modbus server
   */
  virtual boost::system::error_code connect() = 0;
  /**
   * Close connection to Modbus server
   */
  virtual boost::system::error_code close() = 0;
  /**
   * Set slave id of Modbus client instance
   *
   * @param slave_id slave id of this instance
   */
  virtual void slave_id(const std::uint8_t& slave_id);
  /**
   * Read booelan status of bits/coils from Modbus remote device
   * Result will be put into the buffer (containing distinct bits for each
   * address)
   *
   * @param address  address to read
   * @param quantity quantity of bits/coils
   * @param buffer   buffer to write
   */
  virtual void read_bits(const std::uint16_t& address,
                         const std::uint16_t& quantity,
                         std::uint8_t*        buffer);
  /**
   * Read input table of bits/coils from Modbus remote device
   * Result will be put into the buffer
   *
   * @param address  address to read
   * @param quantity quantity of bits/coils
   * @param buffer   buffer to write
   */
  virtual void read_input_bits(const std::uint16_t& address,
                               const std::uint16_t& quantity,
                               std::uint8_t*        buffer);
  /**
   * Read holding registers from Modbus remote device
   * Result will be put into the buffer
   *
   * @param address  address to read
   * @param quantity quantity of bits/coils
   * @param buffer   buffer to write
   */
  virtual void read_registers(const std::uint16_t& address,
                              const std::uint16_t& quantity,
                              std::uint16_t*       buffer);
  /**
   * Read input registers from Modbus remote device
   * Result will be put into the buffer
   *
   * @param address  address to read
   * @param quantity quantity of bits/coils
   * @param buffer   buffer to write
   */
  virtual void read_input_registers(const std::uint16_t& address,
                                    const std::uint16_t& quantity,
                                    std::uint16_t*       buffer);
  /**
   * Write single bit/coil to Modbus remote device
   *
   * @param address  address to read
   * @param value    value to write
   */
  virtual void write_bit(const std::uint16_t& address,
                         const std::uint8_t&  value);
  /**
   * Write single bit/coil to Modbus remote device
   *
   * @param address  address to read
   * @param value    value to write
   */
  virtual void write_register(const std::uint16_t& address,
                              const std::uint16_t& value);
  /**
   * Write bits/coils to Modbus remote device
   *
   * @param address  address to read
   * @param quantity quantity of address
   * @param value    value to write
   */
  virtual void write_bits(const std::uint16_t& address,
                          const std::uint16_t& quantity,
                          const std::uint8_t*  value);
  /**
   * Write registers to Modbus remote device
   *
   * @param address  address to read
   * @param quantity quantity of address
   * @param value    value to write
   */
  virtual void write_registers(const std::uint16_t& address,
                               const std::uint16_t& quantity,
                               const std::uint16_t* value);

 protected:
  /**
   * Get header length of packet that will be sent via specific protocol
   *
   * @return header length
   */
  inline const unsigned int& header_length() const { return header_length_; }
  /**
   * Get transaction id
   *
   * @return transaction
   */
  inline const std::uint16_t& transaction_id() const { return transaction_id_; }
  /**
   * Get slave id
   *
   * @return slave id of device
   */
  inline const std::uint8_t& slave_id() const { return slave_id_; }
  /**
   * Check quantity of bits/coils or register whether inside domain of [1,
   * max_quantity] or not
   *
   * @param quantity     quantity that its size should be 2 bytes
   * @param max_quantity max quantity that should be allowed
   *
   * @return true if quantity is in range
   */
  static bool check_quantity(const std::uint16_t& quantity,
                             const std::uint16_t& max_quantity);
  /**
   * Check whether function is defined or not
   *
   * @param function function in enum form
   *
   * @return true if function is defined in the Modbus
   */
  static bool check_function(const modbus::function& function);
  /**
   * Check whether function is defined or not
   *
   * @param function function in numerical form
   *
   * @return true if function is defined in the Modbus
   */
  static bool check_function(const std::uint8_t& function);
  /**
   * Check whether exception is defined or not
   *
   * @param exception exception in enum form
   *
   * @return true if exception is defined in the Modbus and internal
   */
  static bool check_exception(const modbus::exception& exception);
  /**
   * Check whether exception is defined or not
   *
   * @param exception exception in numerical form
   *
   * @return true if exception is defined in the Modbus and internal
   */
  static bool check_exception(const std::uint8_t& exception);
  /**
   * Human friendly Modbus Exception
   *
   * @param exception exception in enum form
   *
   * @return Modbus exception message
   */
  static const char* exception_message(const modbus::exception& exception);
  /**
   * Prepare request before sending
   *
   * @param req       message to be sent
   * @param function  Modbus function
   * @param address   destination address
   * @param quantity  quantity of address
   *
   * @return length of packet
   */
  virtual unsigned int build_request(std::uint8_t*           req,
                                     const modbus::function& function,
                                     const std::uint16_t&    address,
                                     const std::uint16_t&    quantity) = 0;
  /**
   * Send message to Modbus remote device
   *
   * This could be synchronous or asynchronous
   *
   * @param request request to send
   * @param length  length of message
   *
   * @return ModbusResponse / ModbusError
   */
  virtual std::variant<ModbusResponse, ModbusError> send(
      std::uint8_t*      request,
      const std::size_t& length) = 0;
  /**
   * Split 16 bit to 8 bit HI/LOW
   *
   * @param buffer     buffer to write
   * @param value      value to split HI / LOW
   * @param start_addr start address of buffer that will be splitted. start_addr
   * will be HI and end_addr (start_addr + 1) will be LOW
   */
  static void uint16_to_uint8(std::uint8_t*        buffer,
                              const std::uint16_t& value,
                              const std::uint16_t& start_addr = 0);
  /**
   * Combine 8 bit to 16 bit
   *
   * @param buffer     buffer to read
   * @param value      value to write
   * @param start_addr start address of buffer that will be grouped. start_addr
   * will be HI and end_addr (start_addr + 1) will be LOW
   */
  static void uint8_to_uint16(const std::uint8_t*  buffer,
                              std::uint16_t&       value,
                              const std::uint16_t& start_addr = 0);
  /**
   * Combine 8 bit to 16 bit
   *
   * @param buffer     buffer to read
   * @param start_addr start address of buffer that will be grouped. start_addr
   * will be HI and end_addr (start_addr + 1) will be LOW
   *
   * @return value in 16 bit
   */
  static std::uint16_t uint8_to_uint16(const std::uint8_t*  buffer,
                                       const std::uint16_t& start_addr = 0);
  /**
   * Check confirmation reply from Modbus server
   *
   *
   * @param request          request that has been sent to server
   * @param response         response that has been received from server
   *
   * @return confirmation error or not
   */
  bool check_confirmation(const std::uint8_t* request,
                          const std::uint8_t* response) const;
  /**
   * Check confirmation reply from Modbus server
   *
   * @param request          request that has been sent to server
   * @param response         response that has been received from server
   * @param response_length  response length
   *
   * @return Modbus exception
   */
  modbus::exception check_exception(const std::uint8_t* request,
                                    const std::uint8_t* response,
                                    const std::size_t&  response_length) const;
  /**
   *  Calculate length computed from request
   *
   * Read Modbus_Application_Protocol_Specification_V1_1b3.pdf for more info
   *
   *  @param request buffer request
   *
   *  @return length expected from request
   */
  unsigned int calculate_length_from_request(const std::uint8_t* request) const;
  /**
   * Check whether excepted length from request is equal with response length
   *
   * @param request          request that has been sent to server
   * @param response         response that has been received from server
   * @param response_length  response length
   *
   * @return true if length is not mismatch
   */
  bool check_length(const std::uint8_t* request,
                    const std::uint8_t* response,
                    const std::size_t&  response_length) const;

 protected:
  /**
   * Modbus Constructor
   *
   * @param header_length   header length for specific protocol implementation
   */
  Modbus(const unsigned int& header_length);
  /**
   * Modbus destructor
   *
   * Close connection and destroy Modbus instance
   */
  virtual ~Modbus();

 protected:
  /**
   * Maximum Protocal Data Unit (PDU) Length
   *
   * Read Modbus_Application_Protocol_Specification_V1_1b3.pdf (chapter 4
   * section 1 page 5)
   */
  static constexpr unsigned int MAX_PDU_LENGTH = 253;
  /**
   * Maximum Application Data Unit (ADU) Length
   *
   * Read Modbus_Application_Protocol_Specification_V1_1b3.pdf (chapter 4
   * section 1 page 5)
   */
  static constexpr unsigned int MAX_ADU_LENGTH = 260;
  /**
   * Minimum Request Length to be sent
   *
   * This is to determine buffer size for request
   *
   * TCP = 12
   * RTU = 8
   *
   * Choose TCP as the max
   */
  static constexpr unsigned int MIN_REQ_LENGTH = 12;
  /**
   * Max read bits
   *
   * Value: 1 - 2000 (2 bytes, 0x0001 - 0x07D0)
   *
   * Read Modbus_Application_Protocol_Specification_V1_1b3.pdf (chapter 6
   * section 1 page 11)
   */
  static constexpr std::uint16_t MAX_READ_BITS = 0x07D0;
  /**
   * Max write bits
   *
   * Value: 1 - 1968 (2 bytes, 0x0001 - 0x07B0)
   *
   * Read Modbus_Application_Protocol_Specification_V1_1b3.pdf (chapter 6
   * section 11 page 29)
   */
  static constexpr std::uint16_t MAX_WRITE_BITS = 0x07B0;
  /**
   * Max read registers
   *
   * Value: 1 - 125 (2 bytes, 0x0001 - 0x007D)
   *
   * Read Modbus_Application_Protocol_Specification_V1_1b3.pdf (chapter 6
   * section 3 page 15)
   */
  static constexpr std::uint16_t MAX_READ_REGISTERS = 0x007D;
  /**
   * Max write registers
   *
   * Value: 1 - 123 (2 bytes, 0x0001 - 0x007B)
   *
   * Read Modbus_Application_Protocol_Specification_V1_1b3.pdf (chapter 6
   * section 12 page 30)
   */
  static constexpr std::uint16_t MAX_WRITE_REGISTERS = 0x007B;
  /**
   * Max R/W read registers
   *
   * Value: 1 - 125 (2 bytes, 0x0001 - 0x007D)
   *
   * Read Modbus_Application_Protocol_Specification_V1_1b3.pdf (chapter 6
   * section 17 page 38)
   */
  static constexpr std::uint16_t MAX_RW_READ_REGISTERS = 0x007D;
  /**
   * Max R/W write registers
   *
   * Value: 1 - 121 (2 bytes, 0x0001 - 0x0079)
   *
   * Read Modbus_Application_Protocol_Specification_V1_1b3.pdf (chapter 6
   * section 17 page 38)
   */
  static constexpr std::uint16_t MAX_RW_WRITE_REGISTERS = 0x0079;
  /**
   * Header Length
   *
   * Header of packet that will be sent via specific protocol
   */
  const unsigned int header_length_;
  /**
   * Transaction ID
   *
   * It should be 2 byte (2^8), after it overflows, it will be resetted to 0
   */
  std::uint16_t transaction_id_;
  /**
   * Slave id
   *
   * Default is 0xFF for server because it is useless to set slave id for server
   * 0x00 is for broadcast -> every message with this slave id must be accepted
   * by client
   *
   * Read Modbus_Messaging_Implementation_Guide_V1_0b.pdf (chapter 4
   * section 4.1.2  page 23)
   */
  std::uint8_t slave_id_;

 public:
  /**
   * Maximum Message Length
   *
   * Should be same with MAX_ADU_LENGTH
   */
  static constexpr unsigned int MAX_MESSAGE_LENGTH = MAX_ADU_LENGTH;
  /**
   * Response buffer
   */
  typedef std::array<std::uint8_t, Modbus::MAX_MESSAGE_LENGTH> Response;
  /**
   * Request  buffer
   */
  template <unsigned int REQUEST_LENGTH>
  using Request = std::array<std::uint8_t, REQUEST_LENGTH>;
  /**
   * Max size request  buffer
   */
  typedef Request<Modbus::MAX_MESSAGE_LENGTH> MaxRequest;
  /**
   * Min size request  buffer
   */
  typedef Request<Modbus::MIN_REQ_LENGTH> MinRequest;
};

/**
 * @brief Modbus Response wrapper implementation.
 *
 * @author Ray Andrew
 * @date   July 2020
 */
struct ModbusResponse {
  /** Transaction ID */
  const std::uint16_t transaction_id;
  /** Protocol ID */
  const std::uint16_t protocol_id;
  /** Length of response */
  const std::uint16_t length;
  /** Slave ID */
  const std::uint8_t slave_id;
  /** Modbus function */
  const modbus::function function;
  /** Raw message */
  const std::array<std::uint8_t, Modbus::MAX_MESSAGE_LENGTH> raw_response;
};

/**
 * @brief Modbus Error wrapper implementation.
 *
 * Struct to Modbus exception and connection problem
 *
 * @author Ray Andrew
 * @date   July 2020
 */
struct ModbusError {
  /** Modbus and internal exception */
  const modbus::exception exception;
  /** Error message */
  const char* error;
  /** Internal exception */
  const bool internal = false;
};
}  // namespace networking

NAMESPACE_END

#endif  // LIB_NETWORKING_MODBUS_HPP_
