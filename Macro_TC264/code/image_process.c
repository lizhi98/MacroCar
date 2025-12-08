#include "image_process.h"

#define GrayScale 256
#define grayscale 256
uint16 hist[GrayScale]={0};     //灰度值像素点的数量，数值存放，直方图
float P[GrayScale]={0};         //每个灰度级出现的概率
float PK[GrayScale]={0};         //概率累计和
float MK[GrayScale]={0};        //灰度值累加均值
uint8 img_threshold;            //输出阈值
float imgsize;                  //图像像素总量
uint8  image[MT9V03X_H][MT9V03X_W];

uint8 Ostu(uint8 index[MT9V03X_H][MT9V03X_W])
{
    uint8 threshold;
    imgsize = MT9V03X_H * MT9V03X_W;    //总像素个数
    uint8 images_value_temp;            //中间变量暂时存储

    float sumPK = 0;
    float sumMK = 0;
    float var = 0;
    float vartmp = 0;

    //清零
    for(uint16 i=0;i<GrayScale;i++)
    {
        hist[i]=0;
        P[i]=0;
        PK[i]=0;
        MK[i]=0;
    }

    //获取直方图
    for(uint8 i = 0;i<MT9V03X_H;i++)
    {
        for(uint8 j=0;j<MT9V03X_W;j++)
        {
            images_value_temp = index[i][j];
            hist[images_value_temp]++;
        }
    }

    //求类间方差
    for(uint16 i=0;i<GrayScale;i++)
    {
        P[i]=(float)hist[i]/imgsize;
        PK[i] = sumPK + P[i];
        sumPK=PK[i];
        MK[i] = sumMK+i*P[i];
        sumMK=MK[i];
    }
    //求解最大类间方差的阈值
    for(uint8 i=5;i<245;i++)
    {
        vartmp = ((MK[GrayScale-1] * PK[i] - MK[i]) * (MK[GrayScale - 1] * PK[i] - MK[i])) / (PK[i] * (1 - PK[i]));
        if(vartmp>var)
        {
            var = vartmp;
            threshold = i;      //输出阈值
        }
    }
    return threshold;
}

void Binarization(int width, int height)
{
    img_threshold = Ostu(image);
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            if(image[i][j] > img_threshold)
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

void draw_tangle(int width, int height)
{
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            image[i][j] = 0;             // 左侧3列设为0
            image[i][width - 1 - j] = 0; // 右侧3列设为0
        }
    }
    for(int i=0;i<3;i++)
    {
        for(int j=0;j<width-1;j++)
        {
            image[i][j]=0;
        }
    }
}

uint8 left_start_point = 0;
uint8 right_start_point = MT9V03X_W - 1;
void seek_points(int height, int width)
{
    draw_tangle(width, height);
    left_start_point = 0;
    right_start_point = width - 1;

    for (uint8 i = 1; i < width - 2; i++)
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
void seek_line( int height, int width)
{

    seek_points(height, width);
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
int mid_line_list[MT9V03X_H];
int error_image;
int get_error_image(void)
{
    int error_image = 0;
    int min_raw;
    if(min_raw_l>min_raw_r)
    {
        min_raw=min_raw_r;
    }
    else
    {
        min_raw=min_raw_l;
    }

    int i;
    for(i = min_raw; i < MT9V03X_H - 3;i++)
    {
        mid_line_list[i] = (right_line_list[i] + left_line_list[i]) / 2;
        error_image += weight_array[i] * (94 - mid_line_list[i]);
        error_image /= 500;
    }
    return error_image;
}

void image_process(void)
{
    if(mt9v03x_finish_flag)
    {
        mt9v03x_finish_flag=0;
        memcpy(image[0],mt9v03x_image[0],MT9V03X_IMAGE_SIZE);//复制图像数组
        Binarization(MT9V03X_W, MT9V03X_H);
        seek_points(MT9V03X_H, MT9V03X_W);
        seek_line(MT9V03X_H,MT9V03X_W);
        seek_list(points_count);
    }
   error_image=get_error_image();
}
