#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <png.h>
#include <vector> 
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include <fstream>
#include <cmath>

#define _USE_MATH_DEFINES
#include <math.h>


//#define _CRT_SECURE_NO_DEPRECATE
//#define _CRT_SECURE_NO_WARNINGS


using namespace std;
using namespace cv;


int width, height;
png_byte color_type;
png_byte bit_depth;
png_infop info_ptr;
png_structp png_ptr;
int r, c;
int min_x = 10000, min_y = 10000, max_x = -10000, max_y = -10000;


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


void write_png(char* file_name, int** row_pointers, int new_height, int new_width)
{
    FILE* fp = fopen(file_name, "wb");
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_init_io(png_ptr, fp);
    png_infop info_ptr = png_create_info_struct(png_ptr);

    png_set_IHDR(png_ptr, info_ptr, new_width, new_height,
        8, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_write_info(png_ptr, info_ptr);
    png_bytep row = (png_bytep)malloc(new_width * sizeof(png_byte));
    // Write image data
    int x, y;
    for (y = 0; y < new_height; y++) {
        for (x = 0; x < new_width; x++) {
            &(row[x] = (png_byte)row_pointers[y][x]);
        }
        png_write_row(png_ptr, row);
    }

    // End write
    png_write_end(png_ptr, NULL);

    //png_set_rows(png_ptr, info_ptr, (png_bytepp)row_pointers);
    //png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);
}


vector<vector<float>> multiply_square_matrices(vector<vector<float>> a, vector<vector<float>> b) {
    if (a.size() != b.size()) {
        cout << "Wrong shapes in multiply_vector_and_matrix" << endl;
        abort();
    }

    int size = a.size();
    vector<vector<float>> c(a.size(), vector<float>(a[0].size(), 0));

    for (int i = 0; i < size; i++)
        for (int j = 0; j < size; j++)
            for (int k = 0; k < size; k++)
                c[i][j] += a[i][k] * b[k][j];

    return c;
}


vector<float> multiply_vector_and_matrix(vector<float> vec, vector<vector<float>> a) {
    if (vec.size() != a.size()) {
        cout << "Wrong shapes in multiply_vector_and_matrix" << endl;
        abort();
    }
    vector<float> c(a[0].size(), 0);

    for (int i = 0; i < a[0].size(); i++)
        for (int j = 0; j < a.size(); j++)
            c[i] += vec[j] * a[j][i];

    return c;
}


float findDeterminant(vector<vector<float>> matrix3X3) {
    float det = 0;  // here det is the determinant of the matrix.
    for (int r = 0; r < 3; r++) {
        det = det + (matrix3X3[0][r] *
            (matrix3X3[1][(r + 1) % 3] * matrix3X3[2][(r + 2) % 3] -
                matrix3X3[1][(r + 2) % 3] * matrix3X3[2][(r + 1) % 3]));
    }
    return det;
}


vector<vector<float>> inverse(vector<vector<float>> matrix3X3, float d) {
    vector<vector<float>> inverse_matrix(3, vector<float>(3, 0));

    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 3; c++) {
            inverse_matrix[r][c] = ((matrix3X3[(c + 1) % 3][(r + 1) % 3] *
                matrix3X3[(c + 2) % 3][(r + 2) % 3]) -
                (matrix3X3[(c + 1) % 3][(r + 2) % 3] *
                    matrix3X3[(c + 2) % 3][(r + 1) % 3])) / 
                d;
        }
    }

    return inverse_matrix;
}


float DegreetoRadian(float degree) {
    float radian = degree * M_PI / 180;

    return radian;
}


float clipDegree(float degree) {
    float clipped_degree = degree;
    while (clipped_degree > 360)
        clipped_degree -= 360;

    return clipped_degree;
}


int interpolate(int* pIn, int width, int height, int x, int y)
{
    double b[16];
    double f[16];
    double x_grid[5] = { x - 2.0, x - 1.0, x, x + 1.0, x + 2.0 };
    double y_grid[5] = { y - 2.0, y - 1.0, y, y + 1.0, y + 2.0 };
    unsigned char result;

    b[0] = x_grid[1] * x_grid[0] * x_grid[3] * y_grid[1] * y_grid[0] * y_grid[3] / 4.0;
    b[1] = -x_grid[2] * x_grid[0] * x_grid[3] * y_grid[1] * y_grid[0] * y_grid[3] / 4.0;
    b[2] = -x_grid[1] * x_grid[0] * x_grid[3] * y_grid[2] * y_grid[0] * y_grid[3] / 4.0;
    b[3] = x_grid[2] * x_grid[0] * x_grid[3] * y_grid[2] * y_grid[0] * y_grid[3] / 4.0;;
    b[4] = -x_grid[2] * x_grid[1] * x_grid[0] * y_grid[1] * y_grid[0] * y_grid[3] / 12.0;
    b[5] = -x_grid[1] * x_grid[0] * x_grid[3] * y_grid[2] * y_grid[1] * y_grid[0] / 12.0;
    b[6] = x_grid[2] * x_grid[1] * x_grid[0] * y_grid[2] * y_grid[0] * y_grid[3] / 12.0;
    b[7] = x_grid[2] * x_grid[0] * x_grid[3] * y_grid[2] * y_grid[1] * y_grid[0] / 12.0;
    b[8] = x_grid[2] * x_grid[1] * x_grid[3] * y_grid[1] * y_grid[0] * y_grid[3] / 12.0;
    b[9] = x_grid[1] * x_grid[0] * x_grid[3] * y_grid[2] * y_grid[1] * y_grid[3] / 12.0;
    b[10] = x_grid[2] * x_grid[1] * x_grid[0] * y_grid[2] * y_grid[1] * y_grid[0] / 36.0;
    b[11] = -x_grid[2] * x_grid[1] * x_grid[3] * y_grid[2] * y_grid[0] * y_grid[3] / 12.0;
    b[12] = -x_grid[2] * x_grid[0] * x_grid[3] * y_grid[2] * y_grid[1] * y_grid[3] / 12.0;
    b[13] = -x_grid[2] * x_grid[1] * x_grid[3] * y_grid[2] * y_grid[1] * y_grid[0] / 36.0;
    b[14] = -x_grid[2] * x_grid[1] * x_grid[0] * y_grid[2] * y_grid[1] * y_grid[3] / 36.0;
    b[15] = x_grid[2] * x_grid[1] * x_grid[3] * y_grid[2] * y_grid[1] * y_grid[3] / 36.0;

    if (x + 1 > (double)(width - 1) || x + 2 > (double)(width - 1) || x - 1 < 0 || x - 2 < 0 ||
        y + 1 > (double)(height - 1) || y + 2 > (double)(height - 1) || y - 1 < 0 || y - 2 < 0)
    {
        result = 0;
    }
    else
    {

        f[0] = (double)(*(pIn + (size_t)(x)+(size_t)(y)*width));
        for (int i = 0; i < 16; i++)
        {
            f[i] = (double)(*(pIn + (size_t)(x - 1 + i % 4) + (size_t)(y - 1 + i / 4) * width));
        }

        result = (int)(b[0] * f[5] + b[1] * f[9] + b[2] * f[6] + b[3] * f[10] + b[4] * f[1] + b[5] * f[4] + b[6] * f[2] +
            b[7] * f[8] + b[8] * f[13] + b[9] * f[7] + b[10] * f[0] + b[11] * f[14] + b[12] * f[11] + b[13] * f[12] + b[14] * f[3] + b[15] * f[15]);
    }
    return result;
}


int** rotateImage(png_bytepp row_pointers, int angle, float rad, int rows, int cols, int *nWidth, int *nHeight)
{
    //vector<vector<float>> a = { {1,0,0}, {0,1,0}, {-(float)cols / 2,-(float)rows / 2,1} };
    vector<vector<float>> T = { {cos(rad),sin(rad),0}, {-sin(rad),cos(rad),0}, {0,0,1} };
    //vector<vector<float>> C = { {1,0,0}, {0,1,0}, {(float)cols / 2,(float)rows / 2,1} };
    //vector<vector<float>> T = multiply_square_matrices(multiply_square_matrices(a, b), C);
    float determinant = findDeterminant(T);
    vector<vector<float>> inverse_T = inverse(T, determinant);


    vector<float> p1 = { 0,0,1 };
    vector<float> p2 = { (float)cols, 0, 1 };
    vector<float> p3 = { 0, (float)rows, 1 };
    vector<float> p4 = { (float)cols, (float)rows, 1 };

    vector<float> new_p1 = multiply_vector_and_matrix(p1, T);
    vector<float> new_p2 = multiply_vector_and_matrix(p2, T);
    vector<float> new_p3 = multiply_vector_and_matrix(p3, T);
    vector<float> new_p4 = multiply_vector_and_matrix(p4, T);
    
    if (new_p1[0] < min_x)
        min_x = round(new_p1[0]);
    if (new_p2[0] < min_x)
        min_x = round(new_p2[0]);
    if (new_p3[0] < min_x)
        min_x = round(new_p3[0]);
    if (new_p4[0] < min_x)
        min_x = round(new_p4[0]);

    if (new_p1[1] < min_y)
        min_y = round(new_p1[1]);
    if (new_p2[1] < min_y)
        min_y = round(new_p2[1]);
    if (new_p3[1] < min_y)
        min_y = round(new_p3[1]);
    if (new_p4[1] < min_y)
        min_y = round(new_p4[1]);

    if (new_p1[0] > max_x)
        max_x = round(new_p1[0]);
    if (new_p2[0] > max_x)
        max_x = round(new_p2[0]);
    if (new_p3[0] > max_x)
        max_x = round(new_p3[0]);
    if (new_p4[0] > max_x)
        max_x = round(new_p4[0]);

    if (new_p1[1] > max_y)
        max_y = round(new_p1[1]);
    if (new_p2[1] > max_y)
        max_y = round(new_p2[1]);
    if (new_p3[1] > max_y)
        max_y = round(new_p3[1]);
    if (new_p4[1] > max_y)
        max_y = round(new_p4[1]);

    int dx = abs(min_x), dy = abs(min_y);

    int** pixelNew = new int * [max_y + dy];
    for (int i = 0; i < max_y + dy; i++) {
        pixelNew[i] = new int[max_x + dx];
        for (int j = 0; j < max_x + dx; j++)
            pixelNew[i][j] = 0;
    }

    const int size = *nWidth * *nHeight;

    int index = 0;
    int* pIn = new int[size];
    for (int i = 0; i < *nHeight; i++) {
        png_bytep row = row_pointers[i];
        for (int j = 0; j < *nWidth; j++) {
            png_bytep pxl = &(row[j]);
            pIn[index++] = (int)(pxl[0]);
        }
    }

    for (int i = min_y; i < max_y; i++) {
        for (int j = min_x; j < max_x; j++) {
            vector<float> new_p = { (float)j, (float)i, 1 };
            vector<float> old_p = multiply_vector_and_matrix(new_p, inverse_T);
            int x = (int)old_p[0], y = (int)old_p[1];

            if (x < width && y < height && x >= 0 && y >= 0) {
                png_bytep row = row_pointers[y];
                png_bytep pxl = &(row[x]);
                int val = (int)(pxl[0]);

                if (x > 1 and x < width - 2 and y > 1 and y < height - 2) {
                    int val = interpolate(pIn, *nWidth, *nHeight, x, y);
                    if (val > 255)
                        val = 255;
                    else if (val < 0)
                        val = 0;
                }

                

                pixelNew[(int)new_p[1] + dy][(int)new_p[0] + dx] = (int)val;
            }
                
        }
    }

    *nWidth = max_x + dx;
    *nHeight = max_y + dy;

    return pixelNew;
}

int main(int argc, char* argv[])
{
    char path[] = "C:\\Users\\selib\\OneDrive\\Рабочий стол\\Обработка фотокарточек\\Labs\\Фотокарточки\\2-2.png";
    char out_path[] = "C:\\Users\\selib\\OneDrive\\Рабочий стол\\Обработка фотокарточек\\Labs\\Фотокарточки\\3-out.png";

    png_bytepp row_pointers = read_png(path);
    
    // вводим угол (по часовой)
    float alpha = 45;
    //-----

    // против часовой
    alpha = -alpha;

    int nWidth, nHeight;
    int** new_image = rotateImage(row_pointers, alpha, DegreetoRadian(alpha), height, width, &nWidth, &nHeight);

    write_png(out_path, new_image, nHeight, nWidth);

    return 0;
}
