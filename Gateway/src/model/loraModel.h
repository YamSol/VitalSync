// loraModel.h

#ifndef LORAMODEL_H
#define LORAMODEL_H

#include <LoRa.h>

class LoRaModel {
public:
    void initLoRa();
    String receiveData();
    String encryptData(String data);
};

#endif
