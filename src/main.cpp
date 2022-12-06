#include <Arduino.h>
#include <adns5020.h>

#define resetTimer(x) x = millis()
#define timerExceed(x, y) (millis() - x) > y

#define SCLK 5
#define SDIO 6
#define NCS 7

#define TICKS_MS 2   // 5 ms per tick
#define SENSOR_READ_INTERVAL TICKS_MS
#define ADNS5020_SUM_ADDR 0x09
#define READ_COUNTER_RESET_TIME 1000 // in miliseconds
#define READ_COUNTER_RESET READ_COUNTER_RESET_TIME/TICKS_MS
#define EM_PULSE_WIDTH 200
#define EM_PULSE_CT (EM_PULSE_WIDTH/TICKS_MS)
#define EM_PULSE_CT_MAX_SHIFT_PERCENTAGE 10 
#define EM_PULSE_CT_MAX_SHIFT (EM_PULSE_CT*EM_PULSE_CT_MAX_SHIFT_PERCENTAGE/100)

#define EM_PULSE_CT_MIN (EM_PULSE_CT - EM_PULSE_CT_MAX_SHIFT)
#define EM_PULSE_CT_MAX (EM_PULSE_CT + EM_PULSE_CT_MAX_SHIFT)

#define EM_PULSE_MIN_HEIGHT 10 // in percents
#define EM_PULSE_TRESHOLD (minSum + (minSum*EM_PULSE_MIN_HEIGHT/100))



uint32_t t0, t1, t2, t3, t4, t5;  // timers
uint8_t readCt = 0, currentSum = 0, minSum = 0, pulseTicks = 0;
uint16_t pulseCt = 0;

void setup() {
    Serial.begin(9600);

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


void loop() {
    if (timerExceed(t0, SENSOR_READ_INTERVAL)) {
        resetTimer(t0);
        currentSum = readBurst();//readLoc(ADNS5020_SUM_ADDR);
        if (++readCt > READ_COUNTER_RESET) {
            readCt = 0;
            minSum = currentSum;
        }
        if (minSum > currentSum) {
            minSum = currentSum;
        }

        if (currentSum > EM_PULSE_TRESHOLD) {
            if (++pulseTicks > EM_PULSE_CT_MAX) {
              minSum = currentSum;
              pulseTicks = 0;
            }
        } else {
            if (pulseTicks > 0) { // new pulse is started?
                if (pulseTicks >= EM_PULSE_CT_MIN && pulseTicks <= EM_PULSE_CT_MAX) {
                    // pulse detected
                    pulseCt++;
                }
            }
            pulseTicks = 0;
        }

    }
    
    if (timerExceed(t1, 500)) {
        resetTimer(t1);
        Serial.print("min=");
        Serial.print(minSum);
        Serial.print(", cs=");
        Serial.print(currentSum);
        Serial.print(", pTreshold=");
        Serial.print(EM_PULSE_TRESHOLD);
        Serial.print(", pulseCt=");
        Serial.println(pulseCt);

    }
}
