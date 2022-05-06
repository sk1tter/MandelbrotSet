#include <stdio.h>
#include <math.h>
#include "mandelbrot.h"

const int DISPLAY_WIDTH = 400;
const int DISPLAY_HEIGHT = 400;

/* complex number plane */
const double REAL_MIN = -2.0;
const double REAL_MAX = 2.0;
const double IMAG_MIN = -2.0;
const double IMAG_MAX = 2.0;

const int MAX_COLOR_VALUE = 255;

int main()
{

  int x, y;

  struct Complex c;

  const double SCALE_REAL = (REAL_MAX - REAL_MIN) / DISPLAY_WIDTH;
  const double SCALE_IMAG = (IMAG_MAX - IMAG_MIN) / DISPLAY_HEIGHT;

  FILE *fp;
  char *filename = "images/sequential.ppm";
  char *comment = "# "; /* for ppm header */

  /* use image_data[DISPLAY_HEIGHT][DISPLAY_WIDTH][3] for RGB color; */
  static unsigned char image_data[DISPLAY_HEIGHT][DISPLAY_WIDTH];

  for (x = 0; x < DISPLAY_WIDTH; x++)
  {
    c.real = REAL_MIN + x * SCALE_REAL;

    for (y = 0; y < DISPLAY_HEIGHT; y++)
    {
      c.imag = IMAG_MIN + y * SCALE_IMAG;

      image_data[y][x] = pretty_grey(cal_pixel(c));
    }
  }

  fp = fopen(filename, "wb");
  /* write ASCII header to the file */
  /* P6 for color, P5 for greyscale ppm */
  fprintf(fp, "P5\n %s\n %d\n %d\n %d\n", comment, DISPLAY_WIDTH, DISPLAY_HEIGHT, MAX_COLOR_VALUE);
  /* write image data bytes to the file */
  fwrite(image_data, sizeof(image_data), 1, fp);
  fclose(fp);
  return 0;
}
