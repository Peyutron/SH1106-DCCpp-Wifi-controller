void runMenu() 
{
  if (inMenu) 
  {
    // Navegar por el menú principal con el encoder
    if (encoderPos != lastEncoderPos) 
    {
      if (encoderPos > lastEncoderPos) 
      {
        menuItem = (menuItem + 1) % menuItems;
      } else {
        menuItem = (menuItem - 1 + menuItems) % menuItems;
      }
      lastEncoderPos = encoderPos;
      // showScreen = true;
    }
    
    // Seleccionar opción con botón confirmar
    if (rotaryBtnPressed) 
    {
      if (menuItem == 0)
      {
        if (!startDCC) sendWifiData("<1>");
        else sendWifiData("<0>");
        startDCC = !startDCC;
        showScreen = true;
      }
      else if (menuItem == 5 ) esp_restart();
      else
      {
        inMenu = false;
        inSubMenu = true;
        subMenuItem = 0;
        showScreen = true;
       }
    }
  } 
  else if (inSubMenu) 
  {
    // Manejar submenús
    if (backPressed) 
    {
      showScreen = true;
      inMenu = true;
      inSubMenu = false;
    } 
    else if (confirmPressed) 
    {
      showScreen = true;
      // Acción especial con botón del encoder si se necesita
    }
  }
}

void runMainProgram() 
{
  // Limitar posición del encoder
  if (encoderPos >= barMax) encoderPos = barMax;
  else if (encoderPos <= 0) encoderPos = 0;
  
  if (barPosition != encoderPos) barPosition = encoderPos;
  // Actualizar posición de la barra con el encoder
  
  
  // Si la posición cambió, actualizar velocidad y enviar datos
  if (barPosition != lastBarPosition) 
  {
    locomotoras[currentLocomotiveIndex].velocidad = map(barPosition, 0, barMax, 0, 126); // Guardar en estructura
    lastBarPosition = barPosition;

      sendWifiData(locomotiveData());
      showScreen = true;

  }
  
  // Botón del encoder cambia dirección
  if (rotaryBtnPressed) 
  {
    locomotoras[currentLocomotiveIndex].direccion = !locomotoras[currentLocomotiveIndex].direccion;
    sendWifiData(locomotiveData());
    showScreen = true;
    delay(200);
  }
  
  // Botón confirm cambia de locomotora
  if (confirmPressed) 
  { 
    currentLocomotiveIndex++; // 10 locomotoras
    if (currentLocomotiveIndex >= numIDs) currentLocomotiveIndex = 0;
    actualizarVariablesLocomotora(); // Cargar los datos de la nueva locomotora
    sendWifiData(locomotiveData());  // Enviar los datos de la nueva locomotora
    showScreen = true;
    delay(200);
  }
  
  // Botón back vuelve al menú principal
  if (backPressed) 
  {
    showScreen = true;
    inMenu = true;
    inSubMenu = false;
    delay(200);
  }
}

void runSecondaryProgram()
{
  // Navegar con el encoder
  if (encoderPos != lastEncoderPos) {
    if (encoderPos > lastEncoderPos) {
      currentDesvioIndex = (currentDesvioIndex + 1) % numDesvios;
      showScreen = true;
    } else {
      currentDesvioIndex = (currentDesvioIndex - 1 + numDesvios) % numDesvios;
      showScreen = true;
    }
    lastEncoderPos = encoderPos;
    delay(50);
  }
  
  // Confirm button
  if (confirmPressed) {
    desvios[currentDesvioIndex].estado = !desvios[currentDesvioIndex].estado;
    sendWifiData(accessoryData());
    showScreen = true;
    delay(300);
  }
  
  // Back button
  if (backPressed) {
    inMenu = true;
    inSubMenu = false;
    delay(200);
  }
}

/**
*/
void runConfigIP()
{
  static bool changeValues = true;
  if (inConfirmScreen) 
  {
    handleConfirmScreen();
    return;
  }
  if (changeValues)
  {
    convertToCharArray();
    changeValues = false;
  }
  // Manejo del encoder
  if (encoderPos != lastEncoderPos)
  {
    int cambio = (encoderPos > lastEncoderPos) ? 1 : -1;
    // Cambiar el valor del octeto seleccionado
    ip[octetoSeleccionado] += cambio;
    // Limitar entre 0 y 255
    if (ip[octetoSeleccionado] < 0) ip[octetoSeleccionado] = 255;
    if (ip[octetoSeleccionado] > 255) ip[octetoSeleccionado] = 0;
        
    lastEncoderPos = encoderPos;
    showScreen = true;   // Para refrescar la pantalla
    delay(50);           // Pequeña pausa antirrebote
    }

    // Manejo del botón de confirmación (pasar al siguiente octeto)
    if (rotaryBtnPressed)
    {
        confirmPressed = false; // Resetear flag (debes manejarlo en tu código de botón)
        octetoSeleccionado++;
        if (octetoSeleccionado >= 4)
        {
            // Terminamos de configurar la IP
            octetoSeleccionado = 0;   // Reiniciamos para próxima vez
            configActiva = false;     // Salimos del modo configuración (opcional)
            // Aquí puedes guardar la IP en EEPROM o hacer algo con ella
        }
        showScreen = true;
        delay(50);
    }
    if (confirmPressed)
    {
      confirmPressed = false;
      inConfirmScreen = true;
      confirmMenuOption = 0;      // Iniciamos con "Sí" seleccionado
      showScreen = true;
      delay(50);
    }

}

void handleConfirmScreen() 
{
  // Manejo del encoder para elegir Sí/No
  if (encoderPos != lastEncoderPos) 
  {
    if (encoderPos > lastEncoderPos) 
    {
      confirmMenuOption = (confirmMenuOption + 1) % 2;
    } 
    else
    {
      confirmMenuOption = (confirmMenuOption - 1 + 2) % 2;
    }
    lastEncoderPos = encoderPos;
    showScreen = true;
    delay(50);
  }

  // Mostrar pantalla de confirmación si es necesario
  if (showScreen) 
  {
    showConfirmScreen();
    showScreen = false;
  }

  // Confirmar opción con el botón del encoder
  if (rotaryBtnPressed) 
  {
    rotaryBtnPressed = false;
    if (confirmMenuOption == 0) 
    {  // Sí
      saveIPToEEPROM(ip);           // Guardar en EEPROM
      showIPOnScreen();
    }
    // Salimos de la pantalla de confirmación y de la configuración
    inConfirmScreen = false;
    configActiva = false;           // Volver al menú principal
    showScreen = true;
    delay(50);
  }

  

}

/**
*/
void actualizarVariablesLocomotora() 
{
  // Actualizar la posición del encoder según la velocidad
  encoderPos = map(locomotoras[currentLocomotiveIndex].velocidad, 0, 126, 0, barMax);
  lastBarPosition = encoderPos;
}

void convertToCharArray()
{
  String ip_;
  // Convertir cadena a bytes 
  sscanf(serverIP, "%hhu.%hhu.%hhu.%hhu", 
          &ip[0], &ip[1], 
          &ip[2], &ip[3]);
}