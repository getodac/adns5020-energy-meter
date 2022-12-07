#include <Arduino.h>
#include <adns5020.h>

#define resetTimer(x) x = millis()
#define timerExceed(x, y) (millis() - x) > y

#define SCLK 5
#define SDIO 6
#define NCS 7

#define TICKS_MS 2   // miliseconds per tick
#define SENSOR_READ_INTERVAL TICKS_MS
#define ADNS5020_SUM_ADDR 0x09
#define READ_COUNTER_RESET_TIME 200 // in miliseconds
#define READ_COUNTER_RESET READ_COUNTER_RESET_TIME/TICKS_MS
#define EM_PULSE_WIDTH 68 // in miliseconds
#define EM_PULSE_CT (EM_PULSE_WIDTH/TICKS_MS)
#define EM_PULSE_CT_MAX_SHIFT_PERCENTAGE 10 
#define EM_PULSE_CT_MAX_SHIFT (EM_PULSE_CT*EM_PULSE_CT_MAX_SHIFT_PERCENTAGE/100)
#define PIXEL_SUM_MAX 223

#define EM_PULSE_CT_MIN (EM_PULSE_CT - EM_PULSE_CT_MAX_SHIFT)
#define EM_PULSE_CT_MAX (EM_PULSE_CT + EM_PULSE_CT_MAX_SHIFT)

#define EM_PULSE_MIN_HEIGHT 5 // in percents
#define EM_PULSE_TRESHOLD (minSum + (PIXEL_SUM_MAX*EM_PULSE_MIN_HEIGHT/100)) //(minSum < 20 ? 30 : (minSum + (PIXEL_SUM_MAX*EM_PULSE_MIN_HEIGHT/100)))
#define LED 10


uint32_t t0, t1, t2, t3, t4, t5;  // timers
uint16_t readCt = 0, currentSum = 0, minSum = 0, pulseTicks = 0, lastMin = 0;
uint16_t pulseCt = 0;


void printSensorInfo() {
    Serial.print("min=");
    Serial.print(minSum);
    Serial.print(", cs=");
    Serial.print(currentSum);
    Serial.print(", pTreshold=");
    Serial.print(EM_PULSE_TRESHOLD);
    Serial.print(", pulseCt=");
    Serial.println(pulseCt);
}

void setup() {
    Serial.begin(115200);

    pinMode(SCLK, OUTPUT);
    pinMode(SDIO, OUTPUT);
    pinMode(NCS, OUTPUT);

    pinMode(10, OUTPUT);

    mouse_reset();
    delay(10);

    Serial.print("READ_COUNTER_RESET=");
    Serial.print(READ_COUNTER_RESET);
    Serial.print(", EM_PULSE_CT=");
    Serial.print(EM_PULSE_CT);
    Serial.print(", EM_PULSE_CT_MAX_SHIFT=");
    Serial.print(EM_PULSE_CT_MAX_SHIFT);
    Serial.print(", EM_PULSE_CT_MIN=");
    Serial.print(EM_PULSE_CT_MIN);
    Serial.print(", EM_PULSE_CT_MAX=");
    Serial.print(EM_PULSE_CT_MAX);
    Serial.print(", EM_PULSE_TRESHOLD=");
    Serial.println(EM_PULSE_TRESHOLD);

    Serial.println("Burst reading:");
    startBurstRead();
    for (uint8_t i = 0; i < 7; i++) {
        Serial.print(i + 1);
        Serial.print(") ");
        Serial.println(readBurst());
    }
    //stopBurstRead();
}

uint8_t a[200];
uint8_t i=0;

void printArray() {
    Serial.print("[");
    for(uint8_t x=0;x<200;x++) {
        Serial.print(a[x]);
        Serial.print(",");
    }
    Serial.println("]");
}

void loop() {
    if (timerExceed(t0, SENSOR_READ_INTERVAL)) {
        resetTimer(t0);
        currentSum = readBurst();//readLoc(ADNS5020_SUM_ADDR);
        //a[i++] = currentSum;
        if (minSum > currentSum) {
            minSum = currentSum;
        }

        if (currentSum > EM_PULSE_TRESHOLD) {
            Serial.print(currentSum);
            Serial.print(",");
            if (++pulseTicks > EM_PULSE_CT_MAX) {
                minSum = currentSum;
                pulseTicks = 0;
                Serial.print("PT RESET at tr=");
                Serial.println(EM_PULSE_TRESHOLD);
            }
        } else {
            if (pulseTicks > 0) { // if a pulse is ongoing
                if (pulseTicks >= EM_PULSE_CT_MIN && pulseTicks <= EM_PULSE_CT_MAX) {
                    // pulse detected
                    pulseCt++;
                    Serial.print("PT=");
                    Serial.print(pulseTicks);
                    Serial.print(",tr=");
                    Serial.println(EM_PULSE_TRESHOLD);
                    printSensorInfo();
                }
            }
            pulseTicks = 0;
        }

        if (++readCt > READ_COUNTER_RESET && pulseTicks == 0) {
            readCt = 0;
            minSum = currentSum;
        }
    }
    
    // if (i > 199) {
    //     i = 0;
    //     printArray();
    // }

    if (timerExceed(t1, 500)) {
        resetTimer(t1);
        printSensorInfo();
    }

    if (timerExceed(t2, 2000)) {
        if (digitalRead(LED) == LOW) {
            digitalWrite(LED, HIGH);
        }
        if (timerExceed(t2, 2100)) {
            digitalWrite(LED, LOW);
            resetTimer(t2);
        }
    }

}
