#include "zf_common_headfile.h"

int otsu_threshold_optimized(uint8 *image, int width, int height)
{
    int histogram[256] = {0};
    int total_pixels = width * height;

    // 计算直方图
    for (int i = 0; i < total_pixels; i++)
    {
        histogram[image[i]]++;
    }

    // 计算总灰度值
    long long sum_total = 0;
    for (int i = 0; i < 256; i++)
    {
        sum_total += i * histogram[i];
    }

    // 遍历阈值
    long long sum_background = 0;
    int weight_background = 0;
    int weight_foreground = 0;

    float max_variance = 0.0f;
    int best_threshold = 0;

    for (int t = 0; t < 256; t++)
    {
        weight_background += histogram[t];
        if (weight_background == 0)
            continue;

        weight_foreground = total_pixels - weight_background;
        if (weight_foreground == 0)
            break;

        sum_background += t * histogram[t];

        long long sum_foreground = sum_total - sum_background;

        float mean_background = (float)sum_background / weight_background;
        float mean_foreground = (float)sum_foreground / weight_foreground;

        // 计算类间方差
        float between_variance = (float)weight_background * (float)weight_foreground *
                                 (mean_background - mean_foreground) *
                                 (mean_background - mean_foreground);

        if (between_variance > max_variance)
        {
            max_variance = between_variance;
            best_threshold = t;
        }
    }

    return best_threshold;
}

void Binarization(uint8 *image, int width, int height)
{
    int threshold = otsu_threshold_optimized(image, width, height);
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            if (image[i][j] > threshold)
            {
                image[i][j] = 255;
            }
            else
            {
                image[i][j] = 0;
            }
        }
    }
}

void draw_tangle(uint8 *image, int width, int height)
{
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            image[i][j] = 0;             // 左侧3列设为0
            image[i][width - 1 - j] = 0; // 右侧3列设为0
        }
    }
}

uint8 left_start_point = 0;
uint8 right_start_point = MT9V03X_W - 1;
void seek_points(uint8 *image, int height, int width)
{
    draw_tangle(image, width, height);
    left_start_point = 0;
    right_start_point = width - 1;

    for (int i = 1; i < width - 2; i++)
    {
        if (image[height - 3][i - 1] == 0 && image[height - 3][i] == 255 && image[height - 3][i + 1] == 255)
        {
            left_start_point = i;
        }
        if (image[height - 3][i - 1] == 255 && image[height - 3][i] == 255 && image[height - 3][i + 1] == 0)
        {
            right_start_point = i;
        }
    }
}

#define MAX_POINTS 300
int left_points_raw[MAX_POINTS] = {0};
int left_points_col[MAX_POINTS] = {0};
int right_points_raw[MAX_POINTS] = {0};
int right_points_col[MAX_POINTS] = {0};
int points_count;
int min_raw_l = MT9V03X_H - 3;
int min_raw_r = MT9V03X_H - 3;
void seek_line(uint8 *image, int height, int width)
{

    seek_points(image, height, width);
    points_count = 0;
    min_raw_l = MT9V03X_H - 3;
    min_raw_r = MT9V03X_H - 3;
    int point_l_raw = height - 1;
    int point_l_col = left_start_point;
    int point_r_raw = height - 1;
    int point_r_col = right_start_point;

    int dir_l[4] = {0, 1, 2, 3}; // 0为上，1为右，2为下，3为左
    int dir_r[4] = {0, 1, 2, 3}; // 0为上，1为左，2为下，3为右
    int dir_ll = 0;              // 左当前方向
    int dir_rr = 0;              // 右当前方向
    int turn_l = 0;
    int turn_r = 0;

    for (int i = 0; i < MAX_POINTS; i++)
    {
        // 检查左右点是否接近
        if (((point_l_raw - point_r_raw) * (point_l_raw - point_r_raw) +
             (point_l_col - point_r_col) * (point_l_col - point_r_col)) < 5)
        {
            break;
        }
        if (point_l_raw < min_raw_l)
        {
            min_raw_l = point_l_raw;
        }
        if (point_r_raw < min_raw_r)
        {
            min_raw_r = point_r_raw;
        }
        // 存储当前点
        left_points_raw[points_count] = point_l_raw;
        left_points_col[points_count] = point_l_col;
        right_points_raw[points_count] = point_r_raw;
        right_points_col[points_count] = point_r_col;
        points_count++;

        /* 左线追踪 */
        if (dir_ll == 0)
        { // 向上
            if (image[point_l_raw - 1][point_l_col] == 0)
            {
                dir_ll += 1;
            }
            else if (image[point_l_raw - 1][point_l_col] == 255 &&
                     image[point_l_raw - 1][point_l_col - 1] == 255)
            {
                dir_ll -= 1;
                point_l_raw = point_l_raw - 1;
                point_l_col = point_l_col - 1;
            }
            else if (image[point_l_raw - 1][point_l_col] == 255 &&
                     image[point_l_raw - 1][point_l_col - 1] == 0)
            {
                point_l_raw = point_l_raw - 1;
            }
            else
            {
                break;
            }
            if (dir_ll < 0)
                dir_ll = 3;
        }
        else if (dir_ll == 1)
        { // 向右
            if (image[point_l_raw][point_l_col + 1] == 0)
            {
                dir_ll += 1;
            }
            else if (image[point_l_raw][point_l_col + 1] == 255 &&
                     image[point_l_raw - 1][point_l_col + 1] == 255)
            {
                dir_ll -= 1;
                point_l_raw = point_l_raw - 1;
                point_l_col = point_l_col + 1;
            }
            else if (image[point_l_raw][point_l_col + 1] == 255 &&
                     image[point_l_raw - 1][point_l_col + 1] == 0)
            {
                point_l_col = point_l_col + 1;
            }
            else
            {
                break;
            }
        }
        else if (dir_ll == 2)
        { // 向下
            if (image[point_l_raw + 1][point_l_col] == 0)
            {
                dir_ll += 1;
            }
            else if (image[point_l_raw + 1][point_l_col] == 255 &&
                     image[point_l_raw + 1][point_l_col + 1] == 255)
            {
                dir_ll -= 1;
                point_l_raw = point_l_raw + 1;
                point_l_col = point_l_col + 1;
            }
            else if (image[point_l_raw + 1][point_l_col] == 255 &&
                     image[point_l_raw + 1][point_l_col + 1] == 0)
            {
                point_l_raw = point_l_raw + 1;
            }
            else
            {
                break;
            }
        }
        else if (dir_ll == 3)
        { // 向左
            if (image[point_l_raw][point_l_col - 1] == 0)
            {
                dir_ll += 1;
            }
            else if (image[point_l_raw][point_l_col - 1] == 255 &&
                     image[point_l_raw + 1][point_l_col - 1] == 255)
            {
                dir_ll -= 1;
                point_l_raw = point_l_raw + 1;
                point_l_col = point_l_col - 1;
            }
            else if (image[point_l_raw][point_l_col - 1] == 255 &&
                     image[point_l_raw + 1][point_l_col - 1] == 0)
            {
                point_l_col = point_l_col - 1;
            }
            else
            {
                break;
            }
            if (dir_ll > 3)
                dir_ll = 0;
        }

        /* 右线追踪 */
        if (dir_rr == 0)
        { // 向上
            if (image[point_r_raw - 1][point_r_col] == 0)
            {
                dir_rr += 1;
            }
            else if (image[point_r_raw - 1][point_r_col] == 255 &&
                     image[point_r_raw - 1][point_r_col + 1] == 255)
            {
                dir_rr -= 1;
                point_r_raw = point_r_raw - 1;
                point_r_col = point_r_col + 1;
            }
            else if (image[point_r_raw - 1][point_r_col] == 255 &&
                     image[point_r_raw - 1][point_r_col + 1] == 0)
            {
                point_r_raw = point_r_raw - 1;
            }
            else
            {
                break;
            }
            if (dir_rr < 0)
                dir_rr = 3;
        }
        else if (dir_rr == 1)
        { // 向右（实际上是向左搜索）
            if (image[point_r_raw][point_r_col - 1] == 0)
            {
                dir_rr += 1;
            }
            else if (image[point_r_raw][point_r_col - 1] == 255 &&
                     image[point_r_raw - 1][point_r_col - 1] == 255)
            {
                dir_rr -= 1;
                point_r_raw = point_r_raw - 1;
                point_r_col = point_r_col - 1;
            }
            else if (image[point_r_raw][point_r_col - 1] == 255 &&
                     image[point_r_raw - 1][point_r_col - 1] == 0)
            {
                point_r_col = point_r_col - 1;
            }
            else
            {
                break;
            }
            if (dir_rr > 3)
                dir_rr = 0;
        }
        else if (dir_rr == 2)
        { // 向下
            if (image[point_r_raw + 1][point_r_col] == 0)
            {
                dir_rr += 1;
            }
            else if (image[point_r_raw + 1][point_r_col] == 255 &&
                     image[point_r_raw + 1][point_r_col - 1] == 255)
            {
                dir_rr -= 1;
                point_r_raw = point_r_raw + 1;
                point_r_col = point_r_col - 1;
            }
            else if (image[point_r_raw + 1][point_r_col] == 255 &&
                     image[point_r_raw + 1][point_r_col - 1] == 0)
            {
                point_r_raw = point_r_raw + 1;
            }
            else
            {
                break;
            }
            if (dir_rr > 3)
                dir_rr = 0;
        }
        else if (dir_rr == 3)
        { // 向左（实际上是向右搜索）
            if (image[point_r_raw][point_r_col + 1] == 0)
            {
                dir_rr += 1;
            }
            else if (image[point_r_raw][point_r_col + 1] == 255 &&
                     image[point_r_raw + 1][point_r_col + 1] == 255)
            {
                dir_rr -= 1;
                point_r_raw = point_r_raw + 1;
                point_r_col = point_r_col + 1;
            }
            else if (image[point_r_raw][point_r_col + 1] == 255 &&
                     image[point_r_raw + 1][point_r_col + 1] == 0)
            {
                point_r_col = point_r_col + 1;
            }
            else
            {
                break;
            }
            if (dir_rr > 3)
                dir_rr = 0;
        }
    }
}

uint8 left_line_list[MT9V03X_H];
uint8 right_line_list[MT9V03X_H];
void seek_list(int num)
{

    int j = MT9V03X_H - 3;
    int k = MT9V03X_H - 3;
    for (int i = 0; i < MT9V03X_H; i++)
    {
        left_line_list[i] = 0;
        right_line_list[i] = MT9V03X_W - 1;
    }
    for (int i = 0; i < num; i++)
    {
        if (left_points_raw[i] == j && j > min_raw_l)
        {
            left_line_list[j] = left_points_col[i];
            j--;
        }
        if (right_points_raw[i] == k && k > min_raw_r)
        {
            right_line_list[k] = right_points_col[i];
            k--;
        }
    }
}

int weight_array[MT9V03X_H] = {
    1, 1, 1, 1, 1, 2, 2, 2, 3, 3,
    4, 4, 5, 5, 6, 7, 8, 9, 10, 11,
    12, 14, 15, 17, 19, 21, 23, 25, 27, 29,
    31, 33, 35, 37, 39, 41, 43, 44, 46, 47,
    48, 49, 50, 50, 51, 51, 52, 52, 52, 52,
    52, 52, 51, 51, 50, 50, 49, 48, 47, 46,
    44, 43, 41, 39, 37, 35, 33, 31, 29, 27,
    25, 23, 21, 19, 17, 15, 14, 12, 11, 10,
    9, 8, 7, 6, 5, 5, 4, 4, 3, 3,
    2, 2, 2, 1, 1, 1, 0, 0};
int mid_line_list[MT9V03X_H] = {(int)(MT9V03X_W / 2)} int get_error_image(void)
{
    int error_image = 0;
    int min_raw = min(min_raw_l, min_raw_r);
    for (int i = min_raw; i < MT9V03X_H - 3)
    {
        mid_line_list[i] = (right_line_list[i] + left_line_list[i]) / 2;
        error_image += weight_array[i] * (94 - mid_line_list[i]);
        error_image /= 500;
    }
    return error_image;
}
// void seek_list(int l_x[], int l_y[], int r_x[], int r_y[],
//                int start, int num,
//                int left_line_list[], int right_line_list[],
//                int* list_size) {

//     int range_size = 98 - start + 1;
//     int* list1 = (int*)malloc(range_size * sizeof(int));

//     // 生成从98到start的序列
//     for (int i = 0; i < range_size; i++) {
//         list1[i] = 98 - i;
//     }

//     int j = 0;
//     int k = 0;
//     *list_size = 0;

//     for (int i = 0; i < num; i++) {
//         if (j < range_size && list1[j] == l_y[i]) {
//             left_line_list[*list_size] = l_x[i];
//             j++;
//             (*list_size)++;
//         }
//         if (k < range_size && list1[k] == r_y[i]) {
//             right_line_list[*list_size-1] = r_x[i]; // 注意：这里假设左右线同步
//             k++;
//         }
//         if (k >= range_size && j >= range_size) {
//             break;
//         }
//     }

//     free(list1);
// }
