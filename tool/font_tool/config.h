#pragma once

#include "lgfx_efont_ja.h"

// フォントの選択
static const uint8_t* fontData = lgfx_efont_ja_16;
static const size_t fontDataSize = 467155; // ↑のサイズ

// フォントの出力ファイル名
static const char* outputFileName = "fs_efont_ja_16.bin";

// プリロード文字ブロックの設定
struct PreloadBlock {
	uint16_t start;       // ブロックの開始コード
	uint16_t end;         // ブロックの終了コード
	uint32_t start_addr;  // ブロックの開始アドレス
	uint32_t block_size;  // ブロックのサイズ
};
static PreloadBlock preloadBlock[]= {
	{ 0x3000, 0x303E }, // 句読点
	{ 0x3040, 0x309F }, // ひらがな
	{ 0x30A0, 0x30FF }, // カタカナ
};

#define PRELOAD_BLOCK_CNT (sizeof(preloadBlock) / sizeof(PreloadBlock))