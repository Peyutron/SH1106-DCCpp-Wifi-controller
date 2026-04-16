/** Muestra el menu principal
*/
void showMenu() 
{
  if (inMenu) 
  {
    // Mostrar menú principalSH110X_WHITE
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(25, 0); // Posición del titulo
    display.println("- Main menu -");
    
    
    int separacion = 10;

    // Mostrar lista de desvíos (6 en pantalla con scroll)
    int startIndex = max(0, menuItem - 3);
    int endIndex = min(menuItems, startIndex + 4);
    
    for (int i = startIndex; i < endIndex; i++) 
    {
      int yPos = 10 + (i - startIndex) * 10;

          // Indicador de selección
      if (i == menuItem) 
      {
        display.setCursor(10, yPos);
        display.print(">");
      } 
      else 
      {
        display.setCursor(15, yPos);
      }
    
      if (i == 0)
      {
        display.print(menuNames[i]);
        display.print(" ");
        display.print(startDCC);
      } 
      else display.print(menuNames[i]);
    }
    // Mostrar estado WiFi en la parte inferior
    display.setCursor(0, 56);
    display.print("S:");
    
    if (client.connected()) display.print(serverIP);
    else display.print(clientStatus);
    
    if (wifiConnected)
    {
      display.print(" ");
      display.print(wifiRSSI);
      display.print("dB");
    }
  
    display.display();
  } 
  else if (inSubMenu) 
  {
    //if (menuItem == 0)
    //{
    //  inSubMenu = false;
    //  inMenu = true;
    //}
    if (menuItem == 1) // Main program
    {
      //Serial.println("estas en el menu 1");
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
      //Serial.println("estas en el menu 2");
      runSecondaryProgram();
      if (showScreen)
      {
        showSecondaryScreen();
        showScreen = false;
        return;
      }
    }
    else if (menuItem == 3) // Config IP
    {
      runConfigIP();
      if (showScreen)
      {
        if (inConfirmScreen) showConfirmScreen();
        else
        { 
          showConfigIP();
          showScreen = false;
          return;
        }
      }
    }
    else if (menuItem == 4) showSysInfo(); // Info Sistema

  }
}

/** Pantalla para el control de locomotoras
*/
void showMainScreen() 
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("ID: ");
  display.print(locomotoras[currentLocomotiveIndex].id);
  display.print(" ");
  display.print(locomotoras[currentLocomotiveIndex].shortName);
 
  
  //Dirección DCC
  display.setCursor(10, 10);
  display.print("DCC: ");
  display.print(locomotoras[currentLocomotiveIndex].direccionDCC);
  
  // Mostrar dirección con flechas
  display.setCursor(10, 19);
  display.print("Dir: ");
  if (locomotoras[currentLocomotiveIndex].direccion == 1) display.print("FWD >>");
  else display.print("BCK <<");
  
  // Mostrar velocidad
  display.setCursor(10, 28);
  display.print("Speed: "); display.print(locomotoras[currentLocomotiveIndex].velocidad);
  
  // Dibujar barra de velocidad
  int barWidth = map(locomotoras[currentLocomotiveIndex].velocidad, 0, 126, 0, 60);
  display.drawRect(10, 40, 60, 10, SH110X_WHITE);
  display.fillRect(10, 40, barWidth, 10, SH110X_WHITE);

  display.setCursor(70, 40);
  display.print(" (");
  display.print(map(locomotoras[currentLocomotiveIndex].velocidad, 0, 126, 0, 100));
  display.print("%)");
  

  showServerResponses(lastServerResponse);

  display.display();
}

/** Pantalla para el control de desvíos
*/
void showSecondaryScreen() 
{
  display.clearDisplay();
  display.setTextSize(1);
  
  // Título
  display.setCursor(25, 0);
  display.println(F("- DESVIOS -"));
  
  // Mostrar lista de desvíos (6 en pantalla con scroll)
  int startIndex = max(0, currentDesvioIndex - 2);
  int endIndex = min(numDesvios, startIndex + 5);
  
  for (int i = startIndex; i < endIndex; i++) 
  {
    int yPos = 8 + (i - startIndex) * 8;
    
    // Indicador de selección
    if (i == currentDesvioIndex) 
    {
      display.setCursor(0, yPos);
      display.print(">");
    } 
    else display.setCursor(4, yPos);
    
    // Nombre del desvío
    display.print(desvios[i].nombre);
    
    // Checkbox con estado
    display.setCursor(100, yPos);
    if (desvios[i].estado == 1) display.print("[X]");  // Activado
    else display.print("[ ]");  // Desactivado
  }
  
  display.display();
}

/** Pantalla donde se muestra la información del sistema
*/
void showConfigIP() 
{

  display.clearDisplay();
  display.setCursor(25, 0);
  display.println(F("- Config IP -"));

  // Mostrar la IP con el octeto activo entre corchetes
  display.setCursor(16, 30);
  for (int i = 0; i < 4; i++)
  {
    if (i == octetoSeleccionado)
    {
      display.print('[');
      display.print(ip[i]);
      display.print(']');
    }
    else
    {
    display.print(ip[i]);
    }
    if (i < 3) display.print('.');
  }

  display.display();
  delay(50);
  
}

/** Show Screen to confirm IP
*/
void showConfirmScreen() 
{
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("Guardar IP?"));

    display.setCursor(0, 18);
    if (confirmMenuOption == 0) display.print(F("> Si"));
    else display.print(F("  Si"));

    display.setCursor(0, 27);
    if (confirmMenuOption == 1) display.print(F("> No"));
    else display.print(F("  No"));

    display.display();
}

/** Show ip when update ConfigIP
*/
void showIPOnScreen()
{
  display.clearDisplay();
  display.setCursor(20, 0);
  display.println(F("IP almacenada:"));
  display.setCursor(18, 20);
  updateServerIP();
  display.println(serverIP);
  display.display();
  delay(2000);
}


/** Pantalla donde se muestra la información del sistema
*/
void showSysInfo() 
{
  display.clearDisplay();
  display.setCursor(25, 0); // Posición título
  display.println(F("- SYS INFO -"));
  
  // Temperatura interna
  display.setCursor(0, 9);
  // Leer temperatura interna del ESP32-C3
  internalTemp = temperatureRead();
  display.print(F("Temp ESP32: "));
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
    display.print("Server: ");
    display.print(serverIP);
    // display.print("signal: ");
    // display.print(wifiRSSI);
    // display.print(" dBm");
    display.setCursor(0, 54);
    display.print(F("Quality: "));
    int calidad = map(constrain(wifiRSSI, -100, -50), -100, -50, 0, 100);
    display.print(calidad);
    display.print("%");
  }
  display.display();
  delay(50);
}

/** Muestra las respuestas del servidor en la parte inferior de la pantalla
*/
void showServerResponses(String response)
{
  // display.setCursor(10, 54);
  display.fillRect(15, 54, 100, 20, SH110X_BLACK);

  if (response != "") 
  {
    display.setCursor(20, 56);
    if (response.length() > 12) 
    {
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
