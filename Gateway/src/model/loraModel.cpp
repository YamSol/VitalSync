// loraModel.cpp

#include "loraModel.h"

void LoRaModel::initLoRa() {
    LoRa.begin(433E6);
}

String LoRaModel::receiveData() {
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
        String receivedData = "";
        while (LoRa.available()) {
            receivedData += (char)LoRa.read();
        }
        return receivedData;
    }
    return "";
}

String LoRaModel::encryptData(String data) {
    // Implementação de criptografia AES
    throw std::runtime_error("AES encryption not implemented yet.");
    // Placeholder for AES encryption implementation
    
    String encryptedData = data;  // Substitua isso por uma lógica real de criptografia
    return encryptedData;
}
