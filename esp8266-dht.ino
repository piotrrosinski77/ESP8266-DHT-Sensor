// Import required libraries
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include "WiFiUdp.h"
#include "NTPClient.h"

// Replace with your network credentials
//const char* ssid = "WLAN1-R66FJ9";
//const char* password = "AlejeLipowe10!";

const char* ssid = "NETIASPOT-2.4GHz-5D9Q";
const char* password = "eszewerie95";

#define DHTPIN 0     // Digital pin connected to the DHT sensor (D1)
#define LEDPIN 2 // Digital pin connected to the LED (D2)

// Uncomment the type of sensor in use:
#define DHTTYPE    DHT11     // DHT 11
//define DHTTYPE    DHT22     // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)

const long utcOffsetInSeconds = 7200;

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x3F for a 16 chars and 2 line display

DHT dht(DHTPIN, DHTTYPE);

// current temperature & humidity, updated in loop()
float t = 0.0;
float h = 0.0;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;    // will store last time DHT was updated

// Updates DHT readings every 10 seconds
const long interval = 10000;  

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
     background-color: #333333; /* Ciemnoszary kolor tła */
     color: #ffffff; /* Biały kolor tekstu */
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>ESP8266 DHT Server</h2>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="dht-labels">Temperature</span> 
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-tint" style="color:#00add6;"></i> 
    <span class="dht-labels">Humidity</span>
    <span id="humidity">%HUMIDITY%</span>
    <sup class="units">%</sup>
  </p>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 10000 ) ;
</script>
</html>)rawliteral";

// Replaces placeholder with DHT values
String processor(const String& var){
  //Serial.println(var);
  if(var == "TEMPERATURE"){
    return String(t);
  }
  else if(var == "HUMIDITY"){
    return String(h);
  }
  return String();
}

char daysOfTheWeek[7][12] = {"Niedz.", "Pon.", "Wtorek", "Sroda", "Czw.", "Piatek", "Sobota"};

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

void setup(){

  // LED setup
  pinMode(LEDPIN, OUTPUT);
  // Serial port for debugging purposes
  Serial.begin(115200);
  
  lcd.init();
  lcd.clear();         
  lcd.backlight();      // Make sure backlight is on
  lcd.home();
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println(".");
  }

  // Print ESP8266 Local IP Address
  Serial.println(WiFi.localIP());

  timeClient.begin();

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(t).c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(h).c_str());
  });

  // Start server
  server.begin();
}
 
void loop(){  

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Lost connection");
    digitalWrite(LEDPIN, LOW);
    
    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    Serial.println("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println(".");
      if (WiFi.status() == WL_CONNECTED)
      {
        Serial.println("Back online");
      }
    }
  }

  lcd.setCursor(0, 1);
  timeClient.update();
  lcd.print(daysOfTheWeek[timeClient.getDay()]);
  lcd.print(", ");
  
  if ((timeClient.getHours()) < 10){
    lcd.print("0");
    lcd.print((timeClient.getHours()));
    lcd.print(":");
  }

  else {
    lcd.print(timeClient.getHours());
    lcd.print(":");
  }

  if ((timeClient.getMinutes()) < 10){
    lcd.print("0");
    lcd.print((timeClient.getMinutes()));
    lcd.print(":");
  }

  else {
    lcd.print((timeClient.getMinutes()));
    lcd.print(":");
  }

  if ((timeClient.getSeconds()) < 10){
    lcd.print("0");
    lcd.print((timeClient.getSeconds()));
  }

  else {
    lcd.print((timeClient.getSeconds()));
  }

  delay(1000);

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time you updated the DHT values
    previousMillis = currentMillis;

    // Read temperature as Celsius (the default)
    float newT = dht.readTemperature();
    float newH = dht.readHumidity();

    if (isnan(newT) || (isnan(newH))) {
      Serial.println("Failed to read from DHT sensor!");
      digitalWrite(LEDPIN, LOW);
    }

    if (!(isnan(newT) && (isnan(newH)))){
      lcd.clear();
      lcd.home();

      t = newT;
      Serial.println(t);
      lcd.print(t);
      lcd.write(223);
      lcd.print("C ");
    
      h = newH;
      Serial.println(h);
      lcd.print(h);
      lcd.print("%");
      digitalWrite(LEDPIN, HIGH);
    }
  }
}