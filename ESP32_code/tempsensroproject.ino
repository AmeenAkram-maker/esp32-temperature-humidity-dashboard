#include "DHT.h"
#include <Wire.h> // i^2C protocol needed for communicating with the oled display 
#include <Adafruit_GFX.h> // general functions and graphics for screen
#include <Adafruit_SSD1306.h> // specific driver library for the oled screen
#include <WiFi.h>
#include <WebServer.h>
// the above are essential libraries for each compoennts liek the dht11 sensor and the screen, though i have some wifi and web server stuff for esp32 to communicate with laptop

#define SCREEN_WIDTH 128 // this defines the height and width fo the small oled screen i have in pixels
#define SCREEN_HEIGHT 32
#define OLED_RESET -1  // sets reset pin to zero as it shares with esp32
#define DHTPIN 4
#define DHTTYPE DHT11 // defines the dht pin, type and then creates an object for it 
DHT dht(DHTPIN, DHTTYPE);

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // creates a display object with the correct widht and hegiht, wire is a communication protocol

// bellow are the global variables used in the loop and in various functions.

int amberpin = 14; // amber LED pin 
int greenpin = 27; // green LED pin
int redpin = 12; // red LED pin
int bluepin = 13; // blue LED pin
float temp = 0.0;  // global tempearture variable 
float humidity = 0.0; // global humidity variable
// gpio pins to the respective pins that each signify a different tempearture, blue being cold and red being very hot 
int resetbutton = 19; // this is the reset button that when clicked, autoamtically sets the motorspeed to 0
int pwmpin = 26; // the pin that rapidly injects a current into transsitor base 
int pwmfrequency = 5000; // this is the frequency used to rapidly switch between logic 0 and logic 3.3 
int pwmbits = 8;   // 0–255 duty. so 255 is the maximum and 0 is the minimum (speed of the motor)
int motorspeed = 0;// global motor speed variable
int lastTime = 0; // this is a variable used for the time tracking statement. i use it to essentially track how long in milliseconds the esp32 has been running for 

// WIFI LOGIC BELLOW 

const char* ssid = ""; // network ssid
const char* password = ""; // network password 

WebServer server(80); // creates a server with port 80

void setupserver() { // this starts the web server
  server.on("/data", handledata); // when someoen visits /data then run the hadnel data function
  server.begin(); // literally starts the web server and then the server listens for requests
  Serial.println("http server has started");  // output to the console to show that the server has started
}

void CheckWifi() { // check if the network connection is okay 
  if(WiFi.status() == WL_DISCONNECTED) { // checks if the connection status is anyhting other than connected i.e disconnected
    Serial.println("wifi is not connected, try reconnect"); // if the wifi is not connected output to console 
    WiFi.disconnect(); // disconnect from the network,  and then try again 
    WiFi.begin(ssid, password); // attempt to start the network again 
  }
}

void startWiFi() { // tries to establish the connection between computer and esp32
 
  WiFi.mode(WIFI_STA); // esp32 acts like a server, we are sending the data over.
  WiFi.begin(ssid, password); // start the network with the given SSID and network password

  Serial.println("connecting to the wifi"); // show we are trying to connect to network

  int attempts = 0; // set a number of attempts
  while (WiFi.status() != WL_CONNECTED && attempts < 20) { // check if the network is not connected and we havent reached liomit of 20 connection attempts
    delay(500); // if it is the case above then delay the programme and increment attempts. this runs untill the wifi is connected and we havent reached our limi 
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) { // if the network connects then ouytput the ip address of the esp32
    Serial.println("\nyou have connected to the wifi"); 
    Serial.println(WiFi.localIP());
  }
  else {
      Serial.println("\nyou have not connected to the wifi"); // if the above isnt true then we arent connected, hence output it to terminal 
    }
  }

void handledata() { // intended to send data when pc requests
  // float temp = dht.readTemperature(); // store temp 
  // float humidity = dht.readHumidity(); // store humidity 
  // just realised the above is bad, making four variables in total instead of just 2

  temp = dht.readTemperature(); // new approach i just modify the existying global variables
  humidity = dht.readHumidity(); // do the same again for humidity. 
  String json = "{"; // start of a json string with opening bracket 
  json += "\"temp\":" + String(temp) + ",";
  json += "\"humidity\":" + String(humidity) + ",";
  json += "\"motorspeed\":" + String(motorspeed) + ",";
  json += "\"time\":" + String(millis());
  // all the above are the same its literally just converting each of th variables into a string an then added onto the json string 
  json += "}";
  // json is just text, i have two variables temp and humidity that needs to be converted to text so add it to a json string and then send it over, 200 means sucess in httpo code and application/json tells the pc that it is json data, finally json is the actual data
  server.send(200, "application/json", json); 
}

// below is the setup loop, everything here runs just once and hence it is used for initialization fo cponents as you can see by pinmode etc. it also manages the start up of the network 

void setup() {
  Serial.begin(115200);
  startWiFi(); // moved from loop to here cus you only need it once, call start wifi which initialises the web connection via wifi.begin 
  setupserver(); // set up the web server for sending data 
  pinMode(amberpin, OUTPUT); // set up amber led
  pinMode(redpin, OUTPUT); // set up red led 
  pinMode(greenpin, OUTPUT); // set up green led 
  pinMode(bluepin, OUTPUT); // set up blue led 
  pinMode(resetbutton, INPUT_PULLUP); // this is the reset button, when pressed it stops the motor from spinning. 
  dht.begin(); // set up the DHT sensor 
  ledcAttach(pwmpin, pwmfrequency, pwmbits);// connects gpio pin 26 to a pwm signal which has the frequency i made before and  the number of bits i stated before (8 so 255 levels)
  ledcWrite(pwmpin, 0); // thehn this  just sets the pwm channel to 0. i.e the motor is off to begin. 

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // this is a condition for the OLED display. if it is not setup then output to the screen. inside the begin function i also have the 0x3C which is the communcation address for I^2c. 
    Serial.println("OLED allocation failed");
    while (true);
  }

  display.clearDisplay(); // wipe the display clean before we continue to the main loop 
  display.display();
}

void loop() {
  // this is a non blocking timer which basically only executes the measurements every 2 sconds. im using this as opposed to simply using delay ebcause delay completely stops the esp32 for 2 seconds which is not good practice as  there are many moving elements to the project. 
  if (millis() - lastTime >= 2000) {
    lastTime = millis();
    temp = dht.readTemperature(); // moved them to here so can record the time at whihc they are taken 
    humidity = dht.readHumidity();

    if (isnan(temp) || isnan(humidity)) { // if the readings that we are getting are nans then we output that there was an error and then write to the pin that it is 0. 
    Serial.println("Error reading DHT! EMI detected. Shutting down motor...");
    ledcWrite(pwmpin, 0); // motor should not be running if we have incorrect measurements or readings. also motor might be generating too much noise if this is occuring so temporarily stop it 
    return;               // skip loop since we dont have temperature or humidity readings (valid ones anyway)
  }

  // below is the tempearture threshold logic. all im doing is checking if the temperature is withi a certain range and if it si then set the pwm cycle accordingly and also write to each led to indicate what the status is. i.e cold means blue, hot means red. (put simpl;y)
    if (temp > 33) {
    motorspeed = 240;
    digitalWrite(redpin, HIGH);
    digitalWrite(amberpin, LOW);
    digitalWrite(greenpin, LOW);
    digitalWrite(bluepin, LOW);
  }
  else if (temp >= 30) {
    motorspeed = 192;
    digitalWrite(amberpin, HIGH);
    digitalWrite(redpin, LOW);
    digitalWrite(greenpin, LOW);
    digitalWrite(bluepin, LOW);
  }
  else if (temp >= 24) {
    motorspeed = 128;
    digitalWrite(greenpin, HIGH);
    digitalWrite(amberpin, LOW);
    digitalWrite(redpin, LOW);
    digitalWrite(bluepin, LOW);
  }
  else {
    motorspeed = 0;
    digitalWrite(bluepin, HIGH);
    digitalWrite(amberpin, LOW);
    digitalWrite(redpin, LOW);
    digitalWrite(greenpin, LOW);
  }

  if (digitalRead(resetbutton) == LOW) {
    motorspeed = 0; // this is the reset button, if it is pressed then it becomes a LOW as i used pullup before. this forces the mtor to stop instantly (motorspeed = 0), as opposed to switching supply on and off due to motor stalling on startup
  } 
  
  ledcWrite(pwmpin, motorspeed); // after writing to the LEDS, we want to make sure the pwm pin is updated with the correct pwm cycle 
  // all ledc write does is  sets the duty cycle using the frequency and channel that i defined before 
  
  display.clearDisplay(); // since we are at the end of the loop clear the display 
  display.setTextSize(1); // set the text size to small 
  display.setTextColor(SSD1306_WHITE); // the colour supported by this display is WHITE

  display.setCursor(0, 0); // start at the top left of the screen
  display.print("Temp: "); // output the tempearture
  display.print(temp);

  display.setCursor(0, 10); // start at the middle of the screen
  display.print("Motor: "); // output the motorspeed
  display.print(motorspeed);

  display.setCursor(0, 20); // start at th lower left partt of the screen
  display.print("humidity: ");
  display.print(humidity); // output the humidity
  display.display(); // actually display everything 

  //  serial output, otput the tempearture, motorspeed, and humidity. 
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.print(" | Motor speed: ");
  Serial.println(motorspeed);
  Serial.print("| humidity: ");
  Serial.println(humidity);
  }
  CheckWifi(); // check the wifi connection at the end of the loop
  server.handleClient(); // deals with incoming http requests
}