// gatewayController.cpp

#include "gatewayController.h"

void GatewayController::setup() {
    // Initialize LoRa
    loraModel.initLoRa();
    
    // Initialize the view with Serial output
    gatewayView.printToConsole("Gateway setup complete.");
    
    // Additional setup code can go here
    // For example, you might want to set up other peripherals or configurations
}

void GatewayController::loop() {
    // Example usage
    gatewayView.printToConsole("Gateway running...");
    
    String receivedData = loraModel.receiveData();

    if (!receivedData.isEmpty()) {
        gatewayView.printToConsole("Received data: " + receivedData);
        // String encryptedData = loraModel.encryptData(receivedData);
        // gatewayView.printToConsole("Encrypted data: " + encryptedData);
    }
}
