// Motor A (Left)
int enA = 9;
int in1 = 8;
int in2 = 7;

// Motor B (Right)
int enB = 10;
int in3 = 12;
int in4 = 13;

int speed = 200; // Default startup speed

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

    // Direction Controls
    if (cmd == 'F')      drive(LOW, HIGH, LOW, HIGH);
    else if (cmd == 'B') drive(HIGH, LOW, HIGH, LOW);
    else if (cmd == 'R') drive(HIGH, LOW, LOW, HIGH);
    else if (cmd == 'L') drive(LOW, HIGH, HIGH, LOW);
    else if (cmd == 'S') drive(LOW, LOW, LOW, LOW);
    // Speed Controls (0 - 9)
    else if (cmd >= '0' && cmd <= '9') {
      // Convert char '0'-'9' to integer 0-9
      int val = cmd - '0';
      
      // Map 0-9 to PWM range 0-255
      // We start mapping at 0, but practically motors stall < 60
      speed = map(val, 0, 9, 0, 255); 
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
