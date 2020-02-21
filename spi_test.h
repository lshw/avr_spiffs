void dump(uint32_t addr) {
uint8_t buf1[256]="1234567890abcdef";
delay(100);
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
  dump(0);
  dump(256);
  dump((uint32_t) 4096*512);
  Serial.println("write [0]= 1234");
  _spiffs_write(0, 10, buf1);
  buf1[0]='a';
  Serial.println("write [4096*512]= a234");
  _spiffs_write((uint32_t) 4096*512, 10, buf1);
  Serial.println("write [4096*511]= a234");
  _spiffs_write((uint32_t) 4096*511, 10, buf1);
  dump(0);
  dump(256);
  dump((uint32_t) 4096*511);
  dump((uint32_t) 4096*512);
  _erase((uint32_t)4096*511);
  dump(0);
  dump(256);
  dump((uint32_t) 4096*512);
while(1);
}

