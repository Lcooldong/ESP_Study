void callback(char* topic, byte* message, unsigned int length){
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++){
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];  
  }
  Serial.println();
  if (String(topic) == "esp32Poly/inputData"){
    Serial.print("Changing output to");
    if(messageTemp == "1"){
      Serial.println("on");
    }
    else if(messageTemp == "0"){
      Serial.println("off");  
    }
  }
}
