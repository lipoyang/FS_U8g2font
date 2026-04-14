# ファイルシステム版u8g2フォント (SPRESENSE用)

## 概要
u8g2フォントは組込みマイコン向けのフォントであり、M5UnifiedライブラリやLovyanGFXライブラリでも利用されています。u8g2フォントのデータはプログラムに埋め込んでFlashメモリに格納して使います。日本語フォントの場合、データ量は数百キロバイトになることもあります。

いっぽうSPRESENSEでは、プログラムはFlashメモリからRAM上に展開されてRAM上で実行されます。Flashメモリはファイルシステムで管理されるストレージという扱いです。そのため、SPRESENSEでu8g2の日本語フォントを使うと、数百キロバイトものRAMを使用することになります。1.5メガバイトのRAMを持つSPRESENSEとはいえ、これは少々不都合です。

そこで、u8g2フォントを改修し、必要な文字のフォントデータのみをファイルからRAMに読み出して使えるようにしました。この ```FS_U8g2font``` クラスは、LovyanGFXライブラリの ```IFont``` クラスを継承しているので、LovyanGFXライブラリで利用できます。


## 仕様
- フォントデータはバイナリ形式のファイルとしてSDカードに格納します。
- 英数字など U+0000 ～ U+00FF の文字はRAMに展開します。
- ひらがな、カタカナ、句読点などは文字コードの範囲を指定してRAMに展開できます。
- 漢字などは必要な文字を指定してRAMに展開できます。
- 必要な文字をその都度RAMに展開することもできます。この一時データは古いものから削除されます。

## フォントデータの作成
- [tool/](tool/) にフォントデータ作成ツールがあります。
- Windows の Visual Studio 用のC++のプロジェクトです。
- 特にWindows依存のコードではないので、Makefileを書けば他の環境でもビルドできるはずです。
- [tool/font_tool/config.h](tool/font_tool/config.h) に設定を記述し、ビルドして実行するとフォントデータファイルが出力されます。

```c++:config.h
#pragma once

#include "lgfx_efont_ja.h"

// フォントの選択
static const uint8_t* fontData = lgfx_efont_ja_16;
static const size_t fontDataSize = 467155; // ↑のサイズ

// フォントの出力ファイル名
static const char* outputFileName = "fs_efont_ja_16.bin";

// プリロード文字ブロックの設定
struct PreloadBlock {
	uint16_t start;       // ブロックの開始コード (UTF-16)
	uint16_t end;         // ブロックの終了コード (UTF-16)
	uint32_t start_addr;
	uint32_t block_size;
};
static PreloadBlock preloadBlock[]= {
	{ 0x3000, 0x303E }, // 句読点
	{ 0x3040, 0x309F }, // ひらがな
	{ 0x30A0, 0x30FF }, // カタカナ
};

#define PRELOAD_BLOCK_CNT (sizeof(preloadBlock) / sizeof(PreloadBlock))
```

## フォントデータの利用
### フォントのロード
```c++
#include "FS_U8g2font.h"

lgfx::FS_U8g2font font;

...

  if(font.loadFont("/mnt/sd0/fs_efont_ja_16.bin") != true)
  {
    Serial.println("Font load error!");
  }
```
### 漢字データのロード
```c++
  font.loadGlyph("国破山河在"); // 必要な漢字を列挙
  font.loadGlyph("国破れて山河在り"); // RAM展開済みの文字は無視される
  font.loadGlyph(0x56FD); // UTF-16コードでも指定できる
```

### 一時データのロード
```loadGlyph```との違いは、最大文字数を超えたときに古いデータから順に削除されることです。それ以外は同様です。
```c++
  font.setGlyphCacheSize(256); // 最大256文字までキャッシュする(既定値)
  font.loadGlyphCache("城春草木深");
  font.loadGlyphCache("城春にして草木深し");
  font.loadGlyphCache(0x57CE);
```

### リソースの解放

```c++
  font.unloadGlyphCache();
  font.unloadGlyph();
  font.unloadFont();
```
