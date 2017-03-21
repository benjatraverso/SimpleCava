const int pin = 9;
bool state;

void setup()
{
  Serial.begin(9600);
  pinMode(pin,OUTPUT);
  state = false;
}

void loop()
{
  state = !state;
  digitalWrite(pin, state);
  Serial.println(state);
  delay(2000);
}

