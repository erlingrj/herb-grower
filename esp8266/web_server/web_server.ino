#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "secret.h"


/* Put IP Address details */
IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

ESP8266WebServer server(80);


/*NTP stuff */
const long utc_offset_sec = 3600; //1h we are in utc+1
char days_of_week[7][12] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
WiFiUDP ntp_udp;
NTPClient ntp_client(ntp_udp, "pool.ntp.org", utc_offset_sec);
unsigned long last_ntp_poll = 0;
unsigned long ntp_poll_interval_s = 60*60;

uint8_t hour = 0;
uint8_t minute = 0;
char** day = NULL;

uint8_t on_hour = 6;
uint8_t on_min = 0;

uint8_t off_hour = 23;
uint8_t off_min = 0;

bool LED_timer_on = false;

uint8_t LED_lamp_pin = 14;
bool LED_lamp_status = LOW;


unsigned int get_min(unsigned int ntp_min, unsigned int last_poll_ms, unsigned int now_ms) {
  unsigned int minutes = ntp_min + (now_ms - last_poll_ms)/(60*1000);

  return minutes % 60;
}

unsigned int get_hour(unsigned int ntp_hour, unsigned int last_poll_ms, unsigned int now_ms) {
  unsigned int hours = ntp_hour + (now_ms - last_poll_ms)/(60*60*1000);
  return hours % 24;
  
}


void setup() {
  pinMode(LED_lamp_pin, OUTPUT);

  Serial.begin(115200);



  Serial.println("Connecting to ");
  Serial.println(ssid);


  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println(".");
  }

  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.println("Got IP: "); Serial.println(WiFi.localIP());

  ntp_client.update();

  server.on("/", handle_OnConnect);

  server.on("/led_timer_on", handle_led_timer_on);
  server.on("/led_timer_off", handle_led_timer_off);
  server.on("/led_on", handle_led_on);
  server.on("/led_off", handle_led_off);
  server.on("/get", handle_timer_setpoints);

  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");
}
void loop() {
  server.handleClient();

  unsigned long now = millis();

  
    if ((now - last_ntp_poll) > ntp_poll_interval_s * 1000) {
      ntp_client.update();
      Serial.print(days_of_week[ntp_client.getDay()]);
      Serial.print(", ");
      Serial.print(ntp_client.getHours());
      Serial.print(":");
      Serial.print(ntp_client.getMinutes());
      Serial.print(":");
      Serial.println(ntp_client.getSeconds());
      last_ntp_poll = now;
    }

    hour = ntp_client.getHours();
    minute = ntp_client.getMinutes();
    day = (char **) &days_of_week[ntp_client.getDay()];

  
  

  if (LED_timer_on) {
    if ((hour > on_hour) || (hour == on_hour && minute >= on_min)) {
       if( (hour < off_hour) || (hour == off_hour && minute <  off_min)) {
        LED_lamp_status = HIGH;
       } else {
        LED_lamp_status = LOW;
       }
      
    } else {
      LED_lamp_status = LOW;
    }
  }
  digitalWrite(LED_lamp_pin, !LED_lamp_status);  
}


void handle_OnConnect() {
  server.send(200, "text/html", SendHTML());
}

void handle_led_on() {
  LED_lamp_status = HIGH;
  Serial.println("LAMP ON");
  server.send(200, "text/html", SendHTML());
}

void handle_led_off() {
  LED_lamp_status = LOW;
  Serial.println("LAMP OFF");
  server.send(200, "text/html", SendHTML());
}

void handle_led_timer_on() {
  LED_timer_on = true;
  Serial.println("LED Timer ON");
  server.send(200, "text/html", SendHTML());
}

void handle_led_timer_off() {
  LED_timer_on = false;
  Serial.println("LED Timer OFF");
  server.send(200, "text/html", SendHTML());
}

void handle_timer_setpoints() {
    on_hour = server.arg("on_hour").toInt();
    on_min = server.arg("on_om").toInt();
    off_hour = server.arg("off_hour").toInt();
    off_min = server.arg("off_min").toInt();

    Serial.println("New setpoints");
    server.send(200, "text/html", SendHTML());
}

void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}

String SendHTML() {
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<title>LED Control</title>\n";
  ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr += ".button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr += ".button-on {background-color: #1abc9c;}\n";
  ptr += ".button-on:active {background-color: #16a085;}\n";
  ptr += ".button-off {background-color: #34495e;}\n";
  ptr += ".button-off:active {background-color: #2c3e50;}\n";
  ptr += "p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr += "</style>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<h1>ESP8266 Herb Grower </h1>\n";
  ptr += "<h3>Tada!</h3>\n";
  ptr += "<h3>";
  ptr += days_of_week[ntp_client.getDay()];
  ptr += ",";
  ptr += ntp_client.getHours();
  ptr += ":";
  ptr += ntp_client.getMinutes();
  ptr += ":";
  ptr += ntp_client.getSeconds();
  ptr += "</h3>";
    
  if (LED_lamp_status)
  {
    ptr += "<p>Plant light Status: ON</p><a class=\"button button-on\" href=\"/led_off\">ON</a>\n";
  }
  else
  {
    ptr += "<p>Plant light Status: OFF</p><a class=\"button button-off\" href=\"/led_on\">OFF</a>\n";
  }

  if (LED_timer_on)
  {
    ptr += "<p>Timer: ON</p><a class=\"button button-on\" href=\"/led_timer_off\">ON</a>\n";
  }
  else
  {
    ptr += "<p>TIMER: OFF</p><a class=\"button button-off\" href=\"/led_timer_on\">OFF</a>\n";
  }

  ptr += "<h3>Timer setpoints</h3>\n";
  ptr += "<form action =\"/get\">";
  ptr += "Turn ON at hour: <input type=\"text\" name=\"on_hour\" value=\"";
  ptr += on_hour; 
  ptr += "\"><br><br>";

  ptr += "Turn ON at minute: <input type=\"text\" name=\"on_min\" value=\"";
  ptr += on_min;
  ptr += "\"><br><br>";
  
  ptr += "Turn OFF at hour: <input type=\"text\" name=\"off_hour\" value=\"";
  ptr += off_hour;
  ptr += "\"><br><br>";
  
  ptr += "Turn OFF at minute: <input type=\"text\" name=\"off_min\" value=\"";
  ptr += off_min;
  ptr += "\"><br><br>";

  ptr += "<input type=\"submit\" value=\"Set\">";
  ptr += "</form>";
  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}
