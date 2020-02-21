void dump(uint32_t addr) {
  uint8_t buf1[256]="";
  delay(100);
  memset(buf1,0,sizeof(buf1));
  _spiffs_read(addr, 256, buf1);
  Serial.print("dump ");
  Serial.println(addr);
  for(uint16_t i=0;i<0x10;i++) {
    if(i%0x10==0) Serial.println();
    else
      Serial.write(' ');
    if(buf1[i]<0x10) Serial.write('0');
    Serial.print(buf1[i],HEX);
  }
  Serial.println();
}


void test(){
  uint8_t buf1[256]="1234567890abcdef";
  dump((uint32_t)4096);
  Serial.println("erase 4096");
  _spiffs_erase((uint32_t)4096,4096);
  dump(4096);
  Serial.println("write [4096]= 1234");
  _spiffs_write((uint32_t)4096, 10, buf1);
  dump(4096);

  buf1[0]='a';
  Serial.println("write [4096]= a234");
  _spiffs_write((uint32_t) 4096, 10, buf1);
  dump(4096);
while(1);
}

