#include <iostream>
#include <png.h>
#include "PngProc.h"
#include <algorithm> 

using namespace std;
using namespace NPngProc;


int width, height;
png_byte color_type;
png_byte bit_depth;
png_infop info_ptr;
png_structp png_ptr;


void order_statistic_filter(
    unsigned char* pIn, 
    unsigned char* pOut, 
    size_t width, 
    size_t height, 
    int* kernel, 
    const int kernelSize, 
    int participants_count,
    int rang)
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
            pIn + (i + 1) * width, width * sizeof(unsigned char));
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


    //for (int y = half; y < newHeight - half; y++) {
    //    for (int x = half; x < newWidth - half; x++) {
    //        pOut[(y - half) * width + x - half] = paddedImage[y * newWidth + x];
    //    }
    //}


    for (int y = half; y < newHeight - half; y++) {
        for (int x = half; x < newWidth - half; x++) {
            int* pk = kernel;
            const unsigned char* ps = &paddedImage[(y - half) * newWidth + x - half];
            int* arr = new int[participants_count];
            memset(arr, 0, participants_count * sizeof(int));
            int k = 0;
            for (int v = 0; v < kernelSize; v++) {
                for (int u = 0; u < kernelSize; u++)
                {
                    if (pk[u] > 0) {
                        arr[k] = ps[u] * pk[u];
                        k++;
                    }
                }
                pk += kernelSize;
                ps += newWidth;
            }
            sort(arr, arr + participants_count);
            pOut[(y - half) * width + x - half] = arr[rang - 1];
            delete[] arr;
        }
    }

    delete[] paddedImage;
}


int main(int argc, char* argv[])
{
    char path[] = "C:\\Users\\selib\\OneDrive\\Рабочий стол\\Обработка фотокарточек\\Labs\\Фотокарточки\\Lena_noise.png";
    char out_path[] = "C:\\Users\\selib\\OneDrive\\Рабочий стол\\Обработка фотокарточек\\Labs\\Фотокарточки\\6-out.png";

    //png_bytepp row_pointers = read_png(path);

    size_t iWidth, iHeight;
    unsigned int bpp;
    size_t size = readPngFile(path, 0, 0, 0, 0);
    unsigned char* pIn = new unsigned char[size];
    unsigned char* pOut = new unsigned char[size];

    readPngFile(path, pIn, &iWidth, &iHeight, &bpp);

    // задаем размер апертуры, маску апертуры и ранг
    const int KernelSize = 3;
    int Kernel[] = { 0,1,0,1,1,1,0,1,0 };
    int rang = 4;

    int participants_count = 0;
    for (int i = 0; i < KernelSize * KernelSize; i++)
        if (Kernel[i] > 0)
            participants_count++;

    // ранговая фильтрация
    order_statistic_filter(pIn, pOut, iWidth, iHeight, Kernel, KernelSize, participants_count, rang);

    writePngFile(out_path, pOut, iWidth, iHeight, bpp);

    delete[] pIn;
    delete[] pOut;


    return 0;
}
