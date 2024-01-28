#include"HX711.h"
#include <LiquidCrystal_I2C.h>
#include<ESP32Servo.h>
#include"BluetoothSerial.h"

#define back 27
#define right 18
#define enter 14
#define left 19
#define maxMenu 6
#define period 6000
#define HX_Clock 23
#define HX_Dout 32


long weightUpper=-31629;
long weightLower=-28295;
int menuCount=1;
int mode=1;

Servo solidServo;
Servo liquidServo;
BluetoothSerial bt;
HX711 scale;
LiquidCrystal_I2C display(0x27,16,2); 

void setup() {
  Serial.begin(115200);

  bt.begin("multiState Dispenser");

  display.begin();
  display.backlight();
  
  scale.begin(HX_Dout,HX_Clock);
  
	ESP32PWM::allocateTimer(0);
	ESP32PWM::allocateTimer(1);
	ESP32PWM::allocateTimer(2);
	ESP32PWM::allocateTimer(3);
	solidServo.setPeriodHertz(50);   
	liquidServo.setPeriodHertz(50);    
	solidServo.attach(17, 1000, 2000);   
	liquidServo.attach(5, 1000, 2000);

  pinMode(back,INPUT_PULLUP);
  pinMode(right,INPUT_PULLUP);
  pinMode(enter,INPUT_PULLUP);
  pinMode(left,INPUT_PULLUP);

  solidServo.write(90);
  liquidServo.write(90);

  menu(1);
}



int weightCurrent;
void loop() {
  weightCurrent=processedWeight();
  //when right button pressed
  if(!digitalRead(right)){
    menuCount++;
    if(menuCount>maxMenu){
      menuCount=maxMenu;
    }
    else{
      menu(menuCount);
    }
    delay(500);
  }

  //when left button pressed
  else if(!digitalRead(left)){
    menuCount--;
    if(menuCount<1){
      menuCount=1;
    }
    else{
      menu(menuCount);
    }
    delay(500);
  }

  else if(!digitalRead(enter)){
    execute(menuCount);
    delay(500);
  }
  if(bt.available()){
    char a=bt.read();
    Serial.println(a);
    if(a=='s'){
      a=bt.read();
      if(a=='z'){
        zerobt();
      }
      if(a=='d'){
        dispenseSolidsbt();
      }
      if(a=='a'){
        solidServo.write(180);
      }
      if(a=='k'){
        solidServo.write(90);
      }
    }
  }
  //write bluetooth
}

void menu(int k){
  switch(k){
    case 1:
      displayHomeMenu();
      break;
    case 2:
      dispenseSolidsMenu();
      break;
    case 3:
      dispenseLiquidsMenu();
      break;
    case 4:
      showWeightMenu();
      break;
    case 5:
      zeroMenu();
      break;
    case 6:
      calibrateMenu();
      break;
  }
}

void execute(int f){
  switch(f){
    case 2:
      dispenseSolids();
      break;
    case 3:
      dispenseLiquids();
      break;
    case 4:
      showWeight();
      break;
    case 5:
      zero();
      break;
    case 6:
      calibrate();
      break;
  }
}

void displayHomeMenu(){
  display.clear();
  display.setCursor(0,0);
  display.print(" Dispenser Home");
}

void dispenseSolidsMenu(){
  display.clear();
  display.setCursor(0,0);
  display.print("Solid dispenser:");
  int exit=1;
}
void dispenseLiquidsMenu(){
  display.clear();
  display.setCursor(0,0);
  display.print("Liquid dispenser");
}
void showWeightMenu(){
  display.clear();
  display.setCursor(0,0);
  display.print("Weight: ");
}
void zeroMenu(){
  display.clear();
  display.setCursor(0,0);
  display.print("Set Zero");
}
void calibrateMenu(){
  display.clear();
  display.setCursor(0,0);
  display.print("Calibrate weight");
}



void dispenseSolids(){
  display.setCursor(0,1);
  display.print("quantity:0.00g");
  bool exit=false; 
  //put work here
  float quantity=0;
  while(!exit){
    if(!digitalRead(back)){
      exit=true;
    }
    else if(!digitalRead(enter)){
      if(quantity>0){
        float currReading=processedWeight();
        while(currReading<quantity){
          currReading=processedWeight();
          int speed=map(quantity-currReading,0,quantity*0.8,90,180);
          if(speed>180){
            solidServo.write(180);
          }
          else if(speed<120){
            solidServo.write(120);
          }
          else{
            solidServo.write(speed);
          }
          delay(90);
      }
      display.setCursor(0,1);
      solidServo.write(90);
      display.print("done!          ");
      delay(1000);
      exit=true;
      }
      
    }
    else if(!digitalRead(right)){
      quantity+=0.1;
      display.setCursor(9,1);
      display.print("      ");
      display.setCursor(9,1);
      display.print(quantity);
      display.print("g");
      delay(200);
    }
    else if(!digitalRead(left)){
      quantity-=0.1;
      display.setCursor(9,1);
      display.print("      ");
      display.setCursor(9,1);
      display.print(quantity);
      display.print("g");
      delay(200);
    }
  }
  dispenseSolidsMenu();
  delay(500);
}
void dispenseSolidsbt(){
  char inp;
  float qty=0;
  inp=bt.read();
  qty+=inp-'0';
  int count=1;
  float temp;
  while(inp!='d'){
    inp=bt.read();
     temp=inp-'0';
    for(int i=0;i<count;i++){
      temp=temp/10;
    }
    qty+=temp;
    count++;
  }
  qty-=temp;
  float currReading=processedWeight();
  while(currReading<qty){
    currReading=processedWeight();
    int speed=map(qty-currReading,0,qty*0.8,90,180);
    if(speed>180){
      solidServo.write(180);
    }
    else if(speed<120){
      solidServo.write(120);
    }
    else{
      solidServo.write(speed);
    }
    delay(90);
  }
  solidServo.write(90);
  delay(1000);
}
void dispenseLiquids(){
  display.setCursor(0,1);
  display.print("quantity: ");
  bool exit=false; 
  //put work here
  float quantity=0; 
  while(!exit){
    if(!digitalRead(back)){
      exit=true;
    }
    else if(!digitalRead(enter)){
      float time=(float)period*quantity/10;
      liquidServo.write(180);
      delay(time);
      liquidServo.write(90);
      quantity=0;
      display.setCursor(9,1);
      display.print("      ");
      display.setCursor(9,1);
      display.print(quantity);
      display.print("mL");
      delay(200);
    }
    else if(!digitalRead(right)){
      quantity+=1;
      display.setCursor(9,1);
      display.print("      ");
      display.setCursor(9,1);
      display.print(quantity);
      display.print("mL");
      delay(200);
    }
    else if(!digitalRead(left)){
      quantity-=1;
      display.setCursor(9,1);
      display.print("      ");
      display.setCursor(9,1);
      display.print(quantity);
      display.print("mL");
      delay(200);
    }
  }
  dispenseLiquidsMenu();
  delay(500);
}

void showWeight(){
  display.setCursor(0,1);
  display.print("Reading: ");
  bool exit=false; 
  //put work here
  int weight=0;
  while(!exit){
    if(!digitalRead(back)){
      exit=true;
    }
    display.setCursor(14,1);
    display.print("   ");
    display.setCursor(9,1);
    display.print(processedWeight());
    display.print("g");
    delay(500);
  }
  showWeightMenu();
  delay(500);
}

void zero(){
  display.setCursor(0,1);
  display.print("make zero?");
  int weight=0;
  bool exit=0;
  weightLower=rawAverage(10);
  zeroMenu();
  delay(500);
}

void zerobt(){
  weightLower=rawAverage(10);
}
void calibrate(){
  display.setCursor(0,0);
  display.clear();
  display.print("Keep tray empty");
  delay(2000);
  weightLower=rawAverage(20);
  display.setCursor(0,0);
  display.clear();
  display.print("Place a 2 rupee coin");
  delay(2000);
  weightUpper=rawAverage(20);
  calibrateMenu();
  delay(500);
}

long rawWeight(){
  if(scale.is_ready()){
    return scale.read();
  }
  return 0;
}
long rawAverage(int k){
  long avg=0;
  for(int i=0;i<k;i++){
    if(scale.is_ready()){
      avg+=scale.read();
    }
    else{
      i--;
    }
    delay(50);
  }
  return avg/k;
}
float processedWeight(){
  if(scale.is_ready()){
  float val=(float)map(rawAverage(10),weightLower,weightUpper,0,6000)/100;
  return round(val)/10;
  }
  return 0;
}