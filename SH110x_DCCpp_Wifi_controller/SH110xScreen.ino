/** Muestra el menu principal
*/
void showMenu() 
{
  if (inMenu) 
  {
    // Mostrar menú principal
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(30, 0); // Posición del titulo
    display.println("= Main menu =");
    
    int separacion = 10;
    for (int i = 0; i < menuItems; i++) 
    {
      display.setCursor(0, i*10 + separacion);
      if (i == menuItem) display.print("> ");
      else display.print("  ");
      display.setCursor(10, i*10 + separacion);
      if (i == 0)
      {
        display.print(menuNames[i]);
        display.print(" ");
        display.print(startDCC);
      } else display.print(menuNames[i]);
    }
    
    // Mostrar estado WiFi en la parte inferior
    display.setTextSize(1);
    display.setCursor(0, 56);
    display.print("Server:");
    display.print(clientStatus);
    if (wifiConnected)
    {
      display.print(" ");
      display.print(wifiRSSI);
      display.print("dBm");
    }
    display.display();
  } 
  else if (inSubMenu) 
  {
    if (menuItem == 0) 
    {
        inSubMenu = false;
        inMenu = true;
        if (!startDCC) sendWifiData("<1>");
        else sendWifiData("<0>");
        startDCC = !startDCC;
    } 
    else if (menuItem == 1) // Main program
    {
      runMainProgram();
      if (showScreen)
      {
        showMainScreen();
        showScreen = false;
        return;
      }
    }
    else if (menuItem == 2) // Turnout program
    {
     runSecondaryProgram();
      if (showScreen)
      {
        showSecondaryScreen();
        showScreen = false;
        return;
      }
    }
    else if (menuItem == 3) // Info Sistema
    {
      showSysInfo();
    }
  }
}

/** Pantalla para el control de 10 locomotoras
*/
void showMainScreen() 
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(25, 0);
  display.print("= LOCOMOTORA =");
  
  // Mostrar ID y dirección DCC
  display.setCursor(5, 10);
  display.print("ID: ");
  display.print(locomotoras[currentLocomotiveIndex].id);
  display.print(" (DCC:");
  display.print(locomotoras[currentLocomotiveIndex].direccionDCC);
  display.print(")");
  
  // Mostrar dirección con flechas
  display.setCursor(5, 19);
  display.print("Dir: ");
  if (locomotoras[currentLocomotiveIndex].direccion == 1) display.print("FWD >>");
  else display.print("BCK <<");
  
  // Mostrar velocidad
  display.setCursor(5, 28);
  display.print("Speed: "); display.print(locomotoras[currentLocomotiveIndex].velocidad);
  display.print(" (");
  display.print(map(locomotoras[currentLocomotiveIndex].velocidad, 0, 126, 0, 100));
  display.print("%)");
  
  // Dibujar barra de velocidad
  int barWidth = map(locomotoras[currentLocomotiveIndex].velocidad, 0, 126, 0, 120);
  display.drawRect(4, 40, 120, 10, SH110X_WHITE);
  display.fillRect(4, 40, barWidth, 10, SH110X_WHITE);

  showServerResponses(lastServerResponse);

  display.display();
}

/** Pantalla para el control de 12 desvíos
*/
void showSecondaryScreen() 
{
  display.clearDisplay();
  display.setTextSize(1);
  
  // Título
  display.setCursor(0, 0);
  display.println("= DESVIOS =");
  
  // Mostrar lista de desvíos (6 en pantalla con scroll)
  int startIndex = max(0, currentDesvioIndex - 2);
  int endIndex = min(numDesvios, startIndex + 5);
  
  for (int i = startIndex; i < endIndex; i++) {
    int yPos = 8 + (i - startIndex) * 8;
    
    // Indicador de selección
    if (i == currentDesvioIndex) {
      display.setCursor(0, yPos);
      display.print(">");
    } else {
      display.setCursor(4, yPos);
    }
    
    // Nombre del desvío
    display.print(desvios[i].nombre);
    
    // Checkbox con estado
    display.setCursor(100, yPos);
    if (desvios[i].estado == 1) {
      display.print("[X]");  // Activado
    } else {
      display.print("[ ]");  // Desactivado
    }
  }
  
  display.display();
}

/** Pantalla donde se muestra la información del sistema
*/
void showSysInfo() 
{
  display.clearDisplay();
  display.setCursor(20, 0); // Posición título
  display.println("=SYS INFO=");
  
  // Temperatura interna
  display.setCursor(0, 9);
  // Leer temperatura interna del ESP32-C3
  internalTemp = temperatureRead();
  display.print("Temp ESP32: ");
  display.print(internalTemp, 1);
  display.print(" C");
  
  // Estado WiFi
  display.setCursor(0, 18);
  display.print("WiFi: ");
  display.print(wifiStatus);
  if (wifiConnected) 
  {
    display.setCursor(0, 27);
    display.print("SSID: ");
    display.println(ssid);
    display.setCursor(0, 36);
    display.print("IP: ");
    display.print(WiFi.localIP().toString());
    
    display.setCursor(0, 45);
    display.print("signal: ");
    display.print(wifiRSSI);
    display.print(" dBm");
    display.setCursor(0, 54);
    display.print("Quality: ");
    int calidad = map(constrain(wifiRSSI, -100, -50), -100, -50, 0, 100);
    display.print(calidad);
    display.print("%");
  }
  display.display();
}

/** Muestra las respuestas del servidor en la parte inferior de la pantalla
*/
void showServerResponses(String response)
{
  // display.setCursor(10, 54);
  display.fillRect(10,54, 100, 20, SH110X_BLACK);

  if (response != "") 
  {
    display.setCursor(10, 54);
    if (response.length() > 12) {
      display.print(response.substring(0, 10) + "..");
    } 
    else 
    {
      lastServerResponse = response;
      Serial.println(response);
      display.print(response);
    }
    display.display();
  }
  //
}
