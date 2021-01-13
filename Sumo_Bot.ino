string mode;

void setup() {
  // put your setup code here, to run once:

}

void loop() {


}

void RecordTurn() {
  path[pathLength] = Direction;
  pathLength++;
  simplifyPath();
}

void simplifyPath()
{
  int totalAngle = 0;
  int i, n, Decrmnt;
  // only simplify the path if the second-to-last turn was a 'B'
  if (pathLength < 5) {
    if (path[pathLength - 2] != 'B') {
      n = 3;
      Decrmnt = 2;
    }
    if (pathLength > 3 && path[pathLength - 3] != 'B') {
      n = 5;
      Decrmnt = 4;
    }
    else
      return
    }

  for (i = 1; i <= n; i++)
  {
    switch (path[pathLength - i])
    {
      case 'R':
        totalAngle += 90;
        break;
      case 'L':
        totalAngle += 270;
        break;
      case 'B':
        totalAngle += 180;
        break;
    }
  }

  // Get the angle as a number between 0 and 360 degrees.
  totalAngle = totalAngle % 360;

  // Replace all of those turns with a single one.
  switch (totalAngle)
  {
    case 0:
      path[pathLength - n] = 'S';
      break;
    case 90:
      path[pathLength - n] = 'R';
      break;
    case 180:
      path[pathLength - n] = 'B';
      break;
    case 270:
      path[pathLength - n] = 'L';
      break;
  }

  // The path is now two steps shorter.
  pathLength -= Decrmnt;

}

void ReadSens(int Sens_num) {
  int Sensor [Sens_num], Weight [] = {0, 1000, 2000, 3000, 4000, 5000};

  for (int i = 0 ; i < Sens_num ; i++) {
    Sensor[i] = analogRead(i);
    if (Sensor[i] > 500) {                  //read the sensor that sees black
      Sum += Weight[i];                     //calculate the sum of weights for each sensor that sees black
      DivBy++;                              //count the number of sensor that sees black
    }
    if (Sensor[i] < 500) {                  //count the number of sensor that sees white
      NumWhite++;
    }
  }
  if (NumWhite == Sens_num) {
    mode = Back;
    DivBy = 0;
    NumWhite = 0;
    Sum = 0;
  }

  else if (error == -1000 && DivBy == 4) {
    mode  = Turn_Left;
  }
  else if (error == 1000 && DivBy == 4) {
    mode  = Turn_;
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
  Rspeed = Inspeed - Speed;
  Lspeed = Inspeed + Speed;
  Rspeed = constrain( Rspeed, 0 , 255);
  Lspeed = constrain( Lspeed, 0 , 255);
  analogWrite(10, Lspeed);
  analogWrite(11, Rspeed);
  lastError = error;            // feedback for derivative correction
}
