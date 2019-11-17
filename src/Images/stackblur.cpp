// Stack Blur v1.0
//
// Author: Mario Klingemann <mario@quasimondo.com>
// http://incubator.quasimondo.com
// created Feburary 29, 2004
//
// This is a compromise between Gaussian Blur and Box blur
// It creates much better looking blurs than Box Blur, but is
// 7x faster than my Gaussian Blur implementation.
//
// I called it Stack Blur because this describes best how this
// filter works internally: it creates a kind of moving stack
// of colors whilst scanning through the image. Thereby it
// just has to add one new block of color to the right side
// of the stack and remove the leftmost color. The remaining
// colors on the topmost layer of the stack are either added on
// or reduced by one, depending on if they are on the right or
// on the left side of the stack.
//
// If you are using this algorithm in your code please add
// the following line:
//
// Stack Blur Algorithm by Mario Klingemann <mario@quasimondo.com>
//
// Code change with CUDA by Michael <lioucr@hotmail.com>
// http://home.so-net.net.tw/lioucy
//
#include <stdlib.h>


static unsigned int stack_blur8_mul[] =
{
    512,512,456,512,328,456,335,512,405,328,271,456,388,335,292,512,
    454,405,364,328,298,271,496,456,420,388,360,335,312,292,273,512,
    482,454,428,405,383,364,345,328,312,298,284,271,259,496,475,456,
    437,420,404,388,374,360,347,335,323,312,302,292,282,273,265,512,
    497,482,468,454,441,428,417,405,394,383,373,364,354,345,337,328,
    320,312,305,298,291,284,278,271,265,259,507,496,485,475,465,456,
    446,437,428,420,412,404,396,388,381,374,367,360,354,347,341,335,
    329,323,318,312,307,302,297,292,287,282,278,273,269,265,261,512,
    505,497,489,482,475,468,461,454,447,441,435,428,422,417,411,405,
    399,394,389,383,378,373,368,364,359,354,350,345,341,337,332,328,
    324,320,316,312,309,305,301,298,294,291,287,284,281,278,274,271,
    268,265,262,259,257,507,501,496,491,485,480,475,470,465,460,456,
    451,446,442,437,433,428,424,420,416,412,408,404,400,396,392,388,
    385,381,377,374,370,367,363,360,357,354,350,347,344,341,338,335,
    332,329,326,323,320,318,315,312,310,307,304,302,299,297,294,292,
    289,287,285,282,280,278,275,273,271,269,267,265,263,261,259
};

static unsigned int stack_blur8_shr[] =
{
     9, 11, 12, 13, 13, 14, 14, 15, 15, 15, 15, 16, 16, 16, 16, 17,
    17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 18, 19,
    19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 21,
    21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
    21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22,
    22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
    22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 23,
    23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
    23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
    23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
    23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24
};

enum enumColorOrder
{
    R,
    G,
    B,
    A
};

#ifdef _CALCU_DIFF_TIME
extern float gm_diffTime;
#endif

//////////////////////////////////////////////////////////////////////
//extern "C"
void stack_blur_rgba32(unsigned long* pImage, unsigned w, unsigned h, unsigned r)
{
    unsigned x = 0, y = 0, xp = 0, yp = 0, i = 0, t = 0;
    unsigned stack_ptr = 0;
    unsigned stack_start = 0;

    unsigned char* src_pix_ptr = NULL;
    unsigned char* dst_pix_ptr = NULL;
    unsigned char* stack_pix_ptr = NULL;
    unsigned long* stack_data_ptr = NULL;
    unsigned long* lpStack = NULL;

    unsigned sum_r = 0;
    unsigned sum_g = 0;
    unsigned sum_b = 0;
    unsigned sum_a = 0;
    unsigned sum_in_r = 0;
    unsigned sum_in_g = 0;
    unsigned sum_in_b = 0;
    unsigned sum_in_a = 0;
    unsigned sum_out_r = 0;
    unsigned sum_out_g = 0;
    unsigned sum_out_b = 0;
    unsigned sum_out_a = 0;

    unsigned wm  = (w - 1);
    unsigned hm  = (h - 1);
    unsigned div = 0;
    unsigned mul_sum = 0;
    unsigned shr_sum = 0;
    unsigned row_addr = 0;
    unsigned stride = 0;
#ifdef _CALCU_DIFF_TIME
    LARGE_INTEGER start_time;
    LARGE_INTEGER end_time;
#endif

    if ((NULL == pImage) || (w < 1 || h < 1 || r < 1))
    {
        return;
    }
    else if (r > 254)
    {
        r = 254;
    }

    mul_sum = stack_blur8_mul[r];
    shr_sum = stack_blur8_shr[r];

    div = ((r + r) + 1);

    lpStack = new unsigned long[ div ];
    stack_data_ptr = lpStack;
    y = 0;

#ifdef _CALCU_DIFF_TIME
    ::QueryPerformanceCounter((LARGE_INTEGER*)&start_time);
#endif

    do
    {
        sum_r =
        sum_g =
        sum_b =
        sum_a =
        sum_in_r =
        sum_in_g =
        sum_in_b =
        sum_in_a =
        sum_out_r =
        sum_out_g =
        sum_out_b =
        sum_out_a = 0;

        row_addr = (y * w);
        src_pix_ptr = (unsigned char*)(pImage + row_addr);
        i = 0;

        do
        {
            t = (i + 1);

            stack_pix_ptr = (unsigned char*)(stack_data_ptr + i);

            *(stack_pix_ptr + R) = *(src_pix_ptr + R);
            *(stack_pix_ptr + G) = *(src_pix_ptr + G);
            *(stack_pix_ptr + B) = *(src_pix_ptr + B);
            *(stack_pix_ptr + A) = *(src_pix_ptr + A);

            sum_r += (*(stack_pix_ptr + R) * t);
            sum_g += (*(stack_pix_ptr + G) * t);
            sum_b += (*(stack_pix_ptr + B) * t);
            sum_a += (*(stack_pix_ptr + A) * t);

            sum_out_r += *(stack_pix_ptr + R);
            sum_out_g += *(stack_pix_ptr + G);
            sum_out_b += *(stack_pix_ptr + B);
            sum_out_a += *(stack_pix_ptr + A);

            if (i > 0)
            {
                t = (r + 1 - i);

                if (i <= wm)
                {
                    src_pix_ptr += 4;
                }

                stack_pix_ptr = (unsigned char*)(stack_data_ptr + (i + r));

                *(stack_pix_ptr + R) = *(src_pix_ptr + R);
                *(stack_pix_ptr + G) = *(src_pix_ptr + G);
                *(stack_pix_ptr + B) = *(src_pix_ptr + B);
                *(stack_pix_ptr + A) = *(src_pix_ptr + A);

                sum_r += (*(stack_pix_ptr + R) * t);
                sum_g += (*(stack_pix_ptr + G) * t);
                sum_b += (*(stack_pix_ptr + B) * t);
                sum_a += (*(stack_pix_ptr + A) * t);

                sum_in_r += *(stack_pix_ptr + R);
                sum_in_g += *(stack_pix_ptr + G);
                sum_in_b += *(stack_pix_ptr + B);
                sum_in_a += *(stack_pix_ptr + A);
            }
        }
        while(++i <= r);

        stack_ptr = r;
        xp = r;

        if (xp > wm)
        {
            xp = wm;
        }

        src_pix_ptr = (unsigned char*)(pImage + (xp + row_addr));
        dst_pix_ptr = (unsigned char*)(pImage + row_addr);
        x = 0;

        do
        {
            *(dst_pix_ptr + R) = ((sum_r * mul_sum) >> shr_sum);
            *(dst_pix_ptr + G) = ((sum_g * mul_sum) >> shr_sum);
            *(dst_pix_ptr + B) = ((sum_b * mul_sum) >> shr_sum);
            *(dst_pix_ptr + A) = ((sum_a * mul_sum) >> shr_sum);

            dst_pix_ptr += 4;

            sum_r -= sum_out_r;
            sum_g -= sum_out_g;
            sum_b -= sum_out_b;
            sum_a -= sum_out_a;

            stack_start = (stack_ptr + div - r);

            if (stack_start >= div)
            {
                stack_start -= div;
            }

            stack_pix_ptr = (unsigned char*)(stack_data_ptr + stack_start);

            sum_out_r -= *(stack_pix_ptr + R);
            sum_out_g -= *(stack_pix_ptr + G);
            sum_out_b -= *(stack_pix_ptr + B);
            sum_out_a -= *(stack_pix_ptr + A);

            if (xp < wm)
            {
                src_pix_ptr += 4;
                ++xp;
            }

            *(stack_pix_ptr + R) = *(src_pix_ptr + R);
            *(stack_pix_ptr + G) = *(src_pix_ptr + G);
            *(stack_pix_ptr + B) = *(src_pix_ptr + B);
            *(stack_pix_ptr + A) = *(src_pix_ptr + A);

            sum_in_r += *(stack_pix_ptr + R);
            sum_in_g += *(stack_pix_ptr + G);
            sum_in_b += *(stack_pix_ptr + B);
            sum_in_a += *(stack_pix_ptr + A);

            sum_r += sum_in_r;
            sum_g += sum_in_g;
            sum_b += sum_in_b;
            sum_a += sum_in_a;

            if (++stack_ptr >= div)
            {
                stack_ptr = 0;
            }

            stack_pix_ptr = (unsigned char*)(stack_data_ptr + stack_ptr);

            sum_out_r += *(stack_pix_ptr + R);
            sum_out_g += *(stack_pix_ptr + G);
            sum_out_b += *(stack_pix_ptr + B);
            sum_out_a += *(stack_pix_ptr + A);

            sum_in_r -= *(stack_pix_ptr + R);
            sum_in_g -= *(stack_pix_ptr + G);
            sum_in_b -= *(stack_pix_ptr + B);
            sum_in_a -= *(stack_pix_ptr + A);
        }
        while(++x < w);
    }
    while(++y < h);

    stride = (w << 2);
    stack_data_ptr = lpStack;
    x = 0;

    do
    {
        sum_r =
        sum_g =
        sum_b =
        sum_a =
        sum_in_r =
        sum_in_g =
        sum_in_b =
        sum_in_a =
        sum_out_r =
        sum_out_g =
        sum_out_b =
        sum_out_a = 0;

        src_pix_ptr = (unsigned char*)(pImage + x);
        i = 0;

        do
        {
            t = (i + 1);

            stack_pix_ptr = (unsigned char*)(stack_data_ptr + i);

            *(stack_pix_ptr + R) = *(src_pix_ptr + R);
            *(stack_pix_ptr + G) = *(src_pix_ptr + G);
            *(stack_pix_ptr + B) = *(src_pix_ptr + B);
            *(stack_pix_ptr + A) = *(src_pix_ptr + A);

            sum_r += (*(stack_pix_ptr + R) * t);
            sum_g += (*(stack_pix_ptr + G) * t);
            sum_b += (*(stack_pix_ptr + B) * t);
            sum_a += (*(stack_pix_ptr + A) * t);

            sum_out_r += *(stack_pix_ptr + R);
            sum_out_g += *(stack_pix_ptr + G);
            sum_out_b += *(stack_pix_ptr + B);
            sum_out_a += *(stack_pix_ptr + A);

            if (i > 0)
            {
                t = (r + 1 - i);

                if (i <= hm)
                {
                    src_pix_ptr += stride;
                }

                stack_pix_ptr = (unsigned char*)(stack_data_ptr + (i + r));

                *(stack_pix_ptr + R) = *(src_pix_ptr + R);
                *(stack_pix_ptr + G) = *(src_pix_ptr + G);
                *(stack_pix_ptr + B) = *(src_pix_ptr + B);
                *(stack_pix_ptr + A) = *(src_pix_ptr + A);

                sum_r += (*(stack_pix_ptr + R) * t);
                sum_g += (*(stack_pix_ptr + G) * t);
                sum_b += (*(stack_pix_ptr + B) * t);
                sum_a += (*(stack_pix_ptr + A) * t);

                sum_in_r += *(stack_pix_ptr + R);
                sum_in_g += *(stack_pix_ptr + G);
                sum_in_b += *(stack_pix_ptr + B);
                sum_in_a += *(stack_pix_ptr + A);
            }
        }
        while(++i <= r);

        stack_ptr = r;
        yp = r;

        if (yp > hm)
        {
            yp = hm;
        }

        src_pix_ptr = (unsigned char*)(pImage + (x + (yp * w)));
        dst_pix_ptr = (unsigned char*)(pImage + x);
        y = 0;

        do
        {
            *(dst_pix_ptr + R) = ((sum_r * mul_sum) >> shr_sum);
            *(dst_pix_ptr + G) = ((sum_g * mul_sum) >> shr_sum);
            *(dst_pix_ptr + B) = ((sum_b * mul_sum) >> shr_sum);
            *(dst_pix_ptr + A) = ((sum_a * mul_sum) >> shr_sum);

            dst_pix_ptr += stride;

            sum_r -= sum_out_r;
            sum_g -= sum_out_g;
            sum_b -= sum_out_b;
            sum_a -= sum_out_a;

            stack_start = (stack_ptr + div - r);
            if (stack_start >= div)
            {
                stack_start -= div;
            }

            stack_pix_ptr = (unsigned char*)(stack_data_ptr + stack_start);

            sum_out_r -= *(stack_pix_ptr + R);
            sum_out_g -= *(stack_pix_ptr + G);
            sum_out_b -= *(stack_pix_ptr + B);
            sum_out_a -= *(stack_pix_ptr + A);

            if (yp < hm)
            {
                src_pix_ptr += stride;
                ++yp;
            }

            *(stack_pix_ptr + R) = *(src_pix_ptr + R);
            *(stack_pix_ptr + G) = *(src_pix_ptr + G);
            *(stack_pix_ptr + B) = *(src_pix_ptr + B);
            *(stack_pix_ptr + A) = *(src_pix_ptr + A);

            sum_in_r += *(stack_pix_ptr + R);
            sum_in_g += *(stack_pix_ptr + G);
            sum_in_b += *(stack_pix_ptr + B);
            sum_in_a += *(stack_pix_ptr + A);

            sum_r += sum_in_r;
            sum_g += sum_in_g;
            sum_b += sum_in_b;
            sum_a += sum_in_a;

            if (++stack_ptr >= div)
            {
                stack_ptr = 0;
            }

            stack_pix_ptr = (unsigned char*)(stack_data_ptr + stack_ptr);

            sum_out_r += *(stack_pix_ptr + R);
            sum_out_g += *(stack_pix_ptr + G);
            sum_out_b += *(stack_pix_ptr + B);
            sum_out_a += *(stack_pix_ptr + A);

            sum_in_r -= *(stack_pix_ptr + R);
            sum_in_g -= *(stack_pix_ptr + G);
            sum_in_b -= *(stack_pix_ptr + B);
            sum_in_a -= *(stack_pix_ptr + A);
        }
        while(++y < h);
    }
    while(++x < w);

#ifdef _CALCU_DIFF_TIME
    ::QueryPerformanceCounter((LARGE_INTEGER*)&end_time);
    gm_diffTime = (float)(((double)end_time.QuadPart - (double)start_time.QuadPart) / 1000000);
#endif

    delete lpStack;
    lpStack = NULL;
}
