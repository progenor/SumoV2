#include "expander.h"

Adafruit_MCP23X17 mcp;

/*
 * Check for interrupts on the IO expander
 @return keypad of the returned interrupt
 @return -1 if no interrupt
 */
int checkExpanderInterrupt()
{
  if (!digitalRead(INT_A))
  {
    int x = mcp.getLastInterruptPin();
    mcp.clearInterrupts(); // clear
    return x;
  }
  return -1;
}

bool isExpanderPressed(int pin)
{
  return mcp.digitalRead(pin) == LOW;
}

bool setupExpander()
{
  if (!mcp.begin_I2C(IO_ADDRESS))
  {
    Serial.println("Error.");
    while (1)
      ;
  }

  Serial.println("IO Expander found!");

  pinMode(INT_A, INPUT);

  // IO Expander
  mcp.setupInterrupts(true, false, LOW);

  // Configure MCP pins 0..7 as inputs with pull-up.
  for (uint8_t pin = EXP_PIN_0; pin <= EXP_PIN_7; pin++)
  {
    mcp.pinMode(pin, INPUT_PULLUP);
    mcp.setupInterruptPin(pin, LOW);
  }

  return true;
}