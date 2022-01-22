#include "stdafx.h"
#include <math.h>
#include <algorithm>
#include "NES/NesDefaultVideoFilter.h"
#include "NES/NesConsole.h"
#include "NES/NesConstants.h"
#include "NES/NesPpu.h"
#include "Shared/Video/BaseVideoFilter.h"
#include "Shared/EmuSettings.h"
#include "Shared/Emulator.h"

static constexpr uint32_t _ppuPaletteArgb[11][64] = {
	/* 2C02 */			{ 0xFF666666, 0xFF002A88, 0xFF1412A7, 0xFF3B00A4, 0xFF5C007E, 0xFF6E0040, 0xFF6C0600, 0xFF561D00, 0xFF333500, 0xFF0B4800, 0xFF005200, 0xFF004F08, 0xFF00404D, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFADADAD, 0xFF155FD9, 0xFF4240FF, 0xFF7527FE, 0xFFA01ACC, 0xFFB71E7B, 0xFFB53120, 0xFF994E00, 0xFF6B6D00, 0xFF388700, 0xFF0C9300, 0xFF008F32, 0xFF007C8D, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFEFF, 0xFF64B0FF, 0xFF9290FF, 0xFFC676FF, 0xFFF36AFF, 0xFFFE6ECC, 0xFFFE8170, 0xFFEA9E22, 0xFFBCBE00, 0xFF88D800, 0xFF5CE430, 0xFF45E082, 0xFF48CDDE, 0xFF4F4F4F, 0xFF000000, 0xFF000000, 0xFFFFFEFF, 0xFFC0DFFF, 0xFFD3D2FF, 0xFFE8C8FF, 0xFFFBC2FF, 0xFFFEC4EA, 0xFFFECCC5, 0xFFF7D8A5, 0xFFE4E594, 0xFFCFEF96, 0xFFBDF4AB, 0xFFB3F3CC, 0xFFB5EBF2, 0xFFB8B8B8, 0xFF000000, 0xFF000000 },
	/* 2C03 */			{ 0xFF6D6D6D, 0xFF002491, 0xFF0000DA, 0xFF6D48DA, 0xFF91006D, 0xFFB6006D, 0xFFB62400, 0xFF914800, 0xFF6D4800, 0xFF244800, 0xFF006D24, 0xFF009100, 0xFF004848, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFB6B6B6, 0xFF006DDA, 0xFF0048FF, 0xFF9100FF, 0xFFB600FF, 0xFFFF0091, 0xFFFF0000, 0xFFDA6D00, 0xFF916D00, 0xFF249100, 0xFF009100, 0xFF00B66D, 0xFF009191, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFF6DB6FF, 0xFF9191FF, 0xFFDA6DFF, 0xFFFF00FF, 0xFFFF6DFF, 0xFFFF9100, 0xFFFFB600, 0xFFDADA00, 0xFF6DDA00, 0xFF00FF00, 0xFF48FFDA, 0xFF00FFFF, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFFB6DAFF, 0xFFDAB6FF, 0xFFFFB6FF, 0xFFFF91FF, 0xFFFFB6B6, 0xFFFFDA91, 0xFFFFFF48, 0xFFFFFF6D, 0xFFB6FF48, 0xFF91FF6D, 0xFF48FFDA, 0xFF91DAFF, 0xFF000000, 0xFF000000, 0xFF000000 },
	/* 2C04-0001 */	{ 0xFFFFB6B6, 0xFFDA6DFF, 0xFFFF0000, 0xFF9191FF, 0xFF009191, 0xFF244800, 0xFF484848, 0xFFFF0091, 0xFFFFFFFF, 0xFF6D6D6D, 0xFFFFB600, 0xFFB6006D, 0xFF91006D, 0xFFDADA00, 0xFF6D4800, 0xFFFFFFFF, 0xFF6DB6FF, 0xFFDAB66D, 0xFF6D2400, 0xFF6DDA00, 0xFF91DAFF, 0xFFDAB6FF, 0xFFFFDA91, 0xFF0048FF, 0xFFFFDA00, 0xFF48FFDA, 0xFF000000, 0xFF480000, 0xFFDADADA, 0xFF919191, 0xFFFF00FF, 0xFF002491, 0xFF00006D, 0xFFB6DAFF, 0xFFFFB6FF, 0xFF00FF00, 0xFF00FFFF, 0xFF004848, 0xFF00B66D, 0xFFB600FF, 0xFF000000, 0xFF914800, 0xFFFF91FF, 0xFFB62400, 0xFF9100FF, 0xFF0000DA, 0xFFFF9100, 0xFF000000, 0xFF000000, 0xFF249100, 0xFFB6B6B6, 0xFF006D24, 0xFFB6FF48, 0xFF6D48DA, 0xFFFFFF00, 0xFFDA6D00, 0xFF004800, 0xFF006DDA, 0xFF009100, 0xFF242424, 0xFFFFFF6D, 0xFFFF6DFF, 0xFF916D00, 0xFF91FF6D },
	/* 2C04-0002 */	{ 0xFF000000, 0xFFFFB600, 0xFF916D00, 0xFFB6FF48, 0xFF91FF6D, 0xFFFF6DFF, 0xFF009191, 0xFFB6DAFF, 0xFFFF0000, 0xFF9100FF, 0xFFFFFF6D, 0xFFFF91FF, 0xFFFFFFFF, 0xFFDA6DFF, 0xFF91DAFF, 0xFF009100, 0xFF004800, 0xFF6DB6FF, 0xFFB62400, 0xFFDADADA, 0xFF00B66D, 0xFF6DDA00, 0xFF480000, 0xFF9191FF, 0xFF484848, 0xFFFF00FF, 0xFF00006D, 0xFF48FFDA, 0xFFDAB6FF, 0xFF6D4800, 0xFF000000, 0xFF6D48DA, 0xFF91006D, 0xFFFFDA91, 0xFFFF9100, 0xFFFFB6FF, 0xFF006DDA, 0xFF6D2400, 0xFFB6B6B6, 0xFF0000DA, 0xFFB600FF, 0xFFFFDA00, 0xFF6D6D6D, 0xFF244800, 0xFF0048FF, 0xFF000000, 0xFFDADA00, 0xFFFFFFFF, 0xFFDAB66D, 0xFF242424, 0xFF00FF00, 0xFFDA6D00, 0xFF004848, 0xFF002491, 0xFFFF0091, 0xFF249100, 0xFF000000, 0xFF00FFFF, 0xFF914800, 0xFFFFFF00, 0xFFFFB6B6, 0xFFB6006D, 0xFF006D24, 0xFF919191 },
	/* 2C04-0003 */	{ 0xFFB600FF, 0xFFFF6DFF, 0xFF91FF6D, 0xFFB6B6B6, 0xFF009100, 0xFFFFFFFF, 0xFFB6DAFF, 0xFF244800, 0xFF002491, 0xFF000000, 0xFFFFDA91, 0xFF6D4800, 0xFFFF0091, 0xFFDADADA, 0xFFDAB66D, 0xFF91DAFF, 0xFF9191FF, 0xFF009191, 0xFFB6006D, 0xFF0048FF, 0xFF249100, 0xFF916D00, 0xFFDA6D00, 0xFF00B66D, 0xFF6D6D6D, 0xFF6D48DA, 0xFF000000, 0xFF0000DA, 0xFFFF0000, 0xFFB62400, 0xFFFF91FF, 0xFFFFB6B6, 0xFFDA6DFF, 0xFF004800, 0xFF00006D, 0xFFFFFF00, 0xFF242424, 0xFFFFB600, 0xFFFF9100, 0xFFFFFFFF, 0xFF6DDA00, 0xFF91006D, 0xFF6DB6FF, 0xFFFF00FF, 0xFF006DDA, 0xFF919191, 0xFF000000, 0xFF6D2400, 0xFF00FFFF, 0xFF480000, 0xFFB6FF48, 0xFFFFB6FF, 0xFF914800, 0xFF00FF00, 0xFFDADA00, 0xFF484848, 0xFF006D24, 0xFF000000, 0xFFDAB6FF, 0xFFFFFF6D, 0xFF9100FF, 0xFF48FFDA, 0xFFFFDA00, 0xFF004848 },
	/* 2C04-0004 */	{ 0xFF916D00, 0xFF6D48DA, 0xFF009191, 0xFFDADA00, 0xFF000000, 0xFFFFB6B6, 0xFF002491, 0xFFDA6D00, 0xFFB6B6B6, 0xFF6D2400, 0xFF00FF00, 0xFF00006D, 0xFFFFDA91, 0xFFFFFF00, 0xFF009100, 0xFFB6FF48, 0xFFFF6DFF, 0xFF480000, 0xFF0048FF, 0xFFFF91FF, 0xFF000000, 0xFF484848, 0xFFB62400, 0xFFFF9100, 0xFFDAB66D, 0xFF00B66D, 0xFF9191FF, 0xFF249100, 0xFF91006D, 0xFF000000, 0xFF91FF6D, 0xFF6DB6FF, 0xFFB6006D, 0xFF006D24, 0xFF914800, 0xFF0000DA, 0xFF9100FF, 0xFFB600FF, 0xFF6D6D6D, 0xFFFF0091, 0xFF004848, 0xFFDADADA, 0xFF006DDA, 0xFF004800, 0xFF242424, 0xFFFFFF6D, 0xFF919191, 0xFFFF00FF, 0xFFFFB6FF, 0xFFFFFFFF, 0xFF6D4800, 0xFFFF0000, 0xFFFFDA00, 0xFF48FFDA, 0xFFFFFFFF, 0xFF91DAFF, 0xFF000000, 0xFFFFB600, 0xFFDA6DFF, 0xFFB6DAFF, 0xFF6DDA00, 0xFFDAB6FF, 0xFF00FFFF, 0xFF244800 },
	/* 2C05-01 */		{ 0xFF6D6D6D, 0xFF002491, 0xFF0000DA, 0xFF6D48DA, 0xFF91006D, 0xFFB6006D, 0xFFB62400, 0xFF914800, 0xFF6D4800, 0xFF244800, 0xFF006D24, 0xFF009100, 0xFF004848, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFB6B6B6, 0xFF006DDA, 0xFF0048FF, 0xFF9100FF, 0xFFB600FF, 0xFFFF0091, 0xFFFF0000, 0xFFDA6D00, 0xFF916D00, 0xFF249100, 0xFF009100, 0xFF00B66D, 0xFF009191, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFF6DB6FF, 0xFF9191FF, 0xFFDA6DFF, 0xFFFF00FF, 0xFFFF6DFF, 0xFFFF9100, 0xFFFFB600, 0xFFDADA00, 0xFF6DDA00, 0xFF00FF00, 0xFF48FFDA, 0xFF00FFFF, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFFB6DAFF, 0xFFDAB6FF, 0xFFFFB6FF, 0xFFFF91FF, 0xFFFFB6B6, 0xFFFFDA91, 0xFFFFFF48, 0xFFFFFF6D, 0xFFB6FF48, 0xFF91FF6D, 0xFF48FFDA, 0xFF91DAFF, 0xFF000000, 0xFF000000, 0xFF000000 },
	/* 2C05-02 */		{ 0xFF6D6D6D, 0xFF002491, 0xFF0000DA, 0xFF6D48DA, 0xFF91006D, 0xFFB6006D, 0xFFB62400, 0xFF914800, 0xFF6D4800, 0xFF244800, 0xFF006D24, 0xFF009100, 0xFF004848, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFB6B6B6, 0xFF006DDA, 0xFF0048FF, 0xFF9100FF, 0xFFB600FF, 0xFFFF0091, 0xFFFF0000, 0xFFDA6D00, 0xFF916D00, 0xFF249100, 0xFF009100, 0xFF00B66D, 0xFF009191, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFF6DB6FF, 0xFF9191FF, 0xFFDA6DFF, 0xFFFF00FF, 0xFFFF6DFF, 0xFFFF9100, 0xFFFFB600, 0xFFDADA00, 0xFF6DDA00, 0xFF00FF00, 0xFF48FFDA, 0xFF00FFFF, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFFB6DAFF, 0xFFDAB6FF, 0xFFFFB6FF, 0xFFFF91FF, 0xFFFFB6B6, 0xFFFFDA91, 0xFFFFFF48, 0xFFFFFF6D, 0xFFB6FF48, 0xFF91FF6D, 0xFF48FFDA, 0xFF91DAFF, 0xFF000000, 0xFF000000, 0xFF000000 },
	/* 2C05-03 */		{ 0xFF6D6D6D, 0xFF002491, 0xFF0000DA, 0xFF6D48DA, 0xFF91006D, 0xFFB6006D, 0xFFB62400, 0xFF914800, 0xFF6D4800, 0xFF244800, 0xFF006D24, 0xFF009100, 0xFF004848, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFB6B6B6, 0xFF006DDA, 0xFF0048FF, 0xFF9100FF, 0xFFB600FF, 0xFFFF0091, 0xFFFF0000, 0xFFDA6D00, 0xFF916D00, 0xFF249100, 0xFF009100, 0xFF00B66D, 0xFF009191, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFF6DB6FF, 0xFF9191FF, 0xFFDA6DFF, 0xFFFF00FF, 0xFFFF6DFF, 0xFFFF9100, 0xFFFFB600, 0xFFDADA00, 0xFF6DDA00, 0xFF00FF00, 0xFF48FFDA, 0xFF00FFFF, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFFB6DAFF, 0xFFDAB6FF, 0xFFFFB6FF, 0xFFFF91FF, 0xFFFFB6B6, 0xFFFFDA91, 0xFFFFFF48, 0xFFFFFF6D, 0xFFB6FF48, 0xFF91FF6D, 0xFF48FFDA, 0xFF91DAFF, 0xFF000000, 0xFF000000, 0xFF000000 },
	/* 2C05-04 */		{ 0xFF6D6D6D, 0xFF002491, 0xFF0000DA, 0xFF6D48DA, 0xFF91006D, 0xFFB6006D, 0xFFB62400, 0xFF914800, 0xFF6D4800, 0xFF244800, 0xFF006D24, 0xFF009100, 0xFF004848, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFB6B6B6, 0xFF006DDA, 0xFF0048FF, 0xFF9100FF, 0xFFB600FF, 0xFFFF0091, 0xFFFF0000, 0xFFDA6D00, 0xFF916D00, 0xFF249100, 0xFF009100, 0xFF00B66D, 0xFF009191, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFF6DB6FF, 0xFF9191FF, 0xFFDA6DFF, 0xFFFF00FF, 0xFFFF6DFF, 0xFFFF9100, 0xFFFFB600, 0xFFDADA00, 0xFF6DDA00, 0xFF00FF00, 0xFF48FFDA, 0xFF00FFFF, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFFB6DAFF, 0xFFDAB6FF, 0xFFFFB6FF, 0xFFFF91FF, 0xFFFFB6B6, 0xFFFFDA91, 0xFFFFFF48, 0xFFFFFF6D, 0xFFB6FF48, 0xFF91FF6D, 0xFF48FFDA, 0xFF91DAFF, 0xFF000000, 0xFF000000, 0xFF000000 },
	/* 2C05-05 */		{ 0xFF6D6D6D, 0xFF002491, 0xFF0000DA, 0xFF6D48DA, 0xFF91006D, 0xFFB6006D, 0xFFB62400, 0xFF914800, 0xFF6D4800, 0xFF244800, 0xFF006D24, 0xFF009100, 0xFF004848, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFB6B6B6, 0xFF006DDA, 0xFF0048FF, 0xFF9100FF, 0xFFB600FF, 0xFFFF0091, 0xFFFF0000, 0xFFDA6D00, 0xFF916D00, 0xFF249100, 0xFF009100, 0xFF00B66D, 0xFF009191, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFF6DB6FF, 0xFF9191FF, 0xFFDA6DFF, 0xFFFF00FF, 0xFFFF6DFF, 0xFFFF9100, 0xFFFFB600, 0xFFDADA00, 0xFF6DDA00, 0xFF00FF00, 0xFF48FFDA, 0xFF00FFFF, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFFB6DAFF, 0xFFDAB6FF, 0xFFFFB6FF, 0xFFFF91FF, 0xFFFFB6B6, 0xFFFFDA91, 0xFFFFFF48, 0xFFFFFF6D, 0xFFB6FF48, 0xFF91FF6D, 0xFF48FFDA, 0xFF91DAFF, 0xFF000000, 0xFF000000, 0xFF000000 }
};

static constexpr uint8_t _paletteLut[11][64] = {
	/* 2C02 */      { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63 },
	/* 2C03 */      { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,15,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,15,62,63 },
	/* 2C04-0001 */ { 53,35,22,34,28,9,29,21,32,0,39,5,4,40,8,32,33,62,31,41,60,50,54,18,63,43,46,30,61,45,36,1,14,49,51,42,44,12,27,20,46,7,52,6,19,2,38,46,46,25,16,10,57,3,55,23,15,17,11,13,56,37,24,58 },
	/* 2C04-0002 */ { 46,39,24,57,58,37,28,49,22,19,56,52,32,35,60,11,15,33,6,61,27,41,30,34,29,36,14,43,50,8,46,3,4,54,38,51,17,31,16,2,20,63,0,9,18,46,40,32,62,13,42,23,12,1,21,25,46,44,7,55,53,5,10,45 },
	/* 2C04-0003 */ { 20,37,58,16,11,32,49,9,1,46,54,8,21,61,62,60,34,28,5,18,25,24,23,27,0,3,46,2,22,6,52,53,35,15,14,55,13,39,38,32,41,4,33,36,17,45,46,31,44,30,57,51,7,42,40,29,10,46,50,56,19,43,63,12 },
	/* 2C04-0004 */ { 24,3,28,40,46,53,1,23,16,31,42,14,54,55,11,57,37,30,18,52,46,29,6,38,62,27,34,25,4,46,58,33,5,10,7,2,19,20,0,21,12,61,17,15,13,56,45,36,51,32,8,22,63,43,32,60,46,39,35,49,41,50,44,9 },
	/* 2C05-01 */   { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,15,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,15,62,63 },
	/* 2C05-02 */   { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,15,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,15,62,63 },
	/* 2C05-03 */   { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,15,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,15,62,63 },
	/* 2C05-04 */   { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,15,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,15,62,63 },
	/* 2C05-05 */   { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,15,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,15,62,63 },
};

NesDefaultVideoFilter::NesDefaultVideoFilter(Emulator* emu) : BaseVideoFilter(emu)
{
	InitLookupTable();
}

void NesDefaultVideoFilter::GenerateFullColorPalette(uint32_t paletteBuffer[512])
{
	for(int i = 0; i < 64; i++) {
		for(int j = 1; j < 8; j++) {
			double redColor = (uint8_t)(paletteBuffer[i] >> 16);
			double greenColor = (uint8_t)(paletteBuffer[i] >> 8);
			double blueColor = (uint8_t)paletteBuffer[i];
			if(j & 0x01) {
				//Intensify red
				redColor *= 1.1;
				greenColor *= 0.9;
				blueColor *= 0.9;
			}
			if(j & 0x02) {
				//Intensify green
				greenColor *= 1.1;
				redColor *= 0.9;
				blueColor *= 0.9;
			}
			if(j & 0x04) {
				//Intensify blue
				blueColor *= 1.1;
				redColor *= 0.9;
				greenColor *= 0.9;
			}

			uint8_t r = (uint8_t)(redColor > 255 ? 255 : redColor);
			uint8_t g = (uint8_t)(greenColor > 255 ? 255 : greenColor);
			uint8_t b = (uint8_t)(blueColor > 255 ? 255 : blueColor);

			uint32_t color = 0xFF000000 | (r << 16) | (g << 8) | b;
			paletteBuffer[(j << 6) | i] = color;
		}
	}
}

void NesDefaultVideoFilter::GetFullPalette(uint32_t palette[512], NesConfig& nesCfg, PpuModel model)
{
	if(model == PpuModel::Ppu2C02) {
		memcpy(palette, nesCfg.UserPalette, sizeof(nesCfg.UserPalette));
		if(!nesCfg.IsFullColorPalette) {
			//Generate emphasis palette colors
			GenerateFullColorPalette(palette);
		}
	} else {
		//TODO use custom palette option
		memcpy(palette, _ppuPaletteArgb[(int)model], sizeof(_ppuPaletteArgb[(int)_ppuModel]));
		GenerateFullColorPalette(palette);
	}
}

void NesDefaultVideoFilter::InitLookupTable()
{
	VideoConfig videoCfg = _emu->GetSettings()->GetVideoConfig();
	NesConfig nesCfg = _emu->GetSettings()->GetNesConfig();

	InitConversionMatrix(videoCfg.Hue, videoCfg.Saturation);

	double y, i, q;
	uint32_t palette[512];
	GetFullPalette(palette, nesCfg, _ppuModel);

	for(int pal = 0; pal < 512; pal++) {
		uint32_t pixelOutput = palette[pal];
		double redChannel = ((pixelOutput & 0xFF0000) >> 16) / 255.0;
		double greenChannel = ((pixelOutput & 0xFF00) >> 8) / 255.0;
		double blueChannel = (pixelOutput & 0xFF) / 255.0;

		//Apply brightness, contrast, hue & saturation
		RgbToYiq(redChannel, greenChannel, blueChannel, y, i, q);
		y *= _videoConfig.Contrast * 0.5f + 1;
		y += _videoConfig.Brightness * 0.5f;
		YiqToRgb(y, i, q, redChannel, greenChannel, blueChannel);

		int r = std::min(255, (int)(redChannel * 255));
		int g = std::min(255, (int)(greenChannel * 255));
		int b = std::min(255, (int)(blueChannel * 255));

		_calculatedPalette[pal] = 0xFF000000 | (r << 16) | (g << 8) | b;
	}
}

void NesDefaultVideoFilter::OnBeforeApplyFilter()
{
	VideoConfig config = _emu->GetSettings()->GetVideoConfig();
	NesConfig nesConfig = _emu->GetSettings()->GetNesConfig();

	PpuModel model = ((NesConsole*)_emu->GetConsole())->GetPpu()->GetPpuModel();

	if(_ppuModel != model || _videoConfig.Hue != config.Hue || _videoConfig.Saturation != config.Saturation || _videoConfig.Contrast != config.Contrast || _videoConfig.Brightness != config.Brightness || memcmp(_nesConfig.UserPalette, nesConfig.UserPalette, sizeof(nesConfig.UserPalette)) != 0) {
		_ppuModel = model;
		InitLookupTable();
	}
	_videoConfig = config;
	_nesConfig = nesConfig;
}

void NesDefaultVideoFilter::DecodePpuBuffer(uint16_t* ppuOutputBuffer, uint32_t* outputBuffer, bool displayScanlines)
{
	uint32_t* out = outputBuffer;
	OverscanDimensions overscan = GetOverscan();
	FrameInfo frame = _frameInfo;
	
	uint8_t scanlineIntensity = (uint8_t)((1.0 - _videoConfig.ScanlineIntensity) * 255);
	for(uint32_t i = 0; i < frame.Height; i++) {
		if(displayScanlines && (i + overscan.Top) % 2 == 0) {
			for(uint32_t j = 0; j < frame.Width; j++) {
				*out = ApplyScanlineEffect(ppuOutputBuffer[(i + overscan.Top) * _baseFrameInfo.Width + j + overscan.Left], scanlineIntensity);
				out++;
			}
		} else {
			for(uint32_t j = 0; j < frame.Width; j++) {
				*out = _calculatedPalette[ppuOutputBuffer[(i + overscan.Top) * _baseFrameInfo.Width + j + overscan.Left]];
				out++;
			}
		}
	}
}

void NesDefaultVideoFilter::ApplyFilter(uint16_t* ppuOutputBuffer)
{
	DecodePpuBuffer(ppuOutputBuffer, GetOutputBuffer(), false /* TODO _videoConfig.VideoFilter <= VideoFilterType::BisqwitNtsc */);
}