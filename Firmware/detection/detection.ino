#include <Arduino_FreeRTOS.h>
#include <SoftwareSerial.h>
#include <math.h>

#define analogPin A0
#define BUFFER 10

SoftwareSerial comBridge(10,11);

const float Threshold = 5.0;

int value_counter = 0;
int valueData[BUFFER];
int cachedData[BUFFER];

uint8_t lock = 0;

void calculateTask(void *pvParameter);
void dataTask(void *pvParameter);

void setup() {
  Serial.begin(9600);
  comBridge.begin(9600);

  xTaskCreate(calculateTask, "calculateTask", 120, NULL, 1, NULL);
  xTaskCreate(dataTask, "dataTask", 120, NULL, 1, NULL);
}

void loop() {
}

void calculateTask(void *pvParameter)
{
  while(1)
  {
    if(value_counter == 10)
    {
      if(lock == 0)
      {
        lock = 1;
        comBridge.write((byte*)valueData, sizeof(valueData));
        memcpy(cachedData, valueData, sizeof(cachedData));
      }
      else
      {
        int sum1 = 0;
        int sum2 = 0;

        for(int i = 0; i < BUFFER; i++)
        {
          sum1 += cachedData[i];
          sum2 += valueData[i];
        }

        float avg1 = sum1 / 10.0;
        float avg2 = sum2 / 10.0;
        float calculated_threshold = fabs(avg1 - avg2);

        if(calculated_threshold >= Threshold)
        {
          comBridge.write((byte*)valueData, sizeof(valueData));
          memcpy(cachedData, valueData, sizeof(cachedData)); 
        }
      }
      value_counter = 0;
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void dataTask(void *pvParameter)
{
  while(1)
  {
    if(value_counter < 10)
    {
      valueData[value_counter] = analogRead(analogPin);
      value_counter++;
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}