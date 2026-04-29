/**
  * Version: DCC Controller ESP32 C3
  * Project instructions: 
    - http://lamaquetade.infotronikblog.com//2026-02-27-DCC-controller-ESP32-3C-mini-y-display-SH1106/
    - https://www.infotronikblog.com/2026/01/esp32-c3-super-mini-oled-sh1106.html
  *  
  * GitHub Project repository:
    - https://github.com/Peyutron/SH1106-DCCpp-Wifi-controller
  * GitHub repository:
    - https://github.com/Peyutron
  * Web:
    - https://www.infotronikblog.com
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
  * Board Version 2.0.18
  *
  * Cambios 16-04-2026:
  * - IP configurable en menu principal "Config".
  * - Posibilidad de resetear el módulo ESP32C3 desde menu principal "Restart".
  * - Envía el comando <c> (petición de consumo de la central) cada 8s 
  *   para no perder la conexión con el servidor.
  * - Desvíos y locomotoras tienen su propia estructura de datos.
  * 
**/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <WiFi.h>
#include "logo.h"
#include <EEPROM.h>   // Para Arduino AVR
// Si usas ESP32 o ESP8266, incluye <EEPROM.h> también pero con diferentes métodos.

#define EEPROM_SIZE 4   // Only 4 bytes
#define EEPROM_ADDR_IP 0  // Start address

// Configuración WiFi y servidor (modifica según tus necesidades)
const char* ssid = "YourSSID";
const char* password = "YourPass";
char serverIP[16] = "192.168.1.5";
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
const int menuItems = 6;
const char* menuNames[] = {"Start!", "Main", "Turnouts", "IP Config", "SysInfo", "Restart"};
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
#define MAXSERVERTRY 10

// actualizar Información del sistema
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 5000; // Actualizar cada 5 segundos

// Enviar comando para no perder la conexión
unsigned long lastUpdateCommand = 0;
const unsigned long updateIntervalCommand = 8000; // Send Command <> every 8 segundos

// Objeto para la pantalla SH1106
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


// Variables para el programa principal
const int numIDs = 7; // Número de IDs en el array
int currentLocomotiveIndex = 0; // Índice de la locomotora actual (0-numIDs)

// Estructura para almacenar datos de cada locomotora
struct Locomotora 
{
  String shortName;
  int id;           // ID de la locomotora (1-10)
  int direccionDCC; // Dirección DCC (entero)
  int velocidad;    // Velocidad actual (0-255)
  bool direccion;    // Dirección (1: avance, 0: atrás)
};

// Array de 7 locomotoras
Locomotora locomotoras[numIDs] = 
{
  {"Valenciana", 1, 4, 0, 1}, // ID 1, Dirección DCC 4, velocidad 0, avance
  {"Talgo 352", 5, 7, 0, 1},  // ID 2, Dirección DCC 7, velocidad 0, avance
  {"Infraestr", 3, 6, 0, 1},  // ID 3, Dirección DCC 6, velocidad 0, avance
  {"AVE", 4, 5, 0, 1},        // ID 4, Dirección DCC 5, velocidad 0, avance
  {"DR132", 2, 8, 0, 1},      // ID 5, Dirección DCC 8, velocidad 0, avance
  {"269 Renfe", 6, 9, 0, 1},  // ID 6, Dirección DCC 9, velocidad 0, avance
  {"242 Iber", 7, 11, 0, 1}   // ID 7, Dirección DCC 11, velocidad 0, avance
  
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
const int numDesvios = 10;
Desvio desvios[numDesvios] = 
{
  // {0, 0, 0, "Desvio 1"},   // Dirección 0, Sub 0
  // {1, 0, 0, "Luces 5"},   // Dirección 1, Sub 0
  {1, 1, 0, "luces"},   // Dirección 1, Sub 1
  {1, 2, 0, "Casas"},   // Dirección 1, Sub 2
  {1, 3, 0, "Luces Taller"},   // Dirección 1, Sub 3
  {2, 0, 0, "Desvio 9"},   // Dirección 2, Sub 0
  {2, 1, 0, "Soldador"},  // Dirección 2, Sub 1
  {2, 2, 0, "TV taller"},  // Dirección 2, Sub 2
  {2, 3, 0, "Paso a nivel"},  // Dirección 2, Sub 3
  {3, 0, 0, "Barriada"},  // Dirección 3, Sub 0
  {5, 0, 0, "D.Principal"},  // Dirección 5, Sub 0
  {5, 1, 0, "D.talleres"}  // Dirección 5, Sub 1

};

// Para el menú ConfigIP
int ip[4] = {192, 168, 1, 82};   // NO CAMBIAR
int octetoSeleccionado = -1; // or 0?
bool configActiva = true;

// Para el menú de confirmación
int confirmMenuOption = 0;  // 0 = Sí, 1 = No
bool inConfirmScreen = false;

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
    delayMicroseconds(4);
  }
  rotaryALastState = rotaryAState;
}

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
    Serial.println(F("Error al iniciar SH1106"));
    while(1);
  }
  Serial.println(F("Starting DCCpp Wifi Controller..."));

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
  
  // Inicializar EEPROM
  EEPROM.begin(EEPROM_SIZE);  // En ESP8266/ESP32 es necesario; en AVR no hace falta pero no daña
  delay(50);
  // saveIPToEEPROM(ip); // Only first start
  loadIPFromEEPROM();         // Cargar IP guardada previamente

  // Conectar a WiFi
  wifiConnect();
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

  if (client.connected())
  {
  if (millis() - lastUpdateCommand > updateIntervalCommand) 
    {
      sendRecursiveCommand();
      lastUpdateCommand = millis();
    }
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
    // showScreen = true;
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
    //showScreen = true;
    delay(50);
  } 
  else backPressed = false;
  lastBackState = currentBack;
  
  // Botón rotary
  bool currentRotaryBtn = digitalRead(PIN_ROTARY_BTN);
  if (currentRotaryBtn == LOW && lastRotaryBtnState == HIGH) 
  {
    rotaryBtnPressed = true;
    //showScreen = true;
    delay(50);
  } 
  else 
  {
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

// Save IP data on EEPROM
void saveIPToEEPROM(int * ipArray) 
{   
  Serial.println(F("Guardando IP..."));
  for (int i = 0; i < EEPROM_SIZE; i++) 
  {
    Serial.print(ipArray[i]);
    if (i < EEPROM_SIZE -1 ) Serial.print(F("."));
    EEPROM.write(EEPROM_ADDR_IP + i, ipArray[i]);
    // #ifdef ESP32
    EEPROM.commit();  // Necesario en ESP32
    // #endif
  }
  Serial.println();
}

/** Load IP data from EEPROM and update array ip[]
*/
void loadIPFromEEPROM() 
{
  Serial.print(F("Recuperando IP: "));
  for (int i = 0; i < 4; i++) 
  {
        int val = EEPROM.read(EEPROM_ADDR_IP + i);
        // Si el valor es 255 (EEPROM virgen) o inválido, mantener valor por defecto
        if (val >= 0 && val <= 255) {
          Serial.print(val);
          if (i < EEPROM_SIZE -1 ) Serial.print(F("."));
          ip[i] = val;
        }
  }
  Serial.println();
  updateServerIP();
}
