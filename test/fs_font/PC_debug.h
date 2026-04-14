#pragma once
#include <stdint.h>

// PCデバッグ用ダミークラス定義
namespace lgfx
{
    inline namespace v1
    {
        // dummy class for debug
        struct color_conv_t
        {
            uint32_t c;
            uint32_t convert(uint32_t c) { return c; }
        };

        // dummy class for debug
        class LGFXBase
        {
        public:
            void setRawColor(uint32_t c) {;}
            void startWrite(bool transaction = true) {;}
            void endWrite(void) {;}
            void writeFillRect(int32_t x, int32_t y, int32_t w, int32_t h) {;}
            color_conv_t* getColorConverter(void) { return &_write_conv; }

        private:
            color_conv_t _write_conv;
        };

        struct DataWrapper;
        struct TextStyle;

        enum textdatum_t : uint8_t
            //  0:left   1:centre   2:right
            //  0:top    4:middle   8:bottom   16:baseline
        {
            top_left = 0  // Top left (default)
            , top_center = 1  // Top center
            , top_centre = 1  // Top center
            , top_right = 2  // Top right
            , middle_left = 4  // Middle left
            , middle_center = 5  // Middle center
            , middle_centre = 5  // Middle center
            , middle_right = 6  // Middle right
            , bottom_left = 8  // Bottom left
            , bottom_center = 9  // Bottom center
            , bottom_centre = 9  // Bottom center
            , bottom_right = 10  // Bottom right
            , baseline_left = 16  // Baseline left (Line the 'A' character would sit on)
            , baseline_center = 17  // Baseline center
            , baseline_centre = 17  // Baseline center
            , baseline_right = 18  // Baseline right
        };

        struct FontMetrics
        {
            int16_t width;
            int16_t x_advance;
            int16_t x_offset;
            int16_t height;
            int16_t y_advance;
            int16_t y_offset;
            int16_t baseline;
        };

        struct TextStyle
        {
            uint32_t fore_rgb888 = 0xFFFFFFU;
            uint32_t back_rgb888 = 0;
            float size_x = 1;
            float size_y = 1;
            textdatum_t datum = textdatum_t::top_left;
            int32_t padding_x = 0;
            bool utf8 = true;
            bool cp437 = false;
            // IFont* font = &fonts::Font0;
            // FontMetrics metrics;
        };

        struct IFont
        {
            enum font_type_t
            {
                ft_unknown
                , ft_glcd
                , ft_bmp
                , ft_rle
                , ft_gfx
                , ft_bdf
                , ft_vlw
                , ft_u8g2
                , ft_ttf
            };

            virtual font_type_t getType(void) const { return font_type_t::ft_unknown; }
            virtual void getDefaultMetric(FontMetrics* metrics) const = 0;
            virtual bool updateFontMetric(FontMetrics* metrics, uint16_t uniCode) const = 0;
            virtual bool unloadFont(void) { return false; }
            virtual size_t drawChar(LGFXBase* gfx, int32_t x, int32_t y, uint16_t c, const TextStyle* style, FontMetrics* metrics, int32_t& filled_x) const = 0;

        protected:
            size_t drawCharDummy(LGFXBase* gfx, int32_t x, int32_t y, int32_t w, int32_t h, const TextStyle* style, int32_t& filled_x) const
            {
				// only for debug, not implemented
                return 0;
            }
        };
    }
}
