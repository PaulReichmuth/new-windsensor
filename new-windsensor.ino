#include <senseBoxIO.h>
#include <SPI.h>
#include <SD.h>
#include <NMEAGPS.h>
//-------------------------------------------------------------------------
//  The GPSport.h include file tries to choose a default serial port
//  for the GPS device.  If you know which serial port you want to use,
//  edit the GPSport.h file.

#include <GPSport.h>

//------------------------------------------------------------
// For the NeoGPS example programs, "Streamers" is common set
//   of printing and formatting routines for GPS data, in a
//   Comma-Separated Values text format (aka CSV).  The CSV
//   data will be printed to the "debug output device".
// If you don't need these formatters, simply delete this section.

#include <Streamers.h>

//------------------------------------------------------------
// This object parses received characters
//   into the gps.fix() data structure

static NMEAGPS gps;

//------------------------------------------------------------
//  Define a set of GPS fix information.  It will
//  hold on to the various pieces as they are received from
//  an RMC sentence.  It can be used anywhere in your sketch.

static gps_fix fix;

//----------------------------------------------------------------
//  This function gets called about once per second, during the GPS
//  quiet time.  It's the best place to do anything that might take
//  a while: print a bunch of things, write to SD, send an SMS, etc.
//
//  By doing the "hard" work during the quiet time, the CPU can get back to
//  reading the GPS chars as they come in, so that no chars are lost.

#define MEASUREMENT_INTERVAL_SEC 10 // Change rate of measurement here

const int num_measurements = (60 * 10) / MEASUREMENT_INTERVAL_SEC; // number of measurements to get in 10 Minutes

int measurements_taken = 0;

float measurements[num_measurements];

uint8_t nextSecond = 0;

static void doSomeWork()
{
    // Print all the things!

    trace_all(DEBUG_PORT, gps, fix);

    // Keep calculating average

    if (fix.dateTime.seconds >= nextSecond)
    {
        measurements[measurements_taken] = getWindspeed(A1);
        DEBUG_PORT.print("Took Sample Num.: ");
        DEBUG_PORT.println(i);
        DEBUG_PORT.print("Windgeschwindigkeit grade: ");
        DEBUG_PORT.println(measurements[i]);
        nextSecond = fix.dateTime.seconds + MEASUREMENT_INTERVAL_SEC;
        measurements_taken++;
    }
    if (measurements_taken == num_measurements)
    {
        // print avg
        DEBUG_PORT.print("Durchschnitl. Windgeschwindigkeit: ");
        DEBUG_PORT.println(average(measurements, num_measurements));
        measurements_taken = 0;
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

void setup()

{
    senseBoxIO.powerAll();
    SD.begin(28);
    DEBUG_PORT.begin(9600);
    DEBUG_PORT.print(F("NMEA.INO: started\n"));
    DEBUG_PORT.print(F("  fix object size = "));
    DEBUG_PORT.println(sizeof(gps.fix()));
    DEBUG_PORT.print(F("  gps object size = "));
    DEBUG_PORT.println(sizeof(gps));
    DEBUG_PORT.println(F("Looking for GPS device on " GPS_PORT_NAME));
    DEBUG_PORT.flush();
    gpsPort.begin( 9600 );
    DEBUG_PORT.println("Begin.");
    DEBUG_PORT.print("Taking ");
    DEBUG_PORT.print(num_measurements);
    DEBUG_PORT.println("Samples");
}

void loop()
{
    GPSloop();
}
