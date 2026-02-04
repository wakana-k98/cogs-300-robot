// ========== MOTOR PINS ==========
int enA = 9;
int in1 = 8;
int in2 = 7;
int enB = 10;
int in3 = 12;
int in4 = 13;
int baseSpeed = 150;  // Base driving speed

// Motor trim
float leftTrim = 0.6;
float rightTrim = 1.0;

// ========== ENCODER PINS ==========
const int ENCODER_LEFT = 2;
const int ENCODER_RIGHT = 3;

volatile long leftCount = 0;
volatile long rightCount = 0;

unsigned long lastTime = 0;
long lastLeftCount = 0;
long lastRightCount = 0;

const int ENCODER_SLOTS = 20;
const float WHEEL_DIAMETER = 6.5;
const float CM_PER_COUNT = (PI * WHEEL_DIAMETER) / ENCODER_SLOTS;

// ========== ULTRASONIC SENSORS ==========
const int trigPin1 = 4;   // Front sensor
const int echoPin1 = 5;
const int trigPin2 = 6;   // Side sensor
const int echoPin2 = 11;

long cm1, cm2;  // Distances

// ========== CONTROL MODES ==========
int mode = 0;  // 0 = manual, 1 = follow me, 2 = wall follow

// P-Controller settings
float targetDistance = 25.0;  // Target distance in cm
float Kp = 3.0;  // Proportional gain (tune this!)

// Wall following settings
float wallDistance = 20.0;  // Desired distance from wall
float Kp_wall = 2.0;  // Wall following gain

// ========== INTERRUPT SERVICE ROUTINES ==========
void leftEncoder() {
  leftCount++;
}

void rightEncoder() {
  rightCount++;
}

// ========== ULTRASONIC READING FUNCTION ==========
long readUltrasonic(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long duration = pulseIn(echoPin, HIGH, 30000);  // 30ms timeout
  if (duration == 0) return 999;  // No echo = very far
  
  return (duration / 2) / 29.1;
}

void setup() {
  // Motor setup
  pinMode(enA, OUTPUT); 
  pinMode(in1, OUTPUT); 
  pinMode(in2, OUTPUT);
  pinMode(enB, OUTPUT); 
  pinMode(in3, OUTPUT); 
  pinMode(in4, OUTPUT);

  // Encoder setup
  pinMode(ENCODER_LEFT, INPUT_PULLUP);
  pinMode(ENCODER_RIGHT, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_LEFT), leftEncoder, FALLING);
  attachInterrupt(digitalPinToInterrupt(ENCODER_RIGHT), rightEncoder, FALLING);

  // Ultrasonic setup
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);

  Serial.begin(9600);
  
  Serial.println("========================================");
  Serial.println("🤖 WALL-FOLLOWING ROBOT");
  Serial.println("========================================");
  Serial.println("Commands:");
  Serial.println("  M = Manual mode");
  Serial.println("  1 = Follow Me mode (P-controller)");
  Serial.println("  2 = Wall Following mode");
  Serial.println("  F/B/L/R/S = Manual driving");
  Serial.println("  0-9 = Set speed");
  Serial.println("========================================");
  Serial.println();
  
  lastTime = millis();
}

void loop() {
  // ========== READ SENSORS ==========
  cm1 = readUltrasonic(trigPin1, echoPin1);  // Front/Follow-me sensor
  delayMicroseconds(10);
  cm2 = readUltrasonic(trigPin2, echoPin2);  // Side sensor for wall following

  // ========== TELEMETRY (every 200ms) ==========
  unsigned long currentTime = millis();
  if (currentTime - lastTime >= 200) {
    float deltaTime = (currentTime - lastTime) / 1000.0;
    float leftDistance = leftCount * CM_PER_COUNT;
    float rightDistance = rightCount * CM_PER_COUNT;
    float leftSpeed = ((leftCount - lastLeftCount) * CM_PER_COUNT) / deltaTime;
    float rightSpeed = ((rightCount - lastRightCount) * CM_PER_COUNT) / deltaTime;

    Serial.println("----------------------------------------");
    Serial.print("Mode: ");
    if (mode == 0) Serial.println("MANUAL 🎮");
    else if (mode == 1) Serial.println("FOLLOW ME 👋");
    else if (mode == 2) Serial.println("WALL FOLLOW 🧱");
    
    Serial.print("Sensor 1 (Front): ");
    Serial.print(cm1);
    Serial.print(" cm | Sensor 2 (Side): ");
    Serial.print(cm2);
    Serial.println(" cm");
    
    Serial.print("Encoders - L: ");
    Serial.print(leftCount);
    Serial.print(" R: ");
    Serial.println(rightCount);
    Serial.println();

    lastLeftCount = leftCount;
    lastRightCount = rightCount;
    lastTime = currentTime;
  }

  // ========== MODE BEHAVIORS ==========
  if (mode == 1) {
    // FOLLOW ME MODE (P-Controller)
    followMe();
  }
  else if (mode == 2) {
    // WALL FOLLOWING MODE
    followWall();
  }

  // ========== HANDLE COMMANDS ==========
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    
    if (cmd == 'M' || cmd == 'm') {
      mode = 0;
      drive(LOW, LOW, LOW, LOW);  // Stop
      Serial.println("✅ Manual mode");
    }
    else if (cmd == '1') {
      mode = 1;
      Serial.println("✅ Follow Me mode activated!");
    }
    else if (cmd == '2') {
      mode = 2;
      Serial.println("✅ Wall Following mode activated!");
    }
    else if (mode == 0) {  // Only accept manual commands in manual mode
      if (cmd == 'F') {
        Serial.println("⬆️ Forward");
        drive(LOW, HIGH, LOW, HIGH);
      }
      else if (cmd == 'B') {
        Serial.println("⬇️ Backward");
        drive(HIGH, LOW, HIGH, LOW);
      }
      else if (cmd == 'L') {
        Serial.println("⬅️ Left");
        drive(HIGH, LOW, LOW, HIGH);
      }
      else if (cmd == 'R') {
        Serial.println("➡️ Right");
        drive(LOW, HIGH, HIGH, LOW);
      }
      else if (cmd == 'S') {
        Serial.println("🛑 Stop");
        drive(LOW, LOW, LOW, LOW);
      }
      else if (cmd >= '0' && cmd <= '9') {
        int val = cmd - '0';
        baseSpeed = map(val, 0, 9, 100, 255);
        Serial.print("Speed: ");
        Serial.println(val);
      }
    }
  }

  delay(10);
}

// ========== FOLLOW ME (P-CONTROLLER) ==========
void followMe() {
  // Use front sensor (cm1) to maintain target distance
  float error = cm1 - targetDistance;
  
  // P-controller: output proportional to error
  int speedAdjust = (int)(Kp * error);
  
  // Limit speed
  speedAdjust = constrain(speedAdjust, -255, 255);
  
  if (abs(error) < 3) {
    // Close enough - stop
    drive(LOW, LOW, LOW, LOW);
  }
  else if (error > 0) {
    // Too far - move forward
    driveAtSpeed(speedAdjust, speedAdjust);
  }
  else {
    // Too close - move backward
    driveAtSpeed(speedAdjust, speedAdjust);
  }
}

// ========== WALL FOLLOWING ==========
void followWall() {
  // Use side sensor (cm2) to maintain distance from wall
  float error = cm2 - wallDistance;
  
  // Calculate steering adjustment
  int steerAdjust = (int)(Kp_wall * error);
  steerAdjust = constrain(steerAdjust, -100, 100);
  
  // Drive forward with steering correction
  int leftSpeed = baseSpeed - steerAdjust;   // If too close, slow left wheel (turn away)
  int rightSpeed = baseSpeed + steerAdjust;  // Speed up right wheel
  
  leftSpeed = constrain(leftSpeed, 0, 255);
  rightSpeed = constrain(rightSpeed, 0, 255);
  
  driveAtSpeed(leftSpeed, rightSpeed);
  
  // CORNER DETECTION (simple version)
  // If front sensor sees obstacle close, turn away
  if (cm1 < 15) {
    Serial.println("🚧 Obstacle ahead! Turning...");
    // Turn right
    drive(LOW, HIGH, HIGH, LOW);
    delay(500);
  }
  // If side sensor loses wall, turn toward it
  else if (cm2 > 50) {
    Serial.println("📍 Lost wall! Turning back...");
    // Turn left slightly
    drive(LOW, HIGH, HIGH, LOW);
    delay(300);
  }
}

// ========== DRIVE AT SPECIFIC SPEED ==========
void driveAtSpeed(int leftSpeed, int rightSpeed) {
  if (leftSpeed >= 0 && rightSpeed >= 0) {
    // Both forward
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    analogWrite(enA, leftSpeed * leftTrim);
    
    digitalWrite(in3, LOW);
    digitalWrite(in4, HIGH);
    analogWrite(enB, rightSpeed * rightTrim);
  }
  else if (leftSpeed < 0 && rightSpeed < 0) {
    // Both backward
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    analogWrite(enA, abs(leftSpeed) * leftTrim);
    
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
    analogWrite(enB, abs(rightSpeed) * rightTrim);
  }
  else {
    // Mixed - one forward, one back (turning in place)
    if (leftSpeed >= 0) {
      digitalWrite(in1, LOW);
      digitalWrite(in2, HIGH);
      analogWrite(enA, leftSpeed * leftTrim);
    } else {
      digitalWrite(in1, HIGH);
      digitalWrite(in2, LOW);
      analogWrite(enA, abs(leftSpeed) * leftTrim);
    }
    
    if (rightSpeed >= 0) {
      digitalWrite(in3, LOW);
      digitalWrite(in4, HIGH);
      analogWrite(enB, rightSpeed * rightTrim);
    } else {
      digitalWrite(in3, HIGH);
      digitalWrite(in4, LOW);
      analogWrite(enB, abs(rightSpeed) * rightTrim);
    }
  }
}

// ========== BASIC DRIVE FUNCTION ==========
void drive(int a1, int a2, int b1, int b2) {
  digitalWrite(in1, a1);
  digitalWrite(in2, a2);
  analogWrite(enA, (a1 == a2) ? 0 : baseSpeed * leftTrim); 

  digitalWrite(in3, b2);
  digitalWrite(in4, b1);
  analogWrite(enB, (b1 == b2) ? 0 : baseSpeed * rightTrim);
}