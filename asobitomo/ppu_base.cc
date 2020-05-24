#include "ppu_base.h"
#include "cpu.h"
#include "util.h"
#include "ppu_util.h"

void
PPU::stat(byte value) {
  set_lcd_on(value & (1 << 7));
  set_window_tilemap_offset(value & (1 << 6) ? 0x9c00 : 0x9800);
  set_window_display(value & (1 << 5));
  set_bg_window_tile_data_offset(value & (1 << 4) ? 0x8000 : 0x8800);
  set_bg_tilemap_offset(value & (1 << 3) ? 0x9c00 : 0x9800);
  set_sprite_mode(value & (1 << 2) ?
    PPU::SpriteMode::S8x16 :
    PPU::SpriteMode::S8x8);
  set_sprite_display(value & (1 << 1));
  set_bg_display(value & 0x1);
}

void
PPU::step(long delta) {
  // Dr Mario seems to wait for vblank while in HALT, which leads to an infinite loop
  // unless the PPU is still active during LCD off (why would Dr Mario turn the LCD off?)
  if (!cpu.ppu->lcd_on) return;
  
  ncycles += delta;
  /* 456*144 + 4560 = 70224
   *                                    \
   * OAM         VRAM          hblank   |
   * ------- -------------- ----------  > 144 times per frame
   * 80           172           204     |
   * ---------------------------------  |
   *              456                   /
   *
   *                                    \
   *            vblank                  |
   * -----------------------------------> 10 times per frame
   *              456                   |
   *                                    /
   */
  bool just_transitioned = false;
  switch (mode) {
    case Mode::OAM:
      if (ncycles >= 80) {
        mode = Mode::VRAM;
        ncycles -= 80;
        
        just_transitioned = true;
      }
      
      update_stat_register(just_transitioned);
      break;
    case Mode::VRAM:
      if (ncycles >= 172) {
        mode = Mode::HBLANK;
        ncycles -= 172;
        
        rasterise_line();
        screen->set_row(line, raster.begin(), raster.end());
        
        just_transitioned = true;
      }
      
      update_stat_register(just_transitioned);
      
      break;
    case Mode::HBLANK:
      if (ncycles >= 204) {
        ++line;
        ncycles -= 204;
        
        if (line == 144) {
          mode = Mode::VBLANK;
          // last hblank: blit buffer
          screen->blit();
          debugger->show();
          tilemap->show();
        } else {
          mode = Mode::OAM;
        }
        update_stat_register(true);
      }
      break;
    case Mode::VBLANK:
      // vblank happens every 4560 cycles
      
      // One vblank line
      if (ncycles >= 456) {
        ncycles -= 456;
        ++line;
        
        if (line > 153) {
          mode = Mode::OAM;
          line = 0;
          
          just_transitioned = true;
        }
        
        update_stat_register(just_transitioned);
      }
      
      break;
  }
}

void
PPU::update_stat_register(bool just_transitioned)  {
  byte lyc = cpu.mmu.mem[0xff45];

  byte stat = cpu.mmu.mem[0xff41];
  if (lyc == line) {
    stat |= 0x4; // coincidence flag (bit 2)
  } else {
    stat &= ~0x4;
  }
  
  cpu.mmu.mem[0xff41] = (stat & 0xfc) | static_cast<byte>(mode);
  cpu.mmu.mem[0xff44] = line;
  
  byte IF = cpu.mmu.mem[0xff0f];
  bool set_lcd_interrupt =
    ((stat & 0x40) && (lyc == line)) ||
    (just_transitioned && (
      ((stat & 0x20) && mode == Mode::OAM) ||
      ((stat & 0x10) && mode == Mode::VBLANK) ||
      ((stat & 0x08) && mode == Mode::HBLANK)));
  if (set_lcd_interrupt && !set_lcd_interrupt_prev) {
    IF |= 1 << 1;
  }
  
  set_lcd_interrupt_prev = set_lcd_interrupt;
  
  bool set_vblank_interrupt = mode == Mode::VBLANK && line == 144;
  if (set_vblank_interrupt) {
    IF |= 1 << 0;
  }
  
  cpu.mmu.mem[0xff0f] = IF;
}

static const uint16_t m[256] =
{
  0x0000, 0x0001, 0x0004, 0x0005, 0x0010, 0x0011, 0x0014, 0x0015,
  0x0040, 0x0041, 0x0044, 0x0045, 0x0050, 0x0051, 0x0054, 0x0055,
  0x0100, 0x0101, 0x0104, 0x0105, 0x0110, 0x0111, 0x0114, 0x0115,
  0x0140, 0x0141, 0x0144, 0x0145, 0x0150, 0x0151, 0x0154, 0x0155,
  0x0400, 0x0401, 0x0404, 0x0405, 0x0410, 0x0411, 0x0414, 0x0415,
  0x0440, 0x0441, 0x0444, 0x0445, 0x0450, 0x0451, 0x0454, 0x0455,
  0x0500, 0x0501, 0x0504, 0x0505, 0x0510, 0x0511, 0x0514, 0x0515,
  0x0540, 0x0541, 0x0544, 0x0545, 0x0550, 0x0551, 0x0554, 0x0555,
  0x1000, 0x1001, 0x1004, 0x1005, 0x1010, 0x1011, 0x1014, 0x1015,
  0x1040, 0x1041, 0x1044, 0x1045, 0x1050, 0x1051, 0x1054, 0x1055,
  0x1100, 0x1101, 0x1104, 0x1105, 0x1110, 0x1111, 0x1114, 0x1115,
  0x1140, 0x1141, 0x1144, 0x1145, 0x1150, 0x1151, 0x1154, 0x1155,
  0x1400, 0x1401, 0x1404, 0x1405, 0x1410, 0x1411, 0x1414, 0x1415,
  0x1440, 0x1441, 0x1444, 0x1445, 0x1450, 0x1451, 0x1454, 0x1455,
  0x1500, 0x1501, 0x1504, 0x1505, 0x1510, 0x1511, 0x1514, 0x1515,
  0x1540, 0x1541, 0x1544, 0x1545, 0x1550, 0x1551, 0x1554, 0x1555,
  0x4000, 0x4001, 0x4004, 0x4005, 0x4010, 0x4011, 0x4014, 0x4015,
  0x4040, 0x4041, 0x4044, 0x4045, 0x4050, 0x4051, 0x4054, 0x4055,
  0x4100, 0x4101, 0x4104, 0x4105, 0x4110, 0x4111, 0x4114, 0x4115,
  0x4140, 0x4141, 0x4144, 0x4145, 0x4150, 0x4151, 0x4154, 0x4155,
  0x4400, 0x4401, 0x4404, 0x4405, 0x4410, 0x4411, 0x4414, 0x4415,
  0x4440, 0x4441, 0x4444, 0x4445, 0x4450, 0x4451, 0x4454, 0x4455,
  0x4500, 0x4501, 0x4504, 0x4505, 0x4510, 0x4511, 0x4514, 0x4515,
  0x4540, 0x4541, 0x4544, 0x4545, 0x4550, 0x4551, 0x4554, 0x4555,
  0x5000, 0x5001, 0x5004, 0x5005, 0x5010, 0x5011, 0x5014, 0x5015,
  0x5040, 0x5041, 0x5044, 0x5045, 0x5050, 0x5051, 0x5054, 0x5055,
  0x5100, 0x5101, 0x5104, 0x5105, 0x5110, 0x5111, 0x5114, 0x5115,
  0x5140, 0x5141, 0x5144, 0x5145, 0x5150, 0x5151, 0x5154, 0x5155,
  0x5400, 0x5401, 0x5404, 0x5405, 0x5410, 0x5411, 0x5414, 0x5415,
  0x5440, 0x5441, 0x5444, 0x5445, 0x5450, 0x5451, 0x5454, 0x5455,
  0x5500, 0x5501, 0x5504, 0x5505, 0x5510, 0x5511, 0x5514, 0x5515,
  0x5540, 0x5541, 0x5544, 0x5545, 0x5550, 0x5551, 0x5554, 0x5555
};

PPU::TileRow
PPU::unpack_bits(byte lsb, byte msb) {
  PPU::TileRow result;

  const uint64_t C = m[msb] << 1 | m[lsb];
  uint64_t data = (C & 0xc000) >> 14 |
                  (C & 0x3000) >> 4  |
                  (C & 0x0c00) << 6  |
                  (C & 0x0300) << 16 |
                  (C &   0xc0) << 26 |
                  (C &   0x30) << 36 |
                  (C &   0x0c) << 46 |
                  (C &   0x03) << 56;
  uint64_t* ptr = (uint64_t*)result.data();
  *ptr = data;
  return result;
}

inline void
PPU::set_lcd_on(bool on) {
  lcd_on = on;
}

inline void
PPU::set_window_tilemap_offset(word offset) {
  window_tilemap_offset = offset;
}

inline void
PPU::set_bg_window_tile_data_offset(word offset) {
  bg_window_tile_data_offset = offset;
}

inline void
PPU::set_window_display(bool on) {
  window_display = on;
}

inline void
PPU::set_bg_tilemap_offset(word offset) {
  bg_tilemap_offset = offset;
}

inline void
PPU::set_sprite_mode(SpriteMode mode) {
  sprite_mode = mode;
}

inline void
PPU::set_sprite_display(bool on) {
  sprite_display = on;
}

inline void
PPU::set_bg_display(bool on) {
  bg_display = on;
}


