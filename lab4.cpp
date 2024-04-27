#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <png.h>
#include <vector> 
//#include<opencv2/highgui/highgui.hpp>
//#include<opencv2/imgproc/imgproc.hpp>
#include <fstream>
#include <cmath>

//#define _USE_MATH_DEFINES
//#include <math.h>


using namespace std;
//using namespace cv;


int width, height;
png_byte color_type;
png_byte bit_depth;
png_infop info_ptr;
png_structp png_ptr;



png_bytepp read_png(const char* file_name)
{
    FILE* fp = fopen(file_name, "rb");
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    info_ptr = png_create_info_struct(png_ptr);
    png_init_io(png_ptr, fp);
    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
    png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);
    width = png_get_image_width(png_ptr, info_ptr);
    height = png_get_image_height(png_ptr, info_ptr);
    color_type = png_get_color_type(png_ptr, info_ptr);
    bit_depth = png_get_bit_depth(png_ptr, info_ptr);
    png_destroy_read_struct(&png_ptr, NULL, NULL);

    if (color_type != PNG_COLOR_TYPE_GRAY or bit_depth != 8) {
        std::cout << "Images must be gray-scale and 8 bpp!" << std::endl;
        abort();
    }

    fclose(fp);

    return row_pointers;
}


void write_png(char* file_name, png_bytepp row_pointers)
{
    FILE* fp = fopen(file_name, "wb");
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_init_io(png_ptr, fp);
    png_set_rows(png_ptr, info_ptr, row_pointers);
    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);
}


void popularity(png_bytepp row_pointers, int n, int * most_frequencies)
{
    int frequencies[256];
    for (int i = 0; i < 256; i++)
        frequencies[i] = 0;

    for (int i = 0; i < height; i++) {
        png_bytep row = row_pointers[i];
        for (int j = 0; j < width; j++) {
            png_bytep pxl = &(row[j]);
            frequencies[(int)(pxl[0])]++;
        }
    }

    for (int i = 0; i < n; i++)
        most_frequencies[i] = i;

    for (int i = n; i < 256; i++) {
        int k = 0;
        int min_count = frequencies[most_frequencies[0]];

        for (int j = 1; j < n; j++)
            if (min_count > frequencies[most_frequencies[j]]) {
                min_count = frequencies[most_frequencies[j]];
                k = j;
            }

        if (min_count < frequencies[i])
            most_frequencies[k] = i;

    }

    for (int i = 0; i < n; i++)
        cout << most_frequencies[i] << ' '; //= frequencies[most_frequencies[i]];
    cout << endl;
}


void uniform(int n, int* most_frequencies)
{
    for (int i = 0; i < n; i++)
        most_frequencies[i] = round(255 * i / n);
}

int nearest_val(int old_val, int n, int* most_frequencies)
{
    int k = 0;
    int delta = abs(old_val - most_frequencies[0]);

    for (int i = 1; i < n; i++) {
        if (abs(most_frequencies[i] - old_val) < delta) {
            k = i;
            delta = abs(most_frequencies[i] - old_val);
        }
    }


    return most_frequencies[k];
}


void floyd_steinberg(png_bytepp row_pointers, int n, int* most_frequencies)
{
    int new_val = -1;

    int* next_line_errors = new int[width];
    for (int i = 0; i < width; i++)
        next_line_errors[i] = 0;

    for (int y = 0; y < height - 1; y++) {
        double next_pixel_error = 0;

        int* new_next_line_errors = new int[width];
        for (int i = 0; i < width; i++)
            new_next_line_errors[i] = 0;

        png_bytep row = row_pointers[y];

        png_bytep pxl_val = &(row[0]);
        int tmp_val = pxl_val[0] + next_line_errors[0];
        new_val = nearest_val(tmp_val, n, most_frequencies);
        next_pixel_error = (tmp_val - new_val) * 7 / 16;
        pxl_val[0] = new_val;

        for (int x = 1; x < width - 1; x++) {
            pxl_val = &(row[x]);
            tmp_val = pxl_val[0] + next_line_errors[x] + next_pixel_error;
            new_val = nearest_val(tmp_val, n, most_frequencies);
            next_pixel_error = (tmp_val - new_val) * 7 / 16;

            new_next_line_errors[x - 1] += (tmp_val - new_val) * 3 / 16;
            new_next_line_errors[x] += (tmp_val - new_val) * 5 / 16;
            new_next_line_errors[x + 1] += (tmp_val - new_val) * 1 / 16;

            pxl_val[0] = new_val;
        }

        pxl_val = &(row[width - 1]);
        tmp_val = pxl_val[0] + next_line_errors[width - 1] + next_pixel_error;
        new_val = nearest_val(tmp_val, n, most_frequencies);
        pxl_val[0] = new_val;

        delete[] next_line_errors;
        next_line_errors = new_next_line_errors;
    }


    cout << new_val << endl;
}


int main(int argc, char* argv[])
{
    char path[] = "C:\\Users\\selib\\OneDrive\\Рабочий стол\\Обработка фотокарточек\\Labs\\Фотокарточки\\baboon.png";
    char out_path[] = "C:\\Users\\selib\\OneDrive\\Рабочий стол\\Обработка фотокарточек\\Labs\\Фотокарточки\\4-out.png";

    png_bytepp row_pointers = read_png(path);

    int bpp = 2;
    int n = pow(2, bpp);
    int* most_frequencies = new int[n]; // n интенсивностей

    //popularity(row_pointers, n, most_frequencies);
    uniform(n, most_frequencies);

    floyd_steinberg(row_pointers, n, most_frequencies);


    write_png(out_path, row_pointers);

    return 0;
}
