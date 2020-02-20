#include "spiffs/spiffs.h"
#include <SerialFlash.h>
#include <SPI.h>


/*文件系统结构体*/
static spiffs fs;
uint16_t cs1;

/*页定义*/
#define LOG_PAGE_SIZE      256

void _erase(uint32_t addr){
  addr&=0xffffff;
  addr|=0x20000000;
  digitalWrite(cs1, LOW);
  delay(10);
  SPI.transfer(0x20);
  SPI.transfer((addr>>16)&0xff);
  SPI.transfer((addr>>8)&0xff);
  SPI.transfer(addr&0xff);
  delay(10);
  digitalWrite(cs1, HIGH);
}

static u8_t spiffs_work_buf[LOG_PAGE_SIZE*2];
static u8_t spiffs_fds[32*4];
static u8_t spiffs_cache_buf[(LOG_PAGE_SIZE+32)*2];

uint32_t chipsize,blocksize;
s32_t _spiffs_read(u32_t addr, u32_t size, u8_t *dst) {
  /*
     SPI.transfer16(0x0300 | ((addr >> 16) & 255));
     SPIPORT.transfer16(addr);
     SPIPORT.transfer(p, rdlen);
   */
  SerialFlash.read(addr, dst, size);
  return 0;
}
static s32_t _spiffs_write(u32_t addr, u32_t size, u8_t *src) {
  SerialFlash.write(addr, src, size);
  return 0;
}

static s32_t _spiffs_erase(uint32_t addr, int32_t size) {
  //  Serial.print("erase ");
  //  Serial.print(addr);
  //  Serial.print(",size=");
  //  Serial.println(size);
  uint8_t ch;
  while(size>0) {
    for(uint16_t i=0;i<4096;i++) {
      SerialFlash.read(addr+i, &ch, 1);
      if(ch!=0xff) break;
    }
    if(ch!=0xff) {
      _erase(addr);
    }
    addr+=4096;
    size-=4096;
  }
  return 0;
}

//初始化以及配置
spiffs_config cfg;
void spiffs_init(uint8_t cs) {
  uint8_t buf[256];
  cs1=cs;
  SerialFlash.begin(cs);
  SerialFlash.readID(buf);
  Serial.print(F("id="));
  Serial.print(buf[0],HEX);
  Serial.write(' ');
  Serial.print(buf[1],HEX);
  Serial.write(' ');
  Serial.println(buf[2],HEX);
  chipsize = SerialFlash.capacity(buf);
  blocksize=SerialFlash.blockSize();;
  Serial.print(F("chipsize="));
  Serial.println(chipsize);
  Serial.print(F("blocksize="));
  Serial.println(blocksize);
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
  int32_t ret=SPIFFS_format(&fs);
  if(ret<0)
  { Serial.print(F("format errno=")); Serial.println(SPIFFS_errno(&fs)); }

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
void sys_spiffs_mount_coreflash(void)
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
