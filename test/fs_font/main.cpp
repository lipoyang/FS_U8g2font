#include "FS_U8g2font.h"
#include "U8g2font.h"
#include "lgfx_efont_ja.h"

using namespace lgfx;

U8g2font font1(lgfx_efont_ja_16);
FS_U8g2font font2;

void checkData(uint16_t code)
{
  printf("\ncode = %04X\n", code);

  const uint8_t* data1 = font1.getGlyph(code);
  const uint8_t* data2 = font2.getGlyph(code);
  if (data1 == nullptr) {
    printf("font1 no data\n");
    return;
  }
  if (data2 == nullptr) {
    printf("font2 no data\n");
    return;
  }

  for (int i = 0; i < 8; i++) {
    printf("%02X ", data1[i]);
  }
  printf("\n");
  for (int i = 0; i < 8; i++) {
    printf("%02X ", data2[i]);
  }
  printf("\n");
  for (int i = 0; i < 8; i++) {
    if (data1[i] != data2[i]) {
      printf("mismatched @ [%d]\n", i);
    }
  }
}

// テストベンチ
int main()
{
  if (!font2.loadFont("fs_efont_ja_16.bin")) {
    printf("loadFont failed!\n");
    return 1;
  }

  // 元ヘッダのチェック
  printf("\n");
  printf("start_pos_upper_A = %0X / %0X\n", font1.start_pos_upper_A(), font2.start_pos_upper_A());
  printf("start_pos_lower_a = %0X / %0X\n", font1.start_pos_lower_a(), font2.start_pos_lower_a());
  printf("start_pos_unicode = %0X / %0X\n", font1.start_pos_unicode(), font2.start_pos_unicode());

  // アスキー文字のチェック
  checkData(0x0020);
  checkData(0x0041);
  checkData(0x0061);
  checkData(0x007E);

  // 句読点・かな・カナのチェック
  checkData(0x3000);
  checkData(0x300F);
  checkData(0x3041);
  checkData(0x3093);
  checkData(0x30A1);
  checkData(0x30F3);

  // 漢字のチェック
  font2.loadGlyph(0x4E00);
  font2.loadGlyph(0x5005);
  checkData(0x4E00);
  checkData(0x5005);

  font2.loadGlyphCache(0x9F54);
  font2.loadGlyphCache(0xFF31);
  checkData(0x9F54);
  checkData(0xFF31);

  // 文字列
  // 国:0x56FD, 破:0x7834, れ:0x308C, て:0x3066
  // 山:0x5C71, 河:0x6CB3, 在:0x5728, り:0x308A
  font2.loadGlyph("国破れて山河在り");
  checkData(0x56FD);
  checkData(0x5728);

  font2.unloadGlyph();
  checkData(0x5728);

  font2.unloadFont();
  return 0;
}
