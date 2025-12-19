/*
    Gamepad module provides three different mode namely Digital, JoyStick and Accerleometer.

    You can reduce the size of library compiled by enabling only those modules that you want to
    use. For this first define CUSTOM_SETTINGS followed by defining INCLUDE_modulename.

    Explore more on: https://thestempedia.com/docs/dabble/game-pad-module/
*/
#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE
#include <DabbleESP32.h>

// --- Definições PWM (LEDC para ESP32) ---
// Define a frequência e a resolução do PWM. 8 bits dá 256 níveis de velocidade (0 a 255).
#define FREQUENCIA_PWM 5000  // 5 kHz
#define Reso_PWM 8           // 8 bits

// Define a Zona Morta: Joystick é considerado parado se |valor| < 0.1
#define DEAD_ZONE 0.1

// Canais LEDC (0 a 15) que o ESP32 usará
const int canal_motorA = 0;
const int canal_motorB = 1;
const int MAX_PWM = (1 << Reso_PWM) - 1;
// 255 (2^8 - 1 = 255)

// Motor A "esquerda"
#define IN1 19   //D19 (Forward)
#define IN2 5    //D5  (Backward)
#define EN12 18  //D18 (Enable/PWM)

// Motor B "direita"
#define IN3 12   //D12 (Forward)
#define IN4 21   //D21 (Backward)
#define EN34 22  //D22 (Enable/PWM)


// Função auxiliar para limitar um valor (usado para clamping)
float clamp(float val, float min_val, float max_val) {
  if (val < min_val) return min_val;
  if (val > max_val) return max_val;
  return val;
}


void setup() {
  // Configuração dos pinos de direção e PWM como OUTPUT
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(EN12, OUTPUT);
  pinMode(EN34, OUTPUT);


  // Inicializa motores parados (direção)
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);


  // Configuração PWM (LEDC)
  ledcSetup(canal_motorA, FREQUENCIA_PWM, Reso_PWM);
  ledcAttachPin(EN12, canal_motorA);

  ledcSetup(canal_motorB, FREQUENCIA_PWM, Reso_PWM);
  ledcAttachPin(EN34, canal_motorB);


  // Inicialização Serial e Dabble
  Serial.begin(115200);
  Dabble.begin("robozito3");
}


void loop() {
  Dabble.processInput();
  // Processa a comunicação Bluetooth

  float y_axis = GamePad.getYaxisData();
  // Frente/Trás (Valores de -1.0 a 1.0)
  float x_axis = GamePad.getXaxisData();
  // Esquerda/Direita (Valores de -1.0 a 1.0)

  // 1. Aplicação da ZONA MORTA
  if (abs(y_axis) < DEAD_ZONE && abs(x_axis) < DEAD_ZONE) {
    y_axis = 0.0;
    x_axis = 0.0;
  }

  // 2. Lógica de Tração Diferencial
  // Combina os eixos para obter a velocidade e direção bruta para cada motor
  float rawA = y_axis + x_axis;  // Motor Esquerdo
  float rawB = y_axis - x_axis;  // Motor Direito


  // 3. Limita a velocidade máxima a -1.0 ou 1.0
  rawA = clamp(rawA, -1.0, 1.0);
  rawB = clamp(rawB, -1.0, 1.0);

  // 4. Calcula o valor PWM (0 a 255)
  int speedA = abs(rawA) * MAX_PWM;
  int speedB = abs(rawB) * MAX_PWM;


  // 5. Define a Direção do Motor A (Esquerdo)
  if (rawA > 0) {
    digitalWrite(IN1, LOW);
    // Frente
    digitalWrite(IN2, HIGH);

  } else if (rawA < 0) {
    digitalWrite(IN1, HIGH);
    // Trás
    digitalWrite(IN2, LOW);

  } else {
    digitalWrite(IN1, LOW);
    // Parado
    digitalWrite(IN2, LOW);
  }

  // 6. Define a Direção do Motor B (Direito)
  if (rawB > 0) {
    digitalWrite(IN3, LOW);
    // Frente
    digitalWrite(IN4, HIGH);

  } else if (rawB < 0) {
    digitalWrite(IN3, HIGH);
    // Trás
    digitalWrite(IN4, LOW);

  } else {
    digitalWrite(IN3, LOW);
    // Parado
    digitalWrite(IN4, LOW);
  }


  // 7. Aplica a Velocidade (PWM)
  ledcWrite(canal_motorA, speedA);
  ledcWrite(canal_motorB, speedB);

  // 8. Debug Serial
  Serial.print("speedA: ");
  Serial.print(speedA);
  Serial.print('\t');
  Serial.print("speedB: ");
  Serial.print(speedB);
  Serial.print('\t');
  Serial.print("x_axis: ");
  Serial.print(x_axis);
  Serial.print('\t');
  Serial.print("y_axis: ");
  Serial.println(y_axis);
}