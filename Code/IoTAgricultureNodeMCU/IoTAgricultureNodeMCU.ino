#include <ESP8266WiFi.h>
#include <DHT.h>
//#include <ThingSpeak.h>

#define SoilMoisturePin A0  // used for Arduino and ESP8266
#define RelayPin 4  // Relay Pin
#define DHTPIN 5
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
WiFiClient client;

String writeApiKey = "ZD9H28GE89NGE5W5"; // Enter your Write API key from ThingSpeak
String readApiKey = "G49I8ZULFYU75XH9"; // Enter your Read API key from ThingSpeak
unsigned long myChannelNumber = 1837837;
float humidity = 0.0;
float temperature = 0.0;
float soilMoisture = 0.0;
bool pumpStatus=false;//False==OFF,True==ON

const char *ssid = "TDREnterprises"; // replace with your wifi ssid and wpa2 key
const char *pass = "TDR@1234";
const char* iotServer = "api.thingspeak.com";

void setup() 
{ 
  Serial.begin(9600);
  pinMode(RelayPin,OUTPUT);
  dht.begin();
  delay(10);
  Serial.println("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  //ThingSpeak.begin(client);
}

void loop() 
{
  delay(2000);
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  soilMoisture = analogRead(SoilMoisturePin);
  if (isnan(humidity) || isnan(temperature))
  {
    Serial.println("Failed to read from DHT sensor!");
    humidity=0;
    temperature=0;
    //return;
  }
  Serial.print("Humidity:");
  Serial.println(humidity);
  Serial.print("Temperature:");
  Serial.println(temperature);
  Serial.print("SoilMoisture:");
  Serial.println(soilMoisture);
  
  if(waterPumpManager())
  {
    digitalWrite(RelayPin, HIGH); 
    pumpStatus=false;
  }
  else
  {
   digitalWrite(RelayPin, LOW); 
   pumpStatus=true;
  }
  sendDataToIotServer();
  delay(30000);
} 
bool waterPumpManager()
{
  //write logic / function here to turn On the Pump based on Temperature, Humidity and SoilMoisture
  if(soilMoisture>700)
    return true;
  else
    return false;

}
void sendDataToIotServer()
{
  if (client.connect(iotServer,80)>0) // “184.106.153.149” or api.thingspeak.com
  {
    Serial.println("ThingSpeak Connected");

    String postStr = writeApiKey;
    postStr +="&field1=";//SoilMoisture
    postStr += String(soilMoisture);
    postStr +="&field2=";//Humidity
    postStr += String(humidity);
    postStr +="&field3=";//Temperature
    postStr += String(temperature);
    postStr +="&field4=";//PumpStatus
    postStr += String(pumpStatus);
    postStr += "\r\n\r\n";

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: "+writeApiKey+"\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
  
    Serial.println("Post String:");
    Serial.println( postStr) ;
   
    Serial.println("Sent to Thingspeak.");
  
  }
  client.stop();

}
