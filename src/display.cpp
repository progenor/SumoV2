#include "display.h"

Display::Display()
    : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, display_RESET),
      lastUpdateTime(0)
{
}

bool Display::setup()
{
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    {
        Serial.println("SSD1306 initialization failed!");
        return false;
    }
    display.clearDisplay();
    display.setTextColor(1);
    display.display();
    return true;
}

void Display::print(const char *text, double value)
{
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print(text);
    if (value != -1)
    {
        display.println(value);
    }
    display.display();
}

void Display::clear()
{
    display.clearDisplay();
    display.display();
}

void Display::displayIR(int *irValues, int sensorCount)
{
    if (!shouldUpdate())
        return; // Throttle updates

    display.clearDisplay();
    uint8_t bar_width = SCREEN_WIDTH / sensorCount;
    if (bar_width < 2)
        bar_width = 2;

    for (uint8_t i = 0; i < sensorCount; i++)
    {
        int ir = irValues[i];

        // If sensor reads 1 (HIGH), draw full height; else 0
        uint8_t bar_height = (ir == 1) ? SCREEN_HEIGHT : 0;
        uint8_t x = i * bar_width;
        uint8_t y = SCREEN_HEIGHT - bar_height;

        if (bar_height > 0)
        {
            display.fillRect(x, y, bar_width - 1, bar_height, SSD1306_WHITE);
        }
    }
    display.display();
}

bool Display::shouldUpdate()
{
    unsigned long now = millis();
    if (now - lastUpdateTime >= DISPLAY_REFRESH_MS)
    {
        lastUpdateTime = now;
        return true;
    }
    return false;
}

void Display::drawLoadingScreen(const char *status)
{
    display.clearDisplay();

    // Draw custom bitmap logo
    display.drawBitmap(42, 2, logo, 45, 50, 1);
    // Layer 2
    display.setTextColor(1);
    display.setTextWrap(false);
    display.setCursor(35, 53);
    display.print("Loading...");

    display.display();
}

void Display::drawMainScreen(void)
{
    display.clearDisplay();

    // Pasted_image
    display.drawBitmap(42, 0, logo, 45, 50, 1);

    // Layer 2
    display.setTextColor(1);
    display.setTextWrap(false);
    display.setCursor(23, 56);
    display.setTextSize(1);
    display.print("T.F.S Robotics");
    display.display();
}

static const unsigned char PROGMEM image_Pin_arrow_right_bits[] = {0x04, 0x00, 0x06, 0x00, 0xff, 0x00, 0xff, 0x80, 0xff, 0x00, 0x06, 0x00, 0x04, 0x00};

const char *strategy_names[] = {"Sting", "Speed", "Run", "IMU"};

void Display::drawStrategySelectorScreen(int currentStrategy)
{
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setTextWrap(false);

    // Title
    display.setCursor(0, 0);
    display.print("STRATEGY");

    // Draw three strategy options with highlight
    uint8_t y_positions[] = {14, 26, 38, 50};

    for (int i = 0; i < STRATEGY_COUNT; i++)
    {
        // Highlight current strategy
        if (i == currentStrategy)
        {
            // Draw bitmap arrow
            display.drawBitmap(15, y_positions[i] + 3, image_Pin_arrow_right_bits, 9, 7, 1);
            display.setCursor(30, y_positions[i]);
            display.setTextSize(1);
            display.print(strategy_names[i]);
            display.setTextSize(1);
        }
        else
        {
            display.setCursor(30, y_positions[i]);
            display.print(strategy_names[i]);
        }
    }

    display.display();
}

// Direction indicator arrow bitmaps
static const unsigned char PROGMEM image_arrow_diagonal_left_up_bits[] = {0xff, 0xff, 0xf0, 0x00, 0xff, 0xff, 0xf0, 0x00, 0xff, 0xff, 0xf0, 0x00, 0xff, 0xff, 0xf0, 0x00, 0xff, 0xff, 0xf0, 0x00, 0xff, 0xc0, 0x00, 0x00, 0xff, 0xc0, 0x00, 0x00, 0xff, 0xc0, 0x00, 0x00, 0xff, 0xc0, 0x00, 0x00, 0xff, 0xc0, 0x00, 0x00, 0xf8, 0x3e, 0x00, 0x00, 0xf8, 0x3e, 0x00, 0x00, 0xf8, 0x3e, 0x00, 0x00, 0xf8, 0x3e, 0x00, 0x00, 0xf8, 0x3e, 0x00, 0x00, 0xf8, 0x01, 0xf0, 0x00, 0xf8, 0x01, 0xf0, 0x00, 0xf8, 0x01, 0xf0, 0x00, 0xf8, 0x01, 0xf0, 0x00, 0xf8, 0x01, 0xf0, 0x00, 0x00, 0x00, 0x0f, 0x80, 0x00, 0x00, 0x0f, 0x80, 0x00, 0x00, 0x0f, 0x80, 0x00, 0x00, 0x0f, 0x80, 0x00, 0x00, 0x0f, 0x80};

static const unsigned char PROGMEM image_arrow_diagonal_right_up_bits[] = {0x07, 0xff, 0xff, 0x80, 0x07, 0xff, 0xff, 0x80, 0x07, 0xff, 0xff, 0x80, 0x07, 0xff, 0xff, 0x80, 0x07, 0xff, 0xff, 0x80, 0x00, 0x01, 0xff, 0x80, 0x00, 0x01, 0xff, 0x80, 0x00, 0x01, 0xff, 0x80, 0x00, 0x01, 0xff, 0x80, 0x00, 0x01, 0xff, 0x80, 0x00, 0x3e, 0x0f, 0x80, 0x00, 0x3e, 0x0f, 0x80, 0x00, 0x3e, 0x0f, 0x80, 0x00, 0x3e, 0x0f, 0x80, 0x00, 0x3e, 0x0f, 0x80, 0x07, 0xc0, 0x0f, 0x80, 0x07, 0xc0, 0x0f, 0x80, 0x07, 0xc0, 0x0f, 0x80, 0x07, 0xc0, 0x0f, 0x80, 0x07, 0xc0, 0x0f, 0x80, 0xf8, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x00, 0x00};

static const unsigned char PROGMEM image_arrow_up_bits[] = {0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x0f, 0xff, 0x00, 0x0f, 0xff, 0x00, 0x0f, 0xff, 0x00, 0x0f, 0xff, 0x00, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00};

static const unsigned char PROGMEM image_arrow_down_bits[] = {0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0x0f, 0xff, 0x00, 0x0f, 0xff, 0x00, 0x0f, 0xff, 0x00, 0x0f, 0xff, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00};

void Display::drawDirectionIndicatorScreen(int motorDirection, int leftMotorPWM, int rightMotorPWM)
{
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setTextWrap(false);

    // Title
    display.setCursor(0, 0);
    display.print("DIRECTION");

    // Display arrows based on motor direction
    // Arrow positions: left diagonal (13,27), forward (54,0), right diagonal (93,26), backward (54,36)

    switch (motorDirection)
    {
    case DIRECTION_FORWARD:
        // Show forward arrow
        display.drawBitmap(54, 20, image_arrow_up_bits, 20, 28, 1);
        break;

    case DIRECTION_BACKWARD:
        // Show backward arrow
        display.drawBitmap(54, 20, image_arrow_down_bits, 20, 28, 1);
        break;

    case DIRECTION_LEFT:
        // Show left arrow
        display.drawBitmap(35, 25, image_arrow_diagonal_left_up_bits, 25, 25, 1);
        break;

    case DIRECTION_RIGHT:
        // Show right arrow
        display.drawBitmap(74, 25, image_arrow_diagonal_right_up_bits, 25, 25, 1);
        break;

    case DIRECTION_STOP:
    default:
        // Show nothing
        display.setCursor(50, 30);
        display.print("STOPPED");
        break;
    }

    // Display PWM values at the bottom
    display.setTextSize(1);
    display.setCursor(0, 56);
    display.print("PWM: ");
    display.print(leftMotorPWM);
    display.print(", ");
    display.print(rightMotorPWM);

    display.display();
}

void Display::drawSpeedSelectorScreen(int currentSpeedLevel)
{
    if (!shouldUpdate())
        return;

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setTextWrap(false);

    // Title
    display.setCursor(0, 0);
    display.print("SPEED SELECTOR");

    // Draw three speed options with highlight
    const char *speedNames[] = {"LOW", "MEDIUM", "HIGH"};
    uint8_t y_positions[] = {16, 32, 48};

    for (int i = 0; i < SPEED_LEVEL_COUNT; i++)
    {
        // Highlight current speed level
        if (i == currentSpeedLevel)
        {
            // Draw bitmap arrow
            display.drawBitmap(15, y_positions[i] + 3, image_Pin_arrow_right_bits, 9, 7, 1);
            display.setCursor(30, y_positions[i]);
            display.setTextSize(2);
            display.print(speedNames[i]);
            display.setTextSize(1);
        }
        else
        {
            display.setCursor(30, y_positions[i]);
            display.print(speedNames[i]);
        }
    }

    display.display();
}

void Display::drawBatteryTemperatureScreen(float batteryVoltage, float temperatureC, bool motorTestActive, int motorTestSelection)
{
    if (!shouldUpdate())
        return;

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextWrap(false);

    if (motorTestActive)
    {
        const char *motorTestNames[] = {"forward", "backward", "right", "left"};
        uint8_t y_positions[] = {14, 26, 38, 50};

        display.setTextSize(1);
        display.setCursor(0, 0);
        display.print("motor test");

        for (int i = 0; i < MOTOR_DIAG_COUNT; i++)
        {
            if (i == motorTestSelection)
            {
                display.drawBitmap(15, y_positions[i] + 3, image_Pin_arrow_right_bits, 9, 7, 1);
                display.setCursor(30, y_positions[i]);
                display.print(motorTestNames[i]);
            }
            else
            {
                display.setCursor(30, y_positions[i]);
                display.print(motorTestNames[i]);
            }
        }

        display.display();
        return;
    }

    char voltageText[16];
    snprintf(voltageText, sizeof(voltageText), "%.2f V", batteryVoltage);

    char tempText[16];
    if (isnan(temperatureC))
    {
        snprintf(tempText, sizeof(tempText), "N/A C");
    }
    else
    {
        snprintf(tempText, sizeof(tempText), "%.1f C", temperatureC);
    }

    auto drawCenteredLine = [this](const char *text, int16_t y, uint8_t size)
    {
        int16_t x1 = 0;
        int16_t y1 = 0;
        uint16_t w = 0;
        uint16_t h = 0;
        display.setTextSize(size);
        display.getTextBounds(text, 0, y, &x1, &y1, &w, &h);
        int16_t x = (SCREEN_WIDTH - static_cast<int16_t>(w)) / 2;
        if (x < 0)
        {
            x = 0;
        }
        display.setCursor(x, y);
        display.print(text);
    };

    drawCenteredLine("diagnostics", 2, 1);
    drawCenteredLine(voltageText, 18, 2);
    drawCenteredLine(tempText, 40, 2);

    display.display();
}
