void runMenu() 
{
  if (inMenu) 
  {
    // Navegar por el menú principal con el encoder
    if (encoderPos != lastEncoderPos) {
      if (encoderPos > lastEncoderPos) {
        menuItem = (menuItem + 1) % menuItems;
      } else {
        menuItem = (menuItem - 1 + menuItems) % menuItems;
      }
      lastEncoderPos = encoderPos;
    }
    
    // Seleccionar opción con botón confirmar
    if (rotaryBtnPressed) 
    {
      inMenu = false;
      inSubMenu = true;
      subMenuItem = 0;
    }
  } 
  else if (inSubMenu) 
  {
    // Manejar submenús
    if (backPressed) 
    {
      inMenu = true;
      inSubMenu = false;
    } else if (confirmPressed) {
      // Acción especial con botón del encoder si se necesita
    }
  }
}

void runMainProgram() 
{
  // Limitar posición del encoder
  if (encoderPos >= barMax) encoderPos = barMax;
  else if (encoderPos <= 0) encoderPos = 0;
  
  // Actualizar posición de la barra con el encoder
  barPosition = encoderPos;
  
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
  if (confirmPressed) {
    currentLocomotiveIndex = (currentLocomotiveIndex + 1) % 10; // 10 locomotoras
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

void actualizarVariablesLocomotora() 
{
  // Actualizar la posición del encoder según la velocidad
  encoderPos = map(locomotoras[currentLocomotiveIndex].velocidad, 0, 126, 0, barMax);
  lastBarPosition = encoderPos;
}