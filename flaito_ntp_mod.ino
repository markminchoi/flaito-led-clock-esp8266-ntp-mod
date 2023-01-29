/*
    23.01.29
    flaito led clock + ESP8266 - NTP Mod
    markminchoi

    TM1650.h -> https://github.com/arkhipenko/TM1650/
*/

#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <TM1650.h>
#include <WiFiUdp.h>

const byte digits[4][10] = {{0x77, 0x14, 0x6e, 0x5e, 0x1d, 0x5b, 0x7b, 0x16, 0x7f, 0x5f},  // seg[0]
                            {0x77, 0x22, 0x5e, 0x6e, 0x2b, 0x6d, 0x7d, 0x26, 0x7f, 0x6f},  // seg[1]
                            {0x77, 0x22, 0x5b, 0x6b, 0x2e, 0x6d, 0x7d, 0x23, 0x7f, 0x6f},  // seg[2]
                            {0x77, 0x22, 0x5e, 0x6e, 0x2b, 0x6d, 0x7d, 0x26, 0x7f, 0x6f}}; // seg[3]
                          // 0,    1,    2,    3,    4,    5,    6,    7,    8,    9

const char *ssid = "";
const char *password = "";

const long colonPeriod = 500;
const long pollPeriod = 50;

const int adcThreshold = 260;

unsigned long colonMillis = 0;
unsigned long pollMillis = 0;

int lasts = 0;

WiFiUDP ntpUDP;

NTPClient timeClient(ntpUDP, "kr.pool.ntp.org", 32400); // UTC+9(Seoul)

TM1650 panel;

void displayInit() {
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 4; j++)
            panel.setPosition(j, digits[j][i]);
        delay(100);
    }
}

void displayTime(int h, int m) {
    panel.setPosition(3, digits[3][h / 10]);
    panel.setPosition(2, digits[2][h % 10]);
    panel.setPosition(1, digits[1][m / 10]);
    panel.setPosition(0, digits[0][m % 10]);
}

void setup() {
    pinMode(4, OUTPUT);
    digitalWrite(4, HIGH);

    Serial.begin(115200);

    Wire.begin(2, 0);

    panel.init();
    panel.setBrightness(1);
    panel.displayOn();

    displayInit();

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.println("Connection Failed! Rebooting...");
        delay(5000);
        ESP.restart();
    }

    Serial.println('\n');
    Serial.print("Connected to ");
    Serial.println(WiFi.SSID());
    Serial.print("IP address:\t");
    Serial.println(WiFi.localIP());

    timeClient.begin();
}

void loop() {
    unsigned long currentMillis = millis();

    if (currentMillis - pollMillis >= pollPeriod) {
        pollMillis = currentMillis;

        timeClient.update();

        int h = timeClient.getHours();
        int m = timeClient.getMinutes();
        int s = timeClient.getSeconds();

        if (s != lasts) {
            lasts = s;
            colonMillis = currentMillis;
            displayTime(h, m);
            panel.setDot(0, true);

            if (analogRead(A0) >= adcThreshold)
                digitalWrite(4, LOW);

            else
                digitalWrite(4, HIGH);
        }
    }

    if (currentMillis - colonPMillis == colonPeriod)
        panel.setDot(0, false);
}
