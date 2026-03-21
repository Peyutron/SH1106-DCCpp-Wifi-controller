void wifiConnect() // On setup()
{
  display.clearDisplay();
  display.setCursor(10, 0);
  display.println("Connecting to SSID");
  display.setCursor(10, 9);
  display.print(ssid);
  loadLogo();
  display.display();
  // WiFi.mode(WIFI_STA);
  // esp_wifi_set_ps(WIFI_PS_NONE); // Board version 3.3.7 problems
  WiFi.begin(ssid, password);

  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED && intentos < 10) {
    delay(500);
    display.print(".");
    display.display();
    intentos++;
  }
  
  if (WiFi.status() == WL_CONNECTED) 
  {
    wifiConnected = true;
    wifiStatus = "Connected";
    wifiRSSI = WiFi.RSSI();
    
    display.clearDisplay();
    display.setCursor(10, 0);
    display.print("WiFi ok!");
    display.print("IP: ");
    display.println(WiFi.localIP());
    //loadLogo();    
    display.display();
    conectarServidor();

    delay(1000);
  } 
  else 
  {
    wifiConnected = false;
    wifiStatus = "Error conexion";
    
    display.clearDisplay();
    loadLogo();
    display.setCursor(10, 0);
    display.println("Error WiFi");
    display.setCursor(10, 9);
    display.println("Check credentials!");
    display.display();
    delay(5000);
  }
}

/** Función para conectar al servidor (puedes llamarla cuando necesites)
*/
void conectarServidor() 
{
  static int nextServer = 0;
  display.clearDisplay();
  loadLogo();
  display.setCursor(0, 0);
  display.print("SSID: ");
  display.print(ssid);
  display.display();
  
  WiFi.begin(ssid, password);
  
  int intentos = 0;
  display.setCursor(0, 10);
  while (WiFi.status() != WL_CONNECTED && intentos < 20) {
    delay(500);
    display.print(".");
    display.display();
    intentos++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    //delay(500);
    wifiConnected = true;
    wifiStatus = "Connected";
    wifiRSSI = WiFi.RSSI();
    
    display.clearDisplay();
    loadLogo(); 
    display.setCursor(0, 0);
    display.println("WiFi Connected!");
    display.print("IP: ");
    display.println(WiFi.localIP());    
    Serial.println(F("WiFi Connected!"));
    Serial.print(F("IP: "));
    Serial.println(WiFi.localIP());
    display.display();
    delay(2000);
    display.clearDisplay();
    loadLogo(); 
    display.setCursor(10, 0);
    display.print("Server ");
    // Intentar conexión inicial con el servidor
    // if (client.connect(serversIPS[nextServer].c_str(), serverPort)) 
    int serverTry = 0;
    while (!client.connected() && serverTry < MAXSERVERTRY)
    {
    if (client.connect(serverIP, serverPort)) 
    {
      clientConnected = true;
      clientStatus = "Running";
      display.print(clientStatus);
      Serial.println(clientStatus);
      display.display();
    } 
    else 
    {
      clientStatus = " Error ";
      //display.print(clientStatus);
      Serial.println(clientStatus);
      display.setCursor(0, 9);
      display.print("F: ");
      display.print(serverIP);
      display.print(":");
      display.print(serverPort);
      // Linea 2
      display.setCursor(0, 19);
      display.print("Intento:    ");
      display.fillRect(50, 19, 9, 9, SH110X_BLACK);
      display.setCursor(50, 19);
      display.print(serverTry);
      display.print("/");
      display.print(MAXSERVERTRY);
      Serial.print(F("Check Server IP "));
      Serial.print(serverIP);
      Serial.print(":");
      Serial.print(serverPort);

      display.display();
      serverTry++;
      
    }
    }
    delay(2000);
  } 
  else 
  {
    wifiConnected = false;
    wifiStatus = "Wifi Error";
    
    display.clearDisplay();
    display.setCursor(20, 20);
    display.println("WiFi error");
    display.setCursor(20, 30);
    display.println("Check Server IP ");
    display.display();
    delay(5000);
  }
}

/** Read data from DCC station
*/
void readWifiData()
{
  // Leer respuesta
  String response = "";
  while (client.available()) 
  {
    response += (char)client.read();
  }
  
  response.trim();
  if (response.length() > 0) 
  {
    showServerResponses(response);
  }
  else showServerResponses("");
}

/** Send data to DCC station
*/
void sendWifiData(String dataToSend)
{
  Serial.print("Send: ");
  Serial.println(dataToSend);
  
  // Enviar datos al servidor por WiFi
  if (WiFi.status() == WL_CONNECTED) client.println(dataToSend); // Enviar datos (formato simple) 
  else showServerResponses("Sin WiFi");

  // Aquí puedes agregar el envío real de datos cuando tengas la comunicación
  // Por ejemplo: enviar por WiFi, UART, etc.
}

/** Create locomotive command
*/
String locomotiveData() 
{
  return "<t" +
                  String(currentLocomotiveIndex +1) + " " +
                  String(locomotoras[currentLocomotiveIndex].direccionDCC) + " " + 
                  String(locomotoras[currentLocomotiveIndex].velocidad) + " " + 
                  String(locomotoras[currentLocomotiveIndex].direccion) +
                  ">";
}

/** Create accessory command
*/
String accessoryData()
{
  return "<a " + String(desvios[currentDesvioIndex].direccion) + " " + 
                      String(desvios[currentDesvioIndex].subdireccion) + " " + 
                      String(desvios[currentDesvioIndex].estado) +
                      ">";
}

/** Load logo DCCPP LMD
*/
void loadLogo()
{
  display.drawXBitmap(0, 10, dccpp_lmd_logo_array, DCCpp_LMD_logo_width, DCCpp_LMD_logo_height, SH110X_WHITE);
}