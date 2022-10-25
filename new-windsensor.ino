#include <senseBoxIO.h>
#include <SPI.h>
#include <SD.h>
#include <NMEAGPS.h>
#include <GPSport.h>
#include <Streamers.h>

#define MEASUREMENT_INTERVAL_SEC 10 // Change rate of measurement here

static NMEAGPS gps;
static gps_fix fix;

const int num_measurements = (60 * 10) / MEASUREMENT_INTERVAL_SEC; // number of measurements to get in 10 Minutes

int measurements_taken = 0;

float measurements[num_measurements];

uint8_t nextSecond = 0;

unsigned int day, month, year, hour, minute, second;
File myFile;
const int chipSelect = 28; // D28 on senseBox MCU mini
String fileName = "03_00001.csv";
unsigned int fileCount = 0;
unsigned short lineCount = 0;

static void doSomeWork()
{
    // Print all the things!

    trace_all(DEBUG_PORT, gps, fix);

    // Keep calculating average
    if (fix.valid.time)
    {
        // DEBUG_PORT.println("Valid Time");
        if (fix.dateTime.seconds == nextSecond)
        {
            measurements[measurements_taken] = getWindspeed(A1);
            DEBUG_PORT.print("Took Sample Num.: ");
            DEBUG_PORT.println(measurements_taken);
            DEBUG_PORT.print("Windgeschwindigkeit grade: ");
            DEBUG_PORT.println(measurements[measurements_taken]);
            nextSecond = fix.dateTime.seconds + MEASUREMENT_INTERVAL_SEC;
            if (nextSecond >= 60)
            {
                nextSecond = nextSecond - 60;
            }
            DEBUG_PORT.println(nextSecond);
            measurements_taken++;
            if (fix.valid.date)
            {
                month = fix.dateTime.month;
                day = fix.dateTime.day;
                year = fix.dateTime.year;
                if (fix.dateTime.hours < 10)
                    ;
                hour = fix.dateTime.hours;
                if (fix.dateTime.minutes < 10)
                    ;
                minute = fix.dateTime.minutes;
                if (fix.dateTime.seconds < 10)
                    ;
                second = fix.dateTime.seconds;
                // create timestamp
                char timestamp[64];
                sprintf(timestamp, "%04d-%02d-%02dT%02d:%02d:%02dZ", year, month, day, hour, minute, second);

                // prepare data for writing to SD card
                String dataString = "";

                dataString = "Windspeed";
                dataString += String(",");
                dataString += String(getWindspeed(A1), 2);
                dataString += String(",");
                dataString += String(timestamp);
                dataString += String("\n");
                dataString += "Raw Sensor Data";
                dataString += String(",");
                dataString += String(analogRead(A1));
                dataString += String(",");
                dataString += String(timestamp);
                dataString += String("\n");
                dataString += "Voltage";
                dataString += String(",");
                dataString += String(3.3 * ((float)analogRead(A1)) / 1024);
                dataString += String(",");
                dataString += String(timestamp);
                Serial.println(dataString);

                if (lineCount >= 2490)
                {
                    Serial.println("End of file length.");
                    getNewFileName();
                    lineCount = 0;
                }

                SdFile::dateTimeCallback(dateTimeFun);
                myFile = SD.open(fileName, FILE_WRITE);

                if (myFile)
                {
                    Serial.print("Logging data to " + fileName);
                    myFile.println(dataString);
                    myFile.close();
                    lineCount += 3;
                    Serial.println(" done.");
                }
                else
                {
                    Serial.println("error opening " + fileName);
                    delay(10000);
                }
                Serial.println();
            }
        }
        if (measurements_taken == num_measurements)
        {
            // print avg
            DEBUG_PORT.print("Durchschnitl. Windgeschwindigkeit: ");
            DEBUG_PORT.println(average(measurements, num_measurements));
            measurements_taken = 0;
        }
    }
}

static void GPSloop()
{
    while (gps.available(gpsPort))
    {
        fix = gps.read();
        doSomeWork();
    }
}

float getWindspeed(int sensorPin)
{
    float wind = 0;
    int sensorwert;
    // int t = 100;
    sensorwert = analogRead(sensorPin);
    // DEBUG_PORT.println(sensorwert);
    float volt = 3.3 * ((float)sensorwert) / 1024;
    // DEBUG_PORT.println(volt);
    // delay(t);
    wind = volt * 25;

    return wind;
}

float average(float *array, int len) // assuming array is int.
{
    long sum = 0L; // sum will be larger than an item, long for safety.
    for (int i = 0; i < len; i++)
        sum += array[i];
    return ((float)sum) / len; // average will be fractional, so float may be appropriate.
}

void getNewFileName()
{
    fileCount++;
    char newFileName[12];
    sprintf(newFileName, "03_%05d.csv", fileCount);
    fileName = String(newFileName);
    Serial.println("Updating filename to " + String(newFileName));
}

void dateTimeFun(uint16_t *date, uint16_t *time)
{

    // return date using FAT_DATE macro to format fields
    *date = FAT_DATE(fix.dateTime.year, fix.dateTime.month, fix.dateTime.day);

    // return time using FAT_TIME macro to format fields
    *time = FAT_TIME(fix.dateTime.hours, fix.dateTime.minutes, fix.dateTime.seconds);
}

void setup()

{
    senseBoxIO.powerAll();
    DEBUG_PORT.begin(9600);
    DEBUG_PORT.print(F("NMEA.INO: started\n"));
    DEBUG_PORT.print(F("  fix object size = "));
    DEBUG_PORT.println(sizeof(gps.fix()));
    DEBUG_PORT.print(F("  gps object size = "));
    DEBUG_PORT.println(sizeof(gps));
    DEBUG_PORT.println(F("Looking for GPS device on " GPS_PORT_NAME));
    DEBUG_PORT.flush();

    Serial.print("Initializing SD card...");
    if (!SD.begin(chipSelect))
    {
        Serial.println("initialization failed. Things to check:");
        Serial.println("1. is a card inserted?");
        Serial.println("2. is your wiring correct?");
        Serial.println("3. did you change the chipSelect pin to match your shield or module?");
        Serial.println("Note: press reset or reopen this Serial Monitor after fixing your issue!");
        while (true)
        {
            senseBoxIO.statusRed();
        }
    }
    else
        Serial.println("done.");

    do
    {
        getNewFileName();
    } while (SD.exists(fileName));

    Serial.print("Testing SD card...");
    myFile = SD.open(fileName, FILE_WRITE);
    if (myFile)
    {
        myFile.close();
        delay(1000);
        SD.remove(fileName);
        Serial.println("done.");
    }
    else
    {
        Serial.println("error opening " + fileName);
        while (true)
            ;
    }
    gpsPort.begin(9600);
    DEBUG_PORT.println("Begin.");
    DEBUG_PORT.print("Taking ");
    DEBUG_PORT.print(num_measurements);
    DEBUG_PORT.println("Samples");
}

void loop()
{
    GPSloop();
}
