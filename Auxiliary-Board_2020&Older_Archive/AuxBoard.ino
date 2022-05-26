#include <Servo.h>
int Break_OP = 16;
int Running_Lights_OP = 15;
int Right_Turn_OP = 14;
int Left_Turn_OP = 13;
int Wiper_Servo_IP = 11;
int Break_IP = 6;
int Hazard_IP = 5;
int Running_Lights_IP= 4;
int Left_Turn_IP = 28;
int Right_Turn_IP = 27;
int Wiper_Pot_IP;
int Brightness_IP;
int Wiper_Speed;

Servo Wiper;
int pos = 0;

void setup() {
  pinMode(Break_OP, OUTPUT);
  pinMode(Running_Lights_OP, OUTPUT);
  pinMode(Right_Turn_OP, OUTPUT);
  pinMode(Left_Turn_OP,OUTPUT);
  pinMode(Wiper_Servo_OP,OUTPUT);
  pinMode(Wiper_Servo_IP,INPUT);
  pinMode(Break_IP,INPUT);
  pinMode(Hazard_IP,INPUT);
  pinMode(Running_Lights_IP,INPUT);
  pinMode(Left_Turn_IP,INPUT);
  pinMode(Right_Turn_IP,INPUT);
  Wiper.attach(12);

}

void loop() {
  //Running Lights
  if(digitalRead(Running_Lights_IP) == HIGH)
  {
    digitalWrite(Running_Lights_OP, HIGH);
  }
  else
  {
    digitalWrite(Running_Lights_OP, LOW);
  }
  
  //Break
  if(digitalRead(Break_IP) == HIGH)
  {
    digitalWrite(Break_OP, HIGH);
  }
  else
  {
    digitalWrite(Break_OP,LOW);
  }

  //left turn
  if(digitalRead(Left_Turn_IP) == HIGH)
  {
    digitalWrite(Left_Turn_OP, HIGH);
    delay(500);
    digitalWrite(Left_Turn_OP, LOW);
    delay(500);
  }
  else
  {
    digitalWrite(Left_Turn_OP, LOW);
  }

  //right turn
  if(digitalRead(Right_Turn_IP) == HIGH)
  {
    digitalWrite(Right_Turn_OP), HIGH);
    delay(500);
    digitalWrite(Right_Turn_OP, LOW);
    delay(500);
  }
  else
  {
    digitalWrite(Right_Turn_OP, LOW);
  }

  //Hazard Lights
  if(digitalRead(Hazard_IP) == HIGH){
    digitalWrite(Left_Turn_OP, HIGH);
    digitalWrite(Right_Turn_OP, HIGH)
    delay(500);
    digitalWrite(Left_Turn_OP,LOW);
    digitalWrite(Right_Turn_OP,LOW);
    delay(500);
  }
  else
  {
    digitalWrite(Left_Turn_OP,LOW);
    digitalWrite(Right_Turn_OP, LOW);
  }

  //wiper
  if(digitalRead(Wiper_Servo_IP) == HIGH){
    Wiper_Speed = analogRead(Wiper_Pot_IP)
    for(pos = 0; pos <= 155; pos += 1)// servo from 0 to 155 degrees
    {
      Wiper.write(pos);
      if(digitalRead(Break_IP) == HIGH)
      {
        break;
      }
      delay(15);
    }
    for(pos = 155; pos >= 155; pos -= 1)// servo from 0 to 155 degrees
    {
      Wiper.write(pos);
      if(digitalRead(Break_IP) == HIGH)
      {
        break;
      }
      delay(15);
    }
    if(digitalRead(Break_IP) == HIGH)
      {
        break;
      }
    delay(1000-(2*Wiper_Speed));
  }
}
