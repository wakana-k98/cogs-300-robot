// ========== ULTRASONIC SENSOR ==========
const int trigPin = 4;  // Connected to Trig on ultrasonic sensor
const int echoPin = 5;  // Connected to Echo on ultrasonic sensor

long duration;
long cm;

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  Serial.begin(9600);  // Communication with Arduino 1
  
  delay(1000);  // Wait for Arduino 1 to be ready
}

void loop() {
  // Read ultrasonic sensor
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH);
  cm = (duration / 2) / 29.1;  // Convert to cm
  
  // Send distance to Arduino 1
  Serial.println(cm);
  
  delay(50);  // Read sensor every 50ms
}