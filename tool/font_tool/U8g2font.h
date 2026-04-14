#pragma once

#include <stdint.h>
#include <stdio.h>

struct U8g2font
{
constexpr U8g2font(const uint8_t *u8g2_font) : _font(u8g2_font) {}

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

uint32_t get_end_of_lut() const;
uint32_t get_address(uint16_t encoding) const;

private:
const uint8_t* _font;
};
