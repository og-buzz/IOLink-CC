# IO-Link Library for Teknic ClearCore

This library implements the IO-Link protocol (IEC 61131-9) for the Teknic ClearCore controller. It enables communication with IO-Link devices such as sensors and actuators.

## Overview

IO-Link is a point-to-point communication protocol for connecting sensors and actuators to automation systems. It's standardized as IEC 61131-9 and provides:

- Bi-directional communication
- Process, parameter, and diagnostic data exchange
- Device auto-identification
- Enhanced diagnostics
- Remote configuration

This library allows you to implement IO-Link master functionality on the Teknic ClearCore controller, enabling it to communicate with a wide range of IO-Link devices.

## Requirements

- Teknic ClearCore controller
- IO-Link compatible device(s)
- 3-wire cable for connecting devices (max 20 meters)

## Features

- **IO-Link Master**: Create an IO-Link master on any serial port of the ClearCore
- **Device Management**: Scan for, identify, and communicate with IO-Link devices
- **Process Data Exchange**: Read and write process data with IO-Link devices
- **Parameter Access**: Configure device parameters
- **Diagnostics**: Read diagnostic information from devices
- **Event Handling**: Process events from devices

## Installation

1. Clone this repository into your project directory:
   ```
   git clone https://github.com/yourusername/iolink-clearcore.git
   ```

2. Add the library files to your ClearCore project.

3. Include the main header file in your application:
   ```cpp
   #include "IOLink.h"
   ```

## Getting Started

Here's a basic example of how to use the library:

```cpp
#include "ClearCore.h"
#include "IOLink.h"

// Define which serial port to use for IO-Link communication
#define IO_LINK_PORT ConnectorCOM0

// Create IO-Link master
IOLink::IOLinkMaster ioLinkMaster(IO_LINK_PORT);

void setup() {
    // Configure the IO-Link master
    ioLinkMaster.configure(38400);  // COM2 mode (38.4 kbaud)
    
    // Scan for IO-Link devices
    ioLinkMaster.scanForDevices();
    
    // Get first device (if found)
    std::shared_ptr<IOLink::IOLinkDevice> device = ioLinkMaster.getDevice(0);
    if (device) {
        // Activate the port for the device
        ioLinkMaster.activatePort(0, IOLink::OperationMode::COM2);
    }
}

void loop() {
    // Get device
    std::shared_ptr<IOLink::IOLinkDevice> device = ioLinkMaster.getDevice(0);
    if (!device) {
        return;
    }
    
    // Read process data from the device
    std::vector<uint8_t> processData;
    IOLink::ErrorCode readResult = device->readProcessData(processData);
    
    // Process the data
    if (readResult == IOLink::ErrorCode::NONE) {
        // Do something with the data...
    }
    
    // Process events
    ioLinkMaster.processEvents();
    
    delay(100);  // Wait for next cycle
}
```

## Using with Different IO-Link Devices

The library includes a base `IOLinkDevice` class that can be extended to implement specific device types. An example implementation is provided for a temperature sensor:

```cpp
#include "IOLinkTemperatureSensor.h"

// Create a temperature sensor device
auto sensor = std::make_shared<IOLink::TemperatureSensor>(1, vendorId, productId);

// Read temperature
float temperature = sensor->getTemperatureCelsius();
```

To implement your own device class:

1. Inherit from `IOLink::IOLinkDevice`
2. Override the necessary methods based on your device capabilities
3. Implement device-specific functionality

## Advanced Usage

### Event Handling

You can register a callback function to handle events from IO-Link devices:

```cpp
void eventCallback(uint8_t port, const std::vector<uint8_t>& eventData) {
    // Handle event data
}

// Register callback
ioLinkMaster.registerEventCallback(eventCallback);
```

### Parameter Configuration

Access device parameters using the parameter index:

```cpp
// Read parameter
std::vector<uint8_t> parameterData;
device->readParameter(parameterIndex, subindex, parameterData);

// Write parameter
std::vector<uint8_t> newValue = { 0x01, 0x02, 0x03 };
device->writeParameter(parameterIndex, subindex, newValue);
```

### IODD File Parsing

The library includes an `IOLinkIODD` class for parsing IODD (IO Device Description) files:

```cpp
// Create IODD parser
IOLink::IOLinkIODD iodd("device.xml");

// Parse the IODD file
if (iodd.parse()) {
    // Access device information
    uint32_t vendorId = iodd.getVendorId();
    uint32_t productId = iodd.getProductId();
    const char* productName = iodd.getProductName();
}
```

## Limitations

- This is a basic implementation that may not cover all features of the IO-Link protocol.
- IODD file parsing is a placeholder and needs to be fully implemented.
- Not all error handling cases are covered.

## Extending the Library

Here are some ways you can extend the library for your needs:

1. **Implement More Device Types**: Create classes for specific sensors and actuators.

2. **Add Support for Additional IO-Link Features**: Implement features like data storage, smart sensor profiles, etc.

3. **Enhance IODD Parsing**: Implement full XML parsing of IODD files.

4. **Improve Error Handling**: Add more comprehensive error detection and recovery.

5. **Add Diagnostic Features**: Implement more detailed diagnostic capabilities.

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Resources

- [IO-Link Specification](https://io-link.com/en/index.php)
- [IEC 61131-9 Standard](https://webstore.iec.ch/publication/68534)
- [Teknic ClearCore Documentation](https://teknic.com/files/downloads/clearcore_user_manual.pdf)

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.
