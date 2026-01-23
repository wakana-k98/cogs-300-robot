// Motor A (Left)
int enA = 9;
int in1 = 8;
int in2 = 7;

// Motor B (Right)
int enB = 10;
int in3 = 12;
int in4 = 13;

int speed = 200; // Default speed

void setup() {
  pinMode(enA, OUTPUT); 
  pinMode(in1, OUTPUT); 
  pinMode(in2, OUTPUT);

  pinMode(enB, OUTPUT); 
  pinMode(in3, OUTPUT); 
  pinMode(in4, OUTPUT);

  Serial.begin(9600);
}

void loop() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();

   switch(cmd) {
  case 'R': // Forward was turning left, fix by swapping motor directions
    drive(HIGH, LOW, HIGH, LOW);  // Swap pins for forward
    break;
  case 'L': // Backward was turning right
    drive(LOW, HIGH, LOW, HIGH);
    break;
  case 'B': // Left was moving forward
    drive(LOW, HIGH, HIGH, LOW);
    break;
  case 'F': // Right was moving back
    drive(HIGH, LOW, LOW, HIGH);
    break;
  case 'S': // Stop
  default:
    drive(LOW, LOW, LOW, LOW);
    break;
    }
  }
}

void drive(int a1, int a2, int b1, int b2) {
  digitalWrite(in1, a1);
  digitalWrite(in2, a2);
  analogWrite(enA, (a1 == a2) ? 0 : speed);

  digitalWrite(in3, b1);
  digitalWrite(in4, b2);
  analogWrite(enB, (b1 == b2) ? 0 : speed);
}
