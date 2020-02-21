#include "src/spiffs_api.h"
#include "spi_test.h"

void setup(){
  Serial.begin(115200);
  spiffs_init(11); //cs=11
  sys_spiffs_mount_coreflash();
  char WriteBuf[]={"Hi,Budy! if you get this Message......Congratulations!You have succeeded!!"};
  char ReadBuf[80];
  //打开文件，如果文件不存在，自动创建
  spiffs_file fd = SPIFFS_open(&fs, "my_file", SPIFFS_CREAT | SPIFFS_TRUNC | SPIFFS_RDWR, 0);
  //    Serial.print(F("create file fd="));Serial.println(fd);

  //写文件
  //Serial.println(F("write file"));
  SPIFFS_write(&fs, fd, WriteBuf, sizeof(WriteBuf));
  //{ Serial.print(F("errno ")); Serial.println(SPIFFS_errno(&fs)); }
  SPIFFS_close(&fs, fd);
  //读文件
  //Serial.println(F("read file"));
  fd = SPIFFS_open(&fs, "my_file", SPIFFS_RDWR, 0);
  SPIFFS_read(&fs, fd, ReadBuf, sizeof(WriteBuf));
  //{ Serial.print(F("errno ")); Serial.println(SPIFFS_errno(&fs)); }
  SPIFFS_close(&fs, fd);

  //打印输出文件内容
  Serial.println(ReadBuf);
  while(1) ;
}
void loop() {
}
