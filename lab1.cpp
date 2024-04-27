#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <png.h>

//#define _CRT_SECURE_NO_DEPRECATE
//#define _CRT_SECURE_NO_WARNINGS


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
    }

    fclose(fp);

    return row_pointers;
}


void blending(png_bytepp row_pointers1, png_bytepp row_pointers2, png_bytepp row_pointers3) {
    for (int y = 0; y < height; y++) {
        png_bytep row1 = row_pointers1[y];
        png_bytep row2 = row_pointers2[y];
        png_bytep row3 = row_pointers3[y];
        for (int x = 0; x < width; x++) {
            png_bytep pxl1 = &(row1[x]);
            png_bytep pxl2 = &(row2[x]);
            png_bytep pxl3 = &(row3[x]);

            png_byte result_pxl = (png_byte)(int)((1 - (int)pxl3[0] / 255) * (int)pxl3[0] * (int)pxl1[0] + \
                (1 - (int)pxl3[0] / 255) * (int)pxl3[0] * (int)pxl2[0] + (int)pxl3[0] * (int)pxl3[0] * (int)pxl1[0]);

            pxl1[0] = result_pxl;
        }
    }
}


void mirror(png_bytepp row_pointers, char mode) {
    if (mode != 'h' && mode != 'v') {
        std::cout << "Parameter \"mode\" must be letter \"h\" or letter \"v\"!" << std::endl;
        std::abort();
    }

    if (mode == 'v') {
        for (int y = 0; y < height; y++) {
            png_bytep row = row_pointers[y];

            for (int x = 0; x < width / 2; x++) {
                png_byte left_pxl = row[x];

                row[x] = row[width - x - 1];
                row[width - x - 1] = left_pxl;
            }
        }
    }
    else {
        for (int y = 0; y < height / 2; y++) {
            png_bytep row = row_pointers[y];

            row_pointers[y] = row_pointers[height - y - 1];
            row_pointers[height - y - 1] = row;
        }
    }
}


void transpose(png_bytepp row_pointers) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width / 2; x++) {
            png_byte tmp = row_pointers[y][x];
            row_pointers[y][x] = row_pointers[x][y];
            row_pointers[x][y] = tmp;
        }
    }
}


void write_png(char* file_name, png_bytepp row_pointers)
{
    FILE* fp = fopen(file_name, "wb");
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_init_io(png_ptr, fp);
    png_set_rows(png_ptr, info_ptr, row_pointers);
    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
    /*png_destroy_write_struct(&png_ptr, &info_ptr);*/
    fclose(fp);
}


int main(int argc, char* argv[]) 
{
    char path1[]  = "C:\\Users\\selib\\OneDrive\\Рабочий стол\\Обработка фотокарточек\\Labs\\Фотокарточки\\1-1.png";
    char path2[] = "C:\\Users\\selib\\OneDrive\\Рабочий стол\\Обработка фотокарточек\\Labs\\Фотокарточки\\1-2.png";
    char path3[] = "C:\\Users\\selib\\OneDrive\\Рабочий стол\\Обработка фотокарточек\\Labs\\Фотокарточки\\1-3.png";
    char path4[] = "C:\\Users\\selib\\OneDrive\\Рабочий стол\\Обработка фотокарточек\\Labs\\Фотокарточки\\1-4.png";

    char path_for_blending[] = "C:\\Users\\selib\\OneDrive\\Рабочий стол\\Обработка фотокарточек\\Labs\\Фотокарточки\\1-blending.png";
    char path_for_vertical_mirror[] = "C:\\Users\\selib\\OneDrive\\Рабочий стол\\Обработка фотокарточек\\Labs\\Фотокарточки\\1-vertical_mirror.png";
    char path_for_horizontal_mirror[] = "C:\\Users\\selib\\OneDrive\\Рабочий стол\\Обработка фотокарточек\\Labs\\Фотокарточки\\1-horizontal_mirror.png";
    char path_for_transposition[] = "C:\\Users\\selib\\OneDrive\\Рабочий стол\\Обработка фотокарточек\\Labs\\Фотокарточки\\1-transposed.png";

    png_bytepp row_pointers1 = read_png(path1);
    png_bytepp row_pointers2 = read_png(path2);
    png_bytepp row_pointers3 = read_png(path3);
    png_bytepp row_pointers4 = read_png(path4);

    // data in row_pointers1 is changed after blending function
    blending(row_pointers1, row_pointers2, row_pointers3);
    // data in row_pointers3 is changed after mirror function
    mirror(row_pointers2, 'v');
    // data in row_pointers2 is changed after mirror function
    mirror(row_pointers3, 'h');
    // data in row_pointers4 is changed after transpose function
    transpose(row_pointers4);

    write_png(path_for_blending, row_pointers1);
    write_png(path_for_vertical_mirror, row_pointers2);
    write_png(path_for_horizontal_mirror, row_pointers3);
    write_png(path_for_transposition, row_pointers4);

    png_destroy_write_struct(&png_ptr, &info_ptr);

    return 0;
}
