#define LAMP_PIN D2             // Định nghĩa chân đk đèn led
#define DISTANCE_SENSOR D0      //            chân đk cảm biến khoảng cách
#define LIGHT_SENSOR D1         //            chân đk cảm biến quang trở

#include <ESP8266WiFi.h> 
const char* ssid     = "KevinHoang";  //biến tên Wifi thay đổi khi kết nối đến wifi 
const char* password = "02071976";    //biến pass Wifi thay đổi khi kết nối đến wifi 

WiFiServer server(80);

String header;

float light_value = 0.00, distance_value = 0.00;
int brightness = 0;
unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;

String lamp_state;

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LAMP_PIN, OUTPUT);
  pinMode(DISTANCE_SENSOR, OUTPUT);
  pinMode(LIGHT_SENSOR, OUTPUT);
  
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    Serial.print(".");
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}
float Read_Light(){
  digitalWrite(DISTANCE_SENSOR, LOW);
  digitalWrite(LIGHT_SENSOR, HIGH);
  delay(50);
  light_value = analogRead(A0);
  return light_value;
}
float Read_Distance(){
  digitalWrite(DISTANCE_SENSOR, HIGH);
  digitalWrite(LIGHT_SENSOR, LOW);
  delay(50);
  float volts = analogRead(A0)*0.0048828125;
  distance_value = 29.988 * pow(volts , -1.173);
  return distance_value;
}
void Dim_Light(){
  analogWrite(LAMP_PIN,brightness);
}
void loop(){
  WiFiClient client = server.available();
  digitalWrite(LED_BUILTIN, HIGH);
  if (client) {
    Serial.println("New Client.");
    String currentLine = "";
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) {
      currentTime = millis();         
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        header += c;
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            if (header.indexOf("GET /Lamp/on") >= 0) {
              Serial.println("Lamp on");
              lamp_state = "on";
              digitalWrite(LAMP_PIN,HIGH);
            } else if (header.indexOf("GET /Lamp/off") >= 0) {
              Serial.println("Lamp off");
              lamp_state = "off";
              digitalWrite(LAMP_PIN,LOW);
            } 
            
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta charset=\"UTF-8\" name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println("</style></head>");
            
            client.println("<body><h1>SmartLamp Web Server</h1>");
            
            client.println("<p>Trạng thái đèn: " + lamp_state + "</p>");
    
            if (lamp_state=="off") {
              client.println("<p><a href=\"/Lamp/on\"><button class=\"button\">BẬT</button></a></p>");
            } else {
              client.println("<p><a href=\"/Lamp/off\"><button class=\"button\">TẮT</button></a></p>");
            }
            client.println("<p>Giá trị ánh sáng: " + String(Read_Light(),2) + " lux </p>");
            client.println("<p>Giá trị khoảng cách: " + String(Read_Distance(),2) + " cm </p>");
            client.println("</body></html>");
            
            client.println();
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    header = "";
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
