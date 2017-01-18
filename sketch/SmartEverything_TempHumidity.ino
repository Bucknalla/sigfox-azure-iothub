/*
    @aureleq
    Read Temp/Humidity value and send values over Sigfox everytime the board is powered-on or reset button is pressed

    Telit le51-868-s more information available here:
    http://www.telit.com/products/product-service-selector/product-service-selector/show/product/le51-868-s/
*/

#include <Arduino.h>
#include <Wire.h> // I2C lib
#include <SmeSFX.h> // Sigfox lib
#include <HTS221.h> // Temp/Humditiy Lib

#define DEBUG 0

// The setup function runs once when you press reset or power the board
void setup() {
    if(DEBUG) {
      while(!SerialUSB);
    }
    SerialUSB.begin(9600);

    // Initiate the Wire library and join the I2C bus
    Wire.begin();
    // Init humidity sensor
    smeHumidity.begin();

    /* ------- SIGFOX INIT - BEGIN ------- */
    sfxAntenna.begin();
    int initFinish=1;

    SerialUSB.println("SFX in Command mode");
    sfxAntenna.setSfxConfigurationMode(); // Enter into configuration Mode

    do {
        uint8_t answerReady = sfxAntenna.hasSfxAnswer();
        if (answerReady){
            switch (initFinish){
            case 1:
                SerialUSB.println("SFX in Data mode");
                sfxAntenna.setSfxDataMode();
                initFinish++;
                break;

            case 2:
                initFinish++; // exit
                break;
            }
        }
    } while (initFinish!=3);
    /* ------- SIGFOX INIT - END ------- */
}

// The loop function runs over and over again forever
void loop() {

    // Read Temp/Humidity
    float hum;
    hum = smeHumidity.readHumidity();
    SerialUSB.print("Humidity   : ");
    SerialUSB.print(hum);
    SerialUSB.println(" %");

    float temp;
    temp = smeHumidity.readTemperature();
    SerialUSB.print("Temperature: ");
    SerialUSB.print(temp);
    SerialUSB.println(" celsius");

    // Concat values to payload
    char payload[4];
    sprintf(payload, "%d%d", (int8_t)hum, (int8_t)temp);

    // Send Temp/Humidity over Sigfox
    // sfxSendData() is sub-optimal as it transforms char string to hex
    SerialUSB.println("sending data over the network");
    ledGreenLight(HIGH);
    sfxAntenna.sfxSendData(payload, strlen((char*)payload));

    bool answerReady = sfxAntenna.hasSfxAnswer();

    if (answerReady) {
        if (sfxAntenna.getSfxMode() == sfxDataMode) {

            switch (sfxAntenna.sfxDataAcknoledge()) {
            case SFX_DATA_ACK_START:
                SerialUSB.println("Message being sent...");
                break;

            case SFX_DATA_ACK_PROCESSING:
                SerialUSB.print('.');
                break;

            case SFX_DATA_ACK_OK:
                ledGreenLight(LOW);
                SerialUSB.println(' ');
                SerialUSB.println("Sent Complete!");
                break;

            case SFX_DATA_ACK_KO:
                ledGreenLight(LOW);
                ledRedLight(HIGH);
                SerialUSB.println(' ');
                SerialUSB.println("Error: message couldn't be sent");
                break;
            }
        }
    }
}
