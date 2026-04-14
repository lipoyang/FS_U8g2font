#include <stdio.h>
#include "U8g2font.h"
#include "config.h"

// フォント
static const U8g2font font = { fontData };

// 元のフォントデータのヘッダサイズ
const uint32_t orig_header_size = 23;

bool write_uint8(FILE* fp, uint8_t value) {
	bool ret = fwrite(&value, 1, 1, fp) == 1;
	if (!ret) {
		printf("Error: failed to write uint8_t\n");
	}
	return ret;
}

bool write_uint16(FILE* fp, uint16_t value) {
	uint8_t bytes[2] = { (uint8_t)(value >> 8), (uint8_t)(value & 0xFF) };
	bool ret = fwrite(bytes, 1, 2, fp) == 2;
	if (!ret) {
		printf("Error: failed to write uint16_t\n");
	}
	return ret;
}

bool write_uint32(FILE* fp, uint32_t value) {
	uint8_t bytes[4] = {
		(uint8_t)(value >> 24),
		(uint8_t)((value >> 16) & 0xFF),
		(uint8_t)((value >> 8) & 0xFF),
		(uint8_t)(value & 0xFF)
	};
	bool ret = fwrite(bytes, 1, 4, fp) == 4;
	if (!ret) {
		printf("Error: failed to write uint16_t\n");
	}
	return ret;
}

int main()
{
	printf("Hello!\n");

	//************************************************************
	// 種々のサイズやアドレスの計算
	//************************************************************
	
	// LUTのサイズを計算
	uint32_t lut_end_addr = font.get_end_of_lut();
	uint32_t lut_start_addr = orig_header_size + font.start_pos_unicode();
	uint16_t lut_size = (uint16_t)(lut_end_addr - lut_start_addr);
	printf("lut_size = %04X\n", lut_size);

	// プリロードブロックの開始アドレスとサイズを計算
	uint8_t block_cnt = PRELOAD_BLOCK_CNT;
	bool error = false;
	for (int i = 0; i < (int)block_cnt; i++)
	{
		printf("[Preload bloak %d]\n", i);
		uint16_t start_code = preloadBlock[i].start;
		uint16_t end_code = preloadBlock[i].end;
		uint32_t start_addr = 0;
		uint32_t end_addr = 0;
		for (uint16_t code = start_code; code < end_code; code++) {
			start_addr = font.get_address(code);
			if (start_addr != 0) {
				printf("start_addr = %08X (code = %04X)\n", start_addr, code);
				break;
			}
		}
		for (uint16_t code = end_code; code > start_code; code--) {
			end_addr = font.get_address(code);
			if (end_addr != 0) {
				printf("end_addr = %08X (code = %04X)\n", end_addr, code);
				break;
			}
		}
		if (start_addr == 0 || end_addr == 0)
		{
			error = true;
			printf("Error: no data for block %d\n", i);
			continue;
		}
		uint32_t block_size = end_addr - start_addr;
		printf("block size = %08X (%lu)\n", block_size, block_size);

		preloadBlock[i].start_addr = start_addr;
		preloadBlock[i].block_size = block_size;
	}
	if(error) {
		printf("Error: failed to calculate preload block addresses and sizes\n");
		return 1;
	}

	// データ開始位置の計算
	uint32_t data_start_addr = orig_header_size + lut_size;

	// 新しいヘッダサイズ
	// ・元のヘッダ  : 23バイト
	// ・新しいヘッダサイズ  : 2バイト
	// ・LUTサイズ  : 2バイト
	// ・プリロード文字ブロックの数  : 1バイト
	// ・プリロード文字ブロックの情報 : 12バイト × ブロック数
	// 　　・開始コード : 2バイト
	// 　　・終了コード : 2バイト
	// 　　・開始アドレス : 4バイト
	// 　　・ブロックサイズ : 4バイト
	uint16_t new_header_size = 
		orig_header_size + 2 + 2 + 1 + (12 * block_cnt);
	printf("new_header_size = %d\n", new_header_size);

	// プリロードブロックの開始アドレスを、新しいヘッダの後ろに配置するためのオフセット
	for (int i = 0; i < (int)block_cnt; i++) {
		preloadBlock[i].start_addr += (new_header_size - orig_header_size);
	}

	//************************************************************
	// フォントファイルの出力
	//************************************************************

	// 出力ファイルのオープン
	FILE* fp = fopen(outputFileName, "wb");
	if (fp == NULL) {
		printf("Error: failed to open output file\n");
		return 1;
	}
	// 元のヘッダのコピー
	if (fwrite(fontData, 1, orig_header_size, fp) != orig_header_size) {
		printf("Error: failed to write original header\n");
		fclose(fp);
		return 1;
	}
	// 新しいヘッダのサイズ
	if (!write_uint16(fp, new_header_size)) {
		fclose(fp);
		return 1;
	}
	// LUTのサイズ
	if (!write_uint16(fp, lut_size)) {
		fclose(fp);
		return 1;
	}
	// プリロード文字ブロックの数
	if (!write_uint8(fp, block_cnt)) {
		fclose(fp);
		return 1;
	}
	// プリロード文字ブロックの情報
	for (int i = 0; i < (int)block_cnt; i++) {
		if (!write_uint16(fp, preloadBlock[i].start)) {
			fclose(fp);
			return 1;
		}
		if (!write_uint16(fp, preloadBlock[i].end)) {
			fclose(fp);
			return 1;
		}
		if (!write_uint32(fp, preloadBlock[i].start_addr)) {
			fclose(fp);
			return 1;
		}
		if (!write_uint32(fp, preloadBlock[i].block_size)) {
			fclose(fp);
			return 1;
		}
	}
	// 元のフォントデータのLUTとグリフデータのコピー
	const size_t chunk = 4096;
	size_t offset = orig_header_size;
	size_t remaining = fontDataSize - orig_header_size;
	printf("LUT + Data size = %d\n", (int)remaining);
	while (remaining > 0) {
		size_t n = remaining < chunk ? remaining : chunk;
		size_t written = fwrite(&fontData[offset], 1, n, fp);
		if (written != n) {
			printf("Error: failed to write original data\n");
			break;
		}
		offset += n;
		remaining -= n;
		printf(".");
		fflush(stdout);
	}
	fclose(fp);
	printf("\nCompleted successfully!\n");

	return 0;
}