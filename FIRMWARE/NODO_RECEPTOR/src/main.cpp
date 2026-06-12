#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <esp_wifi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define PIN_BUZZER 25 
#define PIN_RELE 26 // Pin para la baliza LTA-505

//Se declaran las variables globales
typedef struct datosSensores {
  int id;
  float pres;//Bar
  float temp;//ºC
  float hum;//%Humedad Relativa
  float volt;//Voltaje TENG (V)
} datosSensores;

volatile datosSensores datosRuedas[5];//Array de 5 para mapear los ids del 1 al 4 (puede haber hasta 4 neumaticos)
        
unsigned long t_Telemetria = 0;
unsigned long t_OLED = 0;
unsigned long t_Buzzer = 0;
bool estadoBuzzer = false;

const float UMBRAL_TENG = 1.50; // Umbral activacion alerta TENG, asegura que tiene que ser un golpe fuerte para que se active la alarma
unsigned long t_UltimoImpacto = 0;
const unsigned long TIEMPO_RETENCION = 2000; // 2 segundos de margen para ver la alarma visual/sonora

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  datosSensores temp;
  memcpy(&temp, incomingData, sizeof(temp));
  
  // se filtran los IDs válidos por si entra ruido o paquetes corruptos
  if (temp.id >= 1 && temp.id <= 4) {
    datosRuedas[temp.id].id   = temp.id;
    datosRuedas[temp.id].pres = temp.pres;
    datosRuedas[temp.id].temp = temp.temp;
    datosRuedas[temp.id].hum  = temp.hum;
    datosRuedas[temp.id].volt = temp.volt;
  }
}

void setup() {
  Serial.begin(115200);
  
  //Relé de la baliza eempieza apagado 
  pinMode(PIN_RELE, OUTPUT);
  digitalWrite(PIN_RELE, LOW); 

  // Configuración del buzzer pasivo
  ledcAttachPin(PIN_BUZZER, 0);
  ledcWriteTone(0, 0); 

  // Ojo con los cables I2C (SDA/SCL), si hacen mal contacto la pantalla no inicia
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Fallo al iniciar OLED. Revisa el cableado.");
    while(1); 
  }
  
  Wire.setClock(400000); // Se sube la velocidad I2C para que el refresco no ralentice el bucle
  display.clearDisplay();
  display.setTextColor(WHITE);

  // Configuración ESP-NOW
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error iniciando ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  unsigned long t_Actual = millis();
  
  //Se cogen los datos de la rueda 
  float p = datosRuedas[1].pres;
  float t = datosRuedas[1].temp;
  float h = datosRuedas[1].hum;
  float v = datosRuedas[1].volt; 

  //Se comprueban si los voltajes recibidos superan el umbral fijado al inicio
  if (abs(v) >= UMBRAL_TENG) {
    t_UltimoImpacto = t_Actual;
    // Serial.println("IMPACTO TENG DETECTADO");
  }
  
  bool alertaTENG = false;
  if (t_Actual - t_UltimoImpacto <= TIEMPO_RETENCION) {
    alertaTENG = true;
  }
  
  //Condición de presión (p > 0.1 evita que pite al arrancar antes de recibir el primer paquete)
  bool alertaPresBaja = false;
  bool alertaPresAlta = false;
  
  if (p > 0.1 && p < 2.00) {
    alertaPresBaja = true;
  }
  if (p > 2.30) {
    alertaPresAlta = true;
  }
  
  bool alertaPresion = false;
  if (alertaPresBaja || alertaPresAlta) {
    alertaPresion = true;
  }

  //Monitorizacion a 100 Hz para el Serial Studio
  if (t_Actual - t_Telemetria >= 10) {
    t_Telemetria = t_Actual;
    Serial.print("/*TPMS,");
    Serial.print(p); Serial.print(",");
    Serial.print(t); Serial.print(",");
    Serial.print(h); Serial.print(",");
    Serial.print(v, 3); Serial.print(",");
    
    //Alertas Serial Studio 
    int valorAlerta = 0;
    if (alertaTENG) {
      valorAlerta = 100;
    } else if (alertaPresAlta) {
      valorAlerta = 75;
    } else if (alertaPresBaja) {
      valorAlerta = 50;
    }
    
    Serial.print(valorAlerta); 
    Serial.println("*/");
  }

  //Refresco de la OLED a 2 Hz 
  //(evitar parpadeos y cuello de botella)
  if (t_Actual - t_OLED >= 500) {
    t_OLED = t_Actual;
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.println("SISTEMA TPMS - TFG");
    display.drawLine(0, 10, 128, 10, WHITE);
    
    //Se pintan valores en la pantalla OLED
    display.setCursor(0, 15);
    display.print("PRESION: "); display.print(p); display.println(" Bar");
    display.print("TEMP:    "); display.print(t, 1); display.println(" C");
    display.print("HUMEDAD: "); display.print(h, 0); display.println(" %");
    display.print("V. TENG: "); display.print(v, 2); display.println(" V");
    
    //Mensajes de estado inferiores (se invierten los colores para que resalte más)
    display.setCursor(0, 52);
    if (alertaTENG) {
       display.setTextColor(BLACK, WHITE);
       display.print("IMPACTO DETECTADO");
       display.setTextColor(WHITE);
    } else if (alertaPresBaja) {
       display.setTextColor(BLACK, WHITE);
       display.print("ALERTA PRESION BAJA");
       display.setTextColor(WHITE);
    } else if (alertaPresAlta) {
       display.setTextColor(BLACK, WHITE);
       display.print("ALERTA PRESION ALTA");
       display.setTextColor(WHITE);
    } else {
       display.print("SISTEMA OK");
    }
    display.display();
  }

  //Lógica de actuadores (Relé + Buzzer)
  if (alertaTENG) {
    digitalWrite(PIN_RELE, HIGH); // Baliza fija si hay un impacto fuerte
    if (t_Actual - t_Buzzer >= 80) { // Pitido rapidísimo
      t_Buzzer = t_Actual;
      estadoBuzzer = !estadoBuzzer;
      if (estadoBuzzer) ledcWriteTone(0, 2500); else ledcWriteTone(0, 0);
    }
  } 
  else if (alertaPresion) {
    //Presion baja = pita lento y grave;
    //Presion alta = pita rápido y agudo
    unsigned long intervalo = alertaPresBaja ? 500 : 150; 
    if (t_Actual - t_Buzzer >= intervalo) {
      t_Buzzer = t_Actual;
      estadoBuzzer = !estadoBuzzer;
      if (estadoBuzzer) {
        ledcWriteTone(0, alertaPresBaja ? 400 : 1500); 
        digitalWrite(PIN_RELE, HIGH); 
      } else {
        ledcWriteTone(0, 0);
        digitalWrite(PIN_RELE, LOW); 
      }
    }
  } 
  else {
    //Si todo está correcto, se apagan todos los avisos 
    digitalWrite(PIN_RELE, LOW); 
    if (estadoBuzzer) {
      ledcWriteTone(0, 0);
      estadoBuzzer = false; 
    }
  }
}