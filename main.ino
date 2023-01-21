#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

String ssidList[10] = {"TPLINK", "dlink", "ALHNECE8"};
String passwordList[10] = {"arafibi2020", "Mpupin2019", "B3RZEfpTHs"};
//String ssidList[1];
//String passwordList[1];

String ssids[10] = {};
int numOfSSIDs = 0;
bool connected = false;
String strIndex = "";

int co2 = -1;
int o2 = -1;
int ch4 = -1;

const char *host = "smart-city-api.netlify.app";
const int httpsPort = 443;
const char fingerprint[] PROGMEM = "90 75 F4 E0 1B 98 9D 01 B0 58 B3 E3 3B DB DA E0 24 FA 9F 82";
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
    ssids[i] = WiFi.SSID(i);
    numOfSSIDs++;
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
    for(int i = 0; i < numOfSSIDs; i++){
      Serial.print("\t");
      Serial.print(i);
      Serial.println(". " + ssids[i]);
    }
    while(true){
      if(Serial.available()){
        strIndex = strIndex + Serial.readString();
        break;
      }
    }
    Serial.println("Password for " + ssids[strIndex.toInt()] + ": ");
    while(!connected){
      if(Serial.available()){
        String password = Serial.readString();
        Serial.print("Connecting to ");
        Serial.print(ssids[strIndex.toInt()]);
        Serial.print(" using ");
        Serial.println(password);
        connect(ssids[strIndex.toInt()].c_str(), password.c_str());
        Serial.print("\n");
        connected = true;
        break;
      }
    }
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

  Serial.print("CO2: ");
  while(co2 == -1){
    if(Serial.available()){
      co2 = Serial.readString().toInt();
      Serial.println(co2);
    }
  }
  Serial.print("O2: ");
  while(o2 == -1){
    if(Serial.available()){
      o2 = Serial.readString().toInt();
      Serial.println(o2);
    }
  }
  Serial.print("CH4: ");
  while(ch4 == -1){
    if(Serial.available()){
      ch4 = Serial.readString().toInt();
      Serial.println(ch4);
    }
  }
  //String Link = "/.netlify/functions/post/co2=22&o2=108&ch4=6";
  String Link = "/.netlify/functions/post/co2=" + String(co2) + "&o2=" + String(o2) + "&ch4=" + String(ch4);
  
  int r=0;
  while((!httpsClient.connect(host, httpsPort)) && (r < 30)){
      delay(100);
      Serial.print(".");
      r++;
  }
  
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
  o2, co2, ch4 = -1;
}

/*************************************************************************************/
/* CONNECT */
/*************************************************************************************/

void connect(const char *ssid, const char *password){
  WiFi.mode(WIFI_OFF);
  delay(1000);
  WiFi.mode(WIFI_STA);
  
  WiFi.begin(ssid, password);
  Serial.print("Connecting using: ");
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("Password: ");
  Serial.println(password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected.");
  Serial.print(" IP address: ");
  Serial.println(WiFi.localIP());
}
