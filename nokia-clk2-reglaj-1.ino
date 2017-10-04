/*
https://www.hackster.io/microst/thermometer-diode-based-524613
  Thermometer based on 1n4148 silicon diode used as temperature sensor.The thermometer is based on the diode characteristic that the increase of the temperature its forward voltage (VF) is lowered by 2,2mV / ° C.
Fixing the value of Vf = VF0 at ambient temperature t0, the temperature value
t is calculated with the following formula:

t= t0 - [vf(t)- vf0]* K

with K = 1 / 2,2mV

The value of Vf (t) = dtemp -vf0 is obtained by averaging values of 1024 by acquiring as many vf values

The result of t is shown on the serial monitor
*/
// original from http://www.instructables.com/id/Arduino-Watch-Sport/

#include "DHT.h"
#define DHTPIN A0
//#define DHTTYPE DHT22 
#define DHTTYPE DHT11 
DHT dht(DHTPIN, DHTTYPE);


#include <LCD5110_Graph.h> // web: http://www.RinkyDinkElectronics.com/
// It is assumed that the LCD module is connected to
// the following pins using a levelshifter to get the
// correct voltage to the module.
//      SCK  - Pin 8
//      MOSI - Pin 9
//      DC   - Pin 10
//      RST  - Pin 11
//      CS   - Pin 12
//

#include <Encoder.h> // from http://www.pjrc.com/teensy/td_libs_Encoder.html
LCD5110 myGLCD(8,9,10,11,12);
extern unsigned char TinyFont[];
extern unsigned char SmallFont[];
extern unsigned char MediumNumbers[];
extern unsigned char BigNumbers[];
Encoder knob(2, 3); //encoder connected to pins 2 and 3 (and ground)

#include <Wire.h> 
#include "RTClib.h"

RTC_DS1307 RTC;
int f;


int setyeartemp; 
int setmonthtemp;
int setdaytemp;
int sethourstemp;
int setminstemp;
int setsecs = 0;
int maxday; // maximum number of days in the given month
int TimeMins; // number of seconds since midnight
int TimerMode = 2; //mode 0=Off 1=On 2=Auto
int TimeOut = 10;
int TimeOutCounter;


// These variables are for the push button routine
int buttonstate = 0; //flag to see if the button has been pressed, used internal on the subroutine only
int pushlengthset = 500; // value for a long push in mS
int pushlength = pushlengthset; // set default pushlength
int pushstart = 0;// sets default push value for the button going low
int pushstop = 0;// sets the default value for when the button goes back high

int knobval; // value for the rotation of the knob
boolean buttonflag = false; // default value for the button flag



void setup() {
 
   pinMode(4,INPUT);//push button on encoder connected to A0 (and GND)
    digitalWrite(4,HIGH); //Pull A0 high
  Serial.begin(9600);
  dht.begin();
  Wire.begin(); 
  RTC.begin(); 
 // RTC.adjust(DateTime(__DATE__, __TIME__));
  myGLCD.InitLCD();
  myGLCD.setFont(SmallFont);myGLCD.invert(true);
  myGLCD.print("ARDUINO", CENTER, 2);
  myGLCD.print("CLOCK", CENTER, 12);
  myGLCD.setFont(TinyFont);
  myGLCD.print("(@)2017 by", CENTER, 28);
  myGLCD.print("Vlad Gheorghe", CENTER, 35);
  myGLCD.update();
  
  
 for (int p=2; p>0; p--)
 {
    myGLCD.setFont(SmallFont);myGLCD.invert(true);
    myGLCD.drawRect(73,32, 82,42);
    myGLCD.printNumI(p,75,34, 1);
    myGLCD.update();
    delay(1000);
    myGLCD.invert(false);
  }
  myGLCD.clrScr();
}

void loop() {
 

    char resultButton[7] = "null";
 
    int h = dht.readHumidity();
    int t= dht.readTemperature();
  //  myGLCD.setFont(TinyFont);
    myGLCD.setFont(SmallFont);
    myGLCD.printNumI(h,60,33);
    myGLCD.print("%",73,33);myGLCD.print("H=",48,33);
   
    myGLCD.printNumI(t,15,33 , 1); 
    myGLCD.drawLine(42,28,42,45); // Linia Verticala
    myGLCD.drawRect(0,28, 82,44); // dreptunghi
    myGLCD.drawCircle(30,33,1); // simbol grad
    myGLCD.print("C",32,33);      // 
    myGLCD.print("T=",2,33);
 
    DateTime now = RTC.now(); 
    myGLCD.drawRect(0,0,82,10); // dreptunghi
    if (now.day()<10){
    myGLCD.printNumI(now.day(),17,2, 1);  
    }else
    myGLCD.printNumI(now.day(),12,2, 1); 
    myGLCD.print("-",25,2);
    myGLCD.printNumI(now.month(),31,2 , 1); 
    myGLCD.print("-",44,2);
    myGLCD.printNumI(now.year(),50,2, 1); 
    
    myGLCD.setFont(MediumNumbers);
   // myGLCD.invert(true);
    myGLCD.printNumI(now.hour(),1,11); 
    myGLCD.drawRect(26,17,28,19);
    myGLCD.drawRect(26,22,28,24);
   // m=now.minute();
    if (now.minute() <10) {
    myGLCD.print("0",30,11); 
    myGLCD.printNumI(now.minute(),42,11);
    }else
    myGLCD.printNumI(now.minute(),30,11); 
    myGLCD.drawRect(55,17,57,19);
    myGLCD.drawRect(55,22,57,24);
     if (now.second() <10) {
    myGLCD.print("0",59,11);
    myGLCD.printNumI(now.second(),71,11);
     }else
     myGLCD.printNumI(now.second(),59,11);
    f=now.second();
    delay(100);
    myGLCD.update();
    myGLCD.clrScr();
    
     if (TimerMode ==2) {
   
     }
   
    
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    delay (10);
    
    if (pushlength <pushlengthset) {
     myGLCD.clrScr();
     temp();
     // ShortPush ();   
    }
    
       
       //This runs the setclock routine if the knob is pushed for a long time
       if (pushlength >pushlengthset) {
         myGLCD.clrScr();
         DateTime now = RTC.now();
         setyeartemp=now.year(),DEC;
         setmonthtemp=now.month(),DEC;
         setdaytemp=now.day(),DEC;
         sethourstemp=now.hour(),DEC;
         setminstemp=now.minute(),DEC;
         setclock();
         pushlength = pushlengthset;
       };
}



void setclock (){
  myGLCD.clrScr ();
   setyear ();
   myGLCD.clrScr ();
   setmonth ();
   myGLCD.clrScr ();
   setday ();
   myGLCD.clrScr ();
   sethours ();
   myGLCD.clrScr ();
   setmins ();
   myGLCD.clrScr();
  
   
   RTC.adjust(DateTime(setyeartemp,setmonthtemp,setdaytemp,sethourstemp,setminstemp,setsecs));

   delay (1000);
   
}

// subroutine to return the length of the button push.
int getpushlength () {
  buttonstate = digitalRead(4);  
       if(buttonstate == LOW && buttonflag==false) {     
              pushstart = millis();
              buttonflag = true;
          };
          
       if (buttonstate == HIGH && buttonflag==true) {
         pushstop = millis ();
         pushlength = pushstop - pushstart;
         buttonflag = false;
       };
       return pushlength;
}
// The following subroutines set the individual clock parameters
int setyear () {
myGLCD.clrScr();
   myGLCD.setFont(SmallFont);
    myGLCD.print("Set Year",0,0);
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return setyeartemp;
      myGLCD.update();
    }


    knob.write(0);
    delay (50);
    knobval=knob.read();
    if (knobval < -1) { //bit of software de-bounce
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    setyeartemp=setyeartemp + knobval;
    if (setyeartemp < 2017) { //Year can't be older than currently, it's not a time machine.
      setyeartemp = 2017;
       if (setyeartemp > 2035) { //Year can't be older than currently, it's not a time machine.
      setyeartemp = 2035;
    }}
 
    myGLCD.print(String(setyeartemp),0,20);
   
   myGLCD.update();
    setyear();
}
  
int setmonth () {
myGLCD.clrScr();
  
    myGLCD.print("Set Month",0,0);  myGLCD.update();
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return setmonthtemp;
    }

 //   lcd.setCursor (0,1);
    knob.write(0);
    delay (50);
    knobval=knob.read();
    if (knobval < -1) {
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    setmonthtemp=setmonthtemp + knobval;
    if (setmonthtemp < 1) {// month must be between 1 and 12
      setmonthtemp = 1;
    }
    if (setmonthtemp > 12) {
      setmonthtemp=12;
    }
  
    myGLCD.print(String(setmonthtemp),0,20);  myGLCD.update();
   
    setmonth();
}

int setday () {
  if (setmonthtemp == 4 || setmonthtemp == 5 || setmonthtemp == 9 || setmonthtemp == 11) { //30 days hath September, April June and November
    maxday = 30;
  }
  else {
  maxday = 31; //... all the others have 31
  }
  if (setmonthtemp ==2 && setyeartemp % 4 ==0) { //... Except February alone, and that has 28 days clrScr, and 29 in a leap year.
    maxday = 29;
  }
  if (setmonthtemp ==2 && setyeartemp % 4 !=0) {
    maxday = 28;
  }
myGLCD.clrScr();  
  
    myGLCD.print("Set Day",0,0);  myGLCD.update();
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return setdaytemp;
    }

    knob.write(0);
    delay (50);
    knobval=knob.read();
    if (knobval < -1) {
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    setdaytemp=setdaytemp+ knobval;
    if (setdaytemp < 1) {
      setdaytemp = 1;
    }
    if (setdaytemp > maxday) {
      setdaytemp = maxday;
    }
   
    myGLCD.print(String(setdaytemp),0,20);  myGLCD.update();
   
    setday();
}

int sethours () {
myGLCD.clrScr();
   
    myGLCD.print("Set Hours",0,0);  myGLCD.update();
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return sethourstemp;
    }

  
    knob.write(0);
    delay (50);
    knobval=knob.read();
    if (knobval < -1) {
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    sethourstemp=sethourstemp + knobval;
    if (sethourstemp < 1) {
      sethourstemp = 1;
    }
    if (sethourstemp > 23) {
      sethourstemp=23;
    }
  
    myGLCD.print(String(sethourstemp),0,20);  myGLCD.update();
  
    sethours();
}

int setmins () {
myGLCD.clrScr();
  
    myGLCD.print("Set Mins",0,0);  myGLCD.update();
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return setminstemp;
    }

  
    knob.write(0);
    delay (50);
    knobval=knob.read();
    if (knobval < -1) {
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    setminstemp=setminstemp + knobval;
    if (setminstemp < 0) {
      setminstemp = 0;
    }
    if (setminstemp > 59) {
      setminstemp=59;
    }
   
    myGLCD.print(String(setminstemp),0,20);  myGLCD.update();
    
   setmins();
}

void temp(){
   char resultButton[7] = "null";
 
    int h = dht.readHumidity();
    int t= dht.readTemperature();
    //myGLCD.drawRoundRect(0, 22, 82, 44);
 //myGLCD.setFont(SmallFont);
 myGLCD.setFont(BigNumbers);
 //myGLCD.invertText(true);
    myGLCD.printNumI(h,43,16);myGLCD.setFont(SmallFont);
    myGLCD.print("%",75,26);
    //myGLCD.print("Umid=",10,31);
    myGLCD.setFont(BigNumbers);
   // myGLCD.invertText(false);
    myGLCD.printNumI(t,1,5 , 2); 
   // myGLCD.drawLine(42,28,42,45); // Linia Verticala
   // myGLCD.drawRect(0,28, 82,44); // dreptunghi
    myGLCD.drawCircle(35,8,2); // simbol grad
   // myGLCD.setFont(SmallFont);
  //  myGLCD.print("C",55,0);      // 
  //  myGLCD.print("=Temp",40,10);
     myGLCD.update();
     delay(2500);
}
