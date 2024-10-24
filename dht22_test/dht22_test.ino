#include "DHT.h"
#include <Adafruit_Sensor.h>

#define DHTPIN 0    // Pin, do którego podłączony jest DHT22 (GPIO2 to D3 na NodeMCU)
#define DHTTYPE DHT22   // DHT 22 (AM2302)

// Inicjalizacja czujnika DHT
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200); 
  dht.begin();
}

void loop() {
  // Odczyt temperatury i wilgotności
  float wilgotnosc = dht.readHumidity();
  float temperatura = dht.readTemperature();
  
  // Sprawdzenie czy odczyt się udał
  if (isnan(wilgotnosc) || isnan(temperatura)) {
    Serial.println("Nie można odczytać danych z czujnika DHT22!");
  } else {
    Serial.print("Wilgotność: ");
    Serial.print(wilgotnosc);
    Serial.print(" %\t");
    Serial.print("Temperatura: ");
    Serial.print(temperatura);
    Serial.println(" *C");
  }

  // Czas oczekiwania między odczytami (np. 2 sekundy)
  delay(2000);
}
