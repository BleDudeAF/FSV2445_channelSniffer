#include <Arduino.h>
#include <avr/wdt.h>

void change_band(int band);
void search_channel();
void BS_IRQ();
void CS_IRQ();
void locked_blinky();
void change_channel(int i);
void write_digit(int num);

int band_counter = 0;
int channel_counter = 0;
int bounce_time = 0;
int bounce_time2 = 0;
int bool_locked = 0;
int bool_change_band = 0;
int bool_manuel_change_channel = 0;

const int level = 198; //entspricht pwm summe von 3,3V

enum band{
  IRC,
  RACE,
  BANDA,
  BANDE
};

//band select button
const int BUT_BS = 2;
//channel select button
const int BUT_CS = 3;

//channel switches
const int CS1 = 6;
const int CS2 = 8;
const int CS3 = 7;

//band switches
const int BS1 = 9;
const int BS2 = 10;

//rssi pin
const int PIN_RSSI = A7;

//channel lock band lock
const int channel_lock = 4;

//7 segment display
const int SS_A = A5;
const int SS_B = A6;
const int SS_C = A2;
const int SS_D = A1;
const int SS_E = A0;
const int SS_F = A4;
const int SS_G = A3;


void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);

  pinMode(CS1, OUTPUT);
  pinMode(CS2, OUTPUT);
  pinMode(CS3, OUTPUT);

  pinMode(BS1, OUTPUT);
  pinMode(BS2, OUTPUT);

  pinMode(SS_A, OUTPUT);
  pinMode(SS_B, OUTPUT);
  pinMode(SS_C, OUTPUT);
  pinMode(SS_D, OUTPUT);
  pinMode(SS_E, OUTPUT);
  pinMode(SS_F, OUTPUT);
  pinMode(SS_G, OUTPUT);

  digitalWrite(CS1, LOW);
  digitalWrite(CS2, HIGH);
  digitalWrite(CS3, LOW);
  
  digitalWrite(BS1, LOW);
  digitalWrite(BS2, LOW);

  digitalWrite(SS_A, LOW);
  digitalWrite(SS_B, LOW);
  digitalWrite(SS_C, LOW);
  digitalWrite(SS_D, LOW);
  digitalWrite(SS_E, LOW);
  digitalWrite(SS_F, LOW);
  digitalWrite(SS_G, LOW);

  pinMode(BUT_BS, INPUT_PULLUP);
  pinMode(BUT_CS, INPUT_PULLUP);
  pinMode(channel_lock, INPUT_PULLUP);
  pinMode(PIN_RSSI, INPUT);

  attachInterrupt(digitalPinToInterrupt(BUT_BS), &BS_IRQ, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUT_CS), &CS_IRQ, FALLING);
}

void loop() {
  wdt_enable(WDTO_250MS);
  if(bool_locked){
    locked_blinky();
  }
  if(bool_change_band){
    change_band(band_counter);
    bool_change_band = 0;
  }
  if(bool_manuel_change_channel){
    change_channel(channel_counter);
    write_digit(channel_counter+1);
    bool_manuel_change_channel = 0;
  }
  detachInterrupt(BUT_BS);
  detachInterrupt(BUT_CS);
  delay(2);
  attachInterrupt(digitalPinToInterrupt(BUT_BS), &BS_IRQ, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUT_CS), &CS_IRQ, FALLING);
  wdt_reset();
  Serial.println("on");
}
//manual channel switch interrupt service routine
void CS_IRQ(){
  detachInterrupt(digitalPinToInterrupt(BUT_CS));
  if(millis()> (bounce_time2+200)){
    bounce_time2 = millis();

    if(!digitalRead(channel_lock)){
      Serial.println(F("locked!"));
      bool_locked = 1;
    }else{
      //  if(!digitalRead(LED_BUILTIN)){
      // digitalWrite(LED_BUILTIN,HIGH);
      // }else{
      //   digitalWrite(LED_BUILTIN,LOW);
      // }  

      channel_counter++;
      if(channel_counter >7){
        channel_counter = 0;
      }
      bool_manuel_change_channel = 1;
    }
  }
  attachInterrupt(digitalPinToInterrupt(BUT_CS), &CS_IRQ, FALLING);
}

//manual band switch interrupt service routine
void BS_IRQ(){
  detachInterrupt(digitalPinToInterrupt(BUT_BS));
  if(millis()> (bounce_time+200)){
    bounce_time = millis();

    if(!digitalRead(channel_lock)){
      Serial.println(F("locked!"));
      bool_locked = 1;
      
    }else{
      
      // if(!digitalRead(LED_BUILTIN)){
      // digitalWrite(LED_BUILTIN,HIGH);
      // }else{
      //   digitalWrite(LED_BUILTIN,LOW);
      // }  

      band_counter++;
      if(band_counter >3){
        band_counter = 0;
      }
      bool_change_band = 1;
    }
  }
  attachInterrupt(digitalPinToInterrupt(BUT_BS), &BS_IRQ, FALLING);
}

void change_band(int bandselect){

  if(!digitalRead(LED_BUILTIN)){
    digitalWrite(LED_BUILTIN,HIGH);
    }else{
      digitalWrite(LED_BUILTIN,LOW);
    }  
   
  switch(bandselect){
    case IRC:
      digitalWrite(BS1, HIGH);
      digitalWrite(BS2, HIGH);
      Serial.println(F("IRC/FS"));
      break;

    case RACE:
      digitalWrite(BS1, LOW);
      digitalWrite(BS2, LOW);
      Serial.println(F("Raceband"));
      break;

    case BANDA:
      digitalWrite(BS1, LOW);
      digitalWrite(BS2, HIGH);
      Serial.println(F("BandA"));
      break;

    case BANDE:
      digitalWrite(BS1, HIGH);
      digitalWrite(BS2, LOW);
      Serial.println(F("BandE"));
      break;
  }
  search_channel();
}

//LSB counting
void change_channel(int i){
    switch(i){
      case 0:
        digitalWrite(CS1, LOW);
        digitalWrite(CS2, LOW);
        digitalWrite(CS3, LOW);
        break;
      
      case 1:
        digitalWrite(CS1, LOW);
        digitalWrite(CS2, LOW);
        digitalWrite(CS3, HIGH);
        break;

      case 2:
        digitalWrite(CS1, LOW);
        digitalWrite(CS2, HIGH);
        digitalWrite(CS3, LOW);
        break;

      case 3:
        digitalWrite(CS1, LOW);
        digitalWrite(CS2, HIGH);
        digitalWrite(CS3, HIGH);
        break;

      case 4:
        digitalWrite(CS1, HIGH);
        digitalWrite(CS2, LOW);
        digitalWrite(CS3, LOW);
        break;

      case 5:
        digitalWrite(CS1, HIGH);
        digitalWrite(CS2, LOW);
        digitalWrite(CS3, HIGH);
        break;

      case 6:
        digitalWrite(CS1, HIGH);
        digitalWrite(CS2, HIGH);
        digitalWrite(CS3, LOW);
        break;

      case 7:
        digitalWrite(CS1, HIGH);
        digitalWrite(CS2, HIGH);
        digitalWrite(CS3, HIGH);
        break;
    }
}

//RSSI strenght: 1,875 V good signal; 1,148 V channel mismatch; 0,7 V no signal
void search_channel(){
  int rssi = 0;
  int rssi_pair[] = {0,0};

  for(int i = 0; i<8; i++){
    change_channel(i);
    delay(100);
    rssi = analogRead(PIN_RSSI);
    Serial.print(F("Rssi:"));
    Serial.println(rssi);
    if(rssi > rssi_pair[0]){
      rssi_pair[0] = rssi;
      rssi_pair[1] = i;
    }
    if(!digitalRead(LED_BUILTIN)){
      digitalWrite(LED_BUILTIN,HIGH);
    }else{
      digitalWrite(LED_BUILTIN,LOW);
    }
    wdt_reset();
  }

  change_channel(rssi_pair[1]);
  channel_counter = rssi_pair[1];
  write_digit(rssi_pair[1]+1);

  Serial.print(F("channel number:"));
  Serial.println(rssi_pair[1]+1);
}

void locked_blinky(){
  for(int i = 0; i<=5; i++){
    if(!digitalRead(LED_BUILTIN)){
      digitalWrite(LED_BUILTIN,HIGH);
    }else{
      digitalWrite(LED_BUILTIN,LOW);
    }
    delay(30);
  }
  bool_locked = 0;
}

void write_digit(int num){
  //del old numbers
  digitalWrite(SS_A, HIGH);
  digitalWrite(SS_B, HIGH);
  digitalWrite(SS_C, HIGH);
  digitalWrite(SS_D, HIGH);
  digitalWrite(SS_E, HIGH);
  digitalWrite(SS_F, HIGH);
  digitalWrite(SS_G, HIGH);

  switch(num){
    case 1:
      digitalWrite(SS_B, LOW);
      digitalWrite(SS_C, LOW);
      break;

    case 2:
      digitalWrite(SS_A, LOW);
      digitalWrite(SS_B, LOW);
      digitalWrite(SS_G, LOW);
      digitalWrite(SS_E, LOW);
      digitalWrite(SS_D, LOW);
      break;

    case 3:
      digitalWrite(SS_A, LOW);
      digitalWrite(SS_B, LOW);
      digitalWrite(SS_G, LOW);
      digitalWrite(SS_C, LOW);
      digitalWrite(SS_D, LOW);
      break;

    case 4:
      digitalWrite(SS_F, LOW);
      digitalWrite(SS_B, LOW);
      digitalWrite(SS_G, LOW);
      digitalWrite(SS_C, LOW);
      break;

    case 5:
      digitalWrite(SS_A, LOW);
      digitalWrite(SS_F, LOW);
      digitalWrite(SS_G, LOW);
      digitalWrite(SS_C, LOW);
      digitalWrite(SS_D, LOW);
      break;

    case 6:
      digitalWrite(SS_A, LOW);
      digitalWrite(SS_F, LOW);
      digitalWrite(SS_G, LOW);
      digitalWrite(SS_E, LOW);
      digitalWrite(SS_D, LOW);
      digitalWrite(SS_C, LOW);
      break;

    case 7:
      digitalWrite(SS_A, LOW);
      digitalWrite(SS_B, LOW);
      digitalWrite(SS_C, LOW);
      break;
    
    case 8:
      digitalWrite(SS_A, LOW);
      digitalWrite(SS_B, LOW);
      digitalWrite(SS_C, LOW);
      digitalWrite(SS_D, LOW);
      digitalWrite(SS_E, LOW);
      digitalWrite(SS_F, LOW);
      digitalWrite(SS_G, LOW);
      break;
  }
}