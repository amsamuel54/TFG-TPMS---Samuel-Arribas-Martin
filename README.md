# TFG-TPMS-Samuel-Arribas-Martin

# 1. Descripción

Este repositorio se crea con la finalidad de adjuntar los archivos relativos al Trabajo de Fin de Grado: Diseño e Implementación de un Sistema de Simulación de Monitoreo de Presión de Neumáticos (TPMS) Inalámbrico Basado en Protocolos de Comunicación Distribuida del alumno Samuel Arribas Martín.

# 2. Componentes y requisitos previos

El sistema se divide físicamente en dos unidades de procesamiento independientes basadas en el microcontrolador ESP32 DEVKIT V1:

# Nodo Emisor (Adquisición y Transmisión)
Se encarga de la lectura directa de los sensores físicos a alta velocidad y del envío inmediato de las tramas de datos por ESP-NOW.
* Microcontrolador: ESP32 DEVKIT V1.
* Sensores: Sensor ambiental BME280 (I2C) y Nanogenerador Triboeléctrico (TENG).
* Etapa Analógica: Diodos 1N4148 y resistencias 2 M ohmios.

# Nodo Receptor (Interfaz gráfica, Alertas y Monitorizacíon)
Recibe los paquetes de datos a traves de ESP-NOW, procesa los umbrales de seguridad, gestiona la interfaz del usuario y vuelca la información hacia el software de monitorización.
* Microcontrolador: ESP32 DEVKIT V1.
* Interfaz Visual (HMI): Pantalla OLED de 0.96" (controlador SSD1306 vía I2C).
* Actuadores de Alerta: Buzzer pasivo, baliza industrial LTE-505-3TJ y relé 5V de 1 canal con optoacoplador.

Además de los componentes de hardware, será necesaria la instalación de Visual Studio Code + PlatformIO y Serial Studio.

# 3. Pasos para la instalacion y uso.

PASO 1: Descargar e instalar Visual Studio Code y la extensión PlatformIO.
PASO 2: Cargar los proyectos <<NODO_EMISOR>> y <<NODO_RECEPTOR>> por separado en Visual Studio Code, compilar y enviar la información a cada microcontrolador respectivamente.
PASO 3: Descargar e instalar Serial Studio.
Paso 4: En la ventana lateral derecha, configurar el proyecto antes de establecer la conexión con la placa. Cargar el archivo <<Telemetría TPMS V2.ssproj>>, seleccionar el puerto COM adecuado y establecer la tasa de baudios a 115200. El resto de valores pueden permanecer por defecto.
PASO 5: Una vez esté realizada la configuración, pulsar el botón de la esquina superior derecha <<CONECTAR>>.
