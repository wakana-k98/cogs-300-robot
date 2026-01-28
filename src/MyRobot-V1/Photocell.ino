pinMode(pwmPin, OUTPUT);
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
}

void loop() {

  // Read sensor
  sensorValue = analogRead(analogInPin);

  // Map to PWM range
  outputValue = map(sensorValue, 0, 1023, 0, 255);

  // Write PWM (LED brightness or motor speed)
  analogWrite(pwmPin, outputValue);

  // Print results
  Serial.print("Sensor = ");
  Serial.print(sensorValue);
  Serial.print("\t Output = ");
  Serial.println(outputValue);

  // State machine
  if (state == 0) {
    if (sensorValue > threshold) {
      driveForwards();
      state = 1;
    }
  }
  else if (state == 1) {
    if (sensorValue <= threshold) {
      stop();
      state = 0;
    }
  }

  delay(200);
}

// ---------------- FUNCTIONS ----------------

void driveForwards() {
  Serial.println("Driving forward");

  digitalWrite(motorPin1, HIGH);
  digitalWrite(motorPin2, LOW);

  analogWrite(pwmPin, 200); // motor speed (0–255)
}
void stop() {
  Serial.println("Stopping");

  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, LOW);

  analogWrite(pwmPin, 0);
}