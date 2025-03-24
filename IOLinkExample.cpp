/**
 * @file IOLinkExample.cpp
 * @brief Example application demonstrating IO-Link functionality with Teknic ClearCore
 * 
 * This example shows how to set up an IO-Link master on the ClearCore controller
 * and communicate with an IO-Link device (e.g., a sensor or actuator).
 */

#include "ClearCore.h"
#include "IOLink.h"
#include <vector>

// Define which serial port to use for IO-Link communication
// Options: ConnectorCOM0, ConnectorCOM1
#define IO_LINK_PORT ConnectorCOM0

// Define IO-Link operation mode and baud rate
#define IO_LINK_BAUD_RATE 38400  // COM2 mode (38.4 kbaud)

// Define LED indicators
#define STATUS_LED ConnectorLED
#define COMM_LED ConnectorLED2

// Global variables
IOLink::IOLinkMaster* ioLinkMaster = nullptr;

// Function prototypes
void setupIOLink();
void processIOLinkData();
void eventCallback(uint8_t port, const std::vector<uint8_t>& eventData);

/**
 * @brief Main program entry point
 */
int main() {
    // Initialize ClearCore
    // The IOs of the ClearCore need to be configured first
    STATUS_LED.Mode(Connector::OUTPUT_DIGITAL);
    COMM_LED.Mode(Connector::OUTPUT_DIGITAL);
    
    // Turn on Status LED to indicate program is running
    STATUS_LED.State(true);
    
    // Initialize serial port for diagnostic output
    ConnectorUsb.Mode(Connector::USB_CDC);
    ConnectorUsb.Speed(9600);
    ConnectorUsb.Format(8, ConnectorUsb.NoParity, 1);
    ConnectorUsb.PortOpen();
    
    // Print startup message
    ConnectorUsb.SendLine("IO-Link Example for Teknic ClearCore");
    ConnectorUsb.SendLine("--------------------------------------");
    
    // Set up IO-Link communication
    setupIOLink();
    
    // Main program loop
    while (true) {
        // Process IO-Link data
        processIOLinkData();
        
        // Process IO-Link events
        if (ioLinkMaster) {
            ioLinkMaster->processEvents();
        }
        
        // Give other tasks a chance to run
        delay(10);
    }
    
    return 0;
}

/**
 * @brief Set up IO-Link communication on the specified serial port
 */
void setupIOLink() {
    ConnectorUsb.SendLine("Setting up IO-Link communication...");
    
    // Configure the serial port for IO-Link
    IO_LINK_PORT.Mode(Connector::SERIAL);
    
    // Create IO-Link master
    ioLinkMaster = new IOLink::IOLinkMaster(IO_LINK_PORT);
    
    // Configure the IO-Link master
    ioLinkMaster->configure(IO_LINK_BAUD_RATE);
    
    // Register event callback
    ioLinkMaster->registerEventCallback(eventCallback);
    
    // Scan for IO-Link devices
    ConnectorUsb.SendLine("Scanning for IO-Link devices...");
    IOLink::ErrorCode scanResult = ioLinkMaster->scanForDevices();
    
    if (scanResult == IOLink::ErrorCode::NONE) {
        ConnectorUsb.SendLine("Device scan completed successfully");
        
        // Check if we found any devices
        std::shared_ptr<IOLink::IOLinkDevice> device = ioLinkMaster->getDevice(0);
        if (device) {
            ConnectorUsb.Send("Found device with ID: 0x");
            ConnectorUsb.SendLine(device->getDeviceId(), Connector::HEX);
            ConnectorUsb.Send("Vendor ID: 0x");
            ConnectorUsb.SendLine(device->getVendorId(), Connector::HEX);
            ConnectorUsb.Send("Product ID: 0x");
            ConnectorUsb.SendLine(device->getProductId(), Connector::HEX);
            
            // Activate the port for the device
            ConnectorUsb.SendLine("Activating port for device...");
            IOLink::ErrorCode activateResult = ioLinkMaster->activatePort(0, IOLink::OperationMode::COM2);
            
            if (activateResult == IOLink::ErrorCode::NONE) {
                ConnectorUsb.SendLine("Port activated successfully");
            } else {
                ConnectorUsb.SendLine("Failed to activate port");
            }
        } else {
            ConnectorUsb.SendLine("No devices found");
        }
    } else {
        ConnectorUsb.SendLine("Device scan failed");
    }
}

/**
 * @brief Process IO-Link data (read process data from device)
 */
void processIOLinkData() {
    // Check if we have an IO-Link master and a device
    if (!ioLinkMaster) {
        return;
    }
    
    std::shared_ptr<IOLink::IOLinkDevice> device = ioLinkMaster->getDevice(0);
    if (!device) {
        return;
    }
    
    // Toggle communication LED to indicate activity
    COMM_LED.State(!COMM_LED.State());
    
    // Read process data from the device
    std::vector<uint8_t> processData;
    IOLink::ErrorCode readResult = device->readProcessData(processData);
    
    if (readResult == IOLink::ErrorCode::NONE) {
        // Process the data
        ConnectorUsb.Send("Received process data: ");
        for (uint8_t byte : processData) {
            ConnectorUsb.Send("0x");
            ConnectorUsb.Send(byte, Connector::HEX);
            ConnectorUsb.Send(" ");
        }
        ConnectorUsb.SendLine("");
        
        // Example: If this is a temperature sensor, interpret the data
        if (processData.size() >= 2) {
            // Assuming a 16-bit temperature value in tenths of a degree
            int16_t temperature = (processData[0] << 8) | processData[1];
            float temperatureC = temperature / 10.0f;
            
            ConnectorUsb.Send("Temperature: ");
            ConnectorUsb.Send(temperatureC);
            ConnectorUsb.SendLine(" °C");
        }
    }
}

/**
 * @brief Callback function for IO-Link events
 * 
 * @param port Port number where the event occurred
 * @param eventData Event data payload
 */
void eventCallback(uint8_t port, const std::vector<uint8_t>& eventData) {
    ConnectorUsb.Send("Received event on port ");
    ConnectorUsb.Send(port);
    ConnectorUsb.Send(": ");
    
    for (uint8_t byte : eventData) {
        ConnectorUsb.Send("0x");
        ConnectorUsb.Send(byte, Connector::HEX);
        ConnectorUsb.Send(" ");
    }
    
    ConnectorUsb.SendLine("");
}
