#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

#define BLINKER_WIFI
#define BLINKER_MIOT_LIGHT

#include <Blinker.h>
#include "config.inc"

BlinkerButton SwitchLight("switch-light");

int counter = 0;
bool wsState = false;

void updateLightDiandeng() {
    SwitchLight.print(wsState ? "on" : "off");
}

void updateLightMiot() {
    BlinkerMIOT.powerState(wsState ? "on" : "off");
    BlinkerMIOT.print();
}

void switchLight(bool isOn) {
    wsState = isOn;

    digitalWrite(LED_IND, wsState ? LOW : HIGH);

    // Send 2 times
    irsend.sendRaw(wsState ? rawDataOn : rawDataOff, rawDataLen, sendRawInHz);
    delay(100);
    irsend.sendRaw(wsState ? rawDataOn : rawDataOff, rawDataLen, sendRawInHz);

    updateLightDiandeng();
    updateLightMiot();
}

void switch_light_callback(const String & state) {
    BLINKER_LOG("get button state: ", state);
    switchLight(state == "on");
}

void heartbeat() {
    BLINKER_LOG("heartbeat");
    updateLightDiandeng();
}

void dataRead(const String & data)
{
    BLINKER_LOG("Blinker readString: ", data);
}

void miotPowerState(const String & state)
{
    BLINKER_LOG("need set power state: ", state);
    switchLight(state == BLINKER_CMD_ON);
}

void miotQuery(int32_t queryCode)
{
    BLINKER_LOG("MIOT Query codes: ", queryCode);

    switch (queryCode)
    {
    case BLINKER_CMD_QUERY_ALL_NUMBER :
        BLINKER_LOG("MIOT Query All");
        break;
    case BLINKER_CMD_QUERY_POWERSTATE_NUMBER :
        BLINKER_LOG("MIOT Query Power State");
        break;
    default :
        BLINKER_LOG("MIOT Query Other");
        break;
    }

    updateLightMiot();
}

void setup() {
    Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);

#if defined(BLINKER_PRINT)
    BLINKER_DEBUG.stream(BLINKER_PRINT);
#endif

    // init IR
    irsend.begin();

    // Init LED Indicator
    pinMode(LED_IND, OUTPUT);
    digitalWrite(LED_IND, HIGH);

    // Init Blinker
    Blinker.begin(auth, ssid, pswd);
    Blinker.attachData(dataRead);

    Blinker.attachHeartbeat(heartbeat);

    BlinkerMIOT.attachPowerState(miotPowerState);
    BlinkerMIOT.attachQuery(miotQuery);

    SwitchLight.attach(switch_light_callback);

    // Switch off on startup
    switchLight(false);
}

void loop() {
    Blinker.run();
}
