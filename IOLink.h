/**
 * @file IOLink.h
 * @brief IO-Link Protocol Implementation for Teknic ClearCore
 * 
 * This library implements the IO-Link protocol (IEC 61131-9) for the Teknic ClearCore controller.
 * It enables communication with IO-Link devices such as sensors and actuators.
 */

#ifndef IOLINK_H
#define IOLINK_H

#include <ClearCore.h>
#include <vector>
#include <memory>
#include <functional>

namespace IOLink {

// IO-Link operational modes
enum class OperationMode {
    SIO,            // Standard I/O mode (digital input/output)
    COM1,           // COM1 mode (4.8 kbaud)
    COM2,           // COM2 mode (38.4 kbaud)
    COM3            // COM3 mode (230.4 kbaud)
};

// IO-Link message types
enum class MessageType {
    PROCESS_DATA,   // Cyclic process data exchange
    PARAMETER,      // Acyclic parameter access
    DIAGNOSTIC,     // Diagnostic information
    EVENT           // Event information
};

// IO-Link error codes
enum class ErrorCode {
    NONE,
    COMMUNICATION_ERROR,
    DEVICE_ERROR,
    TIMEOUT,
    INVALID_PARAMETER,
    NOT_SUPPORTED
};

/**
 * @class IOLinkDevice
 * @brief Represents an IO-Link device connected to a master
 */
class IOLinkDevice {
public:
    IOLinkDevice(uint8_t deviceId, uint32_t vendorId, uint32_t productId);
    virtual ~IOLinkDevice() = default;
    
    // Device identification
    uint8_t getDeviceId() const { return m_deviceId; }
    uint32_t getVendorId() const { return m_vendorId; }
    uint32_t getProductId() const { return m_productId; }
    
    // Device capabilities
    virtual bool supportsOperationMode(OperationMode mode) const;
    virtual uint8_t getMinCycleTime() const;
    
    // Process data handling
    virtual ErrorCode readProcessData(std::vector<uint8_t>& data);
    virtual ErrorCode writeProcessData(const std::vector<uint8_t>& data);
    
    // Parameter access
    virtual ErrorCode readParameter(uint16_t index, uint8_t subindex, std::vector<uint8_t>& data);
    virtual ErrorCode writeParameter(uint16_t index, uint8_t subindex, const std::vector<uint8_t>& data);
    
    // Diagnostic information
    virtual ErrorCode readDiagnostic(std::vector<uint8_t>& data);
    
private:
    uint8_t m_deviceId;     // Device ID (address)
    uint32_t m_vendorId;    // Vendor ID
    uint32_t m_productId;   // Product ID
};

/**
 * @class IOLinkMaster
 * @brief Implements an IO-Link master for ClearCore
 */
class IOLinkMaster {
public:
    IOLinkMaster(SerialDriver& serialPort);
    virtual ~IOLinkMaster() = default;
    
    // Master configuration
    void configure(uint32_t baudRate = 38400);
    
    // Port management
    ErrorCode activatePort(uint8_t port, OperationMode mode = OperationMode::COM2);
    ErrorCode deactivatePort(uint8_t port);
    
    // Device management
    ErrorCode scanForDevices();
    std::shared_ptr<IOLinkDevice> getDevice(uint8_t port);
    
    // Communication
    ErrorCode sendMessage(uint8_t port, MessageType type, const std::vector<uint8_t>& data);
    ErrorCode receiveMessage(uint8_t port, MessageType type, std::vector<uint8_t>& data, uint32_t timeout = 1000);
    
    // Event handling
    using EventCallback = std::function<void(uint8_t port, const std::vector<uint8_t>& eventData)>;
    void registerEventCallback(EventCallback callback);
    
private:
    SerialDriver& m_serialPort;
    std::vector<std::shared_ptr<IOLinkDevice>> m_devices;
    EventCallback m_eventCallback;
    
    // Internal methods
    void processEvents();
    ErrorCode parseIOLinkMessage(const std::vector<uint8_t>& rawData, MessageType& type, std::vector<uint8_t>& payload);
    std::vector<uint8_t> buildIOLinkMessage(MessageType type, const std::vector<uint8_t>& payload);
};

/**
 * @class IOLinkIODD
 * @brief Handles IO Device Description (IODD) parsing
 * 
 * This class is responsible for parsing IODD XML files and extracting
 * device capabilities, parameters, and process data structure.
 */
class IOLinkIODD {
public:
    IOLinkIODD(const char* ioddFilePath);
    virtual ~IOLinkIODD() = default;
    
    // IODD parsing
    bool parse();
    
    // Device information
    uint32_t getVendorId() const { return m_vendorId; }
    uint32_t getProductId() const { return m_productId; }
    const char* getProductName() const { return m_productName.c_str(); }
    
    // Process data structure
    uint8_t getProcessDataInLength() const { return m_processDataInLength; }
    uint8_t getProcessDataOutLength() const { return m_processDataOutLength; }
    
private:
    const char* m_ioddFilePath;
    uint32_t m_vendorId;
    uint32_t m_productId;
    std::string m_productName;
    uint8_t m_processDataInLength;
    uint8_t m_processDataOutLength;
    
    // Helper methods for XML parsing
    bool parseXML(const char* xmlContent);
};

} // namespace IOLink

#endif // IOLINK_H
