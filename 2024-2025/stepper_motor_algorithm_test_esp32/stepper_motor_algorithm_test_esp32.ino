// Example pins
const int motor_puls[] = {0x1u, 0x2u, 0x4u, 0x8u};
const int motor_dirs[] = {0x16u, 0x32u, 0x64u, 0x128u};
float motor_speeds[] = {500, 750, 100000, 100000}; // steps per s
int motor_steps[] = {100, 200, 300, 400};
const int motors = 2;

#define PACKET_SIZE 10000
// Micro seconds 10^-6 seconds
#define MICROSECOND 0.000001

#define PULSE_DELAY 25 // In microseconds 
#define STEP_DELAY (PULSE_DELAY * 2)  // In microseconds 


#define MIN_MOTOR_SPEED (1 / (MICROSECOND * PACKET_SIZE * STEP_DELAY))
#define MAX_MOTOR_SPEED (1 / (STEP_DELAY * MICROSECOND))

int reorder_array[PACKET_SIZE];

void shuffleArray(int arr[], int size) {
    for (int i = size - 1; i > 0; i--) {
        int j = random(i + 1);  // Get a random index from 0 to i
        // Swap arr[i] and arr[j]
        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

// Only done once 
void generate_reorder_array()
{
    // fill with -1s
    for (int i = 0; i < PACKET_SIZE; i++)
    { reorder_array[i] = i; }

    shuffleArray(reorder_array, PACKET_SIZE);
}

void fill_packet(char *packet)
{
    // Fill with 0s
    for (int i = 0; i < PACKET_SIZE; i++)
    { packet[i] = 0; }


    for (int motor = 0; motor < motors; motor++)
    {
        // Should round down
        if (motor_speeds[motor] < MIN_MOTOR_SPEED) {
          motor_speeds[motor] = MIN_MOTOR_SPEED;
          Serial.printf("Motor speed too low\n");
        }


        const int expected_movements = motor_speeds[motor] * MICROSECOND * PACKET_SIZE * STEP_DELAY;
        if (expected_movements > PACKET_SIZE)
        {
          Serial.printf("Expected movements %d\n", expected_movements);
          break;
        }
        // Serial.printf("Expected movements %d\n", expected_movements);
        
        for (int i = 0; i < expected_movements; i++)
        {
            packet[reorder_array[i]] |= 0x1u << motor;
        }

        // if (expected_movements > motor_steps[motor])
        // {
        //     int iters = 0;
        //     char *packet_head = &(packet[PACKET_SIZE-1]);
        //     while (motor_steps[motor] > 0 && iters < PACKET_SIZE)
        //     {
        //         motor_steps[motor] -= (*packet_head >> motor) & 0x1u;
        //         packet_head--;
        //         iters++;
        //     }
        // } else {
        //     motor_steps[motor] -= expected_movements;
        // }
    }
}

char packet[PACKET_SIZE];

void setup() {
  pinMode(25, OUTPUT);
  pinMode(26, OUTPUT);

  digitalWrite(25, HIGH);

  // pinMode(25, OUTPUT);
  pinMode(14, OUTPUT);

  Serial.begin(9600);

  randomSeed(analogRead(0));

  generate_reorder_array();

  Serial.printf("MIN_MOTOR_SPEED %f, MAX_MOTOR_SPEED %f", MIN_MOTOR_SPEED, MAX_MOTOR_SPEED);
}

void loop() {

  // Serial.println("Packet Start");
  fill_packet(packet);
  for (int i = 0; i < PACKET_SIZE; i++)
  {
    if ((packet[i] & 0x1) == 1)
      digitalWrite(14, HIGH);

    if (((packet[i] & 0x2) >> 1 )== 1)
      digitalWrite(26, HIGH);

    delayMicroseconds(PULSE_DELAY);
    digitalWrite(14, LOW);
    digitalWrite(26, LOW);
    delayMicroseconds(PULSE_DELAY);
  }
  
  // motor_speeds[0] -= 10;
}