/**
 * @file IOLink.cpp
 * @brief IO-Link Protocol Implementation for Teknic ClearCore
 */

#include "IOLink.h"
#include <algorithm>
#include <cstring>

namespace IOLink {

//-----------------------------------------------------------------------------
// IOLinkDevice Implementation
//-----------------------------------------------------------------------------

IOLinkDevice::IOLinkDevice(uint8_t deviceId, uint32_t vendorId, uint32_t productId)
    : m_deviceId(deviceId)
    , m_vendorId(vendorId)
    , m_productId(productId) {
}

bool IOLinkDevice::supportsOperationMode(OperationMode mode) const {
    // Default implementation - override in derived classes
    return mode == OperationMode::COM2;
}

uint8_t IOLinkDevice::getMinCycleTime() const {
    // Default minimum cycle time (2ms)
    return 2;
}

ErrorCode IOLinkDevice::readProcessData(std::vector<uint8_t>& data) {
    // Default implementation - should be overridden in derived classes
    return ErrorCode::NOT_SUPPORTED;
}

ErrorCode IOLinkDevice::writeProcessData(const std::vector<uint8_t>& data) {
    // Default implementation - should be overridden in derived classes
    return ErrorCode::NOT_SUPPORTED;
}

ErrorCode IOLinkDevice::readParameter(uint16_t index, uint8_t subindex, std::vector<uint8_t>& data) {
    // Default implementation - should be overridden in derived classes
    return ErrorCode::NOT_SUPPORTED;
}

ErrorCode IOLinkDevice::writeParameter(uint16_t index, uint8_t subindex, const std::vector<uint8_t>& data) {
    // Default implementation - should be overridden in derived classes
    return ErrorCode::NOT_SUPPORTED;
}

ErrorCode IOLinkDevice::readDiagnostic(std::vector<uint8_t>& data) {
    // Default implementation - should be overridden in derived classes
    return ErrorCode::NOT_SUPPORTED;
}

//-----------------------------------------------------------------------------
// IOLinkMaster Implementation
//-----------------------------------------------------------------------------

IOLinkMaster::IOLinkMaster(SerialDriver& serialPort)
    : m_serialPort(serialPort)
    , m_eventCallback(nullptr) {
    // Initialize the devices vector
    m_devices.clear();
}

void IOLinkMaster::configure(uint32_t baudRate) {
    // Configure the serial port for IO-Link communication
    // Typically using 8 data bits, no parity, 1 stop bit
    m_serialPort.Mode(SerialDriver::RS232);
    m_serialPort.Speed(baudRate);
    m_serialPort.Format(8, SerialDriver::NoParity, 1);
    m_serialPort.FlowControl(SerialDriver::NoFlowControl);
    
    // Enable the serial port
    m_serialPort.PortOpen();
}

ErrorCode IOLinkMaster::activatePort(uint8_t port, OperationMode mode) {
    if (port >= m_devices.size()) {
        return ErrorCode::INVALID_PARAMETER;
    }
    
    // Wakeup sequence for IO-Link device
    // Send wakeup pattern (5+ consecutive 0 bits)
    uint8_t wakeupPattern = 0x00;
    for (int i = 0; i < 10; i++) {
        m_serialPort.SendChar(wakeupPattern);
    }
    
    // Wait for device to respond
    // TODO: Implement proper IO-Link device discovery and activation
    
    return ErrorCode::NONE;
}

ErrorCode IOLinkMaster::deactivatePort(uint8_t port) {
    if (port >= m_devices.size()) {
        return ErrorCode::INVALID_PARAMETER;
    }
    
    // No specific action required to deactivate an IO-Link port
    // Just mark it as inactive in our internal state
    
    return ErrorCode::NONE;
}

ErrorCode IOLinkMaster::scanForDevices() {
    // Clear existing devices
    m_devices.clear();
    
    // IO-Link discovery protocol
    // 1. Send wakeup pattern to all ports
    // 2. Check for responses
    // 3. Identify devices and their capabilities
    // TODO: Implement proper IO-Link device discovery
    
    // For now, let's just create a dummy device for testing
    auto dummyDevice = std::make_shared<IOLinkDevice>(1, 0x12345678, 0x87654321);
    m_devices.push_back(dummyDevice);
    
    return ErrorCode::NONE;
}

std::shared_ptr<IOLinkDevice> IOLinkMaster::getDevice(uint8_t port) {
    if (port < m_devices.size()) {
        return m_devices[port];
    }
    return nullptr;
}

ErrorCode IOLinkMaster::sendMessage(uint8_t port, MessageType type, const std::vector<uint8_t>& data) {
    if (port >= m_devices.size()) {
        return ErrorCode::INVALID_PARAMETER;
    }
    
    // Build IO-Link message
    std::vector<uint8_t> message = buildIOLinkMessage(type, data);
    
    // Send over serial port
    for (uint8_t byte : message) {
        m_serialPort.SendChar(byte);
    }
    
    return ErrorCode::NONE;
}

ErrorCode IOLinkMaster::receiveMessage(uint8_t port, MessageType type, std::vector<uint8_t>& data, uint32_t timeout) {
    if (port >= m_devices.size()) {
        return ErrorCode::INVALID_PARAMETER;
    }
    
    // Wait for response with timeout
    uint32_t startTime = Milliseconds();
    std::vector<uint8_t> rawData;
    
    while ((Milliseconds() - startTime) < timeout) {
        if (m_serialPort.BytesAvailable() > 0) {
            // Read available data
            while (m_serialPort.BytesAvailable() > 0) {
                rawData.push_back(m_serialPort.ReadChar());
            }
            
            // Parse IO-Link message
            MessageType receivedType;
            std::vector<uint8_t> payload;
            ErrorCode result = parseIOLinkMessage(rawData, receivedType, payload);
            
            if (result == ErrorCode::NONE && receivedType == type) {
                data = payload;
                return ErrorCode::NONE;
            }
        }
        
        // Give other tasks a chance to run
        delay(1);
    }
    
    // Timeout occurred
    return ErrorCode::TIMEOUT;
}

void IOLinkMaster::registerEventCallback(EventCallback callback) {
    m_eventCallback = callback;
}

void IOLinkMaster::processEvents() {
    // Check for incoming event messages
    if (m_serialPort.BytesAvailable() > 0) {
        std::vector<uint8_t> rawData;
        
        // Read available data
        while (m_serialPort.BytesAvailable() > 0) {
            rawData.push_back(m_serialPort.ReadChar());
        }
        
        // Parse IO-Link message
        MessageType receivedType;
        std::vector<uint8_t> payload;
        ErrorCode result = parseIOLinkMessage(rawData, receivedType, payload);
        
        // If it's an event message and we have a callback, invoke it
        if (result == ErrorCode::NONE && receivedType == MessageType::EVENT && m_eventCallback) {
            // Determine the port from the message
            // For now, assume port 0
            uint8_t port = 0;
            m_eventCallback(port, payload);
        }
    }
}

ErrorCode IOLinkMaster::parseIOLinkMessage(const std::vector<uint8_t>& rawData, MessageType& type, std::vector<uint8_t>& payload) {
    // Simple IO-Link message parsing
    // Format: [START_BYTE] [TYPE] [LENGTH] [PAYLOAD] [CHECKSUM]
    
    if (rawData.size() < 4) {
        return ErrorCode::COMMUNICATION_ERROR;
    }
    
    const uint8_t START_BYTE = 0xA5;
    
    // Check start byte
    if (rawData[0] != START_BYTE) {
        return ErrorCode::COMMUNICATION_ERROR;
    }
    
    // Extract message type
    uint8_t typeValue = rawData[1];
    switch (typeValue) {
        case 0x01: type = MessageType::PROCESS_DATA; break;
        case 0x02: type = MessageType::PARAMETER; break;
        case 0x03: type = MessageType::DIAGNOSTIC; break;
        case 0x04: type = MessageType::EVENT; break;
        default: return ErrorCode::COMMUNICATION_ERROR;
    }
    
    // Extract payload length
    uint8_t length = rawData[2];
    if (rawData.size() < length + 4) {
        return ErrorCode::COMMUNICATION_ERROR;
    }
    
    // Extract payload
    payload.clear();
    for (uint8_t i = 0; i < length; i++) {
        payload.push_back(rawData[3 + i]);
    }
    
    // Verify checksum (simple XOR)
    uint8_t checksum = rawData[3 + length];
    uint8_t calculatedChecksum = START_BYTE ^ typeValue ^ length;
    for (uint8_t byte : payload) {
        calculatedChecksum ^= byte;
    }
    
    if (checksum != calculatedChecksum) {
        return ErrorCode::COMMUNICATION_ERROR;
    }
    
    return ErrorCode::NONE;
}

std::vector<uint8_t> IOLinkMaster::buildIOLinkMessage(MessageType type, const std::vector<uint8_t>& payload) {
    std::vector<uint8_t> message;
    
    const uint8_t START_BYTE = 0xA5;
    
    // Add start byte
    message.push_back(START_BYTE);
    
    // Add message type
    uint8_t typeValue;
    switch (type) {
        case MessageType::PROCESS_DATA: typeValue = 0x01; break;
        case MessageType::PARAMETER: typeValue = 0x02; break;
        case MessageType::DIAGNOSTIC: typeValue = 0x03; break;
        case MessageType::EVENT: typeValue = 0x04; break;
        default: typeValue = 0x01; break;
    }
    message.push_back(typeValue);
    
    // Add payload length
    message.push_back(static_cast<uint8_t>(payload.size()));
    
    // Add payload
    message.insert(message.end(), payload.begin(), payload.end());
    
    // Calculate and add checksum (simple XOR)
    uint8_t checksum = START_BYTE ^ typeValue ^ static_cast<uint8_t>(payload.size());
    for (uint8_t byte : payload) {
        checksum ^= byte;
    }
    message.push_back(checksum);
    
    return message;
}

//-----------------------------------------------------------------------------
// IOLinkIODD Implementation
//-----------------------------------------------------------------------------

IOLinkIODD::IOLinkIODD(const char* ioddFilePath)
    : m_ioddFilePath(ioddFilePath)
    , m_vendorId(0)
    , m_productId(0)
    , m_productName("")
    , m_processDataInLength(0)
    , m_processDataOutLength(0) {
}

bool IOLinkIODD::parse() {
    // TODO: Implement XML parsing of IODD file
    // For now, just return success
    return true;
}

bool IOLinkIODD::parseXML(const char* xmlContent) {
    // This function would parse the XML content
    // For now, just a placeholder
    return true;
}

} // namespace IOLink
