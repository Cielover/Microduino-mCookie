/*
Name:    all_to_onenet.ino
Created:  2017/7/13 12:26:01
Author: ni baba
*/

// the setup function runs once when you press reset or power the board
#include <ESP8266.h>
#include <Microduino_PM25.h>
#include <Wire.h>
#include <AM2321.h>
#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1284P__) || defined (__AVR_ATmega644P__) || defined(__AVR_ATmega128RFA1__)
#define EspSerial Serial1
#define UARTSPEED  115200
#endif

#define SSID        "bupt"
#define PASSWORD    "12345678"
#define HOST_NAME   "api.heclouds.com"
#define DEVICEID    "9626789"
//#define DEVICEID    "12292901"//new
#define PROJECTID   "91258"
#define HOST_PORT   (80)
String apiKey = "W=89g0k3nHpuxCChCQwP4YuMfqg=";
static const uint8_t stateLEDPin = 4;
char buf[10];
int start = 0;
int end = 0;
SoftwareSerial mySerial(4,5); // RX, TX//......................
PM25 pmSensor(&mySerial);    //使用软串口（2，3）//......................
String mCottenData;
String jsonToSend;
String postString;
String redString;

#define INTERVAL_sensor 2000
unsigned long sensorlastTime = millis();

int m;
float tempOLED, humiOLED, gasOLED, redOLED ,pmOLED;
float sensor_tem, sensor_hum;
//ESP8266 wifi(Serial1);
ESP8266 wifi(&EspSerial);
void setup(void)
{
  pinMode(6, OUTPUT);
  Serial.begin(115200);
  while (!Serial);
  pinMode(0, INPUT);
  pinMode(10, INPUT);
  Serial.print("setup begin\r\n");
  delay(100);
 do{
  WifiInit(EspSerial, UARTSPEED);
  Serial.print("FW Version:");
  Serial.println(wifi.getVersion().c_str());
  if (wifi.setOprToStation()) {
    Serial.print("to station ok\r\n");
    m=1;
  }
  else {
    Serial.print("to station err\r\n");
    m=0;
  }

  if (wifi.joinAP(SSID, PASSWORD)) {
    Serial.print("Join AP success\r\n");
    Serial.print("IP:");
    Serial.println(wifi.getLocalIP().c_str());
//    m=m；
  }
  else {
    Serial.print("Join AP failure\r\n");
    Serial.print("Make sure your SSID, PASS correctly!\r\n");
    m=0;
  }
 }while(!m);

  if (wifi.disableMUX()) {
    Serial.print("single ok\r\n");
  }
  else {
    Serial.print("single err\r\n");
  }
  Serial.print("setup end\r\n");
}

void loop(void)
{ 
  readByAM2321();
  tempOLED=sensor_tem;
  humiOLED = sensor_hum;
  //gasOLED=map(analogRead(A0),0,1023,0,255);
  pmOLED=pmSensor.getPM25();
  redOLED=digitalRead(0);
  gasOLED=digitalRead(10);
  Serial.println(String("infrared:")+ digitalRead(0));
  Serial.println(String("gas:")+ digitalRead(10));
  Serial.println(String("temperature:") + String(String("")+sensor_tem));
  Serial.println(String("humidity:") + String(String("")+sensor_hum));
  Serial.print("PM2.5: ");  
  if(pmSensor.available())
    Serial.println(pmSensor.getPM25());
  else
    Serial.println("none sensor!");
  delay(1000);
  updateData();//add
  //tempOLED = map(analogRead(A1), 0, 1023, 0, 255); //读取传感器中的数据
  //humiOLED = map(analogRead(A2), 0, 1023, 0, 255); 

  if(tempOLED>35.5||humiOLED<5||pmOLED>100||gasOLED==0){
    digitalWrite(6,HIGH);
    Serial.println("cuc");
  }
  else{
    digitalWrite(6,LOW);
    Serial.println("897");
  }
  
}



void updateData() {  //上传数据函数

 delay(5000);
  uint8_t buffer[1024] = {0};
  char infoData[60] = { 0 };

  if (wifi.createTCP(HOST_NAME, HOST_PORT)) {   //创建tcp连接
    Serial.print("create tcp ok\r\n");
  }
  else {
    Serial.print("create tcp err\r\n");
  }


  jsonToSend = "{\"Infrared\":";       //根据数据类型修改格式
  dtostrf(redOLED, 1, 0, buf);
  jsonToSend += "\"" + String(buf) + "\"";
  jsonToSend += ",\"Temperature\":";       //根据数据类型修改格式
  dtostrf(tempOLED, 1, 0, buf);
  jsonToSend += "\"" + String(buf) + "\"";
  jsonToSend += ",\"Humidity\":";
  dtostrf(humiOLED, 1, 0, buf);
  jsonToSend += "\"" + String(buf) + "\"";
  jsonToSend += ",\"Gas\":";
  dtostrf(gasOLED, 1, 0, buf);
  jsonToSend += "\"" + String(buf) + "\"";
  jsonToSend += ",\"PM2.5\":";
  dtostrf(pmOLED, 1, 0, buf);
  jsonToSend += "\"" + String(buf) + "\"";
  jsonToSend += "}";



  postString = "POST /devices/";      //根据数据类型修改格式
  postString += DEVICEID;
  postString += "/datapoints?type=3 HTTP/1.1";
  postString += "\r\n";
  postString += "api-key:";
  postString += apiKey;
  postString += "\r\n";
  postString += "Host:api.heclouds.com\r\n";
  postString += "Connection:close\r\n";
  postString += "Content-Length:";
  postString += jsonToSend.length();
  postString += "\r\n";
  postString += "\r\n";
  postString += jsonToSend;
  postString += "\r\n";
  postString += "\r\n";
  postString += "\r\n";

  const char *postArray = postString.c_str();    //用指针指向数据
  Serial.println(postArray);                      //串口打印数
  wifi.send((const uint8_t*)postArray, strlen(postArray)); //发送数据
  if (wifi.releaseTCP()) {
    Serial.print("release tcp ok\r\n");
  }
  else {
    Serial.print("release tcp err\r\n");
  }

  postArray = NULL;

}
void readByAM2321() {
  AM2321 am2321;
  am2321.read();
   sensor_tem=am2321.temperature/10.0;
  delay(1000);
   sensor_hum=am2321.humidity/10.0;
 
}

