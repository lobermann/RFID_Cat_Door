#include <SoftwareSerial.h>
#include <Servo.h> 
#include <EEPROM.h>

SoftwareSerial RFID(2,3); // RX,TX
Servo myservo;
const int motion_sense_pin = 5;
const int tag1_prog_pin = 3;
const int tag2_prog_pin = 4;
 
int data1 = 0;
int ok = -1;
int rfid_led_yes = 13;
int rfid_led_no = 12;
 
// use first sketch in http://wp.me/p3LK05-3Gk to get your tag numbers
//char tag1[14] = {2, 48, 52, 48, 48, 52, 51, 69, 50, 70, 49, 53, 52, 3};
//char tag2[14] = {2, 50, 48, 48, 48, 50, 51, 51, 69, 70, 56, 67, 53, 3};
char tag1[14];
char tag2[14];
char newtag[14] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0}; // used for read comparisons
int pos = 0;
 
void setup()
{
  RFID.begin(9600);    // start serial to RFID reader
  pinMode(tag1_prog_pin, INPUT);
  pinMode(tag2_prog_pin, INPUT);
  
  Serial.begin(9600);  // start serial to PC 
  pinMode(rfid_led_yes, OUTPUT); // for status LEDs
  pinMode(rfid_led_no, OUTPUT);
  myservo.attach(9);
  pinMode(motion_sense_pin, INPUT);
  
  //Read EEPROM
  for(int i = 0; i < 14; i++)
  {
    tag1[i] = EEPROM.read(i);
    tag2[i] = EEPROM.read(i+14);
  }
}
 
boolean comparetag(char aa[14], char bb[14])
{
  boolean ff = false;
  int fg = 0;
  for (int cc = 0 ; cc < 14 ; cc++)
  {
    if (aa[cc] == bb[cc])
    {
      fg++;
    }
  }
  if (fg == 14)
  {
    ff = true;
  }
  return ff;
}
 
void checkmytags() // compares each tag against the tag just read
{
  ok = 0; // this variable helps decision-making,
  // if it is 1 we have a match, zero is a read but no match,
  // -1 is no read attempt made
  if (comparetag(newtag, tag1) == true)
  {
    ok++;
  }
  if (comparetag(newtag, tag2) == true)
  {
    ok++;
  }
}
 
void readTags()
{
  ok = -1;
  
  if (RFID.available() > 0) 
  {
    Serial.println("Reading Tag");
    // read tag numbers
    delay(100); // needed to allow time for the data to come in from the serial buffer.
 
    for (int z = 0 ; z < 14 ; z++) // read the rest of the tag
    {
      data1 = RFID.read();
      newtag[z] = data1;
    }
    RFID.flush(); // stops multiple reads
    
    Serial.println("Got Tag: ");
    Serial.println(newtag);
 
    // do the tags match up?
    checkmytags();
  }
  
  if(digitalRead(tag1_prog_pin) == HIGH)
  {
    for(int i = 0 ; i < 14 ; i++)
    {
      tag1[i] = newtag[i];
      EEPROM.write(i,newtag[i]);
    }
    digitalWrite(rfid_led_yes, HIGH);
    digitalWrite(rfid_led_no, HIGH);
    delay(10*1000);
    digitalWrite(rfid_led_yes, LOW);
    digitalWrite(rfid_led_no, LOW);
  }
  else if(digitalRead(tag2_prog_pin) == HIGH)
  {
    for(int i = 0 ; i < 14 ; i++)
    {
      tag2[i] = newtag[i];
      EEPROM.write(i+14,newtag[i]);
    }
    digitalWrite(rfid_led_yes, HIGH);
    digitalWrite(rfid_led_no, HIGH);
    delay(10*1000);
    digitalWrite(rfid_led_yes, LOW);
    digitalWrite(rfid_led_no, LOW);
  }
  
  // now do something based on tag type
  if (ok > 0) // if we had a match
  {
    Serial.println("Accepted");
    digitalWrite(rfid_led_yes, HIGH);
    myservo.write(50);
    delay(10*1000);
    myservo.write(93);
    digitalWrite(rfid_led_yes, LOW);
 
    ok = -1;
    RFID.flush();
  }
  else if (ok == 0) // if we didn't have a match
  {
    Serial.println("Rejected");
    digitalWrite(rfid_led_no, HIGH);
    delay(10*1000);
    digitalWrite(rfid_led_no, LOW);
 
    ok = -1;
    RFID.flush();
  }
}

void readMotionSensor()
{
  if(digitalRead(motion_sense_pin) == 1)
  {
    Serial.println("Accepted by IR");
    digitalWrite(rfid_led_yes, HIGH);
    myservo.write(50);
    delay(3000);
    myservo.write(93);
    digitalWrite(rfid_led_yes, LOW);
  }
}
 
void loop()
{
  readTags();
  readMotionSensor();
}
