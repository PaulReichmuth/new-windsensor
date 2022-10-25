float wind = 0; 
int sensorPin = A4; 
int sensorwert; 
int t = 100; 

void setup()
{
  Serial.begin(9600); 
}

void loop()
{
  sensorwert = analogRead(sensorPin); 
  Serial.println(sensorwert);
  float volt = 3.3*((float)sensorwert)/1024;
  Serial.println(volt);
  delay(t); 
  wind = volt*25;
  Serial.print("Windgeschwindigkeit: "); 
  Serial.println(wind); 
}