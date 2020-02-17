#include "Fonts.h" // Include the header file attached to this sketch
#include <LoRa.h>
#include <TTGO.h>
//#include <Wire.h>

//#define TFT_DARKGREY_J    0x18E3
#define TFT_DARKGREY_J    0x2965

TTGOClass *ttgo;

SPIClass LoraSPI(HSPI);

String recv = "";
uint16_t tft_width  = 240;                // ST7789_TFTWIDTH;
uint16_t tft_height = 240;                // ST7789_TFTHEIGHT;
uint16_t k = 0;
uint16_t zero_Y;
uint16_t pix_per_grad;
float TempHyst[240];                      //тест float
float Temp_max;                           //тест float
float Temp_min;                           //тест float
float temp;

void low_energy()
{
  if (ttgo->bl->isOn()) {
    ttgo->closeBL();
    ttgo->stopLvglTick();
    ttgo->displaySleep();
    } 
  else {
    ttgo->startLvglTick();
    ttgo->displayWakeup();
    ttgo->openBL();
    }
}

void pressed(){
    low_energy();
}

void setup() {
  Serial.begin(115200);
  ttgo = TTGOClass::getWatch();
  ttgo->begin();
  ttgo->openBL();
  ttgo->lvgl_begin();
  ttgo->button->setPressedHandler(pressed);
  ttgo->eTFT->fillScreen(TFT_DARKGREY_J);
  ttgo->eTFT->setTextColor(TFT_YELLOW, TFT_DARKGREY_J);
  ttgo->eTFT->setTextSize(1);
  ttgo->eTFT->setRotation(0);     
  
  //! Lora power enable
  ttgo->enableLDO3();
  //ttgo->power->setLDO3Mode(1);
  //ttgo->power->setPowerOutPut(AXP202_LDO3, AXP202_ON);
    
  LoraSPI.begin(TWATCH_LORA_SCK, TWATCH_LORA_MISO, TWATCH_LORA_MOSI, TWATCH_LORA_SS);
  LoRa.setSPI(LoraSPI);
  LoRa.setPins(TWATCH_LORA_SS, TWATCH_LORA_RST, TWATCH_LORA_DI0);
  
  while (1){
  if (!LoRa.begin(868E6)) 
    Serial.println("Starting LoRa failed!");
  else {
    Serial.println("LORA Begin PASS");
    break;  
  }
  delay(1000);
  //LoRa.setSpreadingFactor(12);
  //LoRa.setSignalBandwidth(62.5E3);
  //LoRa.setCodingRate4(8);
  //LoRa.crc();
  }

}

void loop() {
  ttgo->button->loop();
  // try to parse packet
  if (LoRa.parsePacket()) {
    recv = "";
    // received a packet
    Serial.print("Received packet '");

    // read packet
    while (LoRa.available()) {
      recv = LoRa.readString();
      Serial.print(recv);
    }

    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
     
    if (recv.length() <= 6) {                                                                           //Проверяем длину принятого сообщения
      ttgo->eTFT->setFreeFont(FF23);
      ttgo->eTFT->drawString(String("T " + recv + " C"), 10, 10, GFXFF);
      add_val(recv);
      }
    else {
      ttgo->eTFT->setFreeFont(FF23);
      ttgo->eTFT->drawString("T los", 10, 10, GFXFF);
      add_val(String(0));  
      }
    
    Temp_max = get_limits("maximum");
    Serial.println(Temp_max);
    Temp_min = get_limits("minimum");
    Serial.println(Temp_min);
    ttgo->eTFT->setFreeFont(FF17);
    ttgo->eTFT->drawString(String("max " + String(Temp_max, 1) + " C"), 10, 40, GFXFF);
    ttgo->eTFT->drawString(String("min " + String(Temp_min, 1) + " C"), 10, 60, GFXFF);    
    graph_limits(Temp_max, Temp_min);
   }
}

void add_val(String recv){
  //int temp = recv.toInt();
  char TempA[6];                                                                                          //тест float
  recv.toCharArray(TempA, recv.length());                                                                 //тест float
  temp = atof(TempA);                                                                                     //тест float  
  
  if (k < tft_width) {
    TempHyst[k] = temp;
    k++;
    }
  else {
      for (int n = 0; n < tft_width-1; n++) {                                                             //сдвигаем массив на tft_width
        TempHyst[n] = TempHyst[n+1];
        }
      TempHyst[tft_width-1] = temp;                                                                       // добавляем принятое значение в конец массива
    }
}

float get_limits(String lim){                                                                             //рассчитываем минимум и максимум
  Temp_max = TempHyst[0];
  Temp_min = TempHyst[0];
  for (int x = 0; x < tft_width-1; x++){
    if (TempHyst[x] > Temp_max && TempHyst[x] != 0){
      Temp_max = TempHyst[x];
    }
    if (TempHyst[x] < Temp_min && TempHyst[x] != 0){
      Temp_min = TempHyst[x];
    }
  }
  if (lim == "maximum"){
    return Temp_max;  
  }
  if (lim == "minimum"){
    return Temp_min;  
  }
}

void graph_limits(int T_max, int T_min){
  
  if (T_max > 0 && T_min > 0) {
    pix_per_grad = (tft_height/2) / T_max;                                                               // расчитываем масштаб графика - количество пикселей на 1 градус
    int max_temp_graph = tft_height/2 / pix_per_grad;
    int min_temp_graph = 0;
    zero_Y = tft_height;
    Serial.println(pix_per_grad); 
    Serial.println(max_temp_graph);
    Serial.println(min_temp_graph); 
    Serial.println(zero_Y);
    }
  else if (T_max > 0 && T_min < 0) {
    pix_per_grad = (tft_height/2) / (abs(T_max - T_min));                                                // расчитываем масштаб графика - количество пикселей на 1 градус
    int max_temp_graph = T_max + (((tft_height/2) / pix_per_grad) - (abs(T_max - T_min))) / 2;           // расчитываем верхнюю границу графика в соотвествии с масштабом
    int min_temp_graph = - ((tft_height/2) / pix_per_grad - max_temp_graph);                             // расчитываем нижнюю границу графика в соотвествии с масштабом
    zero_Y = tft_height/2 + max_temp_graph * pix_per_grad;                                               // расчитываем Y-координату нулевой линии    
    Serial.println(pix_per_grad); 
    Serial.println(max_temp_graph);
    Serial.println(min_temp_graph); 
    Serial.println(zero_Y);
    }
  else if (T_max < 0 && T_min < 0) {
    pix_per_grad = (tft_height/2) / (abs(T_min));                                                        // расчитываем масштаб графика - количество пикселей на 1 градус
    int max_temp_graph = 0;
    int min_temp_graph = tft_height;
    zero_Y = tft_height/2;
    Serial.println(pix_per_grad); 
    Serial.println(max_temp_graph);
    Serial.println(min_temp_graph); 
    Serial.println(zero_Y);
    }

   for (int n = 0; n < tft_width; n++) {
   int temp_height = TempHyst[n] * pix_per_grad;
    if (temp_height > 0) {
      ttgo->eTFT->drawFastVLine(n, tft_height/2 + (zero_Y-tft_height/2 - temp_height), temp_height, TFT_YELLOW);
      ttgo->eTFT->drawFastVLine(n, tft_height/2, tft_height/2 - temp_height, TFT_DARKGREY_J);
      ttgo->eTFT->drawFastVLine(n, zero_Y, tft_height - zero_Y, TFT_DARKGREY_J); 
      }
    else if (temp_height < 0) {
      ttgo->eTFT->drawFastVLine(n, zero_Y, abs(temp_height), TFT_BLUE); 
      ttgo->eTFT->drawFastVLine(n, zero_Y + abs(temp_height), tft_height - (zero_Y + (abs(temp_height))), TFT_DARKGREY_J);
      ttgo->eTFT->drawFastVLine(n, tft_height/2, zero_Y - tft_height/2, TFT_DARKGREY_J); 
      }
    else {
      ttgo->eTFT->drawPixel(n, zero_Y, TFT_RED);
      ttgo->eTFT->drawFastVLine(n, zero_Y + 1, tft_height-zero_Y-1, TFT_DARKGREY_J); 
      ttgo->eTFT->drawFastVLine(n, tft_height/2, zero_Y - tft_height / 2 - 1, TFT_DARKGREY_J); 
      }
  }
} 
