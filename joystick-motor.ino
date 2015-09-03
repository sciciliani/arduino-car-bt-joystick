#define VERSION     "\n\nAndroTest V2.0 - @kas2014\ndemo for V5.x App"
#include <AFMotor.h>

#define    STX              0x02
#define    ETX              0x03
#define    ledPin       13
#define    SLOW         750                            // Datafields refresh rate (ms)
#define    FAST         250                             // Datafields refresh rate (ms)


byte cmd[8] = {0, 0, 0, 0, 0, 0, 0, 0};                 // bytes received
byte buttonStatus = 0;                                  // first Byte sent to Android device
long previousMillis = 0;                                // will store last time Buttons status was updated
long sendInterval = SLOW;                               // interval between Buttons status transmission (milliseconds)
String displayStatus = "xxxx";                          // message to Android device
AF_DCMotor motorI(1);
AF_DCMotor motorD(2);

void setup()  {
 Serial.begin(9600);
 pinMode(ledPin, OUTPUT);     
 motorI.setSpeed(0);
 motorD.setSpeed(0);
 motorI.run(RELEASE);
 motorD.run(RELEASE);
 
}

void loop() {
 if(Serial.available())  {                           // data received from smartphone
   delay(20);
   cmd[0] =  Serial.read();  
   if(cmd[0] == STX)  {
     int i=1;      
     while(Serial.available())  {
       delay(10);
       cmd[i] = Serial.read();
       if(cmd[i]>127 || i>7)                 break;     // Communication error
       if((cmd[i]==ETX) && (i==2 || i==7))   break;     // Button or Joystick data
       i++;
     }
     if     (i==2)          getButtonState(cmd[1]);    // 3 Bytes  ex: < STX "C" ETX >
     else if(i==7)          getJoystickState(cmd);     // 6 Bytes  ex: < STX "200" "180" ETX >
   }
//   Serial.print((char)cmd);
 } 
}

void sendBlueToothData()  {
 static long previousMillis = 0;                             
 long currentMillis = millis();
 if(currentMillis - previousMillis > sendInterval) {   // send data back to smartphone
   previousMillis = currentMillis; 

// Data frame transmitted back from Arduino to Android device:
// < 0X02   Buttons state   0X01   DataField#1   0x04   DataField#2   0x05   DataField#3    0x03 >  
// < 0X02      "01011"      0X01     "120.00"    0x04     "-4500"     0x05  "Motor enabled" 0x03 >    // example

    Serial.print((char)STX);                                             // Start of Transmission
    Serial.print(getButtonStatusString());   Serial.print((char)0x1);   // buttons status feedback
    Serial.print(GetdataInt1());             Serial.print((char)0x4);   // datafield #1
    Serial.print(GetdataFloat2());           Serial.print((char)0x5);   // datafield #2
    Serial.print(displayStatus);                                         // datafield #3
    Serial.print((char)ETX);                                             // End of Transmission
 }  
}

String getButtonStatusString()  {
 String bStatus = "";
 for(int i=0; i<6; i++)  {
   if(buttonStatus & (B100000 >>i))      bStatus += "1";
   else                                  bStatus += "0";
 }
 return bStatus;
}

int GetdataInt1()  {              // Data dummy values sent to Android device for demo purpose
 static int i= -30;              // Replace with your own code
 i ++;
 if(i >0)    i = -30;
 return i;  
}

float GetdataFloat2()  {           // Data dummy values sent to Android device for demo purpose
 static float i=50;               // Replace with your own code
 i-=.5;
 if(i <-50)    i = 50;
 return i;  
}

void getJoystickState(byte data[8])    {
 float vmax; int vmd; int vmi; float perc;
 int joyX = (data[1]-48)*100 + (data[2]-48)*10 + (data[3]-48);       // obtain the Int from the ASCII representation
 int joyY = (data[4]-48)*100 + (data[5]-48)*10 + (data[6]-48);
 joyX = joyX - 200;                                                  // Offset to avoid
 joyY = joyY - 200;                                                  // transmitting negative numbers

 if(joyX<-100 || joyX>100 || joyY<-100 || joyY>100)     return;      // commmunication error
    if ((joyX == 0) && (joyY == 0)) {
      motorI.run(RELEASE);
      motorD.run(RELEASE);
      return;
    }

    if (joyY > 0) {
      motorI.run(FORWARD);
      motorD.run(FORWARD);
      vmax = joyY * 255 / 100  ;
    } else {
      motorI.run(BACKWARD);
      motorD.run(BACKWARD);  
      vmax = -joyY * 255 / 100     ;
    }
    
  if (joyX == 0) {
    vmd = (int) vmax;
    vmi = (int) vmax;
  }
  else if(joyX > 0) {
    perc = (100.0 -  (float) joyX ) / 100.0;
    vmd = (int)  (vmax * perc);
    vmi = (int) vmax;
  }
  else if(joyX < 0) {
    perc = (100.0  +  (float) joyX ) / 100.0;
    vmi = (int) (vmax * perc);
    vmd = (int) vmax;
  }
  motorI.setSpeed(vmi);
  motorD.setSpeed(vmd);

}

void getButtonState(int bStatus)  {
 switch (bStatus) {
// -----------------  BUTTON #1  -----------------------
   case 'A':
     buttonStatus |= B000001;        // ON
//     // Serial.println("\n** Button_1: ON **");
     // your code...      
     displayStatus = "LED <ON>";
     // Serial.println(displayStatus);
     digitalWrite(ledPin, HIGH);
     break;
   case 'B':
     buttonStatus &= B111110;        // OFF
     // Serial.println("\n** Button_1: OFF **");
     // your code...      
     displayStatus = "LED <OFF>";
     // Serial.println(displayStatus);
     digitalWrite(ledPin, LOW);
     break;

// -----------------  BUTTON #2  -----------------------
   case 'C':
     buttonStatus |= B000010;        // ON
     // Serial.println("\n** Button_2: ON **");
     // your code...      
     displayStatus = "Button2 <ON>";
     // Serial.println(displayStatus);
     break;
   case 'D':
     buttonStatus &= B111101;        // OFF
     // Serial.println("\n** Button_2: OFF **");
     // your code...      
     displayStatus = "Button2 <OFF>";
     // Serial.println(displayStatus);
     break;

// -----------------  BUTTON #3  -----------------------
   case 'E':
     buttonStatus |= B000100;        // ON
     // Serial.println("\n** Button_3: ON **");
     // your code...      
     displayStatus = "Motor #1 enabled"; // Demo text message
     // Serial.println(displayStatus);
     break;
   case 'F':
     buttonStatus &= B111011;      // OFF
     // Serial.println("\n** Button_3: OFF **");
     // your code...      
     displayStatus = "Motor #1 stopped";
     // Serial.println(displayStatus);
     break;

// -----------------  BUTTON #4  -----------------------
   case 'G':
     buttonStatus |= B001000;       // ON
     // Serial.println("\n** Button_4: ON **");
     // your code...      
     displayStatus = "Datafield update <FAST>";
     // Serial.println(displayStatus);
     sendInterval = FAST;
     break;
   case 'H':
     buttonStatus &= B110111;    // OFF
     // Serial.println("\n** Button_4: OFF **");
     // your code...      
     displayStatus = "Datafield update <SLOW>";
     // Serial.println(displayStatus);
     sendInterval = SLOW;
    break;

// -----------------  BUTTON #5  -----------------------
   case 'I':           // configured as momentary button
//      buttonStatus |= B010000;        // ON
     // Serial.println("\n** Button_5: ++ pushed ++ **");
     // your code...      
     displayStatus = "Button5: <pushed>";
     break;
// -----------------  BUTTON #6  -----------------------
   case 'K':
     buttonStatus |= B100000;        // ON
     // Serial.println("\n** Button_6: ON **");
     // your code...      
      displayStatus = "Button6 <ON>"; // Demo text message
    break;
   case 'L':
     buttonStatus &= B011111;        // OFF
     // Serial.println("\n** Button_6: OFF **");
     // your code...      
     displayStatus = "Button6 <OFF>";
     break;
 }
// ---------------------------------------------------------------
}

