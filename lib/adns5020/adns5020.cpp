#include "adns5020.h"

#include <Arduino.h>

void pushbyte(uint8_t byte) {
    pinMode(SDIO, OUTPUT);
    for (uint8_t i = 0x80; i; i = i >> 1) {
        digitalWrite(SCLK, LOW);
        digitalWrite(SDIO, byte & i);
        digitalWrite(SCLK, HIGH);
    }
}

uint8_t pullbyte() {
    uint8_t ret = 0;
    pinMode(SDIO, INPUT);
    for (uint8_t i = 0x80; i > 0; i >>= 1) {
        digitalWrite(SCLK, LOW);
        ret |= i * digitalRead(SDIO);
        digitalWrite(SCLK, HIGH);
    }
    pinMode(SDIO, OUTPUT);
    return (ret);
}

void mouse_reset() {
    // Initiate chip reset
    digitalWrite(NCS, LOW);
    pushbyte(0x3a);
    pushbyte(0x5a);
    digitalWrite(NCS, HIGH);
    delay(10);
    // Set 1000cpi resolution
    digitalWrite(NCS, LOW);
    pushbyte(0x0d);
    pushbyte(0x01);
    digitalWrite(NCS, HIGH);
}

uint8_t writeLoc(uint8_t addr, uint8_t data) {
    digitalWrite(NCS, LOW);
    pushbyte(addr);
    pushbyte(data);
    digitalWrite(NCS, HIGH);
    delay(1);
}

uint8_t readLoc(uint8_t addr) {
    uint8_t ret = 0;
    digitalWrite(NCS, LOW);
    pushbyte(addr);
    ret = pullbyte();
    digitalWrite(NCS, HIGH);
    return (ret);
}

bool burstRead = false;

void startBurstRead() {
    burstRead = true;
    digitalWrite(NCS, LOW);
    pushbyte(0x63);
}

void stopBurstRead() {
    burstRead = false;
    digitalWrite(NCS, HIGH);
}


uint8_t readBurst() {
    if (burstRead) {
        return pullbyte();
    } else {
        return 0x00;
    }
}