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
float rightTrim = 1.0;

// ========== ENCODER PINS ==========
const int ENCODER_LEFT = 2;
const int ENCODER_RIGHT = 3;

// Encoder counting
volatile long leftCount = 0;
volatile long rightCount = 0;

// For speed calculation
unsigned long lastTime = 0;
long lastLeftCount = 0;
long lastRightCount = 0;

// Robot parameters
const int ENCODER_SLOTS = 20;
const float WHEEL_DIAMETER = 6.5;  // cm
const float CM_PER_COUNT = (PI * WHEEL_DIAMETER) / ENCODER_SLOTS;

// ========== ULTRASONIC SENSOR 1 (Front/Side) ==========
const int trigPin1 = 4;
const int echoPin1 = 5;
long duration1, cm1;

// ========== ULTRASONIC SENSOR 2 (Front/Side) ==========
const int trigPin2 = 6;
const int echoPin2 = 11;
long duration2, cm2;

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
  attachInterrupt(digitalPinToInterrupt(ENCODER_LEFT), leftEncoder, FALLING);
  attachInterrupt(digitalPinToInterrupt(ENCODER_RIGHT), rightEncoder, FALLING);

  // Ultrasonic 1 setup
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);

  // Ultrasonic 2 setup
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);

  Serial.begin(9600);
  
  Serial.println("========================================");
  Serial.println("🤖 ROBOT TELEMETRY SYSTEM");
  Serial.println("========================================");
  Serial.println();
  
  lastTime = millis();
}

void loop() {
  // ========== READ ULTRASONIC SENSOR 1 ==========
  digitalWrite(trigPin1, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin1, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin1, LOW);
  
  duration1 = pulseIn(echoPin1, HIGH);
  cm1 = (duration1 / 2) / 29.1;

  // Small delay between sensors
  delayMicroseconds(10);

  // ========== READ ULTRASONIC SENSOR 2 ==========
  digitalWrite(trigPin2, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin2, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin2, LOW);
  
  duration2 = pulseIn(echoPin2, HIGH);
  cm2 = (duration2 / 2) / 29.1;

  // ========== PRINT TELEMETRY (every 100ms) ==========
  unsigned long currentTime = millis();
  if (currentTime - lastTime >= 100) {
    float deltaTime = (currentTime - lastTime) / 1000.0;

    // Calculate distances
    float leftDistance = leftCount * CM_PER_COUNT;
    float rightDistance = rightCount * CM_PER_COUNT;

    // Calculate speeds
    float leftSpeed = ((leftCount - lastLeftCount) * CM_PER_COUNT) / deltaTime;
    float rightSpeed = ((rightCount - lastRightCount) * CM_PER_COUNT) / deltaTime;

    // Print telemetry with labels
    Serial.println("----------------------------------------");
    Serial.print("⏱️  Time: ");
    Serial.print(currentTime);
    Serial.println(" ms");
    
    Serial.println();
    Serial.println("📊 ENCODERS:");
    Serial.print("   Left Count:  ");
    Serial.print(leftCount);
    Serial.print(" ticks  |  Right Count: ");
    Serial.print(rightCount);
    Serial.println(" ticks");
    
    Serial.println();
    Serial.println("📏 DISTANCE:");
    Serial.print("   Left:  ");
    Serial.print(leftDistance, 2);
    Serial.print(" cm  |  Right: ");
    Serial.print(rightDistance, 2);
    Serial.println(" cm");
    
    Serial.println();
    Serial.println("🏃 SPEED:");
    Serial.print("   Left:  ");
    Serial.print(leftSpeed, 2);
    Serial.print(" cm/s  |  Right: ");
    Serial.print(rightSpeed, 2);
    Serial.println(" cm/s");
    
    Serial.println();
    Serial.println("📡 ULTRASONIC SENSORS:");
    Serial.print("   Sensor 1 (Pins 4/5):  ");
    Serial.print(cm1);
    Serial.print(" cm  |  Sensor 2 (Pins 6/11): ");
    Serial.print(cm2);
    Serial.println(" cm");
    
    Serial.println("----------------------------------------");
    Serial.println();

    // Update for next iteration
    lastLeftCount = leftCount;
    lastRightCount = rightCount;
    lastTime = currentTime;
  }

  // ========== HANDLE SERIAL COMMANDS ==========
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    
    // Print command received
    Serial.print("🎮 Command: ");
    
    if (cmd == 'F') {
      Serial.println("FORWARD ⬆️");
      drive(LOW, HIGH, LOW, HIGH);
    }
    else if (cmd == 'B') {
      Serial.println("BACKWARD ⬇️");
      drive(HIGH, LOW, HIGH, LOW);
    }
    else if (cmd == 'L') {
      Serial.println("LEFT ⬅️");
      drive(HIGH, LOW, LOW, HIGH);
    }
    else if (cmd == 'R') {
      Serial.println("RIGHT ➡️");
      drive(LOW, HIGH, HIGH, LOW);
    }
    else if (cmd == 'S') {
      Serial.println("STOP 🛑");
      drive(LOW, LOW, LOW, LOW);
    }
    else if (cmd >= '0' && cmd <= '9') {
      int val = cmd - '0';
      speed = map(val, 0, 9, 100, 255);
      Serial.print("Speed set to: ");
      Serial.print(val);
      Serial.println(" ⚡");
    }
    else {
      Serial.print("Unknown: ");
      Serial.println(cmd);
    }
    Serial.println();
  }

  delay(10);
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