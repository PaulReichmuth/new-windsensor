#include <senseBoxIO.h>

#define MEASUREMENT_INTERVAL_SEC 10 //Change rate of measurement here

const int num_measurements = (60*10)/MEASUREMENT_INTERVAL_SEC;

int measurements[num_measurements];

void setup()

{
    Serial.begin(9600);
    senseBoxIO.powerAll();
    senseBoxIO.statusGreen();
}

void loop()
{
    Serial.print("Windgeschwindigkeit: ");
    Serial.println(getWindspeed(A1));
    delay(100);
}

float getWindspeed(int sensorPin)
{
    float wind = 0;
    int sensorwert;
    //int t = 100;
    sensorwert = analogRead(sensorPin);
    Serial.println(sensorwert);
    float volt = 3.3 * ((float)sensorwert) / 1024;
    Serial.println(volt);
    //delay(t);
    wind = volt * 25;

    return wind;
}