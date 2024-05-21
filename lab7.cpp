//#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <png.h>
#include "PngProc.h"
//#include<opencv2/highgui/highgui.hpp>
//#include<opencv2/imgproc/imgproc.hpp>
#include <cmath>
#include <stack> 
#include <queue>
#include <vector>
#include <queue>
#include <unordered_map>


using namespace std;
using namespace NPngProc;
//using namespace cv;


png_byte color_type;
png_byte bit_depth;
png_infop info_ptr;
png_structp png_ptr;


int hist_then_otsu(unsigned char* pIn, size_t width, size_t height)
{
    const int size = 256;
    int hist[size];

    for (int i = 0; i < size; i++) {
        hist[i] = 0;
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            hist[pIn[y * width + x]] += 1;
        }
    }

    double sigma_w_min = pow(10, 7);
    int t_min = -1;

    for (int t = 0; t < 256; t++) {
        double w_0 = 0; double w_1 = 0; 
        double mean_0 = 0; double variance_0 = 0; 
        double mean_1 = 0; double variance_1 = 0;

        for (int i = 0; i < t; i++) {
            mean_0 += double(hist[i] * i);
        }
        mean_0 /= double(width * height);
        
        for (int i = 0; i < t; i++) {
            variance_0 += (double(i) - mean_0) * (double(i) - mean_0) * double(hist[i]);
        }
        variance_0 /= double(width * height);

        for (int i = t; i < 256; i++) {
            mean_1 += double(hist[i] * i);
        }
        mean_1 /= double(width * height);

        for (int i = t; i < 256; i++) {
            variance_1 += (double(i) - mean_1) * (double(i) - mean_1) * double(hist[i]);
        }
        variance_1 /= double(width * height);

        for (int i = 0; i < t - 1; i++)
            w_0 += double(hist[i]);
        w_0 /= double(width * height);

        for (int i = t; i < 256; i++)
            w_1 += double(hist[i]);
        w_1 /= double(width * height);

        double sigma_w = w_0 * variance_0 + w_1 * variance_1;
        if (sigma_w < sigma_w_min) {
            sigma_w_min = sigma_w;
            t_min = t;
        }
    }

    return t_min;
}


void binarize(unsigned char* pIn, unsigned char* pOut, size_t width, size_t height, int threshold)
{
    for (size_t i = 0; i < width * height; i++)
        pOut[i] = (pIn[i] <= threshold) ? 0 : 255;
}


vector<int> floodFill8(unsigned char* pIn, unsigned char* pOut, int width, int height)
{
    int label_count = 0; // метка для области

    queue<pair<int, int>> queue; // очередь для обхода пикселя и его соседей

    vector<int> labels(width * height, 0); // метки для всех областей картинки

    // ищем все затравки, чтобы полностью перекрасить изображение 
    for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++) {

            // нашли затравку
            if (pIn[y * width + x] == 255 && labels[y * width + x] == 0) {
                queue.push({ x, y }); 
                label_count++; // обошли всех соседей с предыдущей затравки, нашли новую - увеличили метку
                labels[y * width + x] = label_count;
            }

            while (!queue.empty())
            {
                // достаем пиксель, убираем из стека
                auto front = queue.front();
                int cx = front.first;
                int cy = front.second;
                queue.pop();

                int x_left = cx, x_right = cx; // будущие крайние пикселы интервала

                // идем до левой границы интервала
                while (x_left > 0 && pIn[cy * width + x_left] == 255) {
                    labels[cy * width + x_left] = label_count;
                    x_left--; // уменьшаем x_left координату
                }

                // идем до правой границы интервала
                while (x_right < width - 1 && pIn[cy * width + x_right] == 255) {
                    labels[cy * width + x_right] = label_count;
                    x_right++; // увеличиваем x_right координату
                }

                // проверяем можно ли включить крайние точки для 8-связного алгоритма
                x_left = (x_left > 0) ? x_left - 1 : x_left;
                x_right = (x_right < width - 1) ? x_right + 1 : x_right;


                // идем по верхней строке, добавляя в стек левые границы новых интервалов
                int gx = x_left - 1;
                if (cy - 1 >= 0) {
                    while (gx <= x_right) {
                        gx++;

                        bool flag = false; // флаг, что новый интервал не найден
                        if (pIn[(cy - 1) * width + gx] == 255 && labels[(cy - 1) * width + gx] == 0) {
                            queue.push({ gx, cy - 1 });
                            labels[(cy - 1) * width + gx] = label_count;
                            flag = true; // флаг, что новый интервал найден
                        }             

                        // пока не конец найденного и исходного интервала
                        while (flag && gx <= x_right) {

                            // пропускаем белые пикселы нового найденного интервала
                            while (gx <= x_right && pIn[(cy - 1) * width + gx] == 255) {
                                labels[(cy - 1) * width + gx] = label_count;
                                gx++;
                            }

                            // пропускаем черные пикселы после нового найденного интервала
                            while (gx <= x_right && pIn[(cy - 1) * width + gx] == 0)
                                gx++;

                            flag = false; // выходим из это цикла, чтобы снова искать левые границы, которые можно добавить в стек

                        }
                    }
                }

                // идем по нижней строке, добавляя в стек 
                gx = x_left - 1;
                if (cy + 1 < height) {
                    while (gx <= x_right && gx < width) {
                        gx++;

                        bool flag = false; // флаг, что новый интервал не найден
                        if (pIn[(cy + 1) * width + gx] == 255
                            && labels[(cy + 1) * width + gx] == 0) {
                            queue.push({ gx, cy + 1 });
                            labels[(cy + 1) * width + gx] = label_count;
                            flag = true; // флаг, что новый интервал найден
                        }
                        

                        // пока не конец найденного и исходного интервала
                        while (flag && gx <= x_right) {

                            // пропускаем белые пикселы нового найденного интервала
                            while (gx <= x_right && pIn[(cy + 1) * width + gx] == 255) {
                                labels[(cy + 1) * width + gx] = label_count;
                                gx++;
                            }

                            // пропускаем черные пикселы после нового найденного интервала
                            while (gx <= x_right && pIn[(cy + 1) * width + gx] == 0) {
                                gx++;
                            }
                                
                            flag = false; // выходим из это цикла, чтобы снова искать левые границы, которые можно добавить в стек
                        }
                    }
                }
            }
        }

        // перекрашиваем области
        for (int i = 0; i < width * height; i++)
        {
            int brightness = 10;
            pOut[i] = (labels[i] > 0) ? (unsigned char(labels[i] * brightness % 256)) : unsigned char(0);
        }

        return labels;

}


struct Moments
{
    double m00;              // момент 0 - го порядка (масса или площадь)
    double m10, m01;         // два момента 1 - го порядка (центры масс * массу)
    double mu20, mu02, mu11; // три центральных момента 2-го порядка (моменты инерции и смешанный момент инерции)
};


unordered_map<int, ::Moments> find_moments(vector<int> labels, int width, int height)
{
    unordered_map<int, ::Moments> momentsMap;

    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
        {
            int label = labels[y * width + x];
            if (label > 0)
            {
                ::Moments & m = momentsMap[label];
                m.m00 += 1;
                m.m10 += x;
                m.m01 += y;
            }
        }

    for (auto& kvp : momentsMap)
    {
        ::Moments& m = kvp.second;
        double cx = static_cast<double>(m.m10) / m.m00;
        double cy = static_cast<double>(m.m01) / m.m00;

        for (int y = 0; y < height; ++y)
            for (int x = 0; x < width; ++x)
            {
                int label = labels[y * width + x];
                if (label == kvp.first)
                {
                    double dx = x - cx;
                    double dy = y - cy;
                    m.mu20 += dx * dx;
                    m.mu11 += dx * dy;
                    m.mu02 += dy * dy;
                }
            }
    }

    return momentsMap;
}

void print_moments(unordered_map<int, ::Moments>& momentsMap)
{
    for (const auto& kvp : momentsMap)
    {
        const ::Moments& m = kvp.second;
        std::cout << "label " << kvp.first << ":" << std::endl;
        std::cout << "  m00: " << m.m00 << std::endl;
        std::cout << "  m10, m01: (" << static_cast<double>(m.m10) / m.m00 << ", " << static_cast<double>(m.m01) / m.m00 << ")" << std::endl;
        std::cout << "  mu20: " << m.mu20 << std::endl;
        std::cout << "  mu02: " << m.mu02 << std::endl;
        std::cout << "  mu11: " << m.mu11 << std::endl;
    }
}


int main(void)
{
    char in_path[] = "C:\\Users\\selib\\OneDrive\\Рабочий стол\\Обработка фотокарточек\\Labs\\Фотокарточки\\River.png";
    char labeled_out_path[] = "C:\\Users\\selib\\OneDrive\\Рабочий стол\\Обработка фотокарточек\\Labs\\Фотокарточки\\7-8-labeled.png";
    char binarized_out_path[] = "C:\\Users\\selib\\OneDrive\\Рабочий стол\\Обработка фотокарточек\\Labs\\Фотокарточки\\7-8-binarized.png";

    size_t iWidth, iHeight;

    unsigned int bpp;
    size_t size = readPngFile(in_path, 0, 0, 0, 0);


    unsigned char* pIn = new unsigned char[size];
    unsigned char* pOut = new unsigned char[size];
    unsigned char* labelOut = new unsigned char[size];
    

    readPngFile(in_path, pIn, &iWidth, &iHeight, &bpp);

    int width, height;

    width = int(iWidth);
    height = int(iHeight);
    memset(labelOut, 0, width * height);

    int threshold = hist_then_otsu(pIn, width, height);

    binarize(pIn, pOut, width, height, threshold);

    vector<int> labels = floodFill8(pOut, labelOut, width, height);

    auto moments = find_moments(labels, width, height);
    print_moments(moments);


    writePngFile(binarized_out_path, pOut, iWidth, iHeight, bpp);
    writePngFile(labeled_out_path, labelOut, iWidth, iHeight, bpp);




    delete[] pIn;
    delete[] pOut;
    delete[] labelOut;

    return 0;
}
