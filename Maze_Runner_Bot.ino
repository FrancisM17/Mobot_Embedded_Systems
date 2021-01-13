int mode = 0;
# define STOPPED 0
# define Follow 1
# define No_Line 2
# define HorLine 3
# define Rturn 4
# define Lturn 5
char path[200] = {"LBL"};
unsigned char dir;
unsigned char pathLength = 0; // the length of the path
unsigned int Status = 0; // solving = 0; reach end = 1
int pathIndex = 0;
int Sens[6];

int lastError, error = 0, x;
int Rspeed, Lspeed, Speed, Inspeed = 30;
int DivBy = 0, Sum = 0, NumWhite = 0, ave;
double InstError = 0, kp = 0.01, kd = 0.04;// 35 0.018 0.45
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
  Serial.begin(115200);
  delay(5000);
}

void loop() {
  SolveMaze();
  //ReadSens(6);
  /*digitalWrite(Rght0, LOW);
    digitalWrite(Lft0, HIGH);
    digitalWrite(Rght1, HIGH);
    digitalWrite(Lft1, LOW);
    analogWrite(10, 30);
    analogWrite(5, 30);*/
}

void SolveMaze() {
  while (Status == 0) {
    ReadSens(6);
    switch (mode) {
      case No_Line:
        Stop();
        //delay(250);
        while (analogRead(4) < 150) {
          Turn (LOW, HIGH, LOW, HIGH);
        }
        PathRecord('B');
        break;

      case HorLine:
        CheckFarther();
        if (mode != HorLine) {            // if all sensors are black, an intersection
          if (digitalRead(11) == 0) {
            while (analogRead(4) < 100) {
              Turn (LOW, HIGH, LOW, HIGH);
            }
            PathRecord('L');
          }  // or it is a "T" or "Cross"). In both cases, goes to LEFT
          else {
            while (analogRead(0) < 150) {
              Turn (1, 0, 1, 0);   // Right Hand Rule
            }
            PathRecord('R');
            break;
          }
        }
        else mazeEnd();
        break;

      case Rturn:
        if (digitalRead(11) == 0) {
          CheckFarther();
          if (mode == No_Line) {
            while (analogRead(0) < 150) {
              Turn (1, 0, 1, 0);
            }
            PathRecord('R');
            break;
          }
          else {
            error = 0;
            LinePID();
            PathRecord('S');
            break;
          }
        }
        else {
          while (analogRead(0) < 150) {
            Turn (1, 0, 1, 0);
          }
          PathRecord('R');
          break;
        }

      case Lturn:
        if (digitalRead(11) == 1) {
          CheckFarther();
          if (mode == No_Line) {
            while (analogRead(4) < 150) {
              Turn (0, 1, 0, 1);
            }
            PathRecord('L');
            break;
          }
          else {
            error = 0;
            LinePID();
            PathRecord('S');
            break;
          }
        }
        else {
          while (analogRead(4) < 150) {
            Turn (0, 1, 0, 1);
          }
          PathRecord('L');
          break;
        }

      case Follow:
        LinePID();
        break;
    }
  }
}

void PathRecord(char Direction) {             // records the path the bot has taken
  path[pathLength] = Direction;
  pathLength++;
  simplifyPath();
}

void simplifyPath() {                         // eliminates dead end and maps successful turns
  int totalAngle = 0;
  int i, n, Decrmnt;
  // only simplify the path if  there is a 'B' in between odd pathlength i.e LBL (3) RLBRL (5).....
  if (pathLength >= 3 && path[pathLength - 2] == 'B') {  // 1 branch Deadend
    n = 3;
    Decrmnt = 2;
  }
  if (pathLength >= 3 && path[pathLength - 3] == 'B') {  // 2 branch Deadend
    n = 5;
    Decrmnt = 4;
  }
  if (pathLength >= 3 && path[pathLength - 4] == 'B') {  // 3 branch Deadend
    n = 7;
    Decrmnt = 6;
  }
  if (pathLength >= 3 && path[pathLength - 5] == 'B') {  // 4 branch Deadend
    n = 9;
    Decrmnt = 8;
  }
  if (pathLength >= 3 && path[pathLength - 6] == 'B') {  // 5 branch Deadend
    n = 11;
    Decrmnt = 10;
  }
  if (pathLength >= 3 && path[pathLength - 7] == 'B') {  // 6 branch Deadend
    n = 13;
    Decrmnt = 12;
  }
  if (pathLength >= 3 && path[pathLength - 8] == 'B') {  // 7 branch Deadend
    n = 15;
    Decrmnt = 14;
  }
  else
    return;

  for (i = 1; i <= n; i++) {                          //position after u turn relative to 0 degrees
    switch (path[pathLength - i]) {
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
  switch (totalAngle) {
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

  // The path is now x steps shorter.
  pathLength -= Decrmnt;

}

void ReadSens(int Sens_num) {
  int Sensor [Sens_num], Weight [] = {5000, 4000, 3000, 2000, 1000, 0};

  for (int i = 5 ; i >= 0 ; i--) {
    Sensor[i] = analogRead(i);
    if (Sensor[i] > 150) {                  //read the sensor that sees black
      Sum += Weight[i];                     //calculate the sum of weights for each sensor that sees black
      DivBy++;                              //count the number of sensor that sees black
    }
    if (Sensor[i] < 150) {                  //count the number of sensor that sees white
      NumWhite++;
    }
  }

  ave = (Sum / DivBy);                    //calculate the average weight of the sensor that sees black
  error = 2500 - ave;
  /* Serial.print(DivBy);
     Serial.print(" ");
     Serial.print(NumWhite);
     Serial.print(" ");*/
  if (DivBy == 6) {
    mode = HorLine;
    DivBy = 0;
    NumWhite = 0;
    Sum = 0;// all ir sees black (Horizontal Line)
  }
  else if (NumWhite == 6 && digitalRead(3) == 0 && digitalRead(4) == 0  ) {
    mode = No_Line;
    DivBy = 0;
    NumWhite = 0;
    Sum = 0;
  }
  else if (digitalRead(3) == 1 ) { //compare error and number of ir that sees black (DivBy) and use a condition for determining what turn to make
    mode = Lturn;
    DivBy = 0;
    NumWhite = 0;
    Sum = 0;
  }
  else if (digitalRead(4) == 1 ) {
    mode = Rturn;
    DivBy = 0;
    NumWhite = 0;
    Sum = 0;
  }

  else {
    mode = Follow;
    DivBy = 0;
    NumWhite = 0;
    Sum = 0;
  }
  /* Serial.print(digitalRead(3));
    Serial.print(" ");
    Serial.print(digitalRead(4));
    Serial.print(" ");
    Serial.print(Sensor[5]);
    Serial.print(" ");
    Serial.print(Sensor[4]);
    Serial.print(" ");
    Serial.print(Sensor[3]);
    Serial.print(" ");
    Serial.print(Sensor[2]);
    Serial.print(" ");
    Serial.print(Sensor[1]);
    Serial.print(" ");
    Serial.print(Sensor[0]);
    Serial.print(" ");
    Serial.print(mode);
    Serial.println(" ");*/
}

void Turn(int R0, int R1, int L0, int L1) {      // calibrate the combination of HIGH (1) and LOW (0) for motor direction (CW,CCW)
  digitalWrite(Rght0, R0);
  digitalWrite(Rght1, R1);
  digitalWrite(Lft0, L0);
  digitalWrite(Lft1, L1);
  analogWrite(10, 30);
  analogWrite(5, 30 );
}

void mazeEnd() {
  Stop();
  mode = STOPPED;
  Status = 1;
}

void CheckFarther() {                           // if ir sees Horizontal line check or turns check extra inch if there is line or white surface
  //error = 0;
  //LinePID();
  analogWrite(5, Inspeed);
  analogWrite(10, Inspeed);
  delay(200);
  Stop();
  ReadSens(6);                                  //returns mode i.e Lturn, No_Line....
}

void CheckFarther2() {
  error = 0;
  LinePID();
}

void Stop() {
  digitalWrite(Rght0, HIGH);
  digitalWrite(Lft0, HIGH);
  digitalWrite(Rght1, HIGH);
  digitalWrite(Lft1, HIGH);
  analogWrite(10, 0);
  analogWrite(5, 0);
}

void Forward() {
  digitalWrite(Rght0, LOW);
  digitalWrite(Lft0, HIGH);
  digitalWrite(Rght1, HIGH);
  digitalWrite(Lft1, LOW);
  Inspeed = 30  ;
}

void RunMaze () {                               //checks if the returned mode from ReadSens is equal to the solved path stored in path[] array
  while (Status == 1)
  {
    ReadSens(6);
    switch (mode)
    {
      case Follow:
        LinePID();
        break;
      case HorLine:
        if (pathIndex == pathLength) mazeEnd ();
        else {
          TurnMap (path[pathIndex]);
          pathIndex++;
        }
        break;
      case Lturn:
        Serial.print(mode);
        if (pathIndex == pathLength) mazeEnd ();
        else {
          TurnMap (path[pathIndex]);
          Serial.print(path[pathIndex]);
          pathIndex++;
        }
        break;
      case Rturn:
        if (pathIndex == pathLength) mazeEnd ();
        else {
          TurnMap (path[pathIndex]);
          pathIndex++;
        }
        break;
    }
  }
}

void TurnMap (char dir) {                             // turns the bot according to the solved path
  switch (dir)  {
    case 'L': // Turn Left
      /*while (analogRead(5) < 500) {
        Turn (LOW, HIGH, LOW, HIGH);
        }*/
      digitalWrite(Rght0, LOW);
      digitalWrite(Rght1, HIGH);
      digitalWrite(Lft0, LOW);
      digitalWrite(Lft1, HIGH);
      analogWrite(10, 60);
      analogWrite(5, 60);

      break;

    case 'R': // Turn Right
      while (analogRead(0) < 500) {
        Turn (HIGH, LOW, HIGH, LOW);
      }
      break;

    case 'B': // Turn Back
      while (analogRead(5) < 500) {
        Turn (LOW, HIGH, LOW, HIGH);
      }
      break;

    case 'S': // Go Straight
      CheckFarther2();
      break;
  }
}

void LinePID() {
  Forward();
  InstError = (error - lastError);      // instantaneous error
  Speed = (kp * error) + (kd * InstError);  // PD Formula

  Rspeed = Inspeed + Speed;
  Lspeed = Inspeed - Speed;
  Rspeed = constrain( Rspeed, 0 , 255);
  Lspeed = constrain( Lspeed, 0 , 255);
  analogWrite(10, Lspeed);
  analogWrite(5, Rspeed);
  lastError = error;            // feedback for derivative correction
}
