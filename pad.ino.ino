// =====================================================
// CARRITO CONTROLADO POR XBOX MEDIANTE PUERTO SERIAL
// Formato recibido:
// velocidad,giro
//
// Ejemplos:
// 255,0     avanzar
// -255,0    retroceder
// 0,255     girar a la derecha
// 0,-255    girar a la izquierda
// 0,0       detenerse
// =====================================================

// Motor izquierdo
const int ENA = 11;
const int IN1 = 12;
const int IN2 = 10;

// Motor derecho
const int ENB = 9;
const int IN3 = 8;
const int IN4 = 7;

// Tiempo máximo sin recibir comandos antes de detenerse
const unsigned long TIMEOUT_CONTROL = 500;
unsigned long ultimoComando = 0;

// Variables recibidas
int velocidad = 0;
int giro = 0;

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(20);

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  detenerMotores();

  Serial.println("Carrito listo");
}

void loop() {
  recibirComandoSerial();

  // Seguridad: detenerse si se pierde la comunicación
  if (millis() - ultimoComando > TIMEOUT_CONTROL) {
    detenerMotores();
  }
}

// =====================================================
// RECIBIR "velocidad,giro"
// =====================================================
void recibirComandoSerial() {
  if (Serial.available() <= 0) {
    return;
  }

  String mensaje = Serial.readStringUntil('\n');
  mensaje.trim();

  int separador = mensaje.indexOf(',');

  if (separador == -1) {
    return;
  }

  String textoVelocidad = mensaje.substring(0, separador);
  String textoGiro = mensaje.substring(separador + 1);

  velocidad = constrain(textoVelocidad.toInt(), -255, 255);
  giro = constrain(textoGiro.toInt(), -255, 255);

  controlarCarrito(velocidad, giro);

  ultimoComando = millis();
}

// =====================================================
// CONTROL DIFERENCIAL
// =====================================================
void controlarCarrito(int avance, int rotacion) {
  int velocidadIzquierda = 0;
  int velocidadDerecha = 0;

  // Avanzar o retroceder mientras gira
  if (abs(avance) > 10) {
    velocidadIzquierda = avance + rotacion;
    velocidadDerecha = avance - rotacion;
  }
  else {
    // Giro sobre su propio eje:
    // una rueda avanza y la otra retrocede
    velocidadIzquierda = rotacion;
    velocidadDerecha = -rotacion;
  }

  // Normalizar para no superar el rango de PWM
  int maximo = max(
    abs(velocidadIzquierda),
    abs(velocidadDerecha)
  );

  if (maximo > 255) {
    velocidadIzquierda =
      (long)velocidadIzquierda * 255 / maximo;

    velocidadDerecha =
      (long)velocidadDerecha * 255 / maximo;
  }

  moverMotorIzquierdo(velocidadIzquierda);
  moverMotorDerecho(velocidadDerecha);
}

// =====================================================
// MOTOR IZQUIERDO
// =====================================================
void moverMotorIzquierdo(int pwm) {
  pwm = constrain(pwm, -255, 255);

  if (pwm > 0) {
    // Avanzar
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, pwm);
  }
  else if (pwm < 0) {
    // Retroceder
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    analogWrite(ENA, abs(pwm));
  }
  else {
    // Detener
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, 0);
  }
}

// =====================================================
// MOTOR DERECHO
// =====================================================
void moverMotorDerecho(int pwm) {
  pwm = constrain(pwm, -255, 255);

  if (pwm > 0) {
    // Avanzar
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENB, pwm);
  }
  else if (pwm < 0) {
    // Retroceder
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENB, abs(pwm));
  }
  else {
    // Detener
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    analogWrite(ENB, 0);
  }
}

// =====================================================
// DETENER TODOS LOS MOTORES
// =====================================================
void detenerMotores() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}