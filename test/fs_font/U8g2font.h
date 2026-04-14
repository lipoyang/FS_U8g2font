#pragma once
#include "PC_debug.h"

namespace lgfx
{
 inline namespace v1
 {
  struct U8g2font : public lgfx::IFont
  {
    constexpr U8g2font(const uint8_t *u8g2_font) : _font(u8g2_font) {}
    font_type_t getType(void) const override { return ft_u8g2; }

    uint8_t glyph_cnt (void) const { return _font[0]; }
    uint8_t bbx_mode  (void) const { return _font[1]; }
    uint8_t bits_per_0(void) const { return _font[2]; }
    uint8_t bits_per_1(void) const { return _font[3]; }
    uint8_t bits_per_char_width (void) const { return _font[4]; }
    uint8_t bits_per_char_height(void) const { return _font[5]; }
    uint8_t bits_per_char_x     (void) const { return _font[6]; }
    uint8_t bits_per_char_y     (void) const { return _font[7]; }
    uint8_t bits_per_delta_x    (void) const { return _font[8]; }
    int8_t max_char_width (void) const { return _font[ 9]; }
    int8_t max_char_height(void) const { return _font[10]; } /* overall height, NOT ascent. Instead ascent = max_char_height + y_offset */
    int8_t x_offset       (void) const { return _font[11]; }
    int8_t y_offset       (void) const { return _font[12]; }
    int8_t ascent_A    (void) const { return _font[13]; }
    int8_t descent_g   (void) const { return _font[14]; }  /* usually a negative value */
    int8_t ascent_para (void) const { return _font[15]; }
    int8_t descent_para(void) const { return _font[16]; }

    uint16_t start_pos_upper_A(void) const { return _font[17] << 8 | _font[18]; }
    uint16_t start_pos_lower_a(void) const { return _font[19] << 8 | _font[20]; }
    uint16_t start_pos_unicode(void) const { return _font[21] << 8 | _font[22]; }

    void getDefaultMetric(FontMetrics *metrics) const override;
    bool updateFontMetric(FontMetrics *metrics, uint16_t uniCode) const override;
    size_t drawChar(LGFXBase* gfx, int32_t x, int32_t y, uint16_t c, const TextStyle* style, FontMetrics* metrics, int32_t& filled_x) const override;

    const uint8_t* getGlyph(uint16_t encoding) const;

  private:
//  const uint8_t* getGlyph(uint16_t encoding) const;
    const uint8_t* _font;
  };
 }
}
