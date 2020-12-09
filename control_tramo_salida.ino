
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"


RF24 radio(15, 16);
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

int pulsador = 6;
int zumbador = 7;
int led_rojo_1 = 2;
int led_rojo_2 = 3;
int led_rojo_3 = 4;
int led_verde = 5;
int led_naranja = 8;
int detector = A3;
int selector = 10;
boolean estado_selector ;
int estado_detector;
boolean estado_pulsador;
boolean tramo_iniciado = false;
boolean penalizacion = false;
int timer = 5;
boolean inicio = false;
int recibido;
boolean inicio_automatico = false;
unsigned long previousMillis = 0;
long interval = 1000;


void setup() {
  pinMode (detector, INPUT);
  pinMode (pulsador, INPUT);
  pinMode (zumbador, OUTPUT);
  pinMode (led_rojo_1, OUTPUT);
  pinMode (led_rojo_2, OUTPUT);
  pinMode (led_rojo_3, OUTPUT);
  pinMode (led_verde, OUTPUT);
  pinMode (led_naranja, OUTPUT);
  pinMode (selector, INPUT);

  Serial.begin(9600);
  radio.begin();
  radio.setRetries(15, 15);
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1, pipes[1]);
  radio.startListening();


}

void loop() {

  while (radio.available() )
  {
    radio.read( &recibido, sizeof(int) );
    Serial.println(recibido);
    if (recibido == 0)
    {
      final_tramo();
    }
    if (recibido == 1)
    {
      if (estado_selector == LOW)
      {
        secuencia_inicio();
      }
    }
    if (recibido == 3)
    {
      reinicio_sistema();
    }
  }


  estado_selector = digitalRead (selector);

  if (estado_selector == HIGH)
  {
    estado_detector = analogRead(detector);
    //Serial.println(estado_detector);
    if (estado_detector <= 450 and tramo_iniciado==LOW)
    {
      tramo_iniciado = HIGH;
      digitalWrite ( led_verde, HIGH);
      radio.stopListening();
      recibido = 1;
      bool ok = radio.write( &recibido, sizeof(int) );
      digitalWrite (zumbador, HIGH);
      delay(500);
      digitalWrite (zumbador, LOW);
      radio.startListening();

    }

    estado_pulsador = digitalRead (pulsador);

    if (estado_pulsador == LOW and tramo_iniciado == HIGH )
    {
      radio.stopListening();
      recibido = 5;
      bool ok = radio.write( &recibido, sizeof(int) );
      delay(250);
      radio.startListening();
      reinicio_sistema();
    }
  }
  else
  {
    estado_pulsador = digitalRead (pulsador);

    if ( estado_pulsador == LOW and tramo_iniciado == false)
    {
      radio.stopListening();
      recibido = 4;
      bool ok = radio.write( &recibido, sizeof(int) );
      delay(250);
      radio.startListening();
      secuencia_inicio();
    }

    if (estado_pulsador == LOW and tramo_iniciado == true)
    {
      radio.stopListening();
      recibido = 5;
      bool ok = radio.write( &recibido, sizeof(int) );
      delay(250);
      radio.startListening();
      reinicio_sistema();
    }
  }
  delay(2);
}

void secuencia_inicio()
{
  digitalWrite (led_rojo_1, LOW);
  digitalWrite (led_rojo_2, LOW);
  digitalWrite (led_rojo_3, LOW);
  digitalWrite (led_verde, LOW);
  digitalWrite (led_naranja, LOW);
  delay(1000);
  inicio = true ;
  while (inicio == true)
  {
    temporizador();
    timer = timer - 1;
    if (timer == 4)
    {
      digitalWrite (led_rojo_1, HIGH);
    }
    if (timer == 3)
    {
      digitalWrite (led_rojo_2, HIGH);
    }
    if (timer == 2)
    {
      digitalWrite (led_rojo_3, HIGH);
    }
    if (timer == 1)
    {
      int aleatorio = (random(0, 11) * 500);
      Serial.println(aleatorio);
      interval = aleatorio;
      Serial.println(interval);
      temporizador();
      digitalWrite (led_rojo_1, LOW);
      digitalWrite (led_rojo_2, LOW);
      digitalWrite (led_rojo_3, LOW);
      digitalWrite (led_verde, HIGH);
      radio.stopListening();
      if (penalizacion == false)
      {
        recibido = 1;
      }
      else
      {
        recibido = 2;
        penalizacion = false;
      }

      bool ok = radio.write( &recibido, sizeof(int) );
      digitalWrite (zumbador, HIGH);
      delay(1200);
      digitalWrite (zumbador, LOW);
      radio.startListening();
      inicio = false;
      timer = 5;
      tramo_iniciado = true;
      estado_pulsador = HIGH;
    }
  }
}

void final_tramo()
{
  digitalWrite (led_verde, LOW);
  digitalWrite (led_rojo_1, LOW);
  digitalWrite (led_rojo_2, LOW);
  digitalWrite (led_rojo_3, LOW);
  digitalWrite (led_naranja, LOW);
  tramo_iniciado = false;
  digitalWrite (zumbador, HIGH);
  delay(1200);
  digitalWrite (zumbador, LOW);
}

void reinicio_sistema()
{
  digitalWrite (led_rojo_1, LOW);
  digitalWrite (led_rojo_2, LOW);
  digitalWrite (led_rojo_3, LOW);
  digitalWrite (led_verde, LOW);
  digitalWrite (led_naranja, LOW);
  inicio = false;
  tramo_iniciado = false;
  timer = 5;
}
void temporizador()
{
  unsigned long currentMillis;
  Serial.println("temporizador");
  boolean temp = true;
  while (temp == true)
  {
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval)
    {
      previousMillis = currentMillis;
      temp = false;
    }
    estado_detector = analogRead(detector);
    if (estado_detector <= 450)
    {
      penalizacion = true;
      digitalWrite(led_naranja, HIGH);
    }
  }
  interval = 1000;
}
