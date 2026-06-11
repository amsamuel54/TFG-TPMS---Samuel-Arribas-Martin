# TFG-TPMS-Samuel-Arribas-Martin

## 1. Descripción

Este repositorio se crea con la finalidad de adjuntar los archivos relativos al Trabajo de Fin de Grado: Diseño e Implementación de un Sistema de Simulación de Monitoreo de Presión de Neumáticos (TPMS) Inalámbrico Basado en Protocolos de Comunicación Distribuida del alumno Samuel Arribas Martín

## 2.  Objetivos y Requerimientos Funcionales

El objetivo principal es obtener, procesar y monitorizar los datos de los sensores para visualizar el estado del sistema y activar contramedidas de seguridad. Esto se desglosa en los siguientes requerimientos:

* **Monitorización y Adquisición:**
    * *Variables Atmosféricas:* Integración del sensor **BME280** bajo bus **I2C** para lecturas precisas de presión, temperatura y humedad relativa.
    * *Detección de Impactos:* Lectura del transitorio de voltaje generado por un sensor triboeléctrico (TENG) a través de una etapa analógica de acondicionamiento de señal.
* **Comunicación y Procesamiento:**
    * *Enlace Inalámbrico de Baja Latencia:* Conexión robusta punto a punto entre el nodo emisor y el receptor vía **ESP-NOW** con una tasa de refresco y adquisición de **100 Hz**.
    * *Interfaz gráfica:* Visualización en una pantalla **OLED de 0.96"** emulando el comportamiento de los sistemas de asistencia a la conducción **ADAS** de un vehículo moderno.
* **Seguridad, Alertas y Validación:**
    * *Gestión de Alertas:* Activación de periféricos acústicos (**buzzer pasivo**) y un sistema combinado acústico/visual (**baliza industrial LTE-505-3TJ**) al detectar presiones fuera de rango o impactos severos.
    * *Análisis en Tiempo Real:* Monitorización y representación gráfica de datos a **100 Hz** mediante el entorno de software **Serial Studio**.
    * *Prototipado e Industrialización:* Soldadura en placas perforadas, diseño de carcasas a medida en **Fusion360**, laminado con **Ultimaker Cura** e impresión 3D para encapsular y proteger los nodos físicos.

---

## 3. Arquitectura del Hardware

El sistema se divide físicamente en dos unidades de procesamiento independientes basadas en el microcontrolador **ESP32**:

### Módulo A: Nodo Emisor (Adquisición y Transmisión)
Se encarga de la lectura directa de los sensores físicos a alta velocidad y del envío inmediato de las tramas de datos por radiofrecuencia.
* **Microcontrolador:** ESP32.
* **Sensores:** Sensor ambiental BME280 (I2C) y Nanogenerador Triboeléctrico (TENG).
* **Etapa Analógica:** Circuito de protección y fijación de tensión basado en diodos **1N4148** y resistencias **2 M ohmios**.

### Módulo B: Nodo Receptor (Interfaz gráfica, Alertas y Monitorizacíon)
Recibe las tramas inalámbricas, procesa los umbrales de seguridad, gestiona la interfaz del usuario y vuelca la información hacia el software de monitorización.
* **Microcontrolador:** ESP32.
* **Interfaz Visual (HMI):** Pantalla OLED de 0.96" (Controlador SSD1306 vía I2C).
* **Actuadores de Alerta:** Buzzer pasivo (5V, 4kHz) y salida digital hacia baliza industrial (visual/ac
