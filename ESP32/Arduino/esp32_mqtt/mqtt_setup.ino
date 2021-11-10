void setup_wifi(){
  delay(10);
  Serial.println();
  Serial.print("Conneting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");  
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect(){
 while(!client.connected()){
  Serial.print("Attempting MQTT connection...");
  if(client.connect("ESP32Client")){
    Serial.println("reconnected");
    client.publish("esp/variableR", "reconnected!!");
    client.subscribe("esp32Poly/inputData");  
  } else{
    Serial.print("failed, rc=");
    Serial.print(client.state());
    Serial.println(" try again in 5 seconds");
    delay(5000);  
  }
 }  
}
