/*
 * Downloaded from: technik-fan.de/index.php/Open_Energy_Monitor_mit_dem_ESP32
 */
#include <Arduino.h>
#include <WiFi.h>

//Setup variables
int numberOfSamples = 9000;

const char* host = "emoncms-server.de";

//WiFi Watchdog
static bool sta_was_connected = false;

//Set Voltage and current input pins
int inPinI1 = 34;
int inPinI2 = 35;
int inPinI3 = 39;

//First Run Counter
int firstrun = 0;

//Power Calculation
float PowerSum = 0;

//WiFi Part
const char* ssid     = "IoT";
const char* password = "0603075972";

// CT: Voltage depends on current, burden resistor, and turns
#define CT_BURDEN_RESISTOR    62
#define CT_TURNS              1800
#define VOLTAGE               225

//Calibration coeficients
//These need to be set in order to obtain accurate results
//Set the above values first and then calibrate futher using normal calibration method described on how to build it page.
double ICAL = 1.08;

// Initial gueses for ratios, modified by VCAL/ICAL tweaks
double I_RATIO = (long double)CT_TURNS / CT_BURDEN_RESISTOR * 3.3 / 4096 * ICAL;

//Filter variables 1
double lastFilteredI, filteredI;
double sqI,sumI;
//Sample variables
int lastSampleI,sampleI;

//Filter variables 2
double lastFilteredI1, filteredI1;
double sqI1,sumI1;
//Sample variables
int lastSampleI1,sampleI1;

//Filter variables 3
double lastFilteredI3, filteredI3;
double sqI3,sumI3;
//Sample variables
int lastSampleI3,sampleI3;

//Useful value variables
double Irms1;
double Irms2;
double Irms3;
unsigned long timer;

//EspClass ESPm;
String url = "";

//WiFi
void WIFI_Connect()
{
  digitalWrite(2,1);
  WiFi.disconnect();
  Serial.println("Booting Sketch...");
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);
    // Wait for connection
  for (int i = 0; i < 25; i++)
  {
    if ( WiFi.status() != WL_CONNECTED ) {
      delay ( 250 );
      digitalWrite(2,0);
      Serial.print ( "." );
      delay ( 250 );
      digitalWrite(2,1);
    }
  }
  digitalWrite(2,0);
}


void setup() {
  //Set Information LED
  pinMode(2, OUTPUT);
  //Set Analog Inputs
  pinMode(inPinI1,INPUT);
  adcAttachPin(inPinI1);
  pinMode(inPinI2,INPUT);
  adcAttachPin(inPinI2);
  pinMode(inPinI3,INPUT);
  adcAttachPin(inPinI3);
 
  Serial.begin(115200);
  delay(500);
  //WiFi Part
  WIFI_Connect();

}

void loop() {
timer = millis();

//Wifi WatchDog
WiFi.onEvent(WiFiEvent);

//Check if WiFi is here
//Automatically reconnect the ESP32 if the WiFi Router is not there...
if (WiFi.status() != WL_CONNECTED)
    {
      WiFi.onEvent(WiFiEvent);
      WIFI_Connect();
    } else {
    
//**************************************************************************
//Phase1 
  for (int n=0; n<numberOfSamples; n++)
{
   
   //Used for offset removal
   lastSampleI=sampleI;
   
   //Read in voltage and current samples.   
   sampleI = analogRead(inPinI1);
   
   //Used for offset removal
   lastFilteredI = filteredI;
  
   //Digital high pass filters to remove 1.6V DC offset.
   filteredI = 0.9989 * (lastFilteredI+sampleI-lastSampleI);
   
   //Root-mean-square method current
   //1) square current values
   sqI = filteredI * filteredI;
   //2) sum 
   sumI += sqI;
   delay(0.00002);
}

//Calculation of the root of the mean of the voltage and current squared (rms)
//Calibration coeficients applied. 
Irms1 = (I_RATIO * sqrt(sumI / numberOfSamples)) - 0.16;
if ((Irms1 < 0) || (firstrun < 2)){ Irms1 = 0; }; //Set negative Current to zero and ignore the first 2 calculations
sumI = 0;

Serial.println("Irms1:"+String(Irms1));

//**************************************************************************
//Phase2
 for (int n=0; n<numberOfSamples; n++)
{
   
   //Used for offset removal
   lastSampleI1=sampleI1;
   
   //Read in voltage and current samples.   
   sampleI1 = analogRead(inPinI2);
   
   //Used for offset removal
   lastFilteredI1 = filteredI1;
  
   //Digital high pass filters to remove 1.6V DC offset.
   filteredI1 = 0.9989 * (lastFilteredI1+sampleI1-lastSampleI1);
   
   //Root-mean-square method current
   //1) square current values
   sqI1 = filteredI1 * filteredI1;
   //2) sum 
   sumI1 += sqI1;
   delay(0.00002);
}

//Calculation of the root of the mean of the voltage and current squared (rms)
//Calibration coeficients applied. 
Irms2 = (I_RATIO * sqrt(sumI1 / numberOfSamples)) - 0.16; 
if ((Irms2 < 0) || (firstrun < 2)){ Irms2 = 0; }; //Set negative Current to zero and ignore the first 2 calculations
sumI1 = 0;

Serial.println("Irms2:"+String(Irms2));

//**************************************************************************
//Phase3
 for (int n=0; n<numberOfSamples; n++)
{
   
   //Used for offset removal
   lastSampleI3=sampleI3;
   
   //Read in voltage and current samples.   
   sampleI3 = analogRead(inPinI3);
   
   //Used for offset removal
   lastFilteredI3 = filteredI3;
  
   //Digital high pass filters to remove 1.6V DC offset.
   filteredI3 = 0.9989 * (lastFilteredI3+sampleI3-lastSampleI3);
   
   //Root-mean-square method current
   //1) square current values
   sqI3 = filteredI3 * filteredI3;
   //2) sum 
   sumI3 += sqI3;
   delay(0.0002);
}

//Calculation of the root of the mean of the voltage and current squared (rms)
//Calibration coeficients applied. 
Irms3 = (I_RATIO * sqrt(sumI3 / numberOfSamples)) - 0.16;
if ((Irms3 < 0) || (firstrun < 2)){ Irms3 = 0; }; //Set negative Current to zero and ignore the first 2 calculations
sumI3 = 0;
Serial.println("Irms3:"+String(Irms3));

Serial.println("Summe:"+String(Irms1+Irms2+Irms3));

//Calculate Power
PowerSum = ((Irms1+Irms2+Irms3) * VOLTAGE);

//Send the data to the server...
  if ((WiFi.status() == WL_CONNECTED) && (firstrun >= 2)) { //WiFi Check, ignore the first data
    
    // Use WiFiClient class to create TCP connections
    WiFiClient client;
    const int httpPort = 80;
    if (!client.connect(host, httpPort)) {
        Serial.println("connection failed");
        return;
    }
    // We now create a URI for the request
    String url = "/emoncms/input/post.json?node=1&apikey=xxxxxxxxxxxxxxxxxxxxxx&json={";
    //url += value;
    url = url + "Phase1:" + Irms1 + ",";
    url = url + "Phase2:" + Irms2 + ",";
    url = url + "Phase3:" + Irms3 + ",";
    url = url + "PowerSum:" + PowerSum + "}";

    Serial.print("Requesting URL: ");
    Serial.println(url);

    // This will send the request to the server
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");
    unsigned long timeout = millis();
    while (client.available() == 0) {
        if (millis() - timeout > 5000) {
            Serial.println(">>> Client Timeout !");
            client.stop();
            return;
        }
    }

    /*
    // Read all the lines of the reply from server and print them to Serial
    while(client.available()) {
        String line = client.readStringUntil('\r');
        Serial.print(line);
    }*/
    
    }
  
  if (firstrun <= 2) {firstrun++;}; //Counter for Trash Data
  delay(1000);
    }//WiFi Watchdog
}

//Wifi Watchdog
void WiFiEvent(WiFiEvent_t event) {
  Serial.printf("[WiFi-event] event: %d  - ", event);
  switch(event) {
  case SYSTEM_EVENT_STA_GOT_IP:
      Serial.println("WiFi connected");
      Serial.print("IP address: "); Serial.println(WiFi.localIP());
      break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("WiFi lost connection");
      WiFi.begin();   // <<<<<<<<<<<  added  <<<<<<<<<<<<<<<
      ESP.restart();
      break;
  case SYSTEM_EVENT_STA_START:
      Serial.println("ESP32 station start");
      break;
  case SYSTEM_EVENT_STA_CONNECTED:
      Serial.println("ESP32 station connected to AP");
      break;
  case SYSTEM_EVENT_STA_LOST_IP:
      Serial.println("ESP32 Lost IP");
      WiFi.begin();   // <<<<<<<<<<<  added  <<<<<<<<<<<<<<<
      ESP.restart();
      break;
  }
}
