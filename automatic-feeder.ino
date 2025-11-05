//MEGA2560 (V19.0 - Refatorado para estabilidade de memória)
#include <Servo.h>
#include <string.h> // Para strstr, atoi

Servo servo;
const int servoPin = 13;

// --- Constantes de Lógica ---
const int START_HOUR = 7; 
const int END_HOUR = 20;  

// --- V19.0: Buffer de Char para Serial (Remove String global) ---
const int MSG_BUFFER_SIZE = 64;
char serialBuffer[MSG_BUFFER_SIZE];
int bufferIndex = 0;

// --- Variáveis de Estado ---
bool isInitialized = false; // V19.0: Substitui o "magic number" 99
int lastHour;
int feedInterval;
int feedQuantityTime;
int feedOpeningAngle; // V19.0: Typo corrigido
 
void setup() {
  servo.attach(servoPin); 
  Serial.begin(115200);
  Serial3.begin(115200);
  
  servo.write(90);
  pinMode(servoPin, OUTPUT);
  feedInterval = 4;
  feedQuantityTime = 900;
  feedOpeningAngle = 0; 
  
  // Envia o status inicial para o ESP
  Serial3.println("INIT_INTERVAL=" + String(feedInterval));
  Serial3.println("INIT_QUANTITY=" + String(feedQuantityTime));
  Serial3.println("INIT_HOUR=-1"); // Envia -1 (não inicializado)
}

void feedNow(int hour){
  servo.write(feedOpeningAngle);
  delay(feedQuantityTime); 
  servo.write(90); 
  
  Serial.println("FEEDED"); // Debug local
  lastHour = hour;
  isInitialized = true;
  
  // Envia a atualização de hora para o ESP
  Serial3.println("LAST_HOUR=" + String(lastHour));
}

// V19.0: Nova função de parsing (mais rápida e sem "magic numbers")
void parseCommand(char* command) {
    Serial.print("ESP Disse: "); Serial.println(command); // Debug local
    
    char* valuePtr; // Ponteiro para o valor após o '='
    
    if ((valuePtr = strstr(command, "TIME="))) {
        int newTime = atoi(valuePtr + 5); // Converte o número após "TIME="
        feedInterval = newTime;
        Serial.println("NEW TIME SET " + String(newTime));
        Serial3.println("ACK_TIME=" + String(feedInterval)); 
    }
    else if ((valuePtr = strstr(command, "FEED_NOW="))) {
        Serial.println("FEEDING NOW");
        int newHour = atoi(valuePtr + 9);
        feedNow(newHour);
    }
    else if ((valuePtr = strstr(command, "FEED_QUANTITY="))) {
        int newFeedQuantity = atoi(valuePtr + 14);
        feedQuantityTime = newFeedQuantity;
        Serial.println("NEW FEED QUANTITY " + String(newFeedQuantity));
        Serial3.println("ACK_QUANTITY=" + String(feedQuantityTime));
    }
    else if ((valuePtr = strstr(command, "FEED_OPENNING="))) {
        int newFeedOpenning = atoi(valuePtr + 14);
        feedOpeningAngle = newFeedOpenning;
        Serial.println("NEW FEED OPENNING " + String(newFeedOpenning));
    }
    else {
        // Assume que é o comando de atualização de hora
        int hour = atoi(command); 
        Serial.println("UPDATE HOUR: " + String(hour));
        Serial.println("LAST HOUR: " + String(lastHour));

        if (!isInitialized){ // V19.0: Usa booleano
          Serial.println("SYSTEM STARTUP");
          lastHour = hour;
          isInitialized = true;
          Serial3.println("LAST_HOUR=" + String(lastHour));
        }
        else if (hour <= END_HOUR && hour >= START_HOUR) { 
          if(abs(hour - lastHour) >= feedInterval){
            feedNow(hour);
          }
        }
    }
}
 
void loop() {
  // V19.0: Leitura de Serial otimizada (sem classe String)
  while (Serial3.available()) {
    char data = Serial3.read();

    if (data == '\n') { // Fim do comando
      serialBuffer[bufferIndex] = '\0'; // Termina a string
      parseCommand(serialBuffer);
      bufferIndex = 0; // Reseta o buffer
    } 
    else if (bufferIndex < MSG_BUFFER_SIZE - 1) {
      serialBuffer[bufferIndex] = data;
      bufferIndex++;
    }
    // Ignora dados se o buffer estourar
  }
}