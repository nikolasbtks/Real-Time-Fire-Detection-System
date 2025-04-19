# Real Time Fire Detection System

## Overview

This project demonstrates a Real Time Fire Detection System that reads analog sensor data, filters it, and then sends it wirelessly to a Web API for real-time analysis using a Machine Learning Model.


## System Architecture

The general architecture of the systems is divided into five parts:

#### 1. Sensor Processing Unit
The **Sensor Processing Unit**, using the `ATmega328P microcontroller`, retrieves the analog readings from the fire sensor.
   
#### 2. UART Communication
Using **UART communication**, the sensor data is transferred to the **WiFi Communication Unit**.
   
#### 3. WiFi Communication Unit
The **WiFi Communication Unit** uses the `Renesas RA4M1 microcontroller` and the `ESP32-S3 module` to send an HTTP request containing the data to a Web API for analysis.
   
#### 4. Web API
The **Web API** receives the data via a POST request and forwards it to the **machine learning model** for further analysis.
  
#### 5. Machine Learning Model
The **machine learning model** analyzes the data and determines whether:
   - there is no fire
   - there is a suspicious activity
   - the is a fire
     
The result is returned to the Web API, which sends a response with the result.

## Sensor Processing Unit

The **Sensor Processing Unit** is responsible for retrieving analog data from the analog fire sensor. Its main puropose it to generate a **sequence of 10 values**, which is then passed to the **WiFi Communication Unit**.

To avoid sending identical consecutive data, a **filtering mechanism** is used. This mechanism calculates the **average of the current sequence** and compares it with the **average of the previously sent sequence**. If the current average is **greater by at least 5 units**, the sequence is considered valid and is transmitted. 
Otherwise, it is ignored.

This functionality is implemented using **two tasks**:

#### Task 1: Data Collection
   - Retrieves 10 analog values from the fire sensor.
   - Stores them in an array of 10 elements, forming a sequence.

#### Task 2: Filtering and Validation
   - Receives the 10-value array after it is populated.
   - Applies the filtering mechanism to determine if the sequence should be transmitted based on the average value comparison.

## UART Communication 

The **UART Communication** is used for transmitting data from the `ATmega328P` microcontroller  to the `Renesas RA4M1` microcontroller.

More specifically, when the data is ready to be sent from the **Sensor Processing Unit**, a **communication bridge** is created using the `SoftwareSerial` library on the `ATmega328P` microcontroller. This bridge enables serial communication with the `Renesas RA4M1` microcontroller, allowing the data to be successfully transferred to **WiFi Communication Unit**.

Both microcontrollers operate at **5V**, so there is **no need for logic level converter** between them.


## WiFi Communication Unit

The **WiFi Communication Unit** is responsible for sending the sequence of analog data to the **Web API** for processing.
Its main role is to receive the analog sequence data via **UART Communication**, and then transmit it to the Web API through an **HTTP request**.

The unit reads 20 bytes from the **UART Communication**. Then these bytes are stored in a temporary array. And finally, they are converted into 10 integers (2 bytes each) using the little-endian format.

An HTTP request is created using the WiFiS3 library and the ESP32-S3 module.
The request includes:
   - The sequence of analog values.
   - A manufacture code to identify which device sent the data (useful in multi-device systems).

This data is formatted as **JSON** and sent to the Web API for further analysis.

This functionality is implemented using **two tasks**:

#### Task 1: Data Collection
   - Receives the 20 bytes from **UART Communication**.
   - Stores them into a local array.
     
#### Task 2: Request Preparation & Sending
   - Converts the byte array into 10 integers.
   - Constructs a JSON object containing:
      - The analog sequence data. 
      - The manufacture code.
   - Sends the HTTP request to the Web API.

   
## Web API

The Web API creates an interface for real-time fire predictions based on analog sensor data. It is developed using the Flask framework and a pre-trained machine learning model.

A Python virtual environment was created for dependency management. The key libraries used in the project are `flask`, `datetime`, `numpy` and `joblib`.

The overall functionality includes:

#### Model Loading
   - The pre-trained model called `model.pkl` is loaded at the application using `joblib`.

#### API Endpoints
   - `GET /`
     - Represents a health check endpoint. Returns an empty response confirming that the server is running.
   - `POST /process`
     - Accepts a JSON payload containing the `manufacture_code` and the 10-value analog `sequence` from the sensor.
     - Validates the input from the request data and extracts `features` from the provided sequence.
     - Passes the extracted features from the provided sequence into the **machine learning model** to obtain a `prediction`.
     - Generates a `timestamp` include date and time.
     - Returns a JSON resposes containing the `manufacture_code` , the predicted `result`, and the `timestamp`.

#### Deployment
   - The Flask application was hosted on a Raspberry Pi 5. The server listens on `0.0.0.0:5000`, enabling access from devices within the local network.

## Machine Learning Model

The **Machine Learning Model** is developed and trained using Google Colab. It is designed classify analog fire sensor data into three categories:
   - No Fire (sensor values between 900-1024)
   - Suspicious Activity (between 31-899 or around 24-26)
   - Fire (values between 10-30)

Training and evaluation of the model follow these steps:
#### 1. Synthetic Data Generation
   Generates synthetic samples, each with 10 analog sensor readings, and randomly assigns them to one of three classes based on their value patterns. 
#### 2. Feature Extraction
   For each 10-value sequence, basic statistical features are extracted, including the mean, standard deviation, minimum, maximum, range, and mean of consecutive differences. 
#### 3. Training and Evaluation
   Features are split into balanced training (80%) abd test (20%) sets. A Random Forest with 100 trees is trained on the training set, and evaluated on the test set using accuracy and classification report.
#### 4. Test Predictions
  A new 10-value sequence is processed using the feature extraction, and the trained model predicts whether it represents no fire, suspicious activity or fire.

## Examples

Below you can find some example usage or example outputs of the system:

- Sending a 10-value analog sequence like **[1024,1011,1004,1020,1021,990,900,911,1024,990]** results in the prediction: **No Fire**.
- Sending a 10-value analog sequence like **[550,225,330,650,313,423,547,143,120,135]** results in the prediction: **Suspicious Activity**.
- Sending a 10-value analog sequence like **[45,28,26,27,40,34,32,32,32,28]** results in the prediction: **Suspicious Activity**.
- Sending a 10-value analog sequence like **[15,23,12,20,20,21,10,21,22,10]** results in the prediction: **Fire**.

## Installation & Setup

Below you can find a short guide on how to set up and run your system:

1. Flash the Sensor Processing Unit (`detection` sketch) onto the `ATmega328P` and connect the fire sensor for data collection and filtering.
2. Flash the WiFi Communication Unit (`wireless` sketch) onto the `Renesas RA4M1` + `ESP32-S3`.
4. Set up the Web API on a `Raspberry Pi 5` by creating a Python virtual enviroment, installing the required packages and running the server.
5. Connect all the devices in the same **WiFi network**.

#### Note
If you find any bugs or problem, please double-check hardware connections or network settings. Feel free to report any problem.
