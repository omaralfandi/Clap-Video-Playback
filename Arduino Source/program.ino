#include <LiquidCrystal.h>
#include <WiFiUdp.h>
#include <SPI.h>
#include <WiFi101.h>

LiquidCrystal lcd(5, 4, 3, 2, 1, 0);  // sets the interfacing pins


int ip0 = 255;
int ip1 = 255;
int ip2 = 255;
int ip3 = 255;
uint ipIndex;
uint ipIndexStartMillis;
uint startPressedMillis;
const uint ipIndexInterval = 4000;
bool pressed;

float ambientVal = 180;
float spikeVal = 180;
uint soundStartMillis = 0;

const float soundThresh = 55;
const uint soundBackoff = 400;

uint clapStartMillis;
int clapCount;
const uint sendDelay = 1200;

const uint port = 43210;

char ssid[] = "EIKESURF";
char pass[] = "123123123";

int wifiStatus = WL_IDLE_STATUS;

WiFiUDP Udp;


void setup()
{
  digitalWrite(LED_BUILTIN, HIGH);
  lcd.begin(16, 2);
  lcd.print("1  .1  .1  .1  ");
  lcd.cursor();

  pinMode(A0, INPUT); // button
  pinMode(A1, INPUT); // light barrier
  // A2 sound

  connectToWifi();
  Udp.begin(port);
}

void connectToWifi()
{
  while (wifiStatus != WL_CONNECTED)
  {
    lcd.setCursor(0, 1);
    lcd.print("Connecting...");
    lcd.print(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    wifiStatus = WiFi.begin(ssid, pass);

    for (int i = 0; i < 20 && wifiStatus != WL_CONNECTED; i++)
    {
      delay(50);
    }
  }
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print(ssid);
  setCur();
}

void loop()
{
  buttonIpConfig();
  bool clapDetected = detectSound();
  if (clapDetected)
  {
    clapStartMillis = millis();
    clapCount++;
  }
  else
  {
    if (clapCount > 0 && millis() > clapStartMillis + sendDelay)
    {
      sendClapCount();
      clapCount = 0;
    }
  }
}

void sendClapCount()
{
  IPAddress ip(ip0, ip1, ip2, ip3);
  long milli = millis();
  int b3 = milli & 0xFF;
  int b2 = (milli & 0xFF00) >> 8;
  int b1 = (milli & 0xFF0000) >> 16;
  int b0 = (milli & 0xFF000000) >> 24;
  Udp.beginPacket(ip, port);
  Udp.write(b0);
  Udp.write(b1);
  Udp.write(b2);
  Udp.write(b3);
  Udp.write(clapCount);
  Udp.endPacket();
}


bool detectSound()
{
  float cVal = analogRead(A2);
  float cAmbient = ambientVal;
  float cSpike = spikeVal;
  ambientVal = constrain(ambientVal * 0.99 + cVal * 0.01, 100, 900);
  spikeVal = constrain(spikeVal * 0.1 + cVal * 0.9, 100, 900);
  if (spikeVal > 850)
  {
    return false;
  }
  bool loudEnough = spikeVal - ambientVal > soundThresh && cVal < spikeVal;
  if (loudEnough && millis() > soundStartMillis + soundBackoff)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    soundStartMillis = millis();
    Serial.println(spikeVal);
    Serial.println(ambientVal);
    Serial.println(cVal);
    Serial.println();
    spikeVal = ambientVal;
    return true;
  }
  if (soundStartMillis + soundBackoff < millis())
  {
    digitalWrite(LED_BUILTIN, LOW);
  }
  return false;
}


void buttonIpConfig()
{
  bool pressed = !digitalRead(A0);
  setCur();
  if (!pressed)
  {
    if (millis() - ipIndexStartMillis > ipIndexInterval)
    {
      ipIndex++;
      ipIndex %= 4;
      ipIndexStartMillis = millis();
    }
    return;
  }
  if (millis() < startPressedMillis + 200)
  {
    return;
  }
  uint ipIncr = 1;
  if (digitalRead(A1))
  {
    ipIncr = 3;
  }
  startPressedMillis = millis();
  setCur();
  lcd.print("   ");
  setCur();
  if (ipIndex == 0)
  {
    ip0 += ipIncr;
    ip0 %= 256;
    ip0 = max(ip0, 1);
    lcd.print(ip0);
  }
  if (ipIndex == 1)
  {
    ip1 += ipIncr;
    ip1 %= 256;
    ip1 = max(ip1, 1);
    lcd.print(ip1);
  }
  if (ipIndex == 2)
  {
    ip2 += ipIncr;
    ip2 %= 256;
    ip2 = max(ip2, 1);
    lcd.print(ip2);
  }
  if (ipIndex == 3)
  {
    ip3 += ipIncr;
    ip3 %= 256;
    ip3 = max(ip3, 1);
    lcd.print(ip3);
  }
  ipIndexStartMillis = millis();
  setCur();
}

void setCur()
{
  lcd.setCursor((int)(ipIndex * 3 + (int)ipIndex), 0);
}
