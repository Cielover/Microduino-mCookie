int m_time;
#define BuzzerPin6 6

void setup() {
  // put your setup code here, to run once:
  m_time=50;
  pinMode(8, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(digitalRead(8)==1){
  tone(BuzzerPin6,2000);
  delay(m_time);
  noTone(BuzzerPin6);
  delay(m_time);
  }
}
