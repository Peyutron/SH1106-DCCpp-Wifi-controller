/**
  * Version: DCC Controller ESP32 C3
  * Project instructions: 
    - http://lamaquetade.infotronikblog.com/
    - https://www.infotronikblog.com/2026/01/esp32-c3-super-mini-oled-sh1106.html
  *  
  * Project repository:  
  * web: https://www.infotronikblog.com
  * Creator: Carlos MC
  *
  * External Library:
    - Adafruit_GFX:     https://github.com/adafruit/Adafruit-GFX-Library
    - Adafruit_SH110X:  https://github.com/adafruit/Adafruit_SH110x
  *
  * ESP32 Library:
    - WiFi:             https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/src/WiFi.h
    - WiFiClient:       https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/examples/WiFiClient/WiFiClient.ino
  * 
  * 
  * Display -> OLED SH1106 1.3"  
  * Board -> ESP32C3 Dev Module
  * Use CDC on boot "enabled"
  * CPU Frequency "80MHz (wifi)"
  *
**/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include "logo.h"

// Configuración WiFi y servidor (modifica según tus necesidades)
const char* ssid = "YourSSID";
const char* password = "YourPass";
const char* serverIP = "192.168.1.5";
const int serverPort = 2560;

// Pines según tu configuración
#define PIN_CONFIRM 3
#define PIN_BACK 4
#define PIN_ROTARY_BTN 2
#define PIN_ROTARY_A 20
#define PIN_ROTARY_B 21
#define OLED_SDA 6
#define OLED_SCL 7

// Configuración pantalla SH1106
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

// Variables para el encoder rotatorio
volatile int encoderPos = 0;
int lastEncoderPos = 0;
int rotaryAState;
int rotaryALastState;

// Variables para los botones
bool confirmPressed = false;
bool backPressed = false;
bool rotaryBtnPressed = false;
bool lastConfirmState = HIGH;
bool lastBackState = HIGH;
bool lastRotaryBtnState = HIGH;

// Variables de menú
int menuItem = 0;
const int menuItems = 4;
const char* menuNames[] = {"Start!", "Main", "Turnouts", "SysInfo"};
bool inMenu = true;
bool inSubMenu = false;
int subMenuItem = 0;
bool startDCC = false;
bool showScreen = true;

// Variables de estado
float internalTemp = 0.0;
int8_t wifiRSSI = 0;
bool wifiConnected = false;
String wifiStatus = "Desconectado";

// Variables para el servidor
WiFiClient client;
bool clientConnected = false;
String clientStatus = "";
String lastServerResponse = "";

// actualizar Información del sistema
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 5000; // Actualizar cada 5 segundos

// Objeto para la pantalla SH1106
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ISR para el encoder rotatorio
void IRAM_ATTR rotaryEncoder() 
{
  rotaryAState = digitalRead(PIN_ROTARY_A);
  if (rotaryAState != rotaryALastState) {
    if (digitalRead(PIN_ROTARY_B) != rotaryAState) 
    {
      encoderPos++;
    } else {
      encoderPos--;
    }
  }
  rotaryALastState = rotaryAState;
}


// Variables para el programa principal
const int numIDs = 10; // Número de IDs en el array
int currentLocomotiveIndex = 0; // Índice de la locomotora actual (0-9)

// Estructura para almacenar datos de cada locomotora
struct Locomotora {
  int id;           // ID de la locomotora (1-10)
  int direccionDCC; // Dirección DCC (entero)
  int velocidad;    // Velocidad actual (0-255)
  bool direccion;    // Dirección (1: adelante, 0: atrás)
};

// Array de 10 locomotoras
Locomotora locomotoras[numIDs] = {
  {1, 3, 0, 1},   // ID 1, Dirección DCC 3, velocidad 0, adelante
  {5, 5, 0, 1},   // ID 2, Dirección DCC 5, velocidad 0, adelante
  {3, 7, 0, 1},   // ID 3, Dirección DCC 7, velocidad 0, adelante
  {4, 9, 0, 1},   // ID 4, Dirección DCC 9, velocidad 0, adelante
  {5, 11, 0, 1},  // ID 5, Dirección DCC 11, velocidad 0, adelante
  {6, 13, 0, 1},  // ID 6, Dirección DCC 13, velocidad 0, adelante
  {7, 15, 0, 1},  // ID 7, Dirección DCC 15, velocidad 0, adelante
  {8, 17, 0, 1},  // ID 8, Dirección DCC 17, velocidad 0, adelante
  {9, 19, 0, 1},  // ID 9, Dirección DCC 19, velocidad 0, adelante
  {10, 21, 0, 1}  // ID 10, Dirección DCC 21, velocidad 0, adelante
};


// Variables para la barra del encoder
int barPosition = 0;
int lastBarPosition = 0;
const int barMax = 126;


// Estructura para almacenar datos de cada locomotora
struct Desvio
{
  int direccion;      // Dirección principal (0-512)
  int subdireccion;   // Subdirección (0-3)
  int estado;         // Estado (0 o 1)
  String nombre;      // Nombre descriptivo del desvío
};

// Array de desvíos (puedes ajustar la cantidad)
int currentDesvioIndex = 0;  // Índice del desvío actual
const int numDesvios = 12;
Desvio desvios[numDesvios] = 
{
  {0, 0, 0, "Desvio 1"},   // Dirección 0, Sub 0
  {0, 1, 0, "Desvio 2"},   // Dirección 0, Sub 1
  {0, 2, 0, "Desvio 3"},   // Dirección 0, Sub 2
  {0, 3, 0, "Desvio 4"},   // Dirección 0, Sub 3
  {1, 0, 0, "Desvio 5"},   // Dirección 1, Sub 0
  {1, 1, 0, "Desvio 6"},   // Dirección 1, Sub 1
  {1, 2, 0, "Desvio 7"},   // Dirección 1, Sub 2
  {1, 3, 0, "Desvio 8"},   // Dirección 1, Sub 3
  {2, 0, 0, "Desvio 9"},   // Dirección 2, Sub 0
  {2, 1, 0, "Desvio 10"},  // Dirección 2, Sub 1
  {2, 2, 0, "Desvio 11"},  // Dirección 2, Sub 2
  {2, 3, 0, "Desvio 12"}   // Dirección 2, Sub 3
};

/** Configuration setup */
void setup() 
{
  Serial.begin(115200);
  delay(100);
  
  // Inicializar I2C para la pantalla
  Wire.begin(OLED_SDA, OLED_SCL);
  
  // Inicializar pantalla
  if(!display.begin(0x3C, true)) 
  {
    Serial.println("Error al iniciar SH1106");
    while(1);
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);

  // Configurar pines de botones
  pinMode(PIN_CONFIRM, INPUT_PULLUP);
  pinMode(PIN_BACK, INPUT_PULLUP);
  pinMode(PIN_ROTARY_BTN, INPUT_PULLUP);
  pinMode(PIN_ROTARY_A, INPUT_PULLUP);
  pinMode(PIN_ROTARY_B, INPUT_PULLUP);
  
  // Configurar interrupción para encoder
  rotaryALastState = digitalRead(PIN_ROTARY_A);
  attachInterrupt(digitalPinToInterrupt(PIN_ROTARY_A), rotaryEncoder, CHANGE);
  
  // Conectar a WiFi
  wifiConnect();

  actualizarVariablesLocomotora();

}

/** Main Program loop
*/
void loop() 
{
  // Leer botones con debounce simple
  readButtons();
  
  // Manejar navegación del menú
  runMenu();
    
  // Actualizar pantalla
  showMenu();

  // Lee los datos del enviados por el servidor DCC
  readWifiData();

  // Actualizar información del sistema periódicamente
  if (millis() - lastUpdate > updateInterval) 
  {
    updateSysInfo();
    lastUpdate = millis();
  }

}


// Read buttons Confirm, back and rotatory encoder button
void readButtons()
{
  // Botón confirmar
  bool currentConfirm = digitalRead(PIN_CONFIRM);
  if (currentConfirm == LOW && lastConfirmState == HIGH) 
  {
    confirmPressed = true;
    showScreen = true;
    delay(50); // Debounce simple
  } else {
    confirmPressed = false;
  }
  lastConfirmState = currentConfirm;
  
  // Botón back
  bool currentBack = digitalRead(PIN_BACK);
  if (currentBack == LOW && lastBackState == HIGH) 
  {
    backPressed = true;
    showScreen = true;
    delay(50);
  } else backPressed = false;
  lastBackState = currentBack;
  
  // Botón rotary
  bool currentRotaryBtn = digitalRead(PIN_ROTARY_BTN);
  if (currentRotaryBtn == LOW && lastRotaryBtnState == HIGH) {
    rotaryBtnPressed = true;
    showScreen = true;
    delay(50);
  } else {
    rotaryBtnPressed = false;
  }
  lastRotaryBtnState = currentRotaryBtn;
}
/** Update signal RSSI 
*/
void updateSysInfo() 
{
  
  // Actualizar estado WiFi
  if (WiFi.status() == WL_CONNECTED) 
  {
    wifiConnected = true;
    wifiStatus = "Conectado";
    wifiRSSI = WiFi.RSSI();
  } 
  else 
  {
    wifiConnected = false;
    wifiStatus = "Desconectado";
    wifiRSSI = 0;
  }
}
