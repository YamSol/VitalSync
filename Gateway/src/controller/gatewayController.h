// gatewayController.h

#ifndef GATEWAYCONTROLLER_H
#define GATEWAYCONTROLLER_H

#include "../model/loraModel.h"
#include "../view/gatewayView.h"
#include <Arduino.h>

class GatewayController {
public:
    void setup();
    void loop();
private:
    LoRaModel loraModel;
    GatewayView gatewayView = GatewayView(Serial); // Initialize with Serial
};

#endif
