#include <LiquidCrystal.h>
#include <SPI.h>
#include <DS1307RTC.h>
#include <DHT.h>
#include <Wire.h>



const int rs = 8, en = 9, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

unsigned long start, st0p, t1me; // for stopwatch

boolean reset = false; 
boolean swFlag = false; // clears stopwatch instruction

long lastButton = 0; //for debouncing

const int debounceDelay = 50; // delay time

int button = 0;
boolean previousButton = LOW;
boolean currentButton = LOW;
int ledMode = 1; // used to start at home screen

#define DHTPIN 27
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

const int led = 23;
const int buzzer = 25;
int ledState = LOW;

//used to adjust time and date
int newHour;
int newMin;
int newDay;
int newMonth;
int newYear;
int timeMenu;
int alarmMenu;
boolean alarmSet = false;

uint8_t alarmMode, alarmHour = 1, alarmMinute = 0;
long alarmLastButton;
volatile boolean alarmFlag = false;
boolean alarmAk = false;

long printTime; //for serial monitor debugging
int longDelayTime = 1000; // for serial monitor debugging
int timeHour = 0;
int timeMinute = 0;

int day_ = 0;


void setup() {
  // put your setup code here, to run once:
  dht.begin();
  delay(200);
  lcd.begin(16, 2);
  screenWelcome(); // name and course
  attachInterrupt(digitalPinToInterrupt(18), alarmTime, RISING);
  //Serial.begin(9600); //debugging
  pinMode(buzzer, OUTPUT);
  pinMode(led, OUTPUT);
  delay(3000);
  lcd.clear();
  
}

void loop(){
  int x = analogRead(0); // voltage divider (range = 800, 600, 400, 200, 60) (SLCT, LFT, DWN, UP, RGHT)
  if (x < 800 && x > 600){
    if ((millis() - lastButton) > debounceDelay){
      ledMode++; // advance modes by 1
      lcd.clear();
    }
  if (ledMode == 4) ledMode = 1;
  lastButton = millis();
  }
  setMode(ledMode);
  //tmElements_t tm;
  if(alarmHour == timeHour && alarmMinute == timeMinute) {
    if (alarmSet == true) {
      alarmActive();
      alarmAk = true;
      alarmSet = false;
    }
  }
  if (alarmAk == true) {
    int t = analogRead(0);
      if (t < 800) {
        //alarmSet = false;
        digitalWrite(led, LOW);
        noTone(buzzer); 
      }
    }
  
    
  if ((millis() - printTime) > longDelayTime) {
    
  //Serial.println(timeHour);
 // Serial.println(timeMinute);
 // Serial.println(alarmHour);
 // Serial.println(alarmMinute);
 //Serial.println(weekday());
  //printTime = millis();
  }
}  
  
void screenWelcome(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("JVargas: ELD-311");
  lcd.setCursor(0, 1);
  lcd.print("  Final Project ");
}


void setMode(int mode){

  //TIME
  if (mode == 1){ //day and date
    int x = analogRead(0);
    if (x < 600){ // left press activates
      char dayofTheWeek[][16] = {"    SUNDAY   ","   MONDAY   ","   TUESDAY   ","    WEDNESDAY   ","   THURSDAY   ","   FRIDAY   ","     SATURDAY     "};
      lcd.setCursor(0, 0);
      lcd.print(dayofTheWeek[day_]);
     
      lcd.setCursor(0, 1);
      lcd.print("   ");
      print2digits(newMonth);
      lcd.print("/");
      print2digits(newDay);
      lcd.print("/");
      lcd.print(newYear);
      lcd.print("   ");
    }
    
    else {homeScreen();}
  }
  
  else if (mode == 2){ //stopewatch mode
    lcd.setCursor(0, 0);
    lcd.print("STOPWATCH:");
    stopwatch();
  }
 
  else if (mode == 3){ //alarm mode
    swFlag = false;
    lcd.setCursor(0, 0);
    lcd.print("ALARM           ");
    lcd.setCursor(0, 1);
    print2digits(alarmHour);
    lcd.print(":");
    print2digits(alarmMinute);
    lcd.print("   ");
    if (alarmSet == false){
      lcd.print("OFF     ");
    }
    else {
      lcd.print("ON      ");
    }
    int x = analogRead(0); // button (range = 800, 600, 400, 200, 60) (S, L, U, D, R)
    if (x < 600) {
      //if ((millis() - lastButton) > debounceDelay){
      if (x < 600 && x > 400) {  //left button press
         if ((millis() - alarmLastButton) > debounceDelay){
         alarmHour++;
         if (alarmHour == 25) {
          alarmHour = 1; 
         }
         }
         alarmLastButton = millis();
      }
      if (x < 400 && x >200) { //down button press
        if ((millis() - alarmLastButton) > debounceDelay){
        alarmMinute--;
        if (alarmMinute == 0){
          alarmMinute = 59;
        }
        }
        alarmLastButton = millis();
      }
      if (x < 200 && x > 60) { //up button press
        if ((millis() - alarmLastButton) > debounceDelay){
        alarmMinute++;
        if (alarmMinute == 60) {
          alarmMinute = 0;
        }
        }
        alarmLastButton = millis();
      }
      else if (x < 60) { //right button press
        if ((millis() - alarmLastButton) > debounceDelay){
        alarmSet = !alarmSet;
        }
        alarmLastButton = millis();
      }
    }
  }
}


void homeScreen(){
  tmElements_t tm;
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    lcd.print(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  int f1 = f;

  if (RTC.read(tm)) {
    timeHour = tm.Hour;
    timeMinute = tm.Minute;

    day_ = weekday();
    newDay = tm.Day;
    newMonth = tm.Month;
    newYear = year()+ 50;    
    
    lcd.setCursor(0, 0);
    lcd.print("    ");
    print2digits(tm.Hour);
    lcd.print(':');
    print2digits(tm.Minute);
    lcd.print(':');
    print2digits(tm.Second);

    lcd.setCursor(0, 1);
    lcd.print(F("H:"));
    lcd.print(h);
    lcd.print(F("% "));
    //lcd.setCursor(0, 1);
    lcd.print(F(" T:"));
    lcd.print(f1);
    lcd.print((char)223);
    lcd.print(F("F"));
    }
     
  else {
    if (RTC.chipPresent()) {
      lcd.setCursor(16, 2);
      lcd.print("The DS1307 is stopped.  Please run the SetTime");
    } 
    else {
      lcd.setCursor(16, 2);
      lcd.print("DS1307 read error!  Please check the circuitry.");
    }
  }
}


void stopwatch(){
  if (swFlag == false){
    lcd.setCursor(0, 1);
    lcd.print("LEFT 2 STRT/STP");
    swFlag = true;
  }
  int x = analogRead (0);
  if (x < 600) {
      if ((millis() - lastButton) > debounceDelay) {
          if (reset == false) {
            start = millis();
          }
         reset = !reset;
       }
      lastButton = millis();
    }

  if (reset == true) {
      st0p = millis();
      float h, m, s, ms;
      unsigned long over;

      t1me = st0p - start;

      h = int(t1me / 3600000);
      over = t1me % 3600000;
      m = int(over / 60000);
      over = over % 60000;
      s = int(over / 1000);
      ms = over % 1000;
      // display the results
      lcd.setCursor(0, 1);
      lcd.print(h, 0); 
      lcd.print("h "); 
      lcd.print(m, 0);
      lcd.print("m ");
      lcd.print(s, 0);
      lcd.print("s ");
      if (h < 10) {
          lcd.print(ms, 0);
          lcd.print("ms ");
        }
    }
} 

void alarmTime(){
  alarmFlag = true;
}

void alarmActive() {
  // here is where you'd put code that needs to be running all the time.

  // check to see if it's time to blink the LED; that is, if the difference
  // between the current time and last time you blinked the LED is bigger than
  // the interval at which you want to blink the LED.
  unsigned long currentMillis = millis();
  //int ledState = LOW;
  unsigned long previousMillis = 0;
  const long interval = 1000;

  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) {
      ledState = HIGH;
      tone(buzzer, 1000);
    } else {
      ledState = LOW;
      noTone(buzzer);
    }

    // set the LED with the ledState of the variable:
    digitalWrite(led, ledState);
  }
}

void print2digits(int number) {
  if (number >= 0 && number < 10) {
    lcd.print('0');
  }
  lcd.print(number);
}





  
