#include <WiFi.h>
#include <WebServer.h>

#define DEBUG true

WebServer server(80);

void handleRoot()
{
  String html = "<!DOCTYPE html><html>";
  html += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  html += "<style>html { font-family: Arial; text-align: center; margin-top: 50px;}";
  html += ".btn { background-color: #008CBA; border: none; color: white; padding: 16px 32px;";
  html += "text-decoration: none; font-size: 16px; margin: 4px 2px; cursor: pointer; border-radius: 8px;}";
  html += ".btn:active { background-color: #005f7a; }</style>";
  html += "<title>ESP32-S3 UART Trigger</title></head>";
  
  html += "<body><h1>ESP32-S3 Web-to-UART Control</h1>";
  html += "<p>Press the button below to send a message over Serial.</p>";
  
  // HTML Button triggers a background fetch API request to "/msg"
  html += "<button class=\"btn\" onclick=\"sendUartMessage()\">Send UART Message</button>";
  
  // Simple AJAX script so the page doesn't reload
  html += "<script>function sendUartMessage() {";
  html += "  fetch('/msg')";
  html += "  .then(response => response.text())";
  html += "  .then(data => console.log(data));";
  html += "}</script>";
  
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

void handleUartTrigger()
{
  // Send your message over the default hardware UART (Serial via USB)
  Serial.println("UART_TRIGGER: Button pressed on Web Interface!");
  
  // Send an HTTP 200 response back to the browser to acknowledge success
  server.send(200, "text/plain", "Message sent over UART successfully.");
}

void handleNotFound()
{
  server.send(404, "text/plain", "404: Not Found");
}

int loopCount = 0;
void setup()
{
  Serial.begin(115200);
  
  // Set up Waveshare as access point
  Serial.print("Setting up local network");
  WiFi.softAP("ESP32-NW", "password");
  
  // Define routing paths
  server.on("/", handleRoot);
  server.on("/msg", handleUartTrigger); // Bind the new endpoint to the UART function
  server.onNotFound(handleNotFound);
  
  // Start the server
  server.begin();
  Serial.println("HTTP server started at " + WiFi.softAPIP().toString());

  // Connect to the ESP32-NW WiFi network on your phone with the password provided
  // In a browser, navigate to IP in serial output messages
  // Open a serial connection to the board (Serial Monitor in the upper right hand
  //  corner of this window, the icon looks like a plug)
}

void loop()
{
  server.handleClient();
  delay(2);
  
  if (DEBUG) {
    delay(1000); 
    Serial.print(WiFi.softAPIP());
    Serial.println(" Looping..." + String(loopCount++));
  }
}