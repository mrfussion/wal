/* mgs 22-12-2-21 */

const int PinTrig = 5;
const int PinEcho = 4;
const float VelSon = 34000.0;

float distancia;
unsigned long duracion;

void setup()
{
  Serial.begin(115200);
  pinMode(PinTrig, OUTPUT);
  pinMode(PinEcho, INPUT);
}

void loop()
{
  iniciarTrigger();

  duracion = pulseIn(PinEcho, HIGH);

  distancia = duracion * 0.034 /2;
  
  Serial.print(distancia);
  Serial.print("cm");
  Serial.println();
  delay(1000);
}

void iniciarTrigger()
{
  digitalWrite(PinTrig, LOW);
  delayMicroseconds(2);

  digitalWrite(PinTrig, HIGH);
  delayMicroseconds(10);

  digitalWrite(PinTrig, LOW);
}
