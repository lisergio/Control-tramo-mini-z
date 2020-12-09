#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "ssd1306.h"


RF24 radio(15, 16);
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };
int recibido;

int led_rojo = 4;
int led_verde = 6;
int led_naranja = 7;
int pulsador = 3;
int detector = A3;
boolean estado_pulsador;
int estado_detector;
boolean crono = false;
boolean penalizacion = false;

int m, s, l;
int mu = 0, md = 0, su = 0, sd = 0, lu = 0, ld = 0, lc = 0;
int mu2 = 0, md2 = 0, su2 = 0, sd2 = 0, lu2 = 0, ld2 = 0, lc2 = 0;
long int tiempo, tiempo_inicio;

void setup() {
  pinMode (pulsador, INPUT);
  pinMode (detector, INPUT);
  pinMode (led_rojo, OUTPUT);
  pinMode (led_verde, OUTPUT);
  pinMode (led_naranja, OUTPUT);
  digitalWrite( led_rojo, HIGH);
  Serial.begin(9600);
  radio.begin();
  radio.setRetries(15, 15);
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1, pipes[1]);
  radio.startListening();

  ssd1306_setFixedFont(ssd1306xled_font8x16);
  ssd1306_128x64_i2c_init();
  ssd1306_clearScreen();
  ssd1306_printFixed(25, 4, "ESPERANDO", STYLE_NORMAL);
  ssd1306_printFixed(33, 30, "INICIO", STYLE_NORMAL);
  ssd1306_printFixed(26, 55, "DE TRAMO", STYLE_NORMAL);
  delay(1000);
}

void loop() {

  estado_pulsador = digitalRead (pulsador);
  estado_detector = analogRead (detector);


  if (estado_pulsador == LOW )
  {
    digitalWrite (led_verde, LOW);
    digitalWrite (led_rojo, HIGH);
    digitalWrite (led_naranja, LOW);
    ssd1306_clearScreen();
    ssd1306_printFixed(25, 4, "ESPERANDO", STYLE_NORMAL);
    ssd1306_printFixed(33, 30, "INICIO", STYLE_NORMAL);
    ssd1306_printFixed(26, 55, "DE TRAMO", STYLE_NORMAL);
    radio.stopListening();
    recibido = 1;
    bool ok = radio.write( &recibido, sizeof(int) );
    delay(500);
    radio.startListening();
  }
  while (radio.available() )
  {
    radio.read( &recibido, sizeof(int) );
    Serial.println(recibido);
    if (recibido == 1)
    {
      digitalWrite (led_verde, HIGH);
      digitalWrite (led_rojo, LOW);
      digitalWrite (led_naranja, LOW);
      crono = true;
      tiempo_inicio = millis();
      crono_iniciado();
    }
    if (recibido == 2)
    {
      digitalWrite (led_verde, HIGH);
      digitalWrite (led_rojo, LOW);
      crono = true;
      tiempo_inicio = millis();
      digitalWrite (led_naranja, HIGH);
      penalizacion = true;
      crono_iniciado();
    }
    if (recibido == 4)
    {
      digitalWrite (led_verde, LOW);
      digitalWrite (led_rojo, HIGH);
      digitalWrite (led_naranja, LOW);
      crono = false;
      penalizacion = false;
      ssd1306_clearScreen();
      ssd1306_printFixed(25, 4, "ESPERANDO", STYLE_NORMAL);
      ssd1306_printFixed(33, 30, "INICIO", STYLE_NORMAL);
      ssd1306_printFixed(26, 55, "DE TRAMO", STYLE_NORMAL);
    }


  }
  delay(10);
}

void crono_iniciado()
{
  ssd1306_clearScreen();
  while (crono == true)
  {
    if (penalizacion == true)
    {
      ssd1306_setFixedFont(ssd1306xled_font8x16);
      ssd1306_printFixed(25, 0, "PENALIZADO", STYLE_NORMAL);
    }
    tiempo = millis() - tiempo_inicio;
    m = tiempo / 60000;
    mu = m % 10;
    md = (m - mu) / 10;
    s = (tiempo / 1000) - (m * 60);
    su = s % 10;
    sd = (s - su) / 10;
    l = tiempo - (s * 1000) - (m * 60000);
    lu = l % 10;
    ld = ((l - lu) / 10) % 10;
    lc = (l - (ld * 10) - lu) / 100;

    char msg[10];
    sprintf (msg, "%i%i:%i%i:%i%i%i", md, mu, sd, su, lc, ld, lu);

    ssd1306_setFixedFont(ssd1306xled_font8x16);
    ssd1306_printFixed(25, 28, msg, STYLE_BOLD);
    estado_pulsador = digitalRead (pulsador);
    estado_detector = analogRead (detector);
    if ( estado_detector >= 400)
    {
      digitalWrite (led_rojo, HIGH);
      digitalWrite (led_verde, LOW);
      radio.stopListening();
      recibido = 0;
      bool ok = radio.write( &recibido, sizeof(int) );
      delay(500);
      radio.startListening();
      crono = false;
      ssd1306_clearScreen();
      if (penalizacion == true)
      {
        ssd1306_printFixed(0, 4, "TRAMO PENALIZADO", STYLE_NORMAL);
      }
      else
      {
        ssd1306_printFixed(0, 4, "TRAMO FINALIZADO", STYLE_NORMAL);
      }
      ssd1306_printFixed(15, 30, "TIEMPO TOTAL", STYLE_NORMAL);
      if (penalizacion == true)
      {
        tiempo = tiempo + 10000;
        m = tiempo / 60000;
        mu = m % 10;
        md = (m - mu) / 10;
        s = (tiempo / 1000) - (m * 60);
        su = s % 10;
        sd = (s - su) / 10;
        l = tiempo - (s * 1000) - (m * 60000);
        lu = l % 10;
        ld = ((l - lu) / 10) % 10;
        lc = (l - (ld * 10) - lu) / 100;
        sprintf (msg, "%i%i:%i%i:%i%i%i", md, mu, sd, su, lc, ld, lu);
        ssd1306_printFixed(25, 55, msg, STYLE_NORMAL);
      }
      else
      {
        ssd1306_printFixed(25, 55, msg, STYLE_NORMAL);
      }
      penalizacion = false;
    }

    if (estado_pulsador == LOW )
    {
      ssd1306_clearScreen();
      ssd1306_printFixed(0, 32, "TRAMO CANCELADO", STYLE_NORMAL);
      penalizacion = false;
      digitalWrite (led_rojo, HIGH);
      digitalWrite (led_verde, LOW);
      digitalWrite (led_naranja, LOW);
      crono = false;
      radio.stopListening();
      recibido = 3;
      bool ok = radio.write( &recibido, sizeof(int) );
      delay(500);
      radio.startListening();
      delay(2000);
      ssd1306_clearScreen();
      ssd1306_printFixed(25, 4, "ESPERANDO", STYLE_NORMAL);
      ssd1306_printFixed(33, 30, "INICIO", STYLE_NORMAL);
      ssd1306_printFixed(26, 55, "DE TRAMO", STYLE_NORMAL);
    }
    while (radio.available() )
    {
      radio.read( &recibido, sizeof(int) );
      Serial.println(recibido);
      if (recibido == 5 )
      {
        ssd1306_clearScreen();
        ssd1306_printFixed(0, 32, "TRAMO CANCELADO", STYLE_NORMAL);
        penalizacion = false;
        digitalWrite (led_rojo, HIGH);
        digitalWrite (led_verde, LOW);
        digitalWrite (led_naranja, LOW);
        crono = false;
        delay(2000);
        ssd1306_clearScreen();
        ssd1306_printFixed(25, 4, "ESPERANDO", STYLE_NORMAL);
        ssd1306_printFixed(33, 30, "INICIO", STYLE_NORMAL);
        ssd1306_printFixed(26, 55, "DE TRAMO", STYLE_NORMAL);
      }

    }
  }
}
