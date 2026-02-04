// ========== MOTOR PINS ==========
int enA = 9;
int in1 = 8;
int in2 = 7;
int enB = 10;
int in3 = 12;
int in4 = 13;
int speed = 200;

// Motor trim (adjust these to fix veering)
float leftTrim = 0.6;
float rightTrim = 1.0;  // Adjust this value if robot still veers right

// ========== ENCODER PINS ==========
const int ENCODER_LEFT = 2;
const int ENCODER_RIGHT = 3;

// Encoder counting (volatile because used in interrupts)
volatile long leftCount = 0;
volatile long rightCount = 0;

// For speed calculation
unsigned long lastTime = 0;
long lastLeftCount = 0;
long lastRightCount = 0;

// Robot parameters (adjust for your robot)
const int ENCODER_SLOTS = 20;
const float WHEEL_DIAMETER = 6.5;  // cm
const float CM_PER_COUNT = (PI * WHEEL_DIAMETER) / ENCODER_SLOTS;

// ========== PHOTOCELL PIN ==========
const int PHOTOCELL_PIN = A0;
int photocellValue = 0;
const int PHOTOCELL_THRESHOLD = 5;  // Adjust based on your calibration
int state = -1;  // -1 = manual mode, 0 = waiting for start, 1 = leaving start tape, 2 = driving, 3 = done

// ========== INTERRUPT SERVICE ROUTINES ==========
void leftEncoder() {
  leftCount++;
}

void rightEncoder() {
  rightCount++;
}

void setup() {
  // Motor setup
  pinMode(enA, OUTPUT); 
  pinMode(in1, OUTPUT); 
  pinMode(in2, OUTPUT);
  pinMode(enB, OUTPUT); 
  pinMode(in3, OUTPUT); 
  pinMode(in4, OUTPUT);

  // Encoder setup with interrupts
  pinMode(ENCODER_LEFT, INPUT_PULLUP);
  pinMode(ENCODER_RIGHT, INPUT_PULLUP);
  
  // Attach interrupts - this will count encoder pulses automatically
  attachInterrupt(digitalPinToInterrupt(ENCODER_LEFT), leftEncoder, FALLING);
  attachInterrupt(digitalPinToInterrupt(ENCODER_RIGHT), rightEncoder, FALLING);

  Serial.begin(9600);
  
  // Print CSV header
  Serial.println("time_ms,left_count,right_count,left_dist_cm,right_dist_cm,left_speed,right_speed,photocell,state,command");
  
  lastTime = millis();
}

void loop() {
  // ========== READ PHOTOCELL ==========
  photocellValue = analogRead(PHOTOCELL_PIN);

  // ========== PRINT TELEMETRY (every 100ms) ==========
  unsigned long currentTime = millis();
  if (currentTime - lastTime >= 100) {
    float deltaTime = (currentTime - lastTime) / 1000.0;  // seconds

    // Calculate distances
    float leftDistance = leftCount * CM_PER_COUNT;
    float rightDistance = rightCount * CM_PER_COUNT;

    // Calculate speeds
    float leftSpeed = ((leftCount - lastLeftCount) * CM_PER_COUNT) / deltaTime;
    float rightSpeed = ((rightCount - lastRightCount) * CM_PER_COUNT) / deltaTime;

    Serial.print(currentTime);
    Serial.print(" , LeftCount:");
    Serial.print(leftCount);
    Serial.print(" , RightCountL:");
    Serial.print(rightCount);
    Serial.print(" , LeftDistance:");
    Serial.print(leftDistance, 2);
    Serial.print(" , RightDistance:");
    Serial.print(rightDistance, 2);
    Serial.print(" , LeftSpeed: ");
    Serial.print(leftSpeed, 2);
    Serial.print(" , RightSpeed: ");
    Serial.print(rightSpeed, 2);
    Serial.print(" , Photocell: ");
    Serial.print(photocellValue);
    Serial.print(" , PhotoState: ");
    Serial.print(state);
    Serial.print(",");

    // Update for next iteration
    lastLeftCount = leftCount;
    lastRightCount = rightCount;
    lastTime = currentTime;
  }

  // ========== HANDLE SERIAL COMMANDS ==========
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    Serial.println(cmd);  // Print command in telemetry

    // Direction Controls
    if (cmd == 'F')      drive(LOW, HIGH, LOW, HIGH);
    else if (cmd == 'B') drive(HIGH, LOW, HIGH, LOW);
    else if (cmd == 'L') drive(HIGH, LOW, LOW, HIGH);
    else if (cmd == 'R') drive(LOW, HIGH, HIGH, LOW);
    else if (cmd == 'S') drive(LOW, LOW, LOW, LOW);
    // Speed Controls (0-9)
    else if (cmd >= '0' && cmd <= '9') {
      int val = cmd - '0';
      speed = map(val, 0, 9, 100, 255);
    }
    // Auto mode toggle
    else if (cmd == 'A') {
      state = 0;  // Enable automatic photocell mode
      leftCount = 0;  // Reset counters
      rightCount = 0;
      Serial.println("AUTO");
    }
    else if (cmd == 'M') {
      state = -1;  // Disable automatic mode (manual control only)
      drive(LOW, LOW, LOW, LOW);  // Stop motors
      Serial.println("MANUAL");
    }
  } else {
    Serial.println("");  // Empty command column
  }

  // ========== PHOTOCELL AUTO MODE ==========
  // Only run if state >= 0 (disabled when state = -1)
  if (state == 0) {
    // Waiting for white tape to start
    if (photocellValue > PHOTOCELL_THRESHOLD) {
      drive(LOW, HIGH, LOW, HIGH);  // Drive forward
      state = 1;
    }
  }
  else if (state == 1) {
    // Wait until we LEAVE the starting white tape
    if (photocellValue <= PHOTOCELL_THRESHOLD) {
      state = 2;  // Now on dark floor, keep driving
    }
  }
  else if (state == 2) {
    // Driving on dark floor until we hit white tape again
    if (photocellValue > PHOTOCELL_THRESHOLD) {
      drive(LOW, LOW, LOW, LOW);  // Stop at finish line
      state = 3;  // Done
    }
  }

  delay(100);  // Small delay - changed from 500ms to prevent blocking encoder counts
}

// ========== DRIVE FUNCTION ==========
void drive(int a1, int a2, int b1, int b2) {
  digitalWrite(in1, a1);
  digitalWrite(in2, a2);
  analogWrite(enA, (a1 == a2) ? 0 : speed * leftTrim); 

  digitalWrite(in3, b2);
  digitalWrite(in4, b1);
  analogWrite(enB, (b1 == b2) ? 0 : speed * rightTrim);
}
