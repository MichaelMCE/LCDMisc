#include "malloc.h"
#include <stdlib.h>
#include "Image.h"
#include "../stringUtil.h"

#include "../Screen.h"
#include "../Util.h"
#include "../unicode.h"
#include "Image.h"
#include "BMP.h"
#include "tga.h"
#include "jpg.h"
#include "gif.h"
#include "png.h"
#include "psd.h"
#include "stackblur.h"



#define _ARGB(a,r,g,b)	(((a)<<24) | ((r) << 16) | ((g) << 8) | (b))	// Convert to RGB
#define _RGB(r,g,b)		(((r) << 16) | ((g) << 8) | (b))	// Convert to RGB
#define _GetAValue(c)	((int)(((c) & 0xFF000000) >> 24))	// Alpha color component
#define _GetRValue(c)	((int)(((c) & 0x00FF0000) >> 16))	// Red color component
#define _GetGValue(c)	((int)(((c) & 0x0000FF00) >> 8))	// Green color component
#define _GetBValue(c)	((int)( (c) & 0x000000FF))			// Blue color component
#define itofx(x)		((x) << 8)							// Integer to int point
#define ftofx(x)		(int)((x) * 256)					// Float to int point
#define fxtoi(x)		((x) >> 8)							// Fixed point to integer
#define Mulfx(x,y)		(((x) * (y)) >> 8)					// Multiply a int by a int
#define Divfx(x,y)		(((x) << 8) / (y))					// Divide a int by a int



int MakeAllocatedBitImage(ScriptValue &s, int w, int h) {
	if (!AllocateCustomValue(s, sizeof(AllocatedBitImage) + (((w+31)/32)*h-1) * sizeof(int))) return 0;
	AllocatedBitImage *out = (AllocatedBitImage*) s.stringVal->value;
	out->height = h;
	out->width = w;
	//out->data = (unsigned int*) &(((unsigned char*)out)[sizeof(AllocatedBitImage)]);
	return 1;
}

AllocatedBitImage *MakeAllocatedBitImage(int w, int h) {
	AllocatedBitImage *out = (AllocatedBitImage*) malloc(sizeof(AllocatedBitImage) + (((w+31)/32)*h-1) * sizeof(int));
	if (!out) return 0;
	out->height = h;
	out->width = w;
	//out->data = (unsigned int*) &(((unsigned char*)out)[sizeof(AllocatedBitImage)]);
	return out;
}

int Convert4ColorTo3Color(ScriptValue &src, ScriptValue &dst) {
	GenericImage<unsigned char> *srcImage = (GenericImage<unsigned char> *)src.stringVal->value;
	if (srcImage->spp != 4) {
		dst = src;
		return 1;
	}
	GenericImage<unsigned char> *dstImage;
	if (src.stringVal->refCount == 1) {
		dst = src;
		dstImage = srcImage;
	}
	else {
		src.stringVal->refCount--;
		if (!MakeGenericImage<unsigned char>(dst, srcImage->width, srcImage->height, 3)) return 0;
		dstImage = (GenericImage<unsigned char> *)dst.stringVal->value;
	}
	unsigned long memWidth = (srcImage->width*3+3)&~3;
	for (unsigned int y=0; y<srcImage->height; y++) {
		unsigned char* src = &srcImage->pixels[srcImage->memWidth*y];
		unsigned char* dst = &dstImage->pixels[memWidth*y];
		for (unsigned int x=0; x<srcImage->width;x++,src+=4,dst+=3) {
			if (src[3] == 0xFF) {
				dst[0] = src[0];
				dst[1] = src[1];
				dst[2] = src[2];
			}
			else {
				dst[0] = (unsigned char) ((src[0]*((unsigned int)src[3]))/0xFF + (0xFF - src[3]));
				dst[1] = (unsigned char) ((src[1]*((unsigned int)src[3]))/0xFF + (0xFF - src[3]));
				dst[2] = (unsigned char) ((src[2]*((unsigned int)src[3]))/0xFF + (0xFF - src[3]));
			}
		}
	}
	if (dstImage == srcImage) {
		dstImage->spp = 3;
		if (!ResizeGenericImage<unsigned char>(dst)) {
			dst.Release();
			return 0;
		}
	}
	return 1;
}

int Convert2ColorTo1Color(ScriptValue &src, ScriptValue &dst) {
	GenericImage<unsigned char> *srcImage = (GenericImage<unsigned char> *)src.stringVal->value;
	if (srcImage->spp != 2) {
		dst = src;
		return 1;
	}
	GenericImage<unsigned char> *dstImage;
	if (src.stringVal->refCount == 1) {
		dstImage = srcImage;
	}
	else {
		src.stringVal->refCount--;
		if (!MakeGenericImage<unsigned char>(dst, srcImage->width, srcImage->height, 3)) return 0;
		dstImage = (GenericImage<unsigned char> *)dst.stringVal->value;
	}
	unsigned long memWidth = (srcImage->width+3)&~3;
	for (unsigned int y=0; y<srcImage->height; y++) {
		unsigned char* src = &srcImage->pixels[srcImage->memWidth*y];
		unsigned char* dst = &dstImage->pixels[memWidth*y];
		for (unsigned int x=0; x<srcImage->width;x++,src+=2,dst+=1) {
			if (src[1] == 0xFF) {
				dst[0] = src[0];
			}
			else {
				dst[0] = (unsigned char) ((src[0]*((unsigned int)src[1]))/0xFF + (0xFF - src[1]));
			}
		}
	}
	if (dstImage == srcImage) {
		dstImage->spp = 1;
		if (!ResizeGenericImage<unsigned char>(dst)) {
			dst.Release();
			return 0;
		}
	}
	return 1;
}

int Convert3ColorTo1Color(ScriptValue &src, ScriptValue &dst) {
	GenericImage<unsigned char> *srcImage = (GenericImage<unsigned char> *)src.stringVal->value;
	if (srcImage->spp != 3) {
		dst = src;
		return 1;
	}
	GenericImage<unsigned char> *dstImage;
	if (src.stringVal->refCount == 1) {
		dstImage = srcImage;
		dst = src;
	}
	else {
		src.stringVal->refCount--;
		if (!MakeGenericImage<unsigned char>(dst, srcImage->width, srcImage->height, 1)) return 0;
		dstImage = (GenericImage<unsigned char> *)dst.stringVal->value;
	}
	unsigned long memWidth = (srcImage->width+3)&~3;
	for (unsigned int y=0; y<srcImage->height; y++) {
		unsigned char* src = &srcImage->pixels[srcImage->memWidth*y];
		unsigned char* dst = &dstImage->pixels[memWidth*y];
		for (unsigned int x=0; x<srcImage->width;x++,src+=3,dst+=1) {
			dst[0] = (unsigned char) ((1+(unsigned int)src[0] + (unsigned int)src[1]+ (unsigned int)src[2])/3);
			//if (dst[0]!=255)
			//	src=src;
		}
	}
	if (dstImage == srcImage) {
		dstImage->spp = 1;
		if (!ResizeGenericImage<unsigned char>(dst)) {
			dst.Release();
			return 0;
		}
	}
	return 1;
}

int Convert1ColorToBitImage(ScriptValue &src, ScriptValue &dst, int cutoff) {
	GenericImage<unsigned char> *img = (GenericImage<unsigned char> *) src.stringVal->value;
	if (!img || img->spp!=1) return 0;
	if (!MakeAllocatedBitImage(dst, img->width, img->height)) return 0;
	BitImage *out = (BitImage*) dst.stringVal->value;
	int memWidth = ((out->width+31)/32);
	memset(out->data, 0, memWidth*out->height*4);
	int y;
	if (cutoff < 0) {
		int *row1 = ((int*) calloc(2*(img->width*2+2), sizeof(int)));
		if (row1) {
			int *mem = row1;
			row1++;
			int *row2 = row1 + img->width+2;
			for (y=0; y<out->height; y++) {
				unsigned char* src = &img->pixels[img->memWidth*y];
				unsigned char* dst = (unsigned char*) &out->data[memWidth*y];
				for (int x=0; x<out->width; x++) {
					//if (*dst) {
					//	dst=dst;
					//}
					int diff = 0;
					int val = 256*(int)*src + row1[x];
					if (val < 256*0x80) {
						*dst |= 1<<(x&0x7);
						diff = val;
					}
					else {
						diff = val-256*0xFF;
					}
					diff /=16;
					row1[x+1] += 7*diff;
					row2[x-1] += 3*diff;
					row2[x] += 5*diff;
					row2[x+1] += 1*diff;
					row1[x] = 0;
					src++;
					dst += !((x+1)&7);
				}
				int *t = row1;
				row1 = row2;
				row2 = t;
			}
			free(mem);
			return 1;
		}
		cutoff = 0;
	}
	if (!cutoff) {
		__int64 sum = 0;
		int count = 0;
		for (y=0; y<out->height; y++) {
			unsigned char* src = &img->pixels[img->memWidth*y];
			for (int x=0; x<out->width; x++) {
				count ++;
				sum += *src;
				src++;
			}
		}
		cutoff = (int)(sum/count);
	}
	for (y=0; y<out->height; y++) {
		unsigned char* src = &img->pixels[img->memWidth*y];
		unsigned char* dst = (unsigned char*) &out->data[memWidth*y];
		for (int x=0; x<out->width; x++) {
			//if (*dst) {
			//	dst=dst;
			//}
			if (*src < cutoff) {
				*dst |= 1<<(x&0x7);
			}
			src++;
			dst += !((x+1)&7);
		}
	}
	return 1;
}
/*
BitImage *Convert1ColorToBitImage(GenericImage<unsigned char> *img) {
	if (!img || img->spp!=1) return 0;
	BitImage *out = MakeAllocatedBitImage(img->width, img->height);
	if (!out) return 0;
	int memWidth = ((out->width+31)/32);
	memset(out->data, 0, memWidth*out->height*4);
	for (int y=0; y<out->height; y++) {
		unsigned char* src = &img->pixels[img->memWidth*y];
		unsigned char* dst = (unsigned char*) &out->data[memWidth*y];
		for (int x=0; x<out->width; x++) {
			//if (*dst) {
			//	dst=dst;
			//}
			if (*src < 0x80) {
				*dst |= 1<<(x&0x7);
			}
			src++;
			dst += !((x+1)&7);
		}
	}
	return out;
}
//*/


int ConvertTo1ColorImage(ScriptValue &src, ScriptValue &dst) {
	GenericImage<unsigned char> *img = (GenericImage<unsigned char>*) src.stringVal->value;
	if (img->spp >= 3) {
		if (img->spp == 4) {
			ScriptValue mid;
			return (Convert4ColorTo3Color(src, mid) && Convert3ColorTo1Color(mid, dst));
		}
		else {
			return Convert3ColorTo1Color(src, dst);
		}
	}
	else if (img->spp == 2) {
		return Convert2ColorTo1Color(src, dst);
	}
	dst = src;
	return 1;
}

static inline void FlipV (unsigned char *src, unsigned char *des, const int width, const int height, const int pitch)
{
	for (int y = height-1; y >= 0; y--){
		memcpy(des, &src[y*pitch], pitch);
		des += pitch;
	}
}

static inline void FlipH (unsigned char *src, unsigned char *des, const int width, const int height, const int pitch)
{
	const int widthMinus1 = width-1;

	for (int y = 0; y < height; y++){
		int *ldes = (int*)des;
		int *lsrc = (int*)&src[y*pitch];

		for (int x = 0; x < width; x++)
			ldes[x] = lsrc[widthMinus1-x];

		des += pitch;
	}
}

static inline void FlipVH (unsigned char *src, unsigned char *des, const int width, const int height, const int pitch)
{
	const int widthMinus1 = width-1;

	for (int y = height-1; y >= 0; y--){
		int *ldes = (int*)des;
		int *lsrc = (int*)&src[y*pitch];

		for (int x = 0; x < width; x++)
			ldes[x] = lsrc[widthMinus1-x];

		des += pitch;
	}
}

int Flip (ScriptValue &srcv, ScriptValue &dst, const int V_H)
{
	if (V_H < 1 || V_H > 3)  return 0;

	GenericImage<unsigned char> *src = (GenericImage<unsigned char>*)srcv.stringVal->value;
	if (src->spp != 4) return 0;
	const unsigned int outWidth = src->width;
	const unsigned int outHeight =  src->height;

	if (!MakeGenericImage<unsigned char>(dst, outWidth, outHeight, src->spp))
		return 0;

	GenericImage<unsigned char> *out = (GenericImage<unsigned char> *)dst.stringVal->value;

	if (V_H == 1)		// vertical
		FlipV(src->pixels, out->pixels, outWidth, outHeight, out->memWidth);
	else if (V_H == 2)	// horizontal
		FlipH(src->pixels, out->pixels, outWidth, outHeight, out->memWidth);
	else if (V_H == 3)	// horizontal and vertical
		FlipVH(src->pixels, out->pixels, outWidth, outHeight, out->memWidth);

	return 1;
}

int Pixelize (ScriptValue &srcv, ScriptValue &dst, int size)
{
	GenericImage<unsigned char> *src = (GenericImage<unsigned char>*)srcv.stringVal->value;
	if (src->spp != 4) return 0;
	const unsigned int outWidth = src->width;
	const unsigned int outHeight =  src->height;

	if (!MakeGenericImage<unsigned char>(dst, outWidth, outHeight, src->spp))
		return 0;

	GenericImage<unsigned char> *out = (GenericImage<unsigned char> *)dst.stringVal->value;


	const int m_iBpp = src->spp;
	// Calculate pixelize params
	const int _size = max(1, min((outWidth>>4), size));
	const int f_size = Divfx(itofx(1),itofx(_size*_size));
	//const int pitch = outWidth*m_iBpp;
	const int pitch = out->memWidth;
	unsigned char *lpData = (unsigned char*)out->pixels;

	// Pixelize bitmap
	int dwHorizontalOffset;
	int dwVerticalOffset = 0;
	int dwTotalOffset;
	int *lpSrcData = (int*)src->pixels;
	int *lpDstData = (int*)lpData;

	for (int i = 0; i < outHeight; i+=_size){
		dwHorizontalOffset = 0;

		for (int j = 0; j < outWidth; j+=_size){
			// Update total offset
			dwTotalOffset = dwVerticalOffset + dwHorizontalOffset;

			// Update bitmap
			int dwSrcOffset = dwTotalOffset;
			int f_red = 0, f_green = 0, f_blue = 0, f_alpha = 0;

			for (int k = 0; k < _size; k++){
				int m = i + k;
				if (m >= outHeight-1) m = outHeight - 1;

				for (int l = 0; l < _size; l++){
					int n = j + l;
					if (n >= outWidth-1)
						n = outWidth - 1;
					dwSrcOffset = m*pitch + n*m_iBpp;
					f_alpha += itofx(_GetAValue(lpSrcData[dwSrcOffset>>2]));
					f_red += itofx(_GetRValue(lpSrcData[dwSrcOffset>>2]));
					f_green += itofx(_GetGValue(lpSrcData[dwSrcOffset>>2]));
					f_blue += itofx(_GetBValue(lpSrcData[dwSrcOffset>>2]));
				}
			}
			f_alpha = Mulfx(f_size,f_alpha);
			f_red = Mulfx(f_size,f_red);
			f_green = Mulfx(f_size,f_green);
			f_blue = Mulfx(f_size,f_blue);
			int newPixel = _ARGB(fxtoi(f_alpha),fxtoi(f_red),fxtoi(f_green),fxtoi(f_blue));

			for (int k = 0; k<_size; k++){
				int m = i + k;
				if (m >= outHeight-1)
					m = outHeight - 1;

				for (int l = 0; l<_size; l++){
					int n = j + l;
					if (n >= outWidth-1)
						n = outWidth - 1;
					dwSrcOffset = m*pitch + n*m_iBpp;
					lpDstData[dwSrcOffset>>2] = newPixel;
				}
			}

			// Update horizontal offset
			dwHorizontalOffset += (_size*m_iBpp);
		}

		// Update vertical offset
		dwVerticalOffset += (_size*pitch);
	}

	return 1;
}

int Blur (ScriptValue &srcv, ScriptValue &dst, int radius, int unused)
{
	GenericImage<unsigned char> *src = (GenericImage<unsigned char>*)srcv.stringVal->value;
	if (src->spp != 4) return 0;
	const unsigned int outWidth = src->width;
	const unsigned int outHeight =  src->height;

	if (!MakeGenericImage<unsigned char>(dst, outWidth, outHeight, src->spp))
		return 0;

	GenericImage<unsigned char> *out = (GenericImage<unsigned char> *)dst.stringVal->value;
	memcpy(out->pixels, src->pixels, out->memWidth*out->height);

	stack_blur_rgba32((unsigned long*)out->pixels, outWidth, outHeight, radius);

	return 1;
}

int Copy (ScriptValue &srcv, ScriptValue &dst)
{
	GenericImage<unsigned char> *src = (GenericImage<unsigned char>*)srcv.stringVal->value;

	if (!MakeGenericImage<unsigned char>(dst, src->width, src->height, src->spp))
		return 0;

	GenericImage<unsigned char> *out = (GenericImage<unsigned char> *)dst.stringVal->value;
	memcpy(out->pixels, src->pixels, out->memWidth*out->height);

	return 1;
}

/*
mode 0: affect rgba
mode 1: affect rgb
mode 2: affect a
*/

int Fade (ScriptValue &srcv, ScriptValue &dst, double level, int mode)
{
	GenericImage<unsigned char> *src = (GenericImage<unsigned char>*)srcv.stringVal->value;
	if (src->spp != 4) return 0;
	const unsigned int outWidth = src->width;
	const unsigned int outHeight =  src->height;

	if (!MakeGenericImage<unsigned char>(dst, outWidth, outHeight, src->spp))
		return 0;

	GenericImage<unsigned char> *out = (GenericImage<unsigned char> *)dst.stringVal->value;
	//memcpy(out->pixels, src->pixels, out->memWidth*out->height);

	unsigned int *pxin = (unsigned int *)src->pixels;
	unsigned int *pxout = (unsigned int *)out->pixels;
	const int total = outWidth * outHeight;
	if (level > 1.0) level = 1.0;
	else if (level < 0.0) level = 0.5;

	if (mode == 0){
		for (int i = 0; i < total; i++){
			int a = ((*pxin&0xFF000000)>>24) * level;
			int r = ((*pxin&0x00FF0000)>>16) * level;
			int g = ((*pxin&0x0000FF00)>>8) * level;
			int b = (*pxin&0x000000FF) * level;
			pxin++;

			*pxout++ = (a<<24) | (r<<16) | (g<<8) | b;
		}
	}else if (mode == 1){
		for (int i = 0; i < total; i++){
			int a = *pxin&0xFF000000;
			int r = ((*pxin&0x00FF0000)>>16) * level;
			int g = ((*pxin&0x0000FF00)>>8) * level;
			int b = (*pxin&0x000000FF) * level;
			pxin++;

			*pxout++ = a | (r<<16) | (g<<8) | b;
		}
	}else if (mode == 2){
		for (int i = 0; i < total; i++){
			int a = ((*pxin&0xFF000000)>>24) * level;
			int r = *pxin&0x00FF0000;
			int g = *pxin&0x0000FF00;
			int b = *pxin&0x000000FF;
			pxin++;

			*pxout++ = (a<<24) | r | g | b;
		}
	}

	return 1;
}

void ClipRect (RECT &r, const RECT &r2)
{
	int w = r.left - r2.left;
	r.left -= (w>>31) & w;

	w = r.top - r2.top;
	r.top -= (w>>31) & w;

	w = r2.right - r.right;
	r.right += (w>>31) & w;

	w = r2.bottom - r.bottom;
	r.bottom += (w>>31) & w;
}

static inline void AlphaColorPixel (Color4 *dst, const Color4 src)
{
	const unsigned int alpha = src.a + (unsigned int)(src.a>>7);
	const unsigned int evens1 = dst->val & 0xFF00FF;
	const unsigned int evens2 = src.val & 0xFF00FF;
	const unsigned int odds1 = (dst->val>>8) & 0xFF00FF;
	const unsigned int odds2 = (src.val >>8) & 0xFF00FF;
	const unsigned int evenRes = ((((evens2-evens1)*alpha)>>8) + evens1)& 0xFF00FF;
	const unsigned int oddRes = ((odds2-odds1)*alpha + (odds1<<8)) & 0xFF00FF00;
	dst->val = evenRes | oddRes;
}

static inline void drawImage (int dstx, int dsty, int srcx, int srcy, int width, int height, GenericImage<unsigned char> *srci, GenericImage<unsigned char> *desi)
{
	//printf("drawImage %i %i, %i %i\n",dstx, dsty, width, height);

	int dx = dstx - srcx;
	int dy = dsty - srcy;

	RECT r2 = {0, 0, srci->width - 1, srci->height - 1};
	RECT r = {srcx, srcy, srcx+width-1, srcy+height-1};

	ClipRect(r, r2);
	r.left += dx;
	r.right += dx;
	r.top += dy;
	r.bottom += dy;
	//ClipRect(r, r2);

	if (((r.right - r.left) | (r.bottom - r.top)) < 0)
		return;
	if (r.right >= r2.right-1) r.right = r.right-1;
	if (r.bottom >= r2.bottom-1) r.bottom = r.bottom-1;

	//printf("%i %i %i %i, %i %i\n", r.left, r.right, r.top, r.bottom, dx, dy);

	const int desWidth = (desi->width * desi->spp+3)&~3;
	const int srcWidth = (srci->width * srci->spp+3)&~3;
	const int left = r.left * desi->spp;
	const int right = r.right * desi->spp;
	const int cwidth = sizeof(Color4);

	for (int y = r.top, y2 = srcy; y <= r.bottom; y++, y2++){
		Color4 *src = (Color4*)&srci->pixels[(y2*srcWidth)+(srcx*srci->spp)];
		Color4 *dst = (Color4*)&desi->pixels[(y*desWidth)+left];
		for (int x = left; x <= right; x += cwidth)
			AlphaColorPixel(dst++, *src++);
	}
}

int Draw (ScriptValue &srcv, ScriptValue &dstv, int desX, int desY, int srcX, int srcY, int srcWidth, int srcHeight)
{
	//printf("Draw %i %i\n", desX, desY);

	GenericImage<unsigned char> *src = (GenericImage<unsigned char>*)srcv.stringVal->value;
	GenericImage<unsigned char> *des = (GenericImage<unsigned char>*)dstv.stringVal->value;

	if (src->spp != 4 || des->spp != 4) return 0;
	if (desX >= des->width || desY >= des->height) return 0;

	if (srcX < 0) srcX = 0;
	if (srcY < 0) srcY = 0;

	if (srcWidth <= 0) srcWidth = src->width;
	else if (srcWidth > src->width) srcWidth = src->width;
	if (desX+srcWidth >= des->width-1) srcWidth = des->width - desX;

	if (srcHeight <= 0) srcHeight = src->height;
	else if (srcHeight > src->height) srcHeight = src->height;
	if (desY+srcHeight >= des->height-1) srcHeight = des->height - desY;

	drawImage(desX, desY, srcX, srcY, srcWidth, srcHeight, src, des);

	return 1;
}

int Clear (ScriptValue &srcv, ScriptValue &dstv, const int colour)
{
	//printf("Draw %i %i\n", desX, desY);

	GenericImage<unsigned char> *src = (GenericImage<unsigned char>*)srcv.stringVal->value;
	if (src->spp != 4) return 0;

	unsigned int *pixels = (unsigned int*)src->pixels;

	for (int y = 0; y < src->height; y++){
		for (int x = 0; x < src->width; x++)
			pixels[(y*src->width) + x] = colour;
	}
	return 1;
}

int Zoom (ScriptValue &srcv, ScriptValue &dst, double scale)
{
	GenericImage<unsigned char> *src = (GenericImage<unsigned char>*) srcv.stringVal->value;
	if (src->spp != 4) return 0;
	unsigned int outWidth = (unsigned int)(src->width * scale+(0.5));
	unsigned int outHeight =  (unsigned int)(src->height * scale+(0.5));

	if (outWidth < outHeight) {
		if (outWidth < 2) {
			outWidth = 2;
			scale = 1/(double)src->width;
			outHeight =  (unsigned int)(src->height * scale+(0.5));
		}
	}else{
		if (outHeight < 2) {
			outHeight = 2;
			scale = 1/(double)src->height;
			outWidth = (unsigned int)(src->width * scale+(0.5));
		}
	}

	if (scale <1.001 && scale > 0.999) {
		outHeight = src->height;
		outWidth = src->width;
	}

	if (!MakeGenericImage<unsigned char>(dst, outWidth, outHeight, src->spp))
		return 0;

	GenericImage<unsigned char> *out = (GenericImage<unsigned char> *)dst.stringVal->value;
	// return input image.
	if (scale < 1.001 && scale > 0.999) {
		memcpy(out->pixels, src->pixels, out->memWidth*out->height);
		return 1;
	}

	// Just use the sum.
	if (1 /*|| scale < 0.54*/) {
		double xFact = src->width/(double)outWidth;
		double yFact = src->height/(double)outHeight;

		for (unsigned int y = 0; y<outHeight; y++) {
			unsigned int starty = (unsigned int) (y * yFact);
			unsigned int endy = (unsigned int) ((y+1) * yFact);
			if (endy >= src->height) endy = src->height-1;

			for (unsigned int x = 0; x<outWidth; x++) {
				double weightSum = 0;
				double colorSum = 0;
				unsigned int startx = (unsigned int) (x * xFact);
				unsigned int endx = (unsigned int) ((x+1) * xFact);
				if (endx >= src->width) endx = src->width-1;

				for (int c = 0; c<out->spp; c++) {
					double sum = 0;
					double weightSum = 0;

					for (unsigned int y2 = starty; y2 <= endy; y2++) {
						unsigned char *px = &src->pixels[y2 * src->memWidth + out->spp*startx+c];
						double yWeight = 1;

						if (y2 == starty) yWeight = 1 - (y * yFact-starty);
						if (y2 == endy) yWeight = (y+1) * yFact - endy;

						for (unsigned int x2 = startx; x2 <= endx; x2++) {
							double xWeight = 1;
							if (x2 == startx) xWeight = 1 - (x * xFact-startx);
							if (x2 == endx) xWeight = (x+1) * xFact - endx;
							double weight = xWeight * yWeight;
							sum += *px * weight;
							weightSum += weight;
							//out += out->spp;
							px += out->spp;
						}
					}
					out->pixels[y*out->memWidth + x*out->spp + c] = (unsigned char)(sum/weightSum);
				}

			}
		}
	}
	return 1;
}

int LoadMemoryColorImage (ScriptValue &sv, unsigned char *data, int len)
{
	return (LoadPNG(sv, data, len) ||
		LoadJpeg(sv, data, len) ||
		LoadPSD(sv, data, len) ||
		LoadFileBMP(sv, data, len) ||
		LoadTGA(sv, data, len) ||
		LoadGIF(sv, data, len));
}

int LoadMemoryImage (ScriptValue &s, unsigned char *data, int len, double zoom, int cutoff)
{
	ScriptValue sv;
	if (!LoadMemoryColorImage(sv, data, len)) return 0;

	int res = 0;
	ConvertTo1ColorImage(sv, sv);

	if (zoom > 0.00000001) {
		ScriptValue temp;
		if (Zoom(sv, temp, zoom)) {
			sv.Release();
			sv = temp;
		}
	}

	res = Convert1ColorToBitImage(sv, s, cutoff);
	sv.stringVal->Release();

	return res;
}

int LoadBitImage(ScriptValue &s, unsigned char *path, double zoom, int cutoff) {
	int res = 0;
	if (path) {
		wchar_t *path3 = UTF8toUTF16Alloc(path);
		if (!path3) return 0;
		wchar_t *path2 = GetFile(path3);
		free(path3);
		if (!path2) return 0;
		FILE *in = _wfopen(path2, L"rb");
		free(path2);
		if (!in) return 0;

		fseek(in, 0, SEEK_END);
		int len = ftell(in);
		fseek(in, 0, SEEK_SET);
		unsigned char *data = (unsigned char*) malloc(sizeof(unsigned char)*(len));
		if (data) {
			if ((int)fread(data, 1, len, in) == len) {
				res = LoadMemoryImage(s, data, len, zoom, cutoff);
			}
			free(data);
		}
		fclose(in);
	}

	return res;
}


int LoadColorImage(ScriptValue &s, unsigned char *path)
{
	int res = 0;
	if (path) {
		wchar_t *path3 = UTF8toUTF16Alloc(path);
		if (!path3) return 0;

		wchar_t *path2 = GetFile(path3);
		free(path3);
		if (!path2) return 0;

		FILE *in = _wfopen(path2, L"rb");
		free(path2);
		if (!in) return 0;

		fseek(in, 0, SEEK_END);
		int len = ftell(in);
		fseek(in, 0, SEEK_SET);

		unsigned char *data = (unsigned char*)malloc(sizeof(unsigned char)*(len));
		if (data) {
			if ((int)fread(data, 1, len, in) == len) {
				res = LoadMemoryColorImage(s, data, len);
			}
			free(data);
		}
		fclose(in);
	}

	return res;
}

int CreateImage32 (ScriptValue &s, ScriptValue &dst, int width, int height)
{
	if (width < 8) width = 8;
	if (height < 8) height = 8;

	if (!MakeGenericImage<unsigned char>(dst, width, height, 4))
		return 0;

	GenericImage<unsigned char> *out = (GenericImage<unsigned char> *)dst.stringVal->value;
	unsigned int *pixels = (unsigned int*)out->pixels;

	for (int y = 0; y < height; y++){
		for (int x = 0; x < width; x++)
			pixels[(y*width) + x] = 0xFF000000;
	}


	return 1;
}

