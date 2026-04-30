// Wrapper weil VS Code mit .ino weniger gut klarkommt als mit .cpp
#include "app.h"

void setup()
{
  appSetup();
}

void loop()
{
  appLoop();
}
