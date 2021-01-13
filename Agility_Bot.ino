int lastError, error = 0;
int Rspeed, Lspeed, Speed, Inspeed = 30;
int DivBy = 0, Sum = 0, NumWhite = 0, ave;
double InstError = 0, kp = 0.01, kd = 0.04;  // speed 250 kp 0.0752 kd 0
int Lft0 = 9, Lft1 = 8 , Rght0 = 7 , Rght1 = 6, PWMA = 5, PWMB = 10;

void setup() {
  pinMode(PWMA, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(Lft0, OUTPUT);
  pinMode(Lft1, OUTPUT);
  pinMode(Rght0, OUTPUT);
  pinMode(Rght1, OUTPUT);

  digitalWrite(Rght0, LOW);
  digitalWrite(Lft0, HIGH);
  digitalWrite(Rght1, HIGH);
  digitalWrite(Lft1, LOW);

  Serial.begin(9600);
  delay(3000);
}

void loop() {
  Errordetect(6);
  Errcorrect();
  SpeedControl();
}

void Errordetect(int Sens_num) {
  int Sensor [Sens_num], Weight [] = {5000, 4000, 3000, 2000, 1000, 0};

  for (int i = 5 ; i >=0 ; i--) {
    Sensor[i] = analogRead(i);
    if (Sensor[i] >150) {                  //read the sensor that sees black
      Sum += Weight[i];                     //calculate the sum of weights for each sensor that sees black
      DivBy++;                              //count the number of sensor that sees black
    }
    if (Sensor[i] < 150) {                  //count the number of sensor that sees white
      NumWhite++;
    }   
  }
  if (NumWhite == Sens_num) {
    error = lastError;
    DivBy = 0;
    NumWhite = 0;
    Sum = 0;
  }
  else {
    ave = (Sum / DivBy);                    //calculate the average weight of the sensor that sees black
    error = 2500 - ave;
    DivBy = 0;
    NumWhite = 0;
    Sum = 0;   
  }
}

void Errcorrect() {
  InstError = (error - lastError);      // instantaneous error
  Speed = (kp * error) + (kd * InstError);  // PD Formula
}

void SpeedControl() {
  Rspeed = Inspeed + Speed;
  Lspeed = Inspeed - Speed;
  Rspeed = constrain( Rspeed, 0 , 255);
  Lspeed = constrain( Lspeed, 0 , 255);
  analogWrite(10, Lspeed);
  analogWrite(5, Rspeed);
  lastError = error;            // feedback for derivative correction
}
