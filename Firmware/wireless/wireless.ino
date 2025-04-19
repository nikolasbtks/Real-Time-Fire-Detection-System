#include <Arduino_FreeRTOS.h>
#include <WiFiS3.h>
#include <string.h>

const char *manufacture_code = "alphaCode118-2025A"
const char *cred_server = "";
const char *cred_port = "";
const char *cred_ssid = "";
const char *cred_password = "";

int valueData[10];
byte tempBuffer[20];
int byteIndx = 0;

volatile bool dataReady = false;
volatile bool transmittingData = false;

WiFiClient client;

void transmitTask(void *pvParameter);
void communicationTask(void *pvParameter);

void setup() {
  Serial1.begin(9600);

  WiFi.begin(cred_ssid,cred_password);
  while(WiFi.status() != WL_CONNECTED)
  {
    vTaskDelay(pdMS_TO_TICKS(1000));    
  }

  xTaskCreate(transmitTask, "transmitTask", 520, NULL, 1, NULL);
  xTaskCreate(communicationTask, "communicationTask", 520, NULL, 1, NULL);

  vTaskStartScheduler();
}

void loop() {
}

void communicationTask(void *pvParameter)
{
  while(1)
  {
    while(Serial1.available())
    {
      if(byteIndx < 20)
      {
        tempBuffer[byteIndx++] = Serial1.read();
      }
      if(!transmittingData && byteIndx >= 20)
      {
        for(int i = 0; i < 10; i++)
        {
          byte low = tempBuffer[i*2];
          byte high = tempBuffer[i*2+1];
          valueData[i] = low | (high << 8);
        }
        dataReady = true;
        byteIndx = 0;
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));    
  }
}

void transmitTask(void *pvParameter)
{
  while(1)
  {
    if(dataReady == true)
    {
      transmittingData = true;
      if(client.connected())
      {
        client.stop();
      }

      if(client.connect(cred_server,cred_port))
      {
        String jsonPayload = "{ \"manufacture_code\": \"" + String(manufacture_code) + "\", \"sequence\": [";
        for(int i = 0; i < 10; i++)
        {
          jsonPayload += String(valueData[i]);
          if(i <= 8)
          {
            jsonPayload += ",";
          }
        }
        jsonPayload += "]  }";

        client.println("POST /process HTTP/1.1");
        client.print("Host: ");
        client.println(cred_server);
        client.println("Content-Type: application/json");
        client.print("Content-Length: ");
        client.println(jsonPayload.length());
        client.println("Connection: close");
        client.println();
        client.println(jsonPayload);

        client.stop();
        dataReady = false;
        transmittingData = false;
      }
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}