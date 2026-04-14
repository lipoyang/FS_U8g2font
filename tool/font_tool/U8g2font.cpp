#include "U8g2font.h"
#include <algorithm>

#define _DEBUG_
#ifdef _DEBUG_
#define DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif

int utf16_to_utf8(uint16_t utf16, uint8_t* utf8) {
    if (utf16 <= 0x007F) {
        // 1バイト (ASCII範囲)
        utf8[0] = (uint8_t)utf16;
        return 1;
    }
    else if (utf16 <= 0x07FF) {
        // 2バイト
        utf8[0] = (uint8_t)(0xC0 | (utf16 >> 6));
        utf8[1] = (uint8_t)(0x80 | (utf16 & 0x3F));
        return 2;
    }
    else {
        // 3バイト (BMPの残りすべて)
        utf8[0] = (uint8_t)(0xE0 | (utf16 >> 12));
        utf8[1] = (uint8_t)(0x80 | ((utf16 >> 6) & 0x3F));
        utf8[2] = (uint8_t)(0x80 | (utf16 & 0x3F));
        return 3;
    }
}

uint32_t U8g2font::get_end_of_lut() const
{
    const uint8_t* font = &this->_font[23];

    uint16_t encoding = 0xFFFF;
    uint_fast16_t e;
    const uint8_t* unicode_lut;

    font += this->start_pos_unicode();
    unicode_lut = font;

    DEBUG_PRINT("LUT start @ %08X\n", (int)(unicode_lut - _font));

    do
    {
        font += (unicode_lut[0] << 8) + unicode_lut[1];
        e = (unicode_lut[2] << 8) + unicode_lut[3];
        int addr = (int)(unicode_lut - _font);
        int code = (unicode_lut[2] << 8) + unicode_lut[3];
        int size = (unicode_lut[0] << 8) + unicode_lut[1];
        int addr2 = (int)(font - _font);
        //DEBUG_PRINT("LUT %08X : code=%04X size=%04X (%d)\n", addr, code, size, size);
        //DEBUG_PRINT("addr %08X : code=%04X\n", addr2, code);
        unicode_lut += 4;
    } while (e < encoding);

    int end_address = (int)(unicode_lut - _font);

    DEBUG_PRINT("LUT end @ %08X - 1\n", end_address);

    return end_address;
}

uint32_t U8g2font::get_address(uint16_t encoding) const
{
    const uint8_t* font = &this->_font[23];

    if (encoding <= 255)
    {
        if (encoding >= 'a') { font += this->start_pos_lower_a(); }
        else if (encoding >= 'A') { font += this->start_pos_upper_A(); }

        for (; font[1]; font += font[1])
        {
            if (font[0] == encoding) {
				uint32_t addr = (uint32_t)(font - _font);
                return addr;
            }
        }
    }
    else
    {
        uint_fast16_t e;
        const uint8_t* unicode_lut;

        font += this->start_pos_unicode();
        unicode_lut = font;

        do
        {
            font += (unicode_lut[0] << 8) + unicode_lut[1];
            e = (unicode_lut[2] << 8) + unicode_lut[3];
            int addr = (int)(unicode_lut - _font);
            int code = (unicode_lut[2] << 8) + unicode_lut[3];
            int size = (unicode_lut[0] << 8) + unicode_lut[1];
            int addr2 = (int)(font - _font);
            //DEBUG_PRINT("LUT %08X : code=%04X size=%04X (%d)\n", addr, code, size, size);
            //DEBUG_PRINT("addr %08X : code=%04X\n", addr2, code);
            unicode_lut += 4;
        } while (e < encoding);

        for (; 0 != (e = (font[0] << 8) + font[1]); font += font[2])
        {
            if (e == encoding) {
                uint32_t addr = (uint32_t)(font - _font);
                return addr; 
            }
        }
    }
    return 0;
}
