#include <stdlib.h>
#include <algorithm>

#include "U8g2font.h"

#define _DEBUG_
#ifdef _DEBUG_
#define DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif

namespace lgfx
{
 inline namespace v1
 {
  struct u8g2_font_decode_t
  {
    u8g2_font_decode_t(const uint8_t* ptr) : decode_ptr(ptr), decode_bit_pos(0) {}

    const uint8_t* decode_ptr;      /* pointer to the compressed data */
    uint8_t decode_bit_pos;     /* bitpos inside a byte of the compressed data */

    uint_fast8_t get_unsigned_bits(uint_fast8_t cnt)
    {
      uint_fast8_t bit_pos = this->decode_bit_pos;
      uint_fast8_t val = *(this->decode_ptr) >> bit_pos;

      auto bit_pos_plus_cnt = bit_pos + cnt;
      if ( bit_pos_plus_cnt >= 8 )
      {
        bit_pos_plus_cnt -= 8;
        val |= *(++this->decode_ptr) << (8-bit_pos);
      }
      this->decode_bit_pos = bit_pos_plus_cnt;
      return val & ((1U << cnt) - 1);
    }

    int_fast8_t get_signed_bits(uint_fast8_t cnt)
    {
      return (int_fast8_t)get_unsigned_bits(cnt) - (1 << (cnt-1));
    }
  };


  const uint8_t* U8g2font::getGlyph(uint16_t encoding) const
  {
    const uint8_t *font = &this->_font[23];

    if ( encoding <= 255 )
    {
      if ( encoding >= 'a' )      { font += this->start_pos_lower_a(); }
      else if ( encoding >= 'A' ) { font += this->start_pos_upper_A(); }

      for ( ; font[1]; font += font[1])
      {
        if ( font[0] == encoding ) { return font + 2; }  /* skip encoding and glyph size */
      }
    }
    else
    {
      uint_fast16_t e;
      const uint8_t *unicode_lut;

      font += this->start_pos_unicode();
      unicode_lut = font;

      do
      {
        // DEBUG_PRINT("PM code = %04X : %04X, %04X @ %08X\n", encoding,
        //   (unicode_lut[0] << 8) + unicode_lut[1],
        //   (unicode_lut[2] << 8) + unicode_lut[3],
        //   (uint32_t)(font-_font));

        font += (unicode_lut[0] << 8) + unicode_lut[1];
        e     = (unicode_lut[2] << 8) + unicode_lut[3];
        unicode_lut += 4;
      } while ( e < encoding );
      // DEBUG_PRINT("*PM code = %04X : %02X, %04X @ %08X\n", encoding,
      //   (font[0] << 8) + font[1],
      //   font[2],
      //   (uint32_t)(font - _font));

      for ( ; 0 != (e = (font[0] << 8) + font[1]) ; font += font[2])
      {
        if ( e == encoding ) { return font + 3; }  /* skip encoding and glyph size */
      }
    }
    return nullptr;
  }

  void U8g2font::getDefaultMetric(lgfx::FontMetrics *metrics) const
  {
    metrics->height    = max_char_height();
    metrics->y_advance = metrics->height;
    metrics->baseline  = metrics->height + y_offset();
    metrics->y_offset  = -metrics->baseline;
    metrics->x_offset  = 0;
  }

  bool U8g2font::updateFontMetric(lgfx::FontMetrics *metrics, uint16_t uniCode) const
  {
    //printf("updateFontMetric %0X\n",uniCode );
    u8g2_font_decode_t decode(getGlyph(uniCode));
    if ( decode.decode_ptr )
    {
      metrics->width     = decode.get_unsigned_bits(this->bits_per_char_width());
                          decode.get_unsigned_bits(this->bits_per_char_height());
      metrics->x_offset  = decode.get_signed_bits  (this->bits_per_char_x());
                          decode.get_signed_bits  (this->bits_per_char_y());
      metrics->x_advance = decode.get_signed_bits  (this->bits_per_delta_x());
      return true;
    }
    metrics->width = metrics->x_advance = this->max_char_width();
    metrics->x_offset = 0;
    return false;
  }

  size_t U8g2font::drawChar(LGFXBase* gfx, int32_t x, int32_t y, uint16_t uniCode, const TextStyle* style, FontMetrics* metrics, int32_t& filled_x) const
  {
    //printf("drawChar %0X\n",uniCode );
    int32_t sy = 65536 * style->size_y;
    y += (metrics->y_offset * sy) >> 16;
    u8g2_font_decode_t decode(getGlyph(uniCode));
    if ( decode.decode_ptr == nullptr ) return drawCharDummy(gfx, x, y, this->max_char_width(), metrics->height, style, filled_x);

    uint32_t w = decode.get_unsigned_bits(bits_per_char_width());
    uint32_t h = decode.get_unsigned_bits(bits_per_char_height());

    int32_t sx = 65536 * style->size_x;

    int32_t xoffset = (decode.get_signed_bits(bits_per_char_x()) * sx) >> 16;

    int32_t yoffset = -(int32_t)(decode.get_signed_bits(bits_per_char_y()) + h + metrics->y_offset);

    int32_t xAdvance = (decode.get_signed_bits(bits_per_delta_x()) * sx) >> 16;

    uint32_t colortbl[2] = {gfx->getColorConverter()->convert(style->back_rgb888), gfx->getColorConverter()->convert(style->fore_rgb888)};
    bool fillbg = (style->back_rgb888 != style->fore_rgb888);
    int32_t left  = 0;
    int32_t right = 0;
    if (fillbg) {
      left  = std::max<int>(filled_x, x + (xoffset < 0 ? xoffset : 0));
      right = x + std::max<int>(((w * sx) >> 16) + xoffset, xAdvance);
      filled_x = right;
      gfx->setRawColor(colortbl[0]);
    }
    x += xoffset;
    gfx->startWrite();

    if (left < right)
    {
      if (yoffset > 0) {
        gfx->writeFillRect(left, y, right - left, (yoffset * sy) >> 16);
      }
      int32_t y0 = ((yoffset + h)   * sy) >> 16;
      int32_t y1 = (metrics->height * sy) >> 16;
      if (y0 < y1) {
        gfx->writeFillRect(left, y + y0, right - left, y1 - y0);
      }
    }

    if ( w > 0 )
    {
      if (left < right)
      {
        int32_t y0  = (  yoffset      * sy) >> 16;
        int32_t len = (((yoffset + h) * sy) >> 16) - y0;
        if (left < x)
        {
          gfx->writeFillRect(left, y + y0, x - left, len);
        }
        int32_t xwsx = x + ((w * sx) >> 16);
        if (xwsx < right)
        {
          gfx->writeFillRect(xwsx, y + y0, right - xwsx, len);
        }
      }
      left -= x;
      uint32_t ab[2];
      uint32_t lx = 0;
      uint32_t ly = 0;
      int32_t y0 = ((yoffset    ) * sy) >> 16;
      int32_t y1 = ((yoffset + 1) * sy) >> 16;
      do
      {
        ab[0] = decode.get_unsigned_bits(bits_per_0());
        ab[1] = decode.get_unsigned_bits(bits_per_1());
        bool i = 0;
        do
        {
          uint32_t length = ab[i];
          while (length)
          {
            uint32_t len = (length > w - lx) ? w - lx : length;
            length -= len;
            if (i || fillbg)
            {
              int32_t x0 = (lx * sx) >> 16;
              if (!i && x0 < left) x0 = left;
              int32_t x1 = ((lx + len) * sx) >> 16;
              if (x0 < x1)
              {
                gfx->setRawColor(colortbl[i]);
                gfx->writeFillRect( x + x0
                                  , y + y0
                                  , x1 - x0
                                  , y1 - y0);
              }
            }
            lx += len;
            if (lx == w)
            {
              lx = 0;
              ++ly;
              y0 = y1;
              y1 = ((ly + yoffset + 1) * sy) >> 16;
            }
          }
          i = !i;
        } while (i || decode.get_unsigned_bits(1) != 0 );
      } while (ly < h);
    }
    gfx->endWrite();
    return xAdvance;
  }
 }
}
