#include <Arduino.h>
#include <BleGamepad.h>
#include <map>

using namespace std;

#define GAMEPAD_BUTTON_SHIFTER_L BUTTON_1
#define GAMEPAD_BUTTON_SHIFTER_R BUTTON_2
#define GAMEPAD_BUTTON_1         BUTTON_3
#define GAMEPAD_BUTTON_2         BUTTON_4
#define GAMEPAD_BUTTON_3         BUTTON_5
#define GAMEPAD_BUTTON_4         BUTTON_6
#define GAMEPAD_BUTTON_5         BUTTON_7
#define GAMEPAD_BUTTON_6         BUTTON_8
#define GAMEPAD_BUTTON_7         BUTTON_9
#define GAMEPAD_BUTTON_8         BUTTON_10
#define GAMEPAD_BUTTON_9         BUTTON_11
#define GAMEPAD_BUTTON_10        BUTTON_12

#ifdef GBP_DEVICE_TYPE_ESP32S_18650
#define SHIFTER_L_GPIO GPIO_NUM_22
#define SHIFTER_R_GPIO GPIO_NUM_23
#define BUTTON_1_GPIO  GPIO_NUM_21
#define BUTTON_2_GPIO  GPIO_NUM_19
#define BUTTON_3_GPIO  GPIO_NUM_18
#define BUTTON_4_GPIO  GPIO_NUM_17
#define BUTTON_5_GPIO  GPIO_NUM_4
#define BUTTON_6_GPIO  GPIO_NUM_32
#define BUTTON_7_GPIO  GPIO_NUM_33
#define BUTTON_8_GPIO  GPIO_NUM_25
#define BUTTON_9_GPIO  GPIO_NUM_26
#define BUTTON_10_GPIO GPIO_NUM_27
#endif

#ifdef GBP_DEVICE_TYPE_ESP32_S3_DEVKITM_1
#define SHIFTER_L_GPIO GPIO_NUM_47
#define SHIFTER_R_GPIO GPIO_NUM_48
#define BUTTON_1_GPIO  GPIO_NUM_5
#define BUTTON_2_GPIO  GPIO_NUM_6
#define BUTTON_3_GPIO  GPIO_NUM_7
#define BUTTON_4_GPIO  GPIO_NUM_15
#define BUTTON_5_GPIO  GPIO_NUM_16
#define BUTTON_6_GPIO  GPIO_NUM_17
#define BUTTON_7_GPIO  GPIO_NUM_18
#define BUTTON_8_GPIO  GPIO_NUM_8
#define BUTTON_9_GPIO  GPIO_NUM_9
#define BUTTON_10_GPIO GPIO_NUM_10
#endif

std::map<int, int> buttonBindings = {
  { BUTTON_1_GPIO, BUTTON_1 },
  { BUTTON_2_GPIO, BUTTON_2 },
  { BUTTON_3_GPIO, BUTTON_3 },
  { BUTTON_4_GPIO, BUTTON_4 },
  { BUTTON_5_GPIO, BUTTON_5 },
  { BUTTON_6_GPIO, BUTTON_6 },
  { BUTTON_7_GPIO, BUTTON_7 },
  { BUTTON_8_GPIO, BUTTON_8 },
  { BUTTON_9_GPIO, BUTTON_9 },
  { BUTTON_10_GPIO, BUTTON_10 },
  { SHIFTER_L_GPIO, BUTTON_11 },
  { SHIFTER_R_GPIO, BUTTON_12 },
};

BleGamepad bleGamepad("GenericButtonPlate", "srounce");

uint8_t getBatteryPercent(void);
uint8_t batteryLevel;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");
  //Serial.setDebugOutput(true);

  setCpuFrequencyMhz(80);

  BleGamepadConfiguration gamepadConfig;
  gamepadConfig.setControllerType(CONTROLLER_TYPE_JOYSTICK);
  gamepadConfig.setAutoReport(false);
  gamepadConfig.setButtonCount(buttonBindings.size());
  gamepadConfig.setHatSwitchCount(0);
  gamepadConfig.setWhichAxes(false, false, false, false, false, false, false, false);
  gamepadConfig.setWhichSpecialButtons(false, false, false, false, false, false, false, false);
  gamepadConfig.setWhichSimulationControls(false, false, false, false, false);

  gamepadConfig.setVid(0x5411);
  gamepadConfig.setPid(0x6969);
  
  for (const auto &binding : buttonBindings) {
    pinMode(binding.first, INPUT_PULLUP);
  }

  bleGamepad.begin(&gamepadConfig);
}

bool hasConnected = false;
bool canSendBatteryState = false;
unsigned long int connectedTime = 0;

void loop() {
  if (bleGamepad.isConnected())
  {
    unsigned long int now = millis();
    if (hasConnected == false) {
      connectedTime = now;
    }
    hasConnected = true;

    // wait for 3 secs to send battery info
    if (!canSendBatteryState && now - connectedTime > 3000) {
      canSendBatteryState = true;
    }

    if (canSendBatteryState) {
      bleGamepad.setBatteryPowerInformation(POWER_STATE_PRESENT);
      batteryLevel = getBatteryPercent();
      bleGamepad.setBatteryLevel(batteryLevel);
    }

    for (const auto &binding : buttonBindings) {
      if(!digitalRead(binding.first)) {
        bleGamepad.press(binding.second);
      } else {
        bleGamepad.release(binding.second);
      }
    }
    
    bleGamepad.sendReport();
    delay(5);
  } else {
    if (hasConnected == true) {
      hasConnected = false;
      connectedTime = 0;
      canSendBatteryState = false;
    }
  }
}

#define ADC_MAX_VOLTAGE             3.3
#define ADC_MIN_VOLTAGE             0.15
#define ADC_BATTERY_DIVIDER         2.2
#define BATTERY_VOLTAGE_IIR_FILTER  0.01

#define BATTERY_MAX           4.15     // 100% battery
#define BATTERY_MIN           3.2     // 0% battery 

#define BATTERY_ADC_PIN 36

uint8_t getBatteryPercent()
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

  batteryPercent = max(0, min(100, (int)round(batteryPercent)));
  
  return (uint8_t)batteryPercent;
}
