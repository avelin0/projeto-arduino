/**
* Telegram bot + temperature sensor
*
* @author  Bruno Avelino e Heitor Magnani
* @version 1.0
* @since   2021-04-15 
*/

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN            4         // Pin which is connected to the DHT sensor.

// Uncomment the type of sensor in use:
//#define DHTTYPE           DHT11     // DHT 11 
#define DHTTYPE           DHT22     // DHT 22 (AM2302)
//#define DHTTYPE           DHT21     // DHT 21 (AM2301)

DHT_Unified dht(DHTPIN, DHTTYPE);

uint32_t delayMS;

// Initialize Wifi connection to the router
char ssid[] = "";     // your network SSID (name)
char password[] = ""; // your network key

String leitura;
float leituraTemperatura;
float leituraUmidade;

// Initialize Telegram BOT
#define BOTtoken ""  // your Bot Token (Get from Botfather)

BearSSL::WiFiClientSecure client;

UniversalTelegramBot bot(BOTtoken, client);

int Bot_mtbs = 1000; //mean time between scan messages
long Bot_lasttime;   //last time messages' scan has been done

int contador = 0;

void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    String from_name = bot.messages[i].from_name;
    if (from_name == "") from_name = "Guest";
    
    if (text == "/start") {
      bot.sendChatAction(chat_id, "typing");
      delay(1000);
      String welcome = "Bem Vindo ao SubZero Bot, " + from_name + ".\n";
      welcome += "Bot de Automação de Leitura de Sensor do 11º CT.\n\n";
      welcome += "/leitura : para fazer a leitura de temperatura em °C e umidade em %\n";
      
      bot.sendMessage(chat_id, welcome);
    }
    
    if (text == "/leitura") {
        bot.sendChatAction(chat_id, "typing");
        delay(1000);

        leitura = "-> Leitura\r\n";
        leitura += "Temperatura: ";
        leitura += (String) leituraTemperatura;
        leitura += " °C "; 
        leitura += "\r\nUmidade: "; 
        leitura += (String) leituraUmidade;
        leitura += " %"; 
  
        bot.sendMessage(bot.messages[i].chat_id, leitura, "");

    }


  }
}

void setup() {
  Serial.begin(115200);

  // Set WiFi to station mode and disconnect from an AP if it was Previously
  // connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // Attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  client.setInsecure();

  dht.begin();
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.println("Temperature");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" *C");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" *C");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" *C");  
  Serial.println("------------------------------------");
  dht.humidity().getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.println("Humidity");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println("%");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println("%");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println("%");  
  Serial.println("------------------------------------");
  delayMS = sensor.min_delay / 1000;
}

void loop() {
    // Delay between measurements.
  delay(delayMS);
  // Get temperature event and print its value.
  sensors_event_t event;  
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println("Error reading temperature!");
  }
  else {
    Serial.print("Temperature: ");
    Serial.print(event.temperature);
    leituraTemperatura = event.temperature;
    if (leituraTemperatura > 20.00 ) {
      Serial.print(contador);
      if (contador == 0){
        String welcome = "Temperatura Alta de ";
        welcome += (String) leituraTemperatura;
        welcome += " °C";
        bot.sendMessage("", welcome);
        contador += 1;  
      }
      contador += 1;  
      if (contador > 30) contador = 0;
    }
    Serial.println(" *C");
  }
  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println("Error reading humidity!");
  }
  else {
    Serial.print("Humidity: ");
    Serial.print(event.relative_humidity);
    leituraUmidade = event.relative_humidity;
    Serial.println("%");
  }
  
  if (millis() > Bot_lasttime + Bot_mtbs)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);


//    Saída Serial 
    Serial.println(numNewMessages);
    while(numNewMessages) {
      Serial.println("got response");

      handleNewMessages(numNewMessages);
      
      for (int i=0; i<numNewMessages; i++) {

      }
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    Bot_lasttime = millis();
  }
}
