void publishData(){
  unsigned long now = millis();
  if(now - lastMsg > 2000){
    lastMsg = now;
    char tempString[8];
    dtostrf(rVal, 1, 2, tempString);
    Serial.print("Rp : ");
    Serial.println(tempString);
    client.publish("esp/variableR", tempString);
  }
}
