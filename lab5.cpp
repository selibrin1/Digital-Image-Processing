#include <iostream>
#include <png.h>
#include "PngProc.h"

using namespace std;
using namespace NPngProc;


int width, height;
png_byte color_type;
png_byte bit_depth;
png_infop info_ptr;
png_structp png_ptr;


//png_bytepp read_png(const char* file_name)
//{
//    FILE* fp = fopen(file_name, "rb");
//    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
//    info_ptr = png_create_info_struct(png_ptr);
//    png_init_io(png_ptr, fp);
//    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
//    png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);
//    width = png_get_image_width(png_ptr, info_ptr);
//    height = png_get_image_height(png_ptr, info_ptr);
//    color_type = png_get_color_type(png_ptr, info_ptr);
//    bit_depth = png_get_bit_depth(png_ptr, info_ptr);
//    png_destroy_read_struct(&png_ptr, NULL, NULL);
//
//    if (color_type != PNG_COLOR_TYPE_GRAY or bit_depth != 8) {
//        std::cout << "Images must be gray-scale and 8 bpp!" << std::endl;
//        abort();
//    }
//
//    fclose(fp);
//
//    return row_pointers;
//}
//
//
//void write_png_bytepp_to_png(char* file_name, png_bytepp row_pointers)
//{
//    FILE* fp = fopen(file_name, "wb");
//    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
//    png_init_io(png_ptr, fp);
//    png_set_rows(png_ptr, info_ptr, row_pointers);
//    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
//    png_destroy_write_struct(&png_ptr, &info_ptr);
//    fclose(fp);
//}
//
//
//void write_array_to_png(char* file_name, int** row_pointers, int new_height, int new_width)
//{
//    FILE* fp = fopen(file_name, "wb");
//    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
//    png_init_io(png_ptr, fp);
//    png_infop info_ptr = png_create_info_struct(png_ptr);
//
//    png_set_IHDR(png_ptr, info_ptr, new_width, new_height,
//        8, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE,
//        PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
//
//    png_write_info(png_ptr, info_ptr);
//    png_bytep row = (png_bytep)malloc(new_width * sizeof(png_byte));
//
//    int x, y;
//    for (y = 0; y < new_height; y++) {
//        for (x = 0; x < new_width; x++) {
//            row[x] = (png_byte)row_pointers[y][x];
//        }
//        png_write_row(png_ptr, row);
//    }
//    png_write_end(png_ptr, NULL);
//
//    //png_set_rows(png_ptr, info_ptr, (png_bytepp)row_pointers);
//    //png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
//    png_destroy_write_struct(&png_ptr, &info_ptr);
//    fclose(fp);
//}


void convolution2d(unsigned char* pIn, unsigned char* pOut, size_t width, size_t height, double* kernel, int kernelSize)
{
    int half = kernelSize / 2;

    int newWidth = width + 2 * half; // Новая ширина с учетом padding
    int newHeight = height + 2 * half; // Новая высота с учетом padding

    unsigned char* paddedImage = new unsigned char[newWidth * newHeight]; // Создаем новый массив для изображения с padding

    // Заполняем новый массив пустыми значениями
    memset(paddedImage, 0, newWidth * newHeight * sizeof(unsigned char));

    // Копируем исходное изображение в середину нового массива с учетом padding
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            paddedImage[(y + half) * newWidth + (x + half)] = pIn[y * width + x];
        }
    }

    // Заполняем верхнюю и нижнюю границы паддинга зеркальными строками
    for (int i = 0; i < half; ++i) {
        // Верхняя граница
        memcpy(paddedImage + (half - i) * newWidth + half,
            pIn + (i+1) * width, width * sizeof(unsigned char));
        // Нижняя граница
        memcpy(paddedImage + (height + half + i) * newWidth + half,
            pIn + (height - 1 - i) * width, width * sizeof(unsigned char));
    }


    // Заполняем левую и правую границы паддинга зеркальными столбцами
    for (int y = half; y < newHeight - half; ++y) {
        for (int i = 0; i < half; ++i) {
            // Левая граница
            paddedImage[y * newWidth + i] = paddedImage[y * newWidth + 2 * half - i];
            // Правая граница
            paddedImage[y * newWidth + newWidth - half + i - 1] = paddedImage[y * newWidth + newWidth - half - i - 1];
        }
    }

    // убираем полосу
    for (int y = half; y < newHeight - half; ++y)
        paddedImage[y * newWidth + newWidth - half + half - 1] = paddedImage[y * newWidth + newWidth - half - half - 1];

    // Заполняем угловые квадратики паддинга зеркальными значениями
    for (int i = 0; i < half; ++i) {
        for (int j = 0; j < half; ++j) {
            // Верхний левый угол
            paddedImage[(half - i) * newWidth + (half - j)] = paddedImage[(half + i) * newWidth + (half + j)];
            // Верхний правый угол
            paddedImage[(half - i) * newWidth + (width + half + j)] = paddedImage[(half + i) * newWidth + (width + half - 1 - j)];
            // Нижний левый угол
            paddedImage[(height + half + i) * newWidth + (half - j)] = paddedImage[(height + half - 1 - i) * newWidth + (half + j)];
            // Нижний правый угол
            paddedImage[(height + half + i) * newWidth + (width + half + j)] = paddedImage[(height + half - 1 - i) * newWidth + (width + half - 1 - j)];
        }
    }

    for (int i = 0; i < half; ++i) {
        // устраняем оставшиеся полосы
        // 
        // Верхний левый угол
        paddedImage[(half - i) * newWidth + 0] = paddedImage[(half + i) * newWidth + (half + 0)];
        // Нижний левый угол
        paddedImage[(height + half + i) * newWidth + 0] = paddedImage[(height + half - 1 - i) * newWidth + 0];
    }

    //paddedImage[(height + half + i) * newWidth + (half - j)] = paddedImage[(height + half - 1 - i) * newWidth + (half + j)];


    for (int y = half; y < newHeight - half; y++) {
        for (int x = half; x < newWidth - half; x++) {
            pOut[(y - half) * width + x - half] = paddedImage[y * newWidth + x];
        }
    }


    for (int y = half; y < newHeight - half; y++) {
        for (int x = half; x < newWidth - half; x++) {
            double* pk = kernel;
            const unsigned char* ps = &paddedImage[(y - half) * newWidth + x - half];
            int sum = 0;
            for (int v = 0; v < kernelSize; v++) {
                for (int u = 0; u < kernelSize; u++)
                {
                    sum += ps[u] * pk[u];
                }
                pk += kernelSize;
                ps += newWidth;
            }
            if (sum > 255) sum = 255;
            else
                if (sum < 0) sum = 0;
            pOut[(y - half) * width + x - half] = (unsigned char)sum;
        }
    }


    //for (int y = half; y < height - half; y++) {
    //    for (int x = half; x < width - half; x++) {
    //        double* pk = kernel;
    //        const unsigned char* ps = &pIn[(y - half) * width + x - half];
    //        int sum = 0;
    //        for (int v = 0; v < kernelSize; v++) {
    //            for (int u = 0; u < kernelSize; u++)
    //            {
    //                sum += ps[u] * pk[u];
    //            }
    //            pk += kernelSize;
    //            ps += width;
    //        }
    //        if (sum > 255) sum = 255;
    //        else
    //            if (sum < 0) sum = 0;
    //        pOut[y * width + x] = (unsigned char)sum;
    //    }
    //}
}


double* gaussian(int kernelSize, double sigma)
{
    double* Kernel = new double [kernelSize * kernelSize];

    double sum = 0;
    for (int i = 0; i < kernelSize; ++i)
        for (int j = 0; j < kernelSize; ++j) {
            Kernel[i * kernelSize + j] = exp(-(pow(i, 2) + pow(j, 2)) / (2 * pow(sigma, 2)));
            sum += Kernel[i * kernelSize + j];
        }

    for (int i = 0; i < kernelSize * kernelSize; ++i)
        Kernel[i] /= sum;

    return Kernel;
}


int main(int argc, char* argv[])
{
    char path[] = "C:\\Users\\selib\\OneDrive\\Рабочий стол\\Обработка фотокарточек\\Labs\\Фотокарточки\\baboon.png";
    char out_path[] = "C:\\Users\\selib\\OneDrive\\Рабочий стол\\Обработка фотокарточек\\Labs\\Фотокарточки\\5-out.png";

    //png_bytepp row_pointers = read_png(path);

    size_t iWidth, iHeight;
    unsigned int bpp;
    size_t size = readPngFile(path, 0, 0, 0, 0);
    unsigned char* pIn = new unsigned char[size];
    unsigned char* pOut = new unsigned char[size];

    readPngFile(path, pIn, &iWidth, &iHeight, &bpp);


    // задаем размер ядра
    int KernelSize = 3;


    // пишем свое ядро
    // 
    //double Kernel[] = { 0,1,0,1,1,1,0,1,0 };
    //double factor = (double) 1;
    //for (int i = 0; i < sizeof(Kernel) / sizeof(*Kernel); ++i)
    //    Kernel[i] *= factor;


    // линейны ФНЧ, гауссиан
    // 
    //double* Kernel = gaussian(KernelSize, 6);


    // ФВЧ
    // 
    //double Kernel[] = { 0,-1,0,1,0,1,0,-1,0 };
    //double factor = (double)1;
    //for (int i = 0; i < sizeof(Kernel) / sizeof(*Kernel); ++i)
    //    Kernel[i] *= factor;

    //линейный фильтр, повышающий резкость
    //
    double Kernel[] = { -1,-1,-1, -1,16,-1, -1,-1,-1, };
    double factor = (double) 1/8;
    for (int i = 0; i < sizeof(Kernel) / sizeof(*Kernel); ++i)
        Kernel[i] *= factor;


    // свертка
    convolution2d(pIn, pOut, iWidth, iHeight, Kernel, KernelSize);




    writePngFile(out_path, pOut, iWidth, iHeight, bpp);

    //write_png(out_path, row_pointers);

    return 0;
}
