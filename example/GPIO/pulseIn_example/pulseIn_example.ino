/*
 * pulseIn.pde: 用 pulseIn() 量測 button 被按了多久
 */

const int buttonPin = 2;                 // 按鈕(pushbutton)
unsigned long duration;                  // 持續時間

void setup() {
  pinMode(buttonPin, INPUT);             // 把 buttonPin 設置成 INPUT
  Serial.begin(9600);                     // 開啟 Serial port, 通訊速率為 9600 bps
  Serial.println("Press and release button now");
}
 

void loop() {
  // 量測 button 被按下多久 (button 被按下時，狀態為 HIGH)
  // 如果 button 在 1 秒內沒被按下，pulseIn() 會傳回 0
  
  duration = pulseIn(buttonPin, HIGH);  

  // 印出 duration 的內容
  if (duration != 0) {                 
    Serial.print("duration: ");
    Serial.print(duration);
    Serial.println(" microseconds");
  } else {
    Serial.println("timeout");
  }
}

