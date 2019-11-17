
#include "psd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "endianswap.h"



#define SWAP16(X)    ((X)=Endian_SwapLE16(X))
#define SWAP32(X)    ((X)=Endian_SwapLE32(X))

#ifdef _MSC_VER
#define STBI_HAS_LRTOL
#endif

#ifdef STBI_HAS_LRTOL
   #define stbi_lrot(x,y)  _lrotl(x,y)
#else
   #define stbi_lrot(x,y)  (((x) << (y)) | ((x) >> (32 - (y))))
#endif


typedef uint8_t			uint8;
typedef uint16_t		uint16;
typedef int16_t			int16;
typedef uint32_t		uint32;
typedef int32_t			int32;
typedef unsigned int	uint;
typedef unsigned char	stbi_uc;



typedef struct{
   uint32 img_x;
   uint32 img_y;
   int img_n;
   int img_out_n;

   FILE *img_file;
   int from_file;
   int buflen;
   uint8 buffer_start[128];
   uint8 *img_buffer;
   uint8 *img_buffer_end;
} stbi;


typedef struct stbi_gif_lzw_struct {
   int16 prefix;
   uint8 first;
   uint8 suffix;
} stbi_gif_lzw;

typedef struct stbi_gif_struct
{
   int w,h;
   stbi_uc *out;                 // output buffer (always 4 components)
   int flags, bgindex, ratio, transparent, eflags;
   uint8  pal[256][4];
   uint8 lpal[256][4];
   stbi_gif_lzw codes[4096];
   uint8 *color_table;
   int parse, step;
   int lflags;
   int start_x, start_y;
   int max_x, max_y;
   int cur_x, cur_y;
   int line_size;
} stbi_gif;


/*


static inline uint64_t freq;
static inline uint64_t tStart;
static double resolution;

static inline void setRes ()
{
	QueryPerformanceCounter((LARGE_INTEGER*)&tStart);
	QueryPerformanceFrequency((LARGE_INTEGER *)&freq);
	resolution = 1.0 / (double)freq;
}


static inline double getTime ()
{
	uint64_t t1 = 0;
	QueryPerformanceCounter((LARGE_INTEGER*)&t1);
	return ((double)((uint64_t)(t1 - tStart) * resolution) * 1000.0);
}
*/
static inline void start_file(stbi *s, FILE *f)
{
   s->img_file = f;
   s->buflen = sizeof(s->buffer_start);
   s->img_buffer_end = s->buffer_start + s->buflen;
   s->img_buffer = s->img_buffer_end;
   s->from_file = 1;
}

static inline void start_mem (stbi *s, uint8 const *buffer, int len)
{
   s->img_file = NULL;
   s->from_file = 0;

   s->img_buffer = (uint8 *) buffer;
   s->img_buffer_end = (uint8 *) buffer+len;
}

static inline void refill_buffer(stbi *s)
{
   int n = fread(s->buffer_start, 1, s->buflen, s->img_file);
   if (n == 0) {
      s->from_file = 0;
      s->img_buffer = s->img_buffer_end-1;
      *s->img_buffer = 0;
   } else {
      s->img_buffer = s->buffer_start;
      s->img_buffer_end = s->buffer_start + n;
   }
}
/*
static inline int at_eof(stbi *s)
{

   if (s->img_file) {
      if (!feof(s->img_file)) return 0;
      // if feof() is true, check if buffer = end
      // special case: we've only got the special 0 character at the end
      if (s->from_file == 0) return 1;
   }

   return s->img_buffer >= s->img_buffer_end;
}*/

static inline int get8 (stbi *s)
{
   if (s->img_buffer < s->img_buffer_end)
      return *s->img_buffer++;

   if (s->from_file) {
      refill_buffer(s);
      return *s->img_buffer++;
   }

   return 0;
}

static inline uint8 get8u (stbi *s)
{
   return (uint8) get8(s);
}

static inline void skip(stbi *s, int n)
{
   if (s->img_file) {
      int blen = s->img_buffer_end - s->img_buffer;
      if (blen < n) {
         s->img_buffer = s->img_buffer_end;
         fseek(s->img_file, n - blen, SEEK_CUR);
         return;
      }
   }
   s->img_buffer += n;
}

static inline int get16(stbi *s)
{
   int z = get8(s);
   return (z << 8) + get8(s);
}

static inline uint32 get32(stbi *s)
{
   uint32 z = get16(s);
   return (z << 16) + get16(s);
}

static inline int get16le(stbi *s)
{
   int z = get8(s);
   return z + (get8(s) << 8);
}

static inline uint32 get32le(stbi *s)
{
   uint32 z = get16le(s);
   return z + (get16le(s) << 16);
}

static inline uint8 compute_y(int r, int g, int b)
{
   return (uint8) (((r*77) + (g*150) +  (29*b)) >> 8);
}

static inline unsigned char *convert_format (unsigned char *data, int img_n, int req_comp, uint x, uint y)
{
   unsigned char *good;

   if (req_comp == img_n) return data;
   //assert(req_comp >= 1 && req_comp <= 4);

   good = (unsigned char *) malloc(req_comp * x * y);
   if (good == NULL) {
      free(data);
      return NULL;
   }

   for (int j=0; j < (int) y; ++j) {
      unsigned char *src  = data + j * x * img_n   ;
      unsigned char *dest = good + j * x * req_comp;

      #define COMBO(a,b)  ((a)*8+(b))
      #define CASE(a,b)   case COMBO(a,b): for(int i=x-1; i >= 0; --i, src += a, dest += b)
      // convert source image with img_n components to one with req_comp components;
      // avoid switch per pixel, so use switch per scanline and massive macros
      switch (COMBO(img_n, req_comp)) {
         CASE(1,2) dest[0]=src[0], dest[1]=255; break;
         CASE(1,3) dest[0]=dest[1]=dest[2]=src[0]; break;
         CASE(1,4) dest[0]=dest[1]=dest[2]=src[0], dest[3]=255; break;
         CASE(2,1) dest[0]=src[0]; break;
         CASE(2,3) dest[0]=dest[1]=dest[2]=src[0]; break;
         CASE(2,4) dest[0]=dest[1]=dest[2]=src[0], dest[3]=src[1]; break;
         CASE(3,4) dest[0]=src[0],dest[1]=src[1],dest[2]=src[2],dest[3]=255; break;
         CASE(3,1) dest[0]=compute_y(src[0],src[1],src[2]); break;
         CASE(3,2) dest[0]=compute_y(src[0],src[1],src[2]), dest[1] = 255; break;
         CASE(4,1) dest[0]=compute_y(src[0],src[1],src[2]); break;
         CASE(4,2) dest[0]=compute_y(src[0],src[1],src[2]), dest[1] = src[3]; break;
         CASE(4,3) dest[0]=src[0],dest[1]=src[1],dest[2]=src[2]; break;
         default: return NULL;//assert(0);
      }
      #undef CASE
   }

   free(data);
   return good;
}


// ########################################################################################################
// ########################################################################################################
// ######################################        PSD          #############################################
// ########################################################################################################
// ########################################################################################################


static inline int psd_getSize (stbi *s, int *w, int *h)
{
   // Check identifier
   if (get32(s) != 0x38425053)   // "8BPS"
      return 0;

   // Check file type version.
   if (get16(s) != 1)
      return 0;

   // Skip 6 reserved bytes.
   skip(s, 6 );

   // Read the number of channels (R, G, B, A, etc).
   int channelCount = get16(s);
   if (channelCount < 0 || channelCount > 16)
      return 0;

   // Read the rows and columns of the image.
   if (h) *h = get32(s);
   if (w) *w = get32(s);

   // Make sure the depth is 8 bits.
   if (get16(s) != 8)
      return 0;

#if 1
   int colourMode = get16(s);
   //printf("colour mode %i\n", colourMode);
   if (colourMode != 3 && colourMode != 4)
      return 0;
#endif

   return 1;
}


static inline stbi_uc *psd_load (stbi *s, int *x, int *y, int *comp, int req_comp)
{
   int   pixelCount;
   int channelCount, compression;
   int channel, i, count, len;
   int w,h;
   uint8 *out;

   // Check identifier
   if (get32(s) != 0x38425053)   // "8BPS"
      return NULL;

   // Check file type version.
   if (get16(s) != 1)
      return NULL;

   // Skip 6 reserved bytes.
   skip(s, 6 );

   // Read the number of channels (R, G, B, A, etc).
   channelCount = get16(s);
   if (channelCount < 0 || channelCount > 16)
      return NULL;

   // Read the rows and columns of the image.
   h = get32(s);
   w = get32(s);

   // Make sure the depth is 8 bits.
   if (get16(s) != 8)
      return NULL;

   // Make sure the color mode is RGB.
   // Valid options are:
   //   0: Bitmap
   //   1: Grayscale
   //   2: Indexed color
   //   3: RGB color
   //   4: CMYK color
   //   7: Multichannel
   //   8: Duotone
   //   9: Lab color
   int colourMode = get16(s);
   //printf("colour mode %i\n", colourMode);
   if (colourMode != 3 && colourMode != 4)
      return NULL;

   // Skip the Mode Data.  (It's the palette for indexed color; other info for other modes.)
   skip(s,get32(s) );

   // Skip the image resources.  (resolution, pen tool paths, etc)
   skip(s, get32(s) );

   // Skip the reserved data.
   skip(s, get32(s) );

   // Find out if the data is compressed.
   // Known values:
   //   0: no compression
   //   1: RLE compressed
   compression = get16(s);
   if (compression > 1)
      return NULL;

   // Create the destination image.
   out = (stbi_uc *) malloc(4 * w*h);
   if (!out) return NULL;
   pixelCount = w*h;

   // Initialize the data to zero.
   //l_memset( out, 0, pixelCount * 4 );

   // Finally, the image data.
   if (compression) {
      // RLE as used by .PSD and .TIFF
      // Loop until you get the number of unpacked bytes you are expecting:
      //     Read the next source byte into n.
      //     If n is between 0 and 127 inclusive, copy the next n+1 bytes literally.
      //     Else if n is between -127 and -1 inclusive, copy the next byte -n+1 times.
      //     Else if n is 128, noop.
      // Endloop

      // The RLE-compressed data is preceeded by a 2-byte data count for each row in the data,
      // which we're going to just skip.
      skip(s, h * channelCount * 2 );

      // Read the RLE data by channel.
      for (channel = 0; channel < 4; channel++) {
         uint8 *p;

         p = out+channel;
         if (channel >= channelCount) {
            // Fill this channel with default data.
            for (i = 0; i < pixelCount; i++) *p = (channel == 3 ? 255 : 0), p += 4;
         } else {
            // Read the RLE data.
            count = 0;
            while (count < pixelCount) {
               len = get8(s);
               if (len == 128) {
                  // No-op.
               } else if (len < 128) {
                  // Copy next len+1 bytes literally.
                  len++;
                  count += len;
                  while (len) {
                     *p = get8u(s);
                     p += 4;
                     len--;
                  }
               } else if (len > 128) {
                  uint8   val;
                  // Next -len+1 bytes in the dest are replicated from next source byte.
                  // (Interpret len as a negative 8-bit int.)
                  len ^= 0x0FF;
                  len += 2;
                  val = get8u(s);
                  count += len;
                  while (len) {
                     *p = val;
                     p += 4;
                     len--;
                  }
               }
            }
         }
      }

   } else {
      // We're at the raw image data.  It's each channel in order (Red, Green, Blue, Alpha, ...)
      // where each channel consists of an 8-bit value for each pixel in the image.

      // Read the data by channel.
      for (channel = 0; channel < 4; channel++) {
         uint8 *p;

         p = out + channel;
         if (channel > channelCount) {
            // Fill this channel with default data.
            for (i = 0; i < pixelCount; i++) *p = channel == 3 ? 255 : 0, p += 4;
         } else {
            // Read the data.
            for (i = 0; i < pixelCount; i++)
               *p = get8u(s), p += 4;
         }
      }
   }
#if 0
   if (req_comp && req_comp != 4) {
      out = convert_format(out, 4, req_comp, w, h);
      if (out == NULL) return out; // convert_format frees input on failure
   }
#endif
   if (comp) *comp = channelCount;
   *y = h;
   *x = w;

   return out;
}

static inline stbi_uc *stbi_psd_load_from_file(FILE *f, int *x, int *y, int *comp, int req_comp)
{
   stbi s;
   start_file(&s, f);
   return psd_load(&s, x,y,comp,req_comp);
}

static inline stbi_uc *stbi_psd_load(wchar_t const *filename, int *x, int *y, int *comp, int req_comp)
{
   stbi_uc *data;
   FILE *f = _wfopen(filename, L"rb");
   if (!f) return NULL;
   data = stbi_psd_load_from_file(f, x,y,comp,req_comp);
   fclose(f);
   return data;
}

static inline stbi_uc *stbi_psd_load_from_memory (stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp)
{
   stbi s;
   start_mem(&s, buffer, len);
   return psd_load(&s, x,y,comp,req_comp);
}

//int loadPsd (uint8_t *desPixels, const int flags, const wchar_t *filename, const int ox, const int oy)
int LoadPSD (ScriptValue &sv, unsigned char *data, int len)
{
	int width = 0, height = 0, comp;

	uint32_t *pixels = (uint32_t*)stbi_psd_load_from_memory(data, len, &width, &height, &comp, 4);
	if (!pixels) return 0;

	if (!MakeGenericImage<unsigned char>(sv, width, height, 4)){
		free(pixels);
		return 0;
	}
	
	GenericImage<unsigned char> *image = (GenericImage<unsigned char> *)sv.stringVal->value;
	//memcpy(image->pixels, pixels, width*height*4);

	uint8_t *des = (uint8_t*)image->pixels;
	uint8_t *src = (uint8_t*)pixels;

	int tpixels = width * height;
	while(tpixels--){
		*des++ = src[2];
		*des++ = src[1];
		*des++ = src[0];
		*des++ = src[3];
		src += 4;
	}


	free(pixels);
	return 1;
}


