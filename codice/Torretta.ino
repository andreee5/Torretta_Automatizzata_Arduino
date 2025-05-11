#include <Servo.h>

//-----Dichiarazione variabili servomotori
Servo z_servo;                                   //Servo per il rinculo e la spinta del dardo
Servo x_servo;                                   //Servo per la rotazione sull'asse x
Servo y_servo;                                   //Servo per l'inclinazione sull'asse y

const byte x_limite_1 = 0;                       //Const la usi esclusivamente per definire un dato
const byte x_limite_2 = 180;                     //Angoli limite della rotazione sull'asse x del servo (da 0 a 180 gradi)
const byte y_limite_1 = 65;
const byte y_limite_2 = 180;                     //Angoli limite dell'inclinazione sull'asse y del servo (da 65 a 180 gradi)
const byte z_riposo = 180;                       //Angolo del servo quando è a riposo
const byte z_sparo = 45;                        //Angolo del servo necessario per spingere il dardo
int angolo_x = 90;                               //Angoli di partenza dei servo in x e y
int angolo_y = 105;

//-----Dichiarazione variabili joystick
#define JOYSTICK_X A0                            //Define puoi utilizzarlo per definire qualunque cosa, pure del codice sotto forma di macro.
#define JOYSTICK_Y A1
#define JOYSTICK_BUTTON 2
int x, y, button;

//-----Variabili relative ai tempi e agli spari del motore
bool sta_sparando =  false;
bool puo_sparare =  false;

unsigned long sparo_start_time = 0;
unsigned long sparo_current_time = 0;
double sparo_time = 150;

unsigned long rinculo_start_time = 0;
unsigned long rinculo_current_time = 0;
double rinculo_time = 2 * sparo_time;

//-----Dichiarazioni vaiabili motor
bool motors_ON = false;
//PRIMO MOTORE
int DirezioneA_motor1 = 22;
int DirezioneB_motor1 = 24;
int Enable_motor1 = 26;
//SECONDO MOTORE
int DirezioneA_motor2 = 28;
int DirezioneB_motor2 = 30;
int Enable_motor2 = 32;

// ===============================================================================

void setup(){
  //-----Dichiara joystick pin mode
  pinMode(JOYSTICK_BUTTON, INPUT_PULLUP);
  
  //-----Dichiara motor pin mode
  pinMode(DirezioneA_motor1, OUTPUT);
  pinMode(DirezioneB_motor1, OUTPUT);
  pinMode(Enable_motor1, OUTPUT);
  pinMode(DirezioneA_motor2, OUTPUT);
  pinMode(DirezioneB_motor2, OUTPUT);
  pinMode(Enable_motor2, OUTPUT);
  analogWrite(Enable_motor1, 250);             //Abilita il motor1
  analogWrite(Enable_motor2, 250);             //Abilita il motor2
  
  //-----Collega i servo ai pin
  z_servo.attach(9);
  x_servo.attach(10);
  y_servo.attach(11);

  //-----Sequenza di partenza
  z_servo.write(z_riposo);
  x_servo.write(angolo_x);
  delay(1000);
  y_servo.write(angolo_y);

  Serial.begin(9600);                            //Iniziare la comunicazione seriale
}

// ===============================================================================

void loop(){
  x = analogRead(JOYSTICK_X);                    //Lettura spostamento lungo x del joystik
  y = analogRead(JOYSTICK_Y);                    //Lettura spostamento lungo y del joystik
  button = !digitalRead(JOYSTICK_BUTTON);        //Lettura in caso si prema il joystick
  posizionamento_servo();
  rotazione_motor();
  verifica_se_puo_sparare();
  sparo();
}

// ===============================================================================

void rotazione_motor(){
  //-----Avvio e arresto dei motori utilizzando il modulo L298N.
  
  if (!sta_sparando) {                           //Se non sta già sparando o caricando
    if(button == HIGH) {                         //Se il joystick viene premuto
      digitalWrite(DirezioneA_motor1, HIGH);
      digitalWrite(DirezioneB_motor1, LOW);      //Accendo motor1
      digitalWrite(DirezioneA_motor2, HIGH);
      digitalWrite(DirezioneB_motor2, LOW);     //Accendo motor2 in senso opposto a motor1
      motors_ON = true;
      delay(1500);
    }
    else if (!motors_ON) {                       //Se il joystick non viene premuto
      digitalWrite(DirezioneA_motor1, LOW);
      digitalWrite(DirezioneB_motor1, LOW);      //Spengo motor1
      digitalWrite(DirezioneA_motor2, LOW);
      digitalWrite(DirezioneB_motor2, LOW);      //Spengo motor2
    }
  }
}

// ===============================================================================

void posizionamento_servo() {
  //-----Posizionamento attraverso lo spostamento dei servo x e y.
  
  if (x<400 && angolo_x>x_limite_1) {
    angolo_x-=6;
    x_servo.write(angolo_x);
  }
  else if (x>600 && angolo_x<x_limite_2) {
    angolo_x+=6;
    x_servo.write(angolo_x);
  }
  if (y<400 && angolo_y>y_limite_1) {
    angolo_y-=2;
    y_servo.write(angolo_y);
  }
  else if (y>600 && angolo_y<y_limite_2) {
    angolo_y+=2;
    y_servo.write(angolo_y);
  }
   delay(25);
}

// ===============================================================================

void verifica_se_puo_sparare() {
  //-----Verifica se può sparare o no
  
  if (!sta_sparando) {                           //Se non sta già sparando o caricando
    if (button == HIGH) {                        //Se il joystick viene premuto
      puo_sparare = true;                        //Porta la variabile "puo_sparare" a true (vediamo gli effetti nel void sparo())
    }
    else if (!motors_ON) {                       //Se il joystick non viene premuto
      puo_sparare = false;                       //Porta puo_sparare a false (vediamo gli effetti nel void sparo())
    }
  }
}

// ===============================================================================

void sparo() {
  //-----Se può sparare, controlla il tempo trascorso dall'accensione del motor
  //-----Se è inferiore a certi parametri allora avvia la sequenza di sparo
  
  if (puo_sparare && !sta_sparando) {
    sparo_start_time = millis();
    rinculo_start_time = millis();
    sta_sparando = true;
  }

  sparo_current_time = millis();
  rinculo_current_time = millis();

  if (sta_sparando && sparo_current_time - sparo_start_time < sparo_time) {
    z_servo.write(z_sparo);                       //Posiziona il servo per il rinculo e la spinta del dardo in posizione di sparo
  }                                               //(125 gradi --> angolo del servo necessario per spingere il dardo)
  else if (sta_sparando && rinculo_current_time - rinculo_start_time < rinculo_time) {
    z_servo.write(z_riposo);                      //Posiziona il servo per il rinculo e la spinta del dardo in posizione di sparo
  }                                               //(180 gradi --> angolo del servo quando è a riposo)
  else if (sta_sparando && rinculo_current_time - rinculo_start_time > rinculo_time) {
    sta_sparando = false;
    motors_ON = false;
    delay(1000);
  }
}