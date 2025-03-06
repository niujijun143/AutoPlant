// **Relay Pins**
#define PIN_RELAY_1  6 
#define PIN_RELAY_2  5 
#define PIN_RELAY_3  4 
#define PIN_RELAY_4  3 

// **Sensor Pins**
#define PH_SENSOR_PIN A0
#define TDS_SENSOR_PIN A1
#define WATER_LEVEL_PIN 2
#define LED_PIN 13

// **TDS Constants**
#define VREF 5.0
#define SCOUNT 30

int analogBuffer[SCOUNT];
float averageVoltage = 0, tdsValue = 0, temperature = 25;

void setup() {
    Serial.begin(9600);

    // **Initialize Relay Pins as Outputs**
    pinMode(PIN_RELAY_1, OUTPUT);
    pinMode(PIN_RELAY_2, OUTPUT);
    pinMode(PIN_RELAY_3, OUTPUT);
    pinMode(PIN_RELAY_4, OUTPUT);

    // **Initialize Sensor Pins**
    pinMode(LED_PIN, OUTPUT);
    pinMode(WATER_LEVEL_PIN, INPUT_PULLUP);
    pinMode(TDS_SENSOR_PIN, INPUT);

    Serial.println("System Initialized");
}

void loop() {
    // **Read pH Sensor Data**
    int pH_sensor_value = analogRead(PH_SENSOR_PIN);
    float pH_voltage = pH_sensor_value * (5.0 / 1024.0);
    float pH_value = 7 + (pH_voltage - 2.5) / -0.167;

    // **Read Water Level Sensor Data**
    int water_level = digitalRead(WATER_LEVEL_PIN);

    // **Read TDS Sensor Data**
    for (int i = 0; i < SCOUNT; i++) {
        analogBuffer[i] = analogRead(TDS_SENSOR_PIN);
    }
    averageVoltage = getMedianNum(analogBuffer, SCOUNT) * (float)VREF / 1024.0;
    float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0);
    float compensationVoltage = averageVoltage / compensationCoefficient;
    tdsValue = (133.42 * compensationVoltage * compensationVoltage * compensationVoltage
                - 255.86 * compensationVoltage * compensationVoltage
                + 857.39 * compensationVoltage) * 0.5;

    // **Print Sensor Data**
    Serial.print("pH: ");
    Serial.print(pH_value, 2);
    Serial.print(" | Water Level: ");
    Serial.print(water_level ? "HIGH" : "LOW");
    Serial.print(" | TDS: ");
    Serial.print(tdsValue, 0);
    Serial.println(" ppm");

    // **Relay Control Logic Based on Sensor Data**
    if (pH_value < 6.5) {
        Serial.println("Low pH detected, turning ON Relay 1 (Acid Pump)");
        digitalWrite(PIN_RELAY_1, HIGH);
    } else {
        digitalWrite(PIN_RELAY_1, LOW);
    }

    if (pH_value > 7.5) {
        Serial.println("High pH detected, turning ON Relay 2 (Base Pump)");
        digitalWrite(PIN_RELAY_2, HIGH);
    } else {
        digitalWrite(PIN_RELAY_2, LOW);
    }

    if (tdsValue > 500) {
        Serial.println("High TDS detected, turning ON Relay 3 (Water Filter)");
        digitalWrite(PIN_RELAY_3, HIGH);
    } else {
        digitalWrite(PIN_RELAY_3, LOW);
    }

    if (water_level == LOW) {
        Serial.println("Low Water Level detected, turning ON Relay 4 (Water Pump)");
        digitalWrite(PIN_RELAY_4, HIGH);
    } else {
        digitalWrite(PIN_RELAY_4, LOW);
    }

    delay(5000);  // Delay for stability
}

// **Median Filter for Noise Reduction**
int getMedianNum(int bArray[], int iFilterLen) {
    int bTab[iFilterLen];
    for (byte i = 0; i < iFilterLen; i++)
        bTab[i] = bArray[i];

    int i, j, bTemp;
    for (j = 0; j < iFilterLen - 1; j++) {
        for (i = 0; i < iFilterLen - j - 1; i++) {
            if (bTab[i] > bTab[i + 1]) {
                bTemp = bTab[i];
                bTab[i] = bTab[i + 1];
                bTab[i + 1] = bTemp;
            }
        }
    }
    if ((iFilterLen & 1) > 0)
        bTemp = bTab[(iFilterLen - 1) / 2];
    else
        bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;

    return bTemp;
}

