#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

String ssidList[2] = {"TP-LINK", "dlink"};
String passwordList[2] = {"arafibi2020", "Mpupin2019"};

const char *host = "smart-city-api.netlify.app";
const int httpsPort = 443;
const char fingerprint[] PROGMEM = "C2 91 4B FE F1 7A 3B 5F E8 7C B8 81 98 C2 5A BA 7B 7D 92 45";
String command = "";

/*************************************************************************************/
/* SETUP */
/*************************************************************************************/

void setup() {
  Serial.begin(115200);
  delay(3000);
  Serial.println("");
  Serial.println("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");

  int WiFiIndex = -1;
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; i++){
    for(int x = 0; x < sizeof(ssidList); x++){
      if(WiFi.SSID(i) == ssidList[x]){
        WiFiIndex = x;
        }
      }
    }
  if(WiFiIndex != -1){
    String ssid = ssidList[WiFiIndex];
    String password = passwordList[WiFiIndex];
    Serial.println("Connecting to: " + ssid);
    connect(ssid.c_str(), password.c_str());
  }
  else{
    Serial.println("No Known Networks Found!");
  }
  
}

/*************************************************************************************/
/* LOOP */
/*************************************************************************************/

void loop() {
  if(Serial.available()){
    command = command + Serial.readString();
    if(command.indexOf("get") >= 0){ get(); }
    else if(command.indexOf("post") >= 0){
      post();
    }
    command = "";
  }
  delay(100);
}

/*************************************************************************************/
/* GET */
/*************************************************************************************/

void get(){
  Serial.println("\nGET REQUEST");
  WiFiClientSecure httpsClient;
  httpsClient.setFingerprint(fingerprint);
  httpsClient.setTimeout(15000);
  delay(1000);
  
  int r=0;
  while((!httpsClient.connect(host, httpsPort)) && (r < 30)){
      delay(100);
      Serial.print(".");
      r++;
  }

  String Link = "/.netlify/functions/get";
  Serial.print("\t -requesting URL: ");
  Serial.println(host+Link);

  httpsClient.print(String("GET ") + Link + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +               
               "Connection: close\r\n\r\n");
                  
  while (httpsClient.connected()) {
    String line = httpsClient.readStringUntil('\n');
    if (line == "\r") {break;}
  }

  String line;
  while(httpsClient.available()){line = httpsClient.readStringUntil('\n');}
  String json = line;
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, json);
  String co2 = String(doc["co2"]);
  String o2 = String(doc["o2"]);
  String ch4 = String(doc["ch4"]);
  Serial.println("\t -CO2: " + co2);
  Serial.println("\t -O2: " + o2);
  Serial.println("\t -CH4: " + ch4);
}

/*************************************************************************************/
/* POST */
/*************************************************************************************/

void post(){
  Serial.println("\nPOST REQUEST");
  WiFiClientSecure httpsClient;
  httpsClient.setFingerprint(fingerprint);
  httpsClient.setTimeout(15000);
  delay(1000);
  
  int r=0;
  while((!httpsClient.connect(host, httpsPort)) && (r < 30)){
      delay(100);
      Serial.print(".");
      r++;
  }

  String Link = "/.netlify/functions/post/co2=22&o2=108&ch4=6";
  Serial.print("\t -requesting URL: ");
  Serial.println(host+Link);

  httpsClient.print(String("GET ") + Link + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +               
               "Connection: close\r\n\r\n");
                  
  while (httpsClient.connected()) {
    String line = httpsClient.readStringUntil('\n');
    if (line == "\r") {break;}
  }

  String line;
  while(httpsClient.available()){line = httpsClient.readStringUntil('\n');}
  String json = line;
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, json);
  String statusData = String(doc["data"]);
  Serial.println("\t -Data sent: " + statusData);
}
void connect(const char *ssid, const char *password){
  WiFi.mode(WIFI_OFF);
  delay(1000);
  WiFi.mode(WIFI_STA);
  
  WiFi.begin(ssid, password);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected.");
  Serial.print(" IP address: ");
  Serial.println(WiFi.localIP());
}
