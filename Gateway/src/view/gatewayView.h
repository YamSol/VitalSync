// gatewayView.h

#include <Arduino.h>

#ifndef GATEWAYVIEW_H
#define GATEWAYVIEW_H

class GatewayView {
private:
    Stream &outputStream; // Non-static data member to hold the output stream

public:
    // Constructor that accepts an output stream (e.g., Serial)
    GatewayView(Stream &output) : outputStream(output) {}

    template <typename T>
    void printToConsole(const T &message) {
        outputStream.println(message);
    }
    
    void displayMessage(String message);
};

#endif
