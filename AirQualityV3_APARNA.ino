#include "DHTesp.h"
DHTesp dht;

int analogPin = 35; //for CO2 - ESP32 Development Board

//wifi------------------
#include <WiFi.h>
#include <WiFiClient.h>
#include <HTTPClient.h>

const char* ssid =  "Peach";
const char* password =  "9652665840";
String WeatherStationNo = "5"; // 1 - BVRIT Campus, 2 - Narsapur Town, 4 - Madhura Nagar

int status = WL_IDLE_STATUS;
IPAddress server(103,48,51,73);  // InfraBIM

// Initialize the client library
WiFiClient client;
//-------------------

//--Deep Sleep----------
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  300        /* Time ESP32 will go to sleep (in seconds) */

RTC_DATA_ATTR int bootCount = 0;
//-----------------

void setup()
{
  Serial.begin(115200);
  
  //Deep Sleep-----------------------------
   //Increment boot number and print it every reboot
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");
  //------------------------
  
  dht.setup(27, DHTesp::DHT22);

  StartWifi();
  delay(2000);

 
  //Enable Deep Sleep---------------------------
  Serial.println("Going to sleep now");
  delay(1000);
  esp_deep_sleep_start();
  Serial.println("This will never be printed");
  //----------------------------
  
}

void StartWifi()
{
  Serial.println("Attempting to connect to WPA network...");
  Serial.print("SSID: ");
  Serial.println(ssid);

  status = WiFi.begin(ssid, password);

  Serial.println(status);

  int i=0;
  while (WiFi.status() != WL_CONNECTED && i<60) {
    i++;
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  
 if (WiFi.status() == WL_CONNECTED)
 {
    //Serial.println("Connected to WIFI");
      
    // if you get a connection, report back via serial:
    if (client.connect(server, 80)) {
        Serial.println("connected to server");
        Serial.println(WiFi.status());
    
        //Post Data
        String msg = String(dht.getTemperature()) + "," + String(dht.getHumidity()) + "," + String(getCO2());
        PostDataToWebsite(WeatherStationNo + "," + msg); //Weather Station No., Sensor Data 
        
    }
    else
    {
      Serial.println("Not Connected to Website");
     
    }
 }
else
{
  Serial.println("Not Connected to Wifi");
   
}

 // print the received signal strength:
      long rssi = WiFi.RSSI();
      Serial.print("RSSI:");
      Serial.print(150 - (5/3) * abs(rssi));
       Serial.println("%");
  //-------------
  
}

void loop()
{

}

float getCO2()
{

  int adcVal = analogRead(analogPin) + 100; // +100 for 250ppm
  //Serial.println(adcVal);

  float voltage = adcVal * (3.3 / 4095); //- 1.172 ;
  //Serial.println(voltage);

  if (voltage == 0)
  {
    Serial.println("A problem has occurred with the sensor.");
    return 0;
  }
  else if (voltage < 0.4)
  {
    Serial.println("Pre-heating the sensor...");
    return 0;
  }
  else
  {

    float voltageDiference = voltage - 0.4;
    float concentration = (voltageDiference * 5000) / 1.6;

    return concentration; //-250ppm

  }
}

void PostDataToWebsite(String SensorData)
{

  HTTPClient http;    //Declare object of class HTTPClient

  String postData;

  //Post Data
  postData = "data=" + SensorData;
  Serial.println(postData);


  http.begin("http://www.infrabim.in/senddata.aspx");                     //Specify request destination
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");    //Specify content-type header

  int httpCode = http.POST(postData);   //Send the request
  String payload = http.getString();    //Get the response payload

  if (httpCode == 200)
  {
    Serial.println("Sent to Website Successfully");
  }
  else
  {
    Serial.println("Unable Sent Data to Website");
  }

  http.end();  //Close connection


}
