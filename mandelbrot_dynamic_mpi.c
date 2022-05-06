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

const int DATA_TAG = 66;
const int RESULT_TAG = 77;
const int TERMINATE_TAG = 88;

void master(int size)
{
    int i, k, count, row;

    int *colors = (int *)malloc(sizeof(int) * (DISPLAY_WIDTH + 1));

    static unsigned char image_data[DISPLAY_HEIGHT][DISPLAY_WIDTH];

    count = 0;
    row = 0;
    for (k = 1; k < size; k++)
    {
        MPI_Send(&row, 1, MPI_INT, k, DATA_TAG, MPI_COMM_WORLD);
        count++;
        row++;
    }
    MPI_Status status;

    do
    {
        MPI_Recv(colors, DISPLAY_WIDTH, MPI_INT, MPI_ANY_SOURCE, RESULT_TAG, MPI_COMM_WORLD, &status);
        count--;

        if (row < DISPLAY_HEIGHT)
        {
            MPI_Send(&row, 1, MPI_INT, status.MPI_SOURCE, DATA_TAG, MPI_COMM_WORLD);
            row++;
            count++;
        }
        else
        {
            MPI_Send(&row, 1, MPI_INT, status.MPI_SOURCE, TERMINATE_TAG, MPI_COMM_WORLD);
        }
        for (i = 0; i < DISPLAY_WIDTH; i++)
        {
            image_data[colors[0]][i] = colors[i + 1];
        }

    } while (count > 0);

    FILE *fp;
    char *filename = "images/dynamic.ppm";
    char *comment = "# "; /* for ppm header */

    fp = fopen(filename, "wb");
    /* write ASCII header to the file */
    /* P6 for color, P5 for greyscale ppm */
    fprintf(fp, "P5\n %s\n %d\n %d\n %d\n", comment, DISPLAY_WIDTH, DISPLAY_HEIGHT, MAX_COLOR_VALUE);
    /* write image data bytes to the file */
    fwrite(image_data, sizeof(image_data), 1, fp);
    fclose(fp);
}

void worker()
{
    int *colors = malloc(sizeof(int) * (DISPLAY_WIDTH + 1));
    MPI_Status status;
    int row, x, rank;

    const double SCALE_REAL = (REAL_MAX - REAL_MIN) / DISPLAY_WIDTH;
    const double SCALE_IMAG = (IMAG_MAX - IMAG_MIN) / DISPLAY_HEIGHT;

    MPI_Recv(&row, 1, MPI_INT, 0, DATA_TAG, MPI_COMM_WORLD, &status);
    struct Complex c;
    while (status.MPI_TAG == DATA_TAG)
    {
        colors[0] = row;
        c.imag = IMAG_MIN + row * SCALE_IMAG;
        for (x = 0; x < DISPLAY_WIDTH; x++)
        {
            c.real = REAL_MIN + x * SCALE_REAL;
            colors[x + 1] = pretty_grey(cal_pixel(c));
        }
        MPI_Send(colors, DISPLAY_WIDTH, MPI_INT, 0, RESULT_TAG, MPI_COMM_WORLD);
        MPI_Recv(&row, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    }
}

int main(int argc, char *argv[])
{
    MPI_Status status;
    int size, rank, row_size;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (size < 2)
    {
        printf("Need at least 2 processes\n");
        return 1;
    }

    if (rank == 0)
    {
        master(size);
    }
    else
    {
        worker();
    }

    MPI_Finalize();
    return 0;
}
