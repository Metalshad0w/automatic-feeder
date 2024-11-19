#include <Servo.h>
Servo servo; // cria o objeto servo
String msg;//String para armazenar a mensagem recebida pela porta serial 3.
int lastHour;
int feedInterval;
 
void setup() {
  servo.attach(2); //declara pino digital utilizado
  //Defina as porta serial para comunicação com ESP8266.
  Serial.begin(115200);
  Serial3.begin(115200);
  servo.write(25);
  feedInterval = 3;
  lastHour = 99;
}

void feedNow(int hour){
  servo.write(200);
  delay(100);
  servo.write(25); 
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
      
      if(msg.indexOf("TIME_") >= 0){
        int newTime = msg.substring(5, 6).toInt();
        feedInterval = newTime;
        Serial.println("NEW TIME SET " + String(newTime));
      }

      else if(msg.indexOf("FEED_NOW") >= 0){
        Serial.println("FEEDING NOW");
        int newHour = msg.substring(9, 11).toInt();
        feedNow(newHour);
      }

      else{
        int hour = msg.substring(0, 2).toInt();
        Serial.println("UPDATE HOUR: " + String(hour));
        Serial.println("LAST HOUR: " + String(lastHour));
        Serial.println("FEED INTERVAL: " + String(feedInterval));

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
