#include <ESP8266WiFi.h>

const char* ssid = "test";
const char* password = "test";

WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  delay(10);

  // Connect to WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());
}

void loop() {
  // Check if a client has connected
  WiFiClient client = server.available();

  if (client) {
    // Wait until the client sends some data
    Serial.println("-----------------------");
    Serial.println("new client");
    while (!client.available()) {
      delay(1);
    }
  
    // Read the first line of the request
    String clientRequest = client.readStringUntil('\r');
    Serial.println(clientRequest);
    
    client.flush();
  
    long randNumber = random(1000);
    Serial.print("randNumber: ");
    Serial.println(randNumber);
  
    // Prepare the response
    String esp8266Response = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n\r\n" +
        String(randNumber);
  
    // Send the response to the client
    client.print(esp8266Response);
    delay(1);
    Serial.println("Client disonnected");
  
    // The client will actually be disconnected
    // when the function returns and 'client' object is detroyed 
  }
}
