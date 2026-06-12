#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <esp_wifi.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// MAC del receptor, sacada directamente del micro del Nodo Receptor
uint8_t macReceptor[] = {0xD4, 0xE9, 0xF4, 0xB5, 0x2B, 0xBC};

//Se declaran las variables globales
typedef struct datosSensores {
  int id;
  float pres;//Bar
  float temp;//ºC
  float hum; //% Humedad Relativa
  float volt; //Voltaje TENG (V)
} datosSensores;
datosSensores datos;

Adafruit_BME280 bme; 
esp_now_peer_info_t peerInfo;

// Configuración del TENG
const int PIN_TRIBO = 34;
const float V_REF = 3.3;
//const float OFFSET_V = 1.92;/Calibración ajustada para divisores de 10 MΩ
float offset_dinamico = 0;

unsigned long tiempoAnterior = 0;
const long frecuencia = 10;//100 Hz de frecuencia de envío, datos cada 10 milisegundos

void setup() {
  Serial.begin(115200);

  //Calibración para offset dinámico, 
  //se promedian 100 lecturas al arrancar
  float suma = 0;
  for(int i=0; i<100; i++) {
    suma += (analogRead(34) * 3.3) / 4095.0;
    delay(10);
  }
  offset_dinamico = suma / 100.0;
  Serial.print("Offset dinamico calculado: "); 
  Serial.println(offset_dinamico);

  
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);
  
  if (esp_now_init() != ESP_OK) return;
  
  memcpy(peerInfo.peer_addr, macReceptor, 6);
  peerInfo.channel = 1;  
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  //Inicialización Sensor I2C 
  Wire.begin();
  Wire.setClock(400000); 
  
  //La dirección I2C suele ser 0x76, si no es esta es 0x77
  if (!bme.begin(0x76)) {
    Serial.println("CRÍTICO: BME280 no detectado.");
    while (1); 
  }

  //Configuración de muestreo adaptada al BME280
  bme.setSampling(Adafruit_BME280::MODE_NORMAL, 
                  Adafruit_BME280::SAMPLING_X1,//Temperatura
                  Adafruit_BME280::SAMPLING_X1,//Presión
                  Adafruit_BME280::SAMPLING_X1,//Humedad
                  Adafruit_BME280::FILTER_OFF,  
                  Adafruit_BME280::STANDBY_MS_0_5); 
                  
  analogReadResolution(12); 
  pinMode(PIN_TRIBO, INPUT);

  datos.id = 1; 
}

void loop() {
  unsigned long tiempoActual = millis();

  if (tiempoActual - tiempoAnterior >= frecuencia) {
    tiempoAnterior = tiempoActual;

    //Adquisición datos del BME280
    float presRealPa = bme.readPressure();
    datos.pres = (presRealPa / 100000.0) + 1.20;//Conversion a Bares
    datos.temp = bme.readTemperature();
    datos.hum = bme.readHumidity(); //Adquisición real de la humedad

    //Adquisición datos en bruto nanogenerdor triboelectrico y conversion a voltaje
    int lecturaTRIBO = analogRead(PIN_TRIBO);
    float voltajeLeido = (lecturaTRIBO * V_REF) / 4095.0;
    datos.volt = voltajeLeido - offset_dinamico; 

    //Envío por ESP-NOW
    esp_now_send(macReceptor, (uint8_t *) &datos, sizeof(datos));
  }
}
  
