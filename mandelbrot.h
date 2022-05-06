#include <stdio.h>
#include <math.h>

const int MAX_ITER = 256;

struct Complex
{
    double real, imag;
};

int cal_pixel(struct Complex c)
{
    double z_real, z_imag;
    double z_real_2, z_imag_2;

    int count;
    
    z_real = 0.0;
    z_imag = 0.0;
    z_real_2 = z_real * z_real;
    z_imag_2 = z_imag * z_imag;

    for (count = 0; count < MAX_ITER && ((z_real_2 + z_imag_2) < 4.0); count++)
    {
        z_imag = 2 * z_real * z_imag + c.imag;
        z_real = z_real_2 - z_imag_2 + c.real;

        z_real_2 = z_real * z_real;
        z_imag_2 = z_imag * z_imag;
    };
    return count;
}

int pretty_grey(int count)
{
    /* Better looking grey color scheme */
    return ceil(MAX_ITER * sqrt((double)count / (double)MAX_ITER));
}
