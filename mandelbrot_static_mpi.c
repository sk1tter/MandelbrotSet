#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "mpi.h"
#include "mandelbrot.h"

const int DISPLAY_WIDTH = 400;
const int DISPLAY_HEIGHT = 400;

/* complex number plane */
const double REAL_MIN = -2.0;
const double REAL_MAX = 2.0;
const double IMAG_MIN = -2.0;
const double IMAG_MAX = 2.0;

const int MAX_COLOR_VALUE = 255;

const int TAG = 122;

void master(int size, int row_size)
{
    int i, row;
    int *color_data = (int *)malloc(3 * sizeof(int));

    static unsigned char image_data[DISPLAY_HEIGHT][DISPLAY_WIDTH];

    for (i = 1, row = 0; i < size; i++, row += row_size)
    {
        MPI_Send(&row, 1, MPI_INT, i, TAG, MPI_COMM_WORLD);
    }
    for (i = 0; i < DISPLAY_HEIGHT * DISPLAY_WIDTH; i++)
    {
        MPI_Recv(color_data, 3, MPI_INT, MPI_ANY_SOURCE, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        image_data[color_data[0]][color_data[1]] = color_data[2];
    }

    FILE *fp;
    char *filename = "images/static.ppm";
    char *comment = "# "; /* for ppm header */

    fp = fopen(filename, "wb");
    /* write ASCII header to the file */
    /* P6 for color, P5 for greyscale ppm */
    fprintf(fp, "P5\n %s\n %d\n %d\n %d\n", comment, DISPLAY_WIDTH, DISPLAY_HEIGHT, MAX_COLOR_VALUE);
    /* write image data bytes to the file */
    fwrite(image_data, sizeof(image_data), 1, fp);
    fclose(fp);
}

void worker(int row_size)
{
    int x, y;
    struct Complex c;

    int row;
    int *color_data = (int *)malloc(3 * sizeof(int));

    const double SCALE_REAL = (REAL_MAX - REAL_MIN) / DISPLAY_WIDTH;
    const double SCALE_IMAG = (IMAG_MAX - IMAG_MIN) / DISPLAY_HEIGHT;

    MPI_Recv(&row, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    for (x = 0; x < DISPLAY_WIDTH; x++)
    {
        c.real = REAL_MIN + x * SCALE_REAL;
        for (y = row; (y < row + row_size) && (y < DISPLAY_HEIGHT); y++)
        {
            c.imag = IMAG_MIN + y * SCALE_IMAG;
            color_data[0] = y;
            color_data[1] = x;

            color_data[2] = pretty_grey(cal_pixel(c));
            MPI_Send(color_data, 3, MPI_INT, 0, TAG, MPI_COMM_WORLD);
        }
    }
}

int main(int argc, char *argv[])
{
    int size, rank, row_size;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (size < 2)
    {
        printf("Need at least 2 processes\n");
        return 1;
    }

    row_size = (DISPLAY_HEIGHT + size - 2) / (size - 1);

    if (rank == 0)
    {
        master(size, row_size);
    }
    else
    {
        worker(row_size);
    }

    MPI_Finalize();
    return 0;
}
