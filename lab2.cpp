#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <png.h>
#include <vector> 
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include <cmath>
#include <fstream>


//#define _CRT_SECURE_NO_DEPRECATE
//#define _CRT_SECURE_NO_WARNINGS


using namespace std;
using namespace cv;


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



int* draw_hist(png_bytepp row_pointers) {
    const int size = 256;
    int hist[size];

    for (int i = 0; i < size; i++) {
        hist[i] = 0;
    }

    for (int y = 0; y < height; y++) {
        png_bytep row = row_pointers[y];
        for (int x = 0; x < width; x++) {
            png_bytep pxl_val = &(row[x]);
            hist[pxl_val[0]] += 1;
        }
    }

    for (int i = 0; i < size; i++) {
        cout << hist[i] << endl;;
    }

    int hist_w = 712; int hist_h = 560;
    int bin_w = cvRound((double)hist_w / size);

    Mat histImage(hist_h, hist_w, CV_8UC1, Scalar(size, size, size));

    int max = hist[0];
    for (int i = 1; i < size; i++) {
        if (max < hist[i]) {
            max = hist[i];
        }
    }

    for (int i = 0; i < size; i++) {
        hist[i] = ((double)hist[i] / max) * histImage.rows;
    }

    for (int i = 0; i < size; i++)
    {
        line(histImage, Point(bin_w * (i), hist_h),
            Point(bin_w * (i), hist_h - hist[i]),
            Scalar(0, 0, 0), 1, 8, 0);
    }

    namedWindow("Intensity Histogram");
    imshow("Intensity Histogram", histImage);

    waitKey();


    return hist;
}


void print_moments(int hist[])
{
    const int pic_size = width * height;

    int max = 0;
    for (int i = 1; i < 256; i++) {
        if (hist[i] > 0) {
            max = i;
        }
    }

    double mean = 0, variance = 0, kurtosis = 0, skewness = 0, uniformity = 0, entropy = 0;
    for (unsigned int i = 0; i < max; i++) {
        mean += hist[i] * i;
    }
    mean /= pic_size;

    for (unsigned int i = 0; i < max; i++) {
        variance += (i - mean) * (i - mean) * hist[i];
        if ((hist[i] != 0) and (hist[i] / pic_size > 0)){
            entropy += -hist[i] * log2(hist[i] / pic_size);
        }
    }
    variance /= pic_size;
    entropy /= pic_size;

    cout << "mean: " << mean << endl;
    cout << "variance: " << variance << endl;
    cout << "entropy: " << entropy << endl;

    if (variance != 0) {

        for (unsigned int i = 0; i < max; i++) {
            kurtosis += (pow((i - mean), 4) * hist[i] - 3) / pow(variance, 4);
            skewness += (pow((i - mean), 3) * hist[i]) / pow(variance, 3);
            uniformity += hist[i] * hist[i];
        }
        kurtosis /= pic_size;
        skewness /= pic_size;
        uniformity /= pic_size;

        cout << "kurtosis: " << kurtosis << endl;
        cout << "skewness: " << skewness << endl;
        cout << "uniformity: " << uniformity << endl;
    }
    else
    {
        cout << "Kurtosis, Skewness and Uniformity do not exist due to zero Variance." << endl;
    }
}


void write_cooccurence_matrix(png_bytepp row_pointers, int dr, int dc) {
    if (dr < 0 or dc < 0) {
        cout << "dr and dc must be positive!" << endl;
        abort();
    }
    int cooccurence_matrix[256][256];
    int pairs_num = 0;

    for (int y = 0; y < height - dr; y++) {
        png_bytep row1 = row_pointers[y];
        png_bytep row2 = row_pointers[y+dr];
        for (int x = 0; x < width - dc; x++) {
            png_bytep pxl1 = &(row1[x]);
            png_bytep pxl2 = &(row2[x+dc]);
            cooccurence_matrix[pxl1[0]][pxl2[0]]++;
            pairs_num++;
        }
    }

    double energy = 0;
    for (int y = 0; y < 256; y++)
        for (int x = 0; x < 256; x++)
            energy += pow(cooccurence_matrix[x][y], 2);
    energy /= pairs_num * pairs_num;

    ofstream myfile;
    myfile.open("cooccurence_matrix.csv");

    myfile << "Energy" << "," << ("%f", energy) << endl;
    myfile << ",";
    for (int y = 0; y < 256; y++)
        myfile << ("%d", y) << ",";
    myfile << "\n";

    for (int y = 0; y < 256; y++) {
        myfile << ("%d", y) << ",";
        for (int x = 0; x < 256; x++)
            myfile << ("%d", cooccurence_matrix[x][y]) << ",";
        myfile << "\n";
    }
    myfile.close();
}


int main(int argc, char* argv[])
{
    char path[] = "C:\\Users\\selib\\OneDrive\\Рабочий стол\\Обработка фотокарточек\\Labs\\Фотокарточки\\2-2.png";

    png_bytepp row_pointers = read_png(path);
    int *hist = draw_hist(row_pointers);

    print_moments(hist);
    write_cooccurence_matrix(row_pointers, 50, 50);

    return 0;
}
