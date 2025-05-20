#include <Arduino.h>
#include <BleGamepad.h>

#define SHIFTER_L_GPIO 22
#define SHIFTER_R_GPIO 23

#define GAMEPAD_BUTTON_SHIFTER_L BUTTON_1
#define GAMEPAD_BUTTON_SHIFTER_R BUTTON_2

BleGamepad bleGamepad("GenericButtonPlate", "srounce");

uint8_t getBatteryPercent(void);
uint8_t batteryLevel;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");
  //Serial.setDebugOutput(true);

  setCpuFrequencyMhz(120);

  BleGamepadConfiguration gamepadConfig;
  gamepadConfig.setControllerType(CONTROLLER_TYPE_JOYSTICK);
  gamepadConfig.setAutoReport(false);
  gamepadConfig.setButtonCount(2);
  gamepadConfig.setHatSwitchCount(0);
  gamepadConfig.setWhichAxes(false, false, false, false, false, false, false, false);
  gamepadConfig.setWhichSpecialButtons(false, false, false, false, false, false, false, false);
  gamepadConfig.setWhichSimulationControls(false, false, false, false, false);

  gamepadConfig.setVid(0x5411);
  gamepadConfig.setPid(0x6969);
  
  pinMode(SHIFTER_L_GPIO, INPUT_PULLUP);
  pinMode(SHIFTER_R_GPIO, INPUT_PULLUP);

  bleGamepad.begin(&gamepadConfig);
}

void loop() {
  if (bleGamepad.isConnected())
  {
    batteryLevel = getBatteryPercent();
    bleGamepad.setBatteryLevel(batteryLevel);
    
    if(!digitalRead(SHIFTER_L_GPIO)) 
      bleGamepad.press(GAMEPAD_BUTTON_SHIFTER_L);
    else
      bleGamepad.release(GAMEPAD_BUTTON_SHIFTER_L);
  
    if(!digitalRead(SHIFTER_R_GPIO)) 
      bleGamepad.press(GAMEPAD_BUTTON_SHIFTER_R);
    else
      bleGamepad.release(GAMEPAD_BUTTON_SHIFTER_R);
    
    bleGamepad.sendReport();
    delay(5);
  }
}

#define ADC_MAX_VOLTAGE             3.3
#define ADC_MIN_VOLTAGE             0.15
#define ADC_BATTERY_DIVIDER         2.2
#define BATTERY_VOLTAGE_IIR_FILTER  0.01

#define BATTERY_MAX           4.15     // 100% battery
#define BATTERY_MIN           3.2     // 0% battery 

#define BATTERY_ADC_PIN 36

uint8_t getBatteryPercent(void)
{
  static float adcValue;
  float batteryVoltage;
  float batteryPercent;
  
  if(adcValue == 0) 
    adcValue = analogRead(BATTERY_ADC_PIN); // First read
  else
    adcValue = analogRead(BATTERY_ADC_PIN)*BATTERY_VOLTAGE_IIR_FILTER + adcValue*(1-BATTERY_VOLTAGE_IIR_FILTER);

  // Calculate voltage
  batteryVoltage = (adcValue * (ADC_MAX_VOLTAGE - ADC_MIN_VOLTAGE) / 4095 + ADC_MIN_VOLTAGE) * ADC_BATTERY_DIVIDER;
  
  // Percent estimation (depending very much on battery type, so simple linear calculation used
  batteryPercent = (batteryVoltage - BATTERY_MIN) * 100 / (BATTERY_MAX - BATTERY_MIN);

  if(batteryPercent < 0) batteryPercent = 0;
  if(batteryPercent > 100) batteryPercent = 100;
  
  return (uint8_t)batteryPercent;
}
