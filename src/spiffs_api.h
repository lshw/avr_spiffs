#include "spiffs/spiffs.h"
#include <SPI.h>
#define SPICONFIG   SPISettings(50000000, MSBFIRST, SPI_MODE0)

/*文件系统结构体*/
static spiffs fs;
uint16_t cs1;

/*页定义*/
#define LOG_PAGE_SIZE      256


uint8_t manufacture_id=0,memory_type,capacity;
uint32_t chipsize;
uint32_t _read_jedec_chipsize() {
  if(manufacture_id==0) {
    SPI.beginTransaction(SPICONFIG);
    digitalWrite(cs1, LOW);
    SPI.transfer(0x9F);
    manufacture_id=SPI.transfer(0);
    memory_type=SPI.transfer(0);
    capacity=SPI.transfer(0);
    digitalWrite(cs1, HIGH);
    SPI.endTransaction();
  }
  chipsize = 1024*1024; // unknown chips, default to 1 MByte

  if (capacity >= 16 && capacity <= 31) 
    chipsize = 1ul << capacity;
  else
    if (capacity >= 32 && capacity <= 37) 
      chipsize = 1ul << (capacity - 6);
    else
      if (capacity==0 || capacity==0xff)
	chipsize = 0;
  return chipsize;
}

void _write_enable(){
  SPI.beginTransaction(SPICONFIG);
  digitalWrite(cs1, LOW);
  SPI.transfer(0x06);
  digitalWrite(cs1, HIGH);
  delayMicroseconds(1);
  SPI.endTransaction();
}
void _write_disable(){
  SPI.beginTransaction(SPICONFIG);
  digitalWrite(cs1, LOW);
  SPI.transfer(0x04);
  digitalWrite(cs1, HIGH);
  delayMicroseconds(1);
  SPI.endTransaction();
}

void _erase_all() {
  _write_enable();
  SPI.beginTransaction(SPICONFIG);
  digitalWrite(cs1, LOW);
  SPI.transfer(0xC7);
  digitalWrite(cs1, HIGH);
  SPI.endTransaction();
  Serial.println(F("erase all:"));
  for(int8_t i=0;i<25;i++) {
    Serial.write('.');
    delay(1000); //doc:25s
  }
  Serial.println();
  _write_disable();
}

void _erase(uint32_t addr){
  addr&=0xffffff;
  SPI.beginTransaction(SPICONFIG);
  digitalWrite(cs1, LOW);
  SPI.transfer(0x20);
  SPI.transfer((addr>>16)&0xff);
  SPI.transfer((addr>>8)&0xff);
  SPI.transfer(addr&0xff);
  delay(100);
  digitalWrite(cs1, HIGH);
  SPI.endTransaction();
  delay(60); //doc:50ms
}
s32_t _spiffs_read(u32_t addr, u32_t size, u8_t *dst) {
  digitalWrite(cs1, LOW);
  SPI.transfer(0x03);
  SPI.transfer((addr>>16)&0xff);
  SPI.transfer((addr>>8)&0xff);
  SPI.transfer(addr&0xff);
  SPI.transfer(dst, size);
  digitalWrite(cs1, HIGH);
  return 0;
}
void _spiffs_write_page(u32_t addr ,u16_t size, const void *src) {
  const uint8_t *p = (const uint8_t *) src;
  SPI.beginTransaction(SPICONFIG);
  digitalWrite(cs1, LOW);
  SPI.transfer(0x02);
  SPI.transfer((addr>>16)&0xff);
  SPI.transfer((addr>>8)&0xff);
  SPI.transfer(addr&0xff);
  do{
    SPI.transfer(*p++);

  }while(--size>0);

  digitalWrite(cs1, HIGH);
  SPI.endTransaction();
  delay(1); //doc:0.6ms
}
static s32_t _spiffs_write(u32_t addr, u32_t size, u8_t *src) {
  _write_enable();
  while(size>0) {
    if(size>256){
      _spiffs_write_page(addr,256,src);
      addr+=256;
      size-=256;
    }else{
      _spiffs_write_page(addr,size,src);
      break;
    }
  }
  _write_disable();
  return 0;
}

static s32_t _spiffs_erase(uint32_t addr, int32_t size) {
  uint8_t ch;
  _write_enable();
  while(size>0) {
    for(uint16_t i=0;i<4096;i++) {
      _spiffs_read(addr+i,1,&ch);
      if(ch!=0xff) break;
    }
    if(ch!=0xff) {
      _erase(addr);
    }
    addr+=4096;
    size-=4096;
  }
  _write_disable();
  return 0;
}

//初始化以及配置
spiffs_config cfg;
static u8_t spiffs_work_buf[LOG_PAGE_SIZE*2];
static u8_t spiffs_fds[32*4];
static u8_t spiffs_cache_buf[(LOG_PAGE_SIZE+32)*2];

void spiffs_init(uint8_t cs) {
  uint8_t buf[256];
  cs1=cs;
  pinMode(cs1,OUTPUT);
  digitalWrite(cs1,HIGH);
  SPI.begin();
  _read_jedec_chipsize();
  Serial.print(F("id="));
  Serial.print(manufacture_id,HEX);
  Serial.write(' ');
  Serial.print(memory_type,HEX);
  Serial.write(' ');
  Serial.println(capacity,HEX);
  Serial.print(F("chipsize="));
  Serial.println(chipsize);
  cfg.hal_erase_f = &_spiffs_erase;
  cfg.hal_read_f = &_spiffs_read;
  cfg.hal_write_f = &_spiffs_write;
  cfg.phys_size = chipsize;//chipsize/32;
  cfg.phys_addr = 0;//chipsize-chipsize/16;
  cfg.phys_erase_block = 4096;
  cfg.log_block_size =4096;
  cfg.log_page_size = 256;
#if SPIFFS_FILEHDL_OFFSET
  cfg.fh_ix_offset = 1000;
#endif
}

//格式化
void sys_spiffs_format(void)
{
  //Serial.println("format");

  /* 格式化之前要先unmount */
  SPIFFS_unmount(&fs);
  /*
     int32_t ret=SPIFFS_format(&fs);
     if(ret<0)
     { Serial.print(F("format errno=")); Serial.println(SPIFFS_errno(&fs)); }
   */
  _erase_all();
  int res = SPIFFS_mount(&fs,
      &cfg,
      spiffs_work_buf,
      spiffs_fds,
      sizeof(spiffs_fds),
      spiffs_cache_buf,
      sizeof(spiffs_cache_buf),
      0);
  // { Serial.print(F("errno ")); Serial.println(SPIFFS_errno(&fs)); }
}

//挂载spiffs文件系统
void sys_spiffs_mount(void)
{
  int res = SPIFFS_mount(&fs,
      &cfg,
      spiffs_work_buf,
      spiffs_fds,
      sizeof(spiffs_fds),
      spiffs_cache_buf,
      sizeof(spiffs_cache_buf),
      0);
  if(SPIFFS_ERR_NOT_A_FS == res )
  {
    sys_spiffs_format();
  }
}
