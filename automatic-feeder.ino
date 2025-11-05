//MEGA2560 (V18.1 - Corrigido com START_HOUR e END_HOUR)
#include <Servo.h>
Servo servo; // cria o objeto servo
String msg; // String para armazenar a mensagem recebida pela porta serial 3.

// --- Constantes de Hardware e Lógica ---
const int servoPin = 13;
const int START_HOUR = 7; // V18.1: Constante adicionada
const int END_HOUR = 20;  // V18.1: Constante adicionada

// --- Variáveis de Estado ---
int lastHour;
int feedInterval;
int feedQuantityTime;
int feedOpenning;
 
void setup() {
  servo.attach(servoPin); // declara pino digital utilizado
  // Serial (Debug)
  Serial.begin(115200);
  // Serial3 (Comunicação com ESP8266)
  Serial3.begin(115200);
  
  servo.write(90);
  pinMode(servoPin, OUTPUT);
  feedInterval = 4;
  feedQuantityTime = 900;
  lastHour = 99; // 99 indica que ainda não alimentou
  feedOpenning = 0;

  // V18.0: Envia o status inicial para o ESP
  Serial3.println("INIT_HOUR=" + String(lastHour));
  Serial3.println("INIT_INTERVAL=" + String(feedInterval));
  Serial3.println("INIT_QUANTITY=" + String(feedQuantityTime));
}

void feedNow(int hour){
  servo.write(feedOpenning);
  delay(feedQuantityTime); // O delay aqui é aceitável, pois o Mega não tem outras tarefas críticas
  servo.write(90); 
  
  Serial.println("FEEDED"); // Debug local
  lastHour = hour;
  
  // V18.0: Envia a atualização de hora para o ESP
  Serial3.println("LAST_HOUR=" + String(lastHour));
}
 
void loop() {
  // Aguarde dados vindos do ESP8266 (Serial3)
  if (Serial3.available()) {
    char data = Serial3.read();
    msg += data;
    if(msg.indexOf('\n') > 0){
      Serial.print("ESP Disse: " + msg); // Debug local
      
      if(msg.indexOf("TIME=") >= 0){
        int newTime = msg.substring(5, 7).toInt(); // Corrigido para 2 dígitos
        feedInterval = newTime;
        Serial.println("NEW TIME SET " + String(newTime));
        Serial3.println("ACK_TIME=" + String(feedInterval)); // Envia confirmação
      }

      else if(msg.indexOf("FEED_NOW") >= 0){
        Serial.println("FEEDING NOW");
        int newHour = msg.substring(9, 11).toInt();
        feedNow(newHour);
        // feedNow() já envia o LAST_HOUR
      }

      else if(msg.indexOf("FEED_QUANTITY=") >= 0){
        int newFeedQuantity = msg.substring(14, 18).toInt();
        feedQuantityTime = newFeedQuantity;
        Serial.println("NEW FEED QUANTITY " + String(newFeedQuantity));
        Serial3.println("ACK_QUANTITY=" + String(feedQuantityTime)); // Envia confirmação
      }

      // Lógica de alimentação automática (baseada na hora enviada pelo ESP)
      else{
        int hour = msg.substring(0, 2).toInt();
        Serial.println("UPDATE HOUR: " + String(hour));
        Serial.println("LAST HOUR: " + String(lastHour));

        if (lastHour >= 90){
          Serial.println("SYSTEM STARTUP");
          lastHour = hour;
          // Envia o LastHour inicializado para o ESP
          Serial3.println("LAST_HOUR=" + String(lastHour));
        }

        // V18.1: Agora usa as constantes START_HOUR e END_HOUR
        else if (hour <= END_HOUR && hour >= START_HOUR) { 
          if(abs(hour - lastHour) >= feedInterval){
            feedNow(hour);
            // feedNow() já envia o LAST_HOUR
          }
        }
      }
      
      msg = "";
    }
    delay(50);
  }
}