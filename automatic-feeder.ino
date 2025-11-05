//MEGA2560
#include <Servo.h>
Servo servo; // cria o objeto servo
String msg;//String para armazenar a mensagem recebida pela porta serial 3.
const int servoPin = 13;
int lastHour;
int feedInterval;
int feedQuantityTime;
int feedOpenning;
 
void setup() {
  servo.attach(servoPin); //declara pino digital utilizado
  //Defina as porta serial para comunicação com ESP8266.
  Serial.begin(115200);
  Serial3.begin(115200);
  servo.write(90);
  pinMode(servoPin, OUTPUT);
  feedInterval = 4;
  feedQuantityTime = 500;
  lastHour = 99;
  feedOpenning = 0;
}

void feedNow(int hour){
  servo.write(feedOpenning);
  delay(feedQuantityTime);
  servo.write(90); 
  Serial.println("FEEDED");
  lastHour = hour;
}
 
void loop() {
  //Aguarde dados vindos do ESP8266.
  if (Serial3.available()) {
    //Leitura de um byte.
    
    char data = Serial3.read();
    msg += data;
    if(msg.indexOf('\n') > 0){
      Serial.print(msg);
      
      if(msg.indexOf("TIME=") >= 0){
        int newTime = msg.substring(5, 6).toInt();
        feedInterval = newTime;
        Serial.println("NEW TIME SET " + String(newTime));
      }

      else if(msg.indexOf("FEED_NOW") >= 0){
        Serial.println("FEEDING NOW");
        int newHour = msg.substring(9, 11).toInt();
        feedNow(newHour);
      }

      else if(msg.indexOf("FEED_QUANTITY=") >= 0){
        int newFeedQuantity = msg.substring(14, 18).toInt();
        feedQuantityTime = newFeedQuantity;
        Serial.println("NEW FEED QUANTITY " + String(newFeedQuantity));
      }

      else if(msg.indexOf("FEED_OPENNING=") >= 0){
        int newFeedOpenning = msg.substring(14, 18).toInt();
        feedOpenning = newFeedOpenning;
        Serial.println("NEW FEED OPENNING " + String(newFeedOpenning));
      }

      else{
        int hour = msg.substring(0, 2).toInt();
        Serial.println("UPDATE HOUR: " + String(hour));
        Serial.println("LAST HOUR: " + String(lastHour));
        Serial.println("FEED INTERVAL: " + String(feedInterval));
        Serial.println("FEED OPENNING: " + String(feedOpenning));

        if (lastHour >= 90){
          Serial.println("SYSTEM STARTUP");
          lastHour = hour;
        }

        else if (hour <= 20 && hour >= 7) {
          if(abs(hour - lastHour) >= feedInterval){
            feedNow(hour);
          }
        }
      }
      
      msg = "";
    }
    delay(50);
  }
}
