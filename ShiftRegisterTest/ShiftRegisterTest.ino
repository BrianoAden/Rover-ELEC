const int SER_PIN = 13;
const int RCLK_PIN = 14;
const int OE_PIN = 25;
const int SRCLK_PIN = 27;
const int SRCLR_PIN = 26;

const int STEP = 33;

class ShiferRegister {
  public:
    int SER_PIN, OE_PIN, RCLK_PIN, SRCLK_PIN, SRCLR_PIN;
    int output_count = 8;

    void setup()
    {
      pinMode(SER_PIN, OUTPUT);
      pinMode(OE_PIN, OUTPUT);
      pinMode(RCLK_PIN, OUTPUT);
      pinMode(SRCLK_PIN, OUTPUT);
      pinMode(SRCLR_PIN, OUTPUT);
    }

    void setPins(int *pins)
    {
      digitalWrite(OE_PIN, LOW);
      digitalWrite(SRCLR_PIN, HIGH);
      for (int i = 0; i < output_count; i++)
      {
        if (pins[output_count-i-1] == 1)
          digitalWrite(SER_PIN, HIGH);
        else
          digitalWrite(SER_PIN, LOW);

        delay(10);
        digitalWrite(SRCLK_PIN, HIGH);
        delay(10);
        digitalWrite(SRCLK_PIN, LOW);

        delay(10);
        digitalWrite(RCLK_PIN, HIGH);
        delay(10);
        digitalWrite(RCLK_PIN, LOW);

        
      }
    }
};

ShiferRegister reg;

void setup() {
  // put your setup code here, to run once:
  reg.SER_PIN = SER_PIN;
  reg.OE_PIN = OE_PIN;
  reg.SRCLK_PIN = SRCLK_PIN;
  reg.RCLK_PIN = RCLK_PIN;
  reg.SRCLR_PIN = SRCLR_PIN;

  reg.setup();

  int pins[] = {0, 0, 1, 1, 0, 0, 0, 0};
  reg.setPins(pins);

}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(STEP, HIGH);
  delay(10);
  digitalWrite(STEP, LOW);
  delay(10);
}
