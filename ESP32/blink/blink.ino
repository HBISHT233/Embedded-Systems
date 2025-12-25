// the setup function runs once when you press reset or power the board
#define LED_BUILTIN 2
#define pin3  3
#define pin4 4
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  Serial.begin(115200);  // Start serial communication at 9600 baud
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(pin3, OUTPUT);
  pinMode(pin4, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  digitalWrite(pin3, HIGH);
  digitalWrite(pin4, HIGH);
  delay(1000);                      // wait for a second
  //Serial.print("LED ON \n\r");
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
  digitalWrite(pin3, LOW);
  digitalWrite(pin4, LOW);
  delay(1000);                      // wait for a second
  //Serial.print("LED OFF \n\r");
}
