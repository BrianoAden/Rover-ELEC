void setup() {
  // put your setup code here, to run once:
  Serial.begin(57600);

  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  int c = Serial.read();
  if (c == '0')
  {
    digitalWrite(9, 0);
    digitalWrite(10, 0);
    digitalWrite(11, 0);
    digitalWrite(12, 0);
    digitalWrite(13, 1);
  } else if (c == '1') {
    digitalWrite(9, 1);
    digitalWrite(10, 0);
    digitalWrite(11, 0);
    digitalWrite(12, 0);
    digitalWrite(13, 0);
  } else if (c == '2') {
    digitalWrite(9, 0);
    digitalWrite(10, 1);
    digitalWrite(11, 0);
    digitalWrite(12, 0);
    digitalWrite(13, 0);
  } else if (c == '3') {
    digitalWrite(9, 0);
    digitalWrite(10, 0);
    digitalWrite(11, 1);
    digitalWrite(12, 0);
    digitalWrite(13, 0);
  } else if (c == '4') {
    digitalWrite(9, 0);
    digitalWrite(10, 0);
    digitalWrite(11, 0);
    digitalWrite(12, 1);
    digitalWrite(13, 0);
  }
  delay(10);
}
