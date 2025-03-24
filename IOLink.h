/**
 * @file IOLinkTemperatureSensor.h
 * @brief Implementation of an IO-Link temperature sensor
 * 
 * This file demonstrates how to extend the IOLinkDevice class
 * to implement a specific type of IO-Link device (temperature sensor).
 */

#ifndef IOLINK_TEMPERATURE_SENSOR_H
#define IOLINK_TEMPERATURE_SENSOR_H

#include "IOLink.h"

namespace IOLink {

/**
 * @class TemperatureSensor
 * @brief Implementation of an IO-Link temperature sensor device
 * 
 * This class extends IOLinkDevice to implement a temperature sensor
 * with specific process data format and parameter access.
 */
class TemperatureSensor : public IOLinkDevice {
public:
    // Constructor with device ID, vendor ID and product ID
    TemperatureSensor(uint8_t deviceId, uint32_t vendorId, uint32_t productId);
    virtual ~TemperatureSensor() = default;
    
    // Device capabilities
    bool supportsOperationMode(OperationMode mode) const override;
    uint8_t getMinCycleTime() const override;
    
    // Process data handling
    ErrorCode readProcessData(std::vector<uint8_t>& data) override;
    ErrorCode writeProcessData(const std::vector<uint8_t>& data) override;
    
    // Parameter access
    ErrorCode readParameter(uint16_t index, uint8_t subindex, std::vector<uint8_t>& data) override;
    ErrorCode writeParameter(uint16_t index, uint8_t subindex, const std::vector<uint8_t>& data) override;
    
    // Temperature-specific methods
    float getTemperatureCelsius() const;
    float getTemperatureFahrenheit() const;
    
    // Alarm thresholds
    ErrorCode setTemperatureThresholds(float lowAlarm, float highAlarm);
    ErrorCode getTemperatureThresholds(float& lowAlarm, float& highAlarm);
    
    // Unit configuration
    enum class TemperatureUnit {
        CELSIUS,
        FAHRENHEIT,
        KELVIN
    };
    
    ErrorCode setTemperatureUnit(TemperatureUnit unit);
    TemperatureUnit getTemperatureUnit() const;
    
private:
    float m_currentTemperature;      // Current temperature reading
    float m_lowAlarmThreshold;       // Low temperature alarm threshold
    float m_highAlarmThreshold;      // High temperature alarm threshold
    TemperatureUnit m_unit;          // Current temperature unit
    
    // Internal methods
    float convertTemperature(float value, TemperatureUnit fromUnit, TemperatureUnit toUnit);
};

} // namespace IOLink

#endif // IOLINK_TEMPERATURE_SENSOR_H
