#include "image_process.h"

#define ERROR_IMAGE_LINE 70
#define ERROR_IMAGE_LINE_2_D 53

//连续BJT
#define ERROR_IMAGE_LINE_2_BJT 53

uint8 ERROR_IMAGE_LINE_2 = ERROR_IMAGE_LINE_2_D;

#define STROP_FEATURE_T 1

#define IMG_THRESHOLD_DIFF 15

#define encoder_count_image 32500

#define index_num 74
int T_index_list[index_num] = {
    0,1,1,1,
    1,1,0,0,
    1,0,1,0,1, // 13
    0,1,1,-1,1, // 18
    -1,-1,-1,-1,// 22
    0,-1,1,0,-1, // 27
    -1,0,0, // 31   -1
    1,0,0,0, // 35
    1,-1,-1,-1, // 39
    0,-1,1, // 43    -1
    -1,0,-1,-1, // 47
    -1,1,1,-1,   //51
    0,-1,-1,1,   //55
    0,0,-1,1,      //59
    0,0,1,-1,1, //5       //64
    -1,1,1,-1,      //68
    -1,-1,0,0,      //72
    -1,1,0,1       //76
};    ///74


uint32 image_test = 0;
uint8 image_start_flag=0;

#define GrayScale 256
#define grayscale 256
uint16 hist[GrayScale]={0};     //灰度值像素点的数量，数值存放，直方图
float P[GrayScale]={0};         //每个灰度级出现的概率
float PK[GrayScale]={0};        //概率累计和
float MK[GrayScale]={0};        //灰度值累加均值
uint8 img_threshold=0;            //输出阈值
float imgsize;                  //图像像素总量

uint8 (*image)[MT9V03X_W];

uint8 Ostu(uint8 index[MT9V03X_H][MT9V03X_W])
{
    uint8 threshold=128;    //初始阈值
    imgsize = MT9V03X_H * MT9V03X_W;    //总像素个数
    uint8 images_value_temp;            //中间变量暂时存储

    float sumPK = 0;
    float sumMK = 0;
    float var = -1.0f;
    float vartmp = 0;
    const float eps = 1e-6f;

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
        float w0 = PK[i];
        float w1 = 1.0f - w0;
        float denom = w0 * w1;

        // 避免分母为 0 导致 NaN/Inf，跳过无效阈值
        if(denom <= eps)
        {
            continue;
        }

        float diff = MK[GrayScale-1] * w0 - MK[i];
        vartmp = (diff * diff) / denom;

        if(vartmp > var)
        {
            var = vartmp;
            threshold = i;      //输出阈值
        }
    }
    return threshold;
}

void Binarization()
{

    uint8 threshold= Ostu(image);
    if(img_threshold==0)
    {
        img_threshold=threshold;
    }
    else 
    {
        // if(image_start_flag==0)
        // {
        //     img_threshold=threshold;
        // }
        // else
        // {
            if(img_threshold-threshold>IMG_THRESHOLD_DIFF)
            {
                img_threshold=img_threshold-IMG_THRESHOLD_DIFF;
            }
            else if(threshold-img_threshold>IMG_THRESHOLD_DIFF)
            {
                img_threshold=img_threshold+IMG_THRESHOLD_DIFF;
            }
            else
            {
                img_threshold=threshold;
            }
        // }
        
    }
    for (int i = 0; i < MT9V03X_H; i++)
    {
        for (int j = 0; j < MT9V03X_W; j++)
        {
            if(image[i][j] > img_threshold)
            {
                image[i][j] = white_point;
            }
            else
            {
                image[i][j] = black_point;
            }
        }
    }
}

void remove_white_noise_filter() {
    for (int i = 1; i < MT9V03X_H - 1; i++) {
        for (int j = 1; j < MT9V03X_W - 1; j++) {
            if (image[i][j] == white_point) {
                int white_neighbor_count = 0;
                if (image[i - 1][j] == white_point) white_neighbor_count++;
                if (image[i + 1][j] == white_point) white_neighbor_count++;
                if (image[i][j - 1] == white_point) white_neighbor_count++;
                if (image[i][j + 1] == white_point) white_neighbor_count++;
                if (white_neighbor_count < 2) {
                    image[i][j] = black_point;
                }
            }
        }
    }
}

#define left_last_point 0
#define right_last_point image_w-1
uint8 left_line_list[MT9V03X_H]={0};
uint8 right_line_list[MT9V03X_H]={image_w-1};
uint8 mid_line_list[MT9V03X_H];
uint8 left_lost_times = 0;
uint8 right_lost_times = 0;
int lost_row_down_l=0;
int lost_row_down_r=0;
int lost_condition=0;
int detect_row_diff(const int row_diff[5], int limit)
{
    int max_value = row_diff[0];
    int min_value = row_diff[0];

    for(int i = 1; i < 5; i++)
    {
        if(row_diff[i] > max_value)
        {
            max_value = row_diff[i];
        }
        if(row_diff[i] < min_value)
        {
            min_value = row_diff[i];
        }
    }

    return ((max_value - min_value) < limit) ? TRUE : FALSE;
}

void search_line()
{
    left_lost_times = 0;
    right_lost_times = 0;
    lost_condition = 0;
    //第一行特殊处理
    for(uint8 i=right_last_point-1;i>left_last_point;i--)
    {
        if(image[MT9V03X_H-1][0]==white_point)
        {
            left_line_list[MT9V03X_H-1]=0;
            break;
        }
        else if(image[MT9V03X_H-1][i-1]==black_point && image[MT9V03X_H-1][i]==white_point && image[MT9V03X_H-1][i+1]==white_point)
        {
            left_line_list[MT9V03X_H-1]=i;
            break;
        }
        else{
            left_line_list[MT9V03X_H-1]=0;
        }
    }
    for(uint8 i=left_last_point+1;i<right_last_point;i++)
    {
        if(image[MT9V03X_H-1][right_last_point]==white_point)
        {
            right_line_list[MT9V03X_H-1]=image_w-1;
            break;
        }
        else if(image[MT9V03X_H-1][i-1]==white_point && image[MT9V03X_H-1][i]==white_point && image[MT9V03X_H-1][i+1]==black_point)
        {
            right_line_list[MT9V03X_H-1]=i;
            break;
        }
        else{
            right_line_list[MT9V03X_H-1]=image_w-1;
        }
    }
    mid_line_list[MT9V03X_H-1]=(uint8)(((uint16)left_line_list[MT9V03X_H-1]+(uint16)right_line_list[MT9V03X_H-1])/2);
    //从下往上搜线
    for (int i=MT9V03X_H-2;i>=0;i--)
    {
        int left_start_col;
        int right_start_col;
        //左侧起始点根据上一行的线位置进行调整
        if(image[i][left_line_list[i+1]]==white_point)
        {
            left_start_col=left_line_list[i+1]+5;
            if(left_start_col>right_last_point-1)
            {
                left_start_col=image_w-1;
            }
            for(int j=left_start_col; j>left_last_point; j--)
            {
                if(image[i][j-1]==black_point && image[i][j]==white_point && image[i][j+1]==white_point)
                {
                    left_line_list[i]=j;
                    break;
                }
                else{
                    left_line_list[i]=0;
                }
            }
        }
        else
        {
            left_start_col=left_line_list[i+1]-5;
            if(left_start_col<left_last_point+1)
            {
                left_start_col=left_last_point+1;
            }
            for(int j=left_start_col; j<right_last_point; j++)
            {
                if(image[i][j-1]==black_point && image[i][j]==white_point && image[i][j+1]==white_point)
                {
                    left_line_list[i]=j;
                    break;
                }
                else{
                    left_line_list[i]=0;
                }
            }
        }
        if(image[i][right_line_list[i+1]]==white_point)
        {
            right_start_col=right_line_list[i+1]-5;
            if(right_start_col<left_last_point+1)
            {
                right_start_col=left_last_point+1;
            }
            for(int j=right_start_col; j<right_last_point; j++)
            {
                if(image[i][j-1]==white_point && image[i][j]==white_point && image[i][j+1]==black_point)
                {
                    right_line_list[i]=j;
                    break;
                }
                else{
                    right_line_list[i]=image_w-1;
                }
            }
        }
        else
        {
            right_start_col=right_line_list[i+1]+5;
            if(right_start_col>right_last_point-1)  
            {
                right_start_col=right_last_point-1;
            }
            for(int j=right_start_col; j>left_last_point; j--)
            {
                if(image[i][j-1]==white_point && image[i][j]==white_point && image[i][j+1]==black_point)
                {
                    right_line_list[i]=j;
                    break;
                }
                else{
                    right_line_list[i]=image_w-1;
                }
            }
        }
        if(i<90)
        {
            //按实际情况来进行更改
            // if(image[i][0]==white_point && left_line_list[i]==0)
            // {
            //     left_lost_times++;
            // }
            // if(image[i][image_w-1]==white_point&& right_line_list[i]==image_w-1)
            // {
            //     right_lost_times++;
            // }
            if(image[i][20]==white_point)
            {
                left_lost_times++;
            }
            if(image[i][image_w-21]==white_point) 
            // if(image[i][image_w-31]==white_point)   
            {
                right_lost_times++;
            }
        }

        if(image[i][0]==white_point && left_line_list[i]==0)
        {
            if(lost_condition==0)
            {
                lost_condition=1;
                lost_row_down_l=i;
            }
        }
        if(image[i][image_w-1]==white_point && right_line_list[i]==image_w-1)
        {
            if(lost_condition==0)
            {
                lost_condition=1;
                lost_row_down_r=i;
            }
        }
        
        mid_line_list[i]=(uint8)(((uint16)left_line_list[i]+(uint16)right_line_list[i])/2);
    }

    // for (int i=MT9V03X_H-2 ;i>=0;i--)
    // {
    //     int left_start_col=left_line_list[i+1]-20;
    //     int right_start_col=right_line_list[i+1]+20;
    //     if(left_start_col<1)
    //     {
    //         left_start_col=1;
    //     }
    //     if(right_start_col>MT9V03X_W-3)
    //     {
    //         right_start_col=MT9V03X_W-3;
    //     }
    //     //单独处理第一列
    //     if(image[i][0]==white_point)
    //     {
    //         left_line_list[i]=0;
    //     }
    //     else{
    //         for(uint8 j=left_start_col;j<right_start_col;j++)
    //         {
    //             if(image[i][j-1]==black_point && image[i][j]==white_point && image[i][j+1]==white_point)
    //             {
    //                 left_line_list[i]=j;
    //                 break;
    //             }
    //             else{
    //                 left_line_list[i]=0;
    //             }
    //         }
    //     }
    //     //单独处理最后一列
    //     if(image[i][MT9V03X_W-2]==white_point)
    //     {
    //         right_line_list[i]=MT9V03X_W-1;
    //     }
    //     else{
    //         for(uint8 j=right_start_col;j>=left_start_col ;j--)
    //         {
    //             if(image[i][j-1]==white_point && image[i][j]==white_point && image[i][j+1]==black_point)
    //             {
    //                 right_line_list[i]=j;
    //                 break;
    //             }
    //             else{
    //                 right_line_list[i]=MT9V03X_W-1;
    //             }
    //         }
    //     }
    //     if(i<90)
    //     {
    //         if(left_line_list[i]==0 &&image[i][left_line_list[i]]==white_point)
    //         {
    //             left_lost_times++;
    //         }
    //         if(right_line_list[i]==MT9V03X_W-1&&image[i][MT9V03X_W-2]==white_point)
    //         {
    //             right_lost_times++;
    //         }
    //     }
    //     mid_line_list[i]=(uint8)(((uint16)left_line_list[i]+(uint16)right_line_list[i])/2);
    // }


    // for(int i=MT9V03X_H-1 ;i>=0;i--)
    // {
    //     for(int j=left_end_col;j<right_end_col;j++)
    //     {
    //         if(image[i][j-2]==black_point && image[i][j-1]==black_point && image[i][j]==white_point && image[i][j+1]==white_point)
    //         {
    //             left_line_list[i]=(uint8)j;
    //             break;
    //         }
    //         else{
    //             left_line_list[i]=0;
    //         }
    //     }
    //     for(int j=right_end_col;j>=left_end_col ;j--)
    //     {
    //         if(image[i][j-1]==white_point && image[i][j]==white_point && image[i][j+1]==black_point && image[i][j+2]==black_point)
    //         {
    //             right_line_list[i]=(uint8)j;
    //             break;
    //         }
    //         else{
    //             right_line_list[i]=MT9V03X_W-1;
    //         }
    //     }
}


void left_line_offset(uint8 raw)
{
    for(int i=raw;i>=0;i--)
    {
        mid_line_list[i]=0;
    }
}
void right_line_offset(uint8 raw)
{
    for(int i=raw;i>=0;i--)
    {
        mid_line_list[i]=image_w-1;
    }
}
void Add_Line(int x1,int y1,int x2,int y2)//补中线
{
    int i,max,a1,a2;
    int hx;
    if(x1>=image_w-1)//起始点位置校正，排除数组越界的可能
       x1=image_w-1;
    else if(x1<=0)
        x1=0;
    if(y1>=image_h-1)
        y1=image_h-1;
    else if(y1<=0)
        y1=0;
    if(x2>=image_w-1)
        x2=image_w-1;
    else if(x2<=0)
        x2=0;
    if(y2>=image_h-1)
        y2=image_h-1;
    else if(y2<=0)
         y2=0;
    a1=y1;
    a2=y2;
    if(a1>a2)//坐标互换
    {
        max=a1;
        a1=a2;
        a2=max;
    }

    
    for(i=a1;i<=a2;i++)//根据斜率补线即可
    {       //-46      //45 //-46 +89
        hx=(i-y1)*(x2-x1)/(y2-y1)+ x1;
        if(hx>=image_w-1)
            hx=image_w-1;
        else if(hx<=0)
            hx=0;
        mid_line_list[i]=(uint8)hx;
    }
}
void Add_Line1(int x1,int y1,int x2,int y2)//补中线
{
    int i,max,a1,a2;
    int hx;
    if(x1>=image_w-1)//起始点位置校正，排除数组越界的可能
       x1=image_w-1;
    else if(x1<=0)
        x1=0;
    if(y1>=image_h-1)
        y1=image_h-1;
    else if(y1<=0)
        y1=0;
    if(x2>=image_w-1)
        x2=image_w-1;
    else if(x2<=0)
        x2=0;
    if(y2>=image_h-1)
        y2=image_h-1;
    else if(y2<=0)
         y2=0;
    a1=y1;
    a2=y2;
    if(a1>a2)//坐标互换
    {
        max=a1;
        a1=a2;
        a2=max;
    }

    
    for(i=a1;i<=a2;i++)//根据斜率补线即可
    {       //-46      //45 //-46 +89
        hx=(i-y1)*parameterB+ x1;
        if(hx>=image_w-1)
            hx=image_w-1;
        else if(hx<=0)
            hx=0;
        mid_line_list[i]=(uint8)hx;
    }
}
int longest_col=0;
void detect_longest_col()
{
    longest_col=0;
    for(int i=MT9V03X_H-11;i>0;i--)
    {
        if((left_line_list[i]==0&& left_line_list[i-1]==0)||(right_line_list[i]==image_w-1&& right_line_list[i-1]==image_w-1))
        {
            longest_col=i;
            break;
        }
    }
}


float parameterA;
float parameterB;
void regression(int type, int startline, int endline)//最小二乘法拟合曲线，分别拟合中线，左线，右线,type表示拟合哪几条线
{
    int i = 0;
    int sumlines = endline - startline;
    int sumX = 0;
    int sumY = 0;
    float averageX = 0;
    float averageY = 0;
    float sumUp = 0;
    float sumDown = 0;
    if (type == 0)      //拟合中线
    {
        for (i = startline; i < endline; i++)
        {
            sumX += i;
            sumY += mid_line_list[i];
        }
        if (sumlines != 0)
        {
            averageX = (float)(sumX / sumlines);     //x的平均值
            averageY = (float)(sumY / sumlines);     //y的平均值
        }
        else
        {
            averageX = 0;     //x的平均值
            averageY = 0;     //y的平均值
        }
        for (i = startline; i < endline; i++)
        {
            sumUp += (mid_line_list[i] - averageY) * (i - averageX);
            sumDown += (i - averageX) * (i - averageX);
        }
        if (sumDown == 0) parameterB = 0;
        else parameterB = sumUp / sumDown;
        parameterA = averageY - parameterB * averageX;
    }
    else if (type == 1)//拟合左线
    {
        for (i = startline; i < endline; i++)
        {
            sumX += i;
            sumY += left_line_list[i];
        }
        if (sumlines == 0) sumlines = 1;
        averageX = (float)(sumX / sumlines);     //x的平均值
        averageY = (float)(sumY / sumlines);     //y的平均值
        for (i = startline; i < endline; i++)
        {
            sumUp += (left_line_list[i] - averageY) * (i - averageX);
            sumDown += (i - averageX) * (i - averageX);
        }
        if (sumDown == 0) parameterB = 0;
        else parameterB = sumUp / sumDown;
        parameterA = averageY - parameterB * averageX;
    }
    else if (type == 2)//拟合右线
    {
        for (i = startline; i < endline; i++)
        {
            sumX += i;
            sumY += right_line_list[i];
        }
        if (sumlines == 0) sumlines = 1;
        averageX = (float)(sumX / sumlines);     //x的平均值
        averageY = (float)(sumY / sumlines);     //y的平均值
        for (i = startline; i < endline; i++)
        {
            sumUp += (right_line_list[i] - averageY) * (i - averageX);
            sumDown += (i - averageX) * (i - averageX);
        }
        if (sumDown == 0) parameterB = 0;
        else parameterB = sumUp / sumDown;
        parameterA = averageY - parameterB * averageX;

    }

}
int monileft[MT9V03X_H];
int moniright[MT9V03X_H];
int monimiddle[MT9V03X_H];
void monileftfuzhi(float A, float B, int start_point, int end_point)
{
    int m;
    for (m = start_point; m <= end_point; m++)
    {
        if ((B * m + A) >= 255) monileft[m] = 255;
        if ((B * m + A) <= 0) monileft[m] = 0;
        else if (0 < (B * m + A) && (B * m + A) < 255) monileft[m] = (int)(B * m + A);
    }
}
void monirightfuzhi(float A, float B, int start_point, int end_point)
{
    int m;
    for (m = start_point; m <= end_point; m++)
    {
        if ((B * m + A) >= 255) moniright[m] = 255;
        if ((B * m + A) <= 0) moniright[m] = 0;
        else if (0 < (B * m + A) && (B * m + A) < 255) moniright[m] = (int)(B * m + A);
    }
}

void monizhongfuzhi(float A, float B, int start_point, int end_point)
{
    int m;
    for (m = start_point; m <= end_point; m++)
    {
         if ((B * m + A) >= 255) monimiddle[m] = 255;
         if ((B * m + A) <= 0) monimiddle[m] = 0;
         else if (0 < (B * m + A) && (B * m + A) < 255) monimiddle[m] = (int)(B * m + A);
    }
}

double pianfangleft;
double pianfangright;
double pianfangmid;
void pianfangcal(int begin, int end, int type)
{
    int i = 0;
    if (type == 1)//左线拟合差平方计算
    {
        pianfangleft = 0;
        regression(1, begin, end);
        monileftfuzhi(parameterA, parameterB, (int)begin, (int)end);
        for (i = begin; i <= end; i++)
        {
            pianfangleft = pianfangleft + (left_line_list[i] - monileft[i]) * (left_line_list[i] - monileft[i]);
        }
        pianfangleft = pianfangleft / (end - begin + 1);
    }
    if (type == 2)//右线拟合差平方计算
    {
        pianfangright = 0;
        regression(2, begin, end);
        monirightfuzhi(parameterA, parameterB, (int)begin, (int)end);
        for (i = begin; i <= end; i++)
        {
            pianfangright = pianfangright + (right_line_list[i] - moniright[i]) * (right_line_list[i] - moniright[i]);
        }
        pianfangright = pianfangright / (end - begin + 1);
    }
    if (type == 0)//中线拟合差平方计算
    {
        pianfangmid = 0;
        regression(0, begin, end);
        monizhongfuzhi(parameterA, parameterB, (int)begin, (int)end);
        int fangjun = 0;
        int junfang = 0;
        for (i = begin; i <= end; i++)
        {
            fangjun = fangjun + (mid_line_list[i]) * (mid_line_list[i]);
        }
        fangjun = fangjun / (end - begin + 1);
        for (i = begin; i <= end; i++)
        {
            junfang = junfang + (mid_line_list[i]);
        }
        junfang = junfang / (end - begin + 1);
        junfang = junfang * junfang;
        pianfangmid = (fangjun - junfang) * 1.0;
    }

}
uint8 feature_label=0;
uint8 deceleration_label=0;
uint8 feature_raw_l=0;
uint8 feature_raw_r=0;


int feature_T_index=0;
// int feature_T_index=54;

#define feature_raw 10    
uint8 detect_feature_row=0;
uint8 detect_feature_row_l=0;
int up_feature_row=MT9V03X_H-1;
int feature_row_run=0;
#define col_left 40
#define col_right 145
int feature_row_down=0;
int feature_row_up=0;
int activate_condition_l=0;
int activate_condition_r=0;
#define row_diff_feature 10

void detect_feature()
{
    // printf("left_lost_times=%d, right_lost_times=%d\n", left_lost_times, right_lost_times);
    feature_raw_l=0;
    feature_raw_r=0;
    detect_feature_row=0;
    feature_row_down=0;
    feature_row_up=0;
    feature_label=0;
    up_feature_row=MT9V03X_H-1;
    // if(feature_label==1)
    // {
    //     if(T_index_list[feature_T_index-1]==0 &&feature_T_left==1)
    //     {
    //         for(int i=MT9V03X_H-11;i>=feature_raw;i--)
    //         {
    //             detect_feature_row=0;
    //             if(left_line_list[i]-left_line_list[i-1]>10 || left_line_list[i]-left_line_list[i-2]>10 || left_line_list[i]-left_line_list[i-3]>10 || left_line_list[i]-left_line_list[i-4]>10)
    //             {
    //                 detect_feature_row=(uint8)i;
    //             }
    //             if(detect_feature_row >20)
    //             {
    //                 int seek_row_start=detect_feature_row-20;
    //                 int seek_row_end=detect_feature_row+10;
    //                 int seek_col_start=left_line_list[detect_feature_row]-10;;
    //                 if(seek_col_start>30 && seek_col_start<image_w-30)
    //                 {
    //                     int up_row=0;
    //                     int down_row=0;
    //                     int row_diff[5]={0};
    //                     if(feature_raw_l)
    //                     {
    //                         int k=0;
    //                         for(int i=seek_col_start-10;i>seek_col_start-25;i=i-3)
    //                         {
    //                             up_row = 0;
    //                             down_row = 0;
    //                             for(int j=seek_row_start;j<seek_row_end;j++)
    //                             {
    //                                 if(image[j+1][i]==black_point && image[j][i]==white_point && image[j-1][i]==white_point)
    //                                 {
    //                                     down_row=j;
    //                                 }
    //                                 else if(image[j+1][i]==white_point && image[j][i]==white_point && image[j-1][i]==black_point)
    //                                 {
    //                                     up_row=j;
    //                                 }
    //                             }
    //                             if(up_row<up_feature_row && up_row!=0)
    //                             {
    //                                 up_feature_row=up_row;
    //                             }
    //                             if(up_row!=0 && down_row!=0)
    //                             {
    //                                 row_diff[k]=down_row-up_row;
    //                             }
    //                             k++;
    //                         }
    //                         if(detect_row_diff(row_diff, 10) &&row_diff[0]!=0 && row_diff[1]!=0 && row_diff[2]!=0 && row_diff[3]!=0 && row_diff[4]!=0)
    //                         {
    //                             feature_row_run=detect_feature_row;
    //                             break;
    //                         }
    //                     }
    //                 }
    //             }
    //         }
    //     }
    //     up_feature_row=MT9V03X_H-1;
    //     if(T_index_list[feature_T_index]==0&&feature_T_right==1)
    //     {
    //         for(int i=MT9V03X_H-11;i>=feature_raw;i--)
    //         {
    //             detect_feature_row=0;
    //             if(right_line_list[i-1]-right_line_list[i]>10 || right_line_list[i-2]-right_line_list[i]>10 || right_line_list[i-3]-right_line_list[i]>10 || right_line_list[i-4]-right_line_list[i]>10)
    //             {
    //                 detect_feature_row=(uint8)i;
    //                 feature_raw_r=1;
    //             }
    //             if(detect_feature_row >20)
    //             {
    //                 int seek_row_start=detect_feature_row-20;
    //                 int seek_row_end=detect_feature_row+10;
    //                 int seek_col_start=right_line_list[detect_feature_row]+10;;
    //                 if(seek_col_start>30 && seek_col_start<image_w-30)
    //                 {
    //                     int up_row=0;
    //                     int down_row=0;
    //                     int row_diff[5]={0};
    //                     if(feature_raw_r)
    //                     {
    //                         int k=0;
    //                         for(int i=seek_col_start;i<seek_col_start+15;i=i+3)
    //                         {
    //                             up_row = 0;
    //                             down_row = 0;
    //                             for(int j=seek_row_start;j<seek_row_end;j++)
    //                             {
    //                                 if(image[j+1][i]==black_point && image[j][i]==white_point && image[j-1][i]==white_point)
    //                                 {
    //                                     down_row=j;
    //                                 }
    //                                 else if(image[j+1][i]==white_point && image[j][i]==white_point && image[j-1][i]==black_point)
    //                                 {
    //                                     up_row=j;
    //                                 }
    //                             }
    //                             if(up_row<up_feature_row && up_row!=0)
    //                             {
    //                                 up_feature_row=up_row;
    //                             }
    //                             if(up_row!=0 && down_row!=0)
    //                             {
    //                                 row_diff[k]=down_row-up_row;
    //                             }
    //                             k++;
    //                         }
    //                         if(detect_row_diff(row_diff, 10) &&row_diff[0]!=0 && row_diff[1]!=0 && row_diff[2]!=0 && row_diff[3]!=0 && row_diff[4]!=0)
    //                         {
    //                             feature_row_run=detect_feature_row;
    //                             break;
    //                         }
    //                     }
    //                 }
    //             }
    //         }
    //     }
    // }
    // else if(feature_label==0)
    // {
    //     if((T_index_list[feature_T_index]==-1||T_index_list[feature_T_index]==0)&&left_lost_times>=3)
    //     { 
    //         for(int i=MT9V03X_H-41;i>=feature_raw;i--)
    //         {
    //             detect_feature_row=0;
    //             if(left_line_list[i]-left_line_list[i-1]>10 || left_line_list[i]-left_line_list[i-2]>10 || left_line_list[i]-left_line_list[i-3]>10 || left_line_list[i]-left_line_list[i-4]>10)
    //             {
    //                 detect_feature_row=(uint8)i;
    //                 feature_raw_l=1;
    //             }
    //             if(detect_feature_row >20)
    //             {
    //                 int seek_row_start=detect_feature_row-20;
    //                 int seek_row_end=detect_feature_row+15;
    //                 int seek_col_start=left_line_list[detect_feature_row]-10;;
    //                 if(seek_col_start>30 && seek_col_start<image_w-30)
    //                 {
    //                     int up_row=0;
    //                     int down_row=0;
    //                     int row_diff[5]={0};
    //                     if(feature_raw_l)
    //                     {
    //                         int k=0;
    //                         for(int i=seek_col_start-10;i>seek_col_start-25;i=i-3)
    //                         {
    //                             up_row = 0;
    //                             down_row = 0;
    //                             for(int j=seek_row_start;j<seek_row_end;j++)
    //                             {
    //                                 if(image[j+1][i]==black_point && image[j][i]==white_point && image[j-1][i]==white_point)
    //                                 {
    //                                     down_row=j;
    //                                 }
    //                                 else if(image[j+1][i]==white_point && image[j][i]==white_point && image[j-1][i]==black_point)
    //                                 {
    //                                     up_row=j;
    //                                 }
    //                             }
    //                             if(up_row<up_feature_row && up_row!=0)
    //                             {
    //                                 up_feature_row=up_row;
    //                             }
    //                             if(up_row!=0 && down_row!=0)
    //                             {
    //                                 row_diff[k]=down_row-up_row;
    //                             }
    //                             k++;
    //                         }
    //                         if(detect_row_diff(row_diff, 10) &&row_diff[0]!=0 && row_diff[1]!=0 && row_diff[2]!=0 && row_diff[3]!=0 && row_diff[4]!=0)
    //                         {
    //                             feature_row_run=detect_feature_row;
    //                             feature_label = 1;
    //                             break;
    //                         }
    //                     }
    //                 }
    //             }
    //         }
    //     }
    //     up_feature_row=MT9V03X_H-1;
    //     if((T_index_list[feature_T_index]==1||T_index_list[feature_T_index]==0)&&right_lost_times>=3)
    //     {
    //         for(int i=MT9V03X_H-41;i>=feature_raw;i--)
    //         {
    //             detect_feature_row=0;
    //             if(right_line_list[i-1]-right_line_list[i]>10 || right_line_list[i-2]-right_line_list[i]>10 || right_line_list[i-3]-right_line_list[i]>10 || right_line_list[i-4]-right_line_list[i]>10)
    //             {
    //                 detect_feature_row=(uint8)i;
    //                 feature_raw_r=1;
    //             }
    //             if(detect_feature_row >20)
    //             {
    //                 int seek_row_start=detect_feature_row-20;
    //                 int seek_row_end=detect_feature_row+15;
    //                 int seek_col_start=right_line_list[detect_feature_row]+10;;
    //                 if(seek_col_start>30 && seek_col_start<image_w-30)
    //                 {
    //                     int up_row=0;
    //                     int down_row=0;
    //                     int row_diff[5]={0};
    //                     if(feature_raw_r)
    //                     {
    //                         int k=0;
    //                         for(int i=seek_col_start;i<seek_col_start+15;i=i+3)
    //                         {
    //                             up_row = 0;
    //                             down_row = 0;
    //                             for(int j=seek_row_start;j<seek_row_end;j++)
    //                             {
    //                                 if(image[j+1][i]==black_point && image[j][i]==white_point && image[j-1][i]==white_point)
    //                                 {
    //                                     down_row=j;
    //                                 }
    //                                 else if(image[j+1][i]==white_point && image[j][i]==white_point && image[j-1][i]==black_point)
    //                                 {
    //                                     up_row=j;
    //                                 }
    //                             }
    //                             if(up_row<up_feature_row && up_row!=0)
    //                             {
    //                                 up_feature_row=up_row;
    //                             }
    //                             if(up_row!=0 && down_row!=0)
    //                             {
    //                                 row_diff[k]=down_row-up_row;
    //                             }
    //                             k++;
    //                         }
    //                         if(detect_row_diff(row_diff, 10) &&row_diff[0]!=0 && row_diff[1]!=0 && row_diff[2]!=0 && row_diff[3]!=0 && row_diff[4]!=0)
    //                         {
    //                             feature_row_run=detect_feature_row;
    //                             feature_label = 1;
    //                             break;
    //                         }                         
    //                     }
    //                 }
    //             }
    //         }
    //     }
    // }

    int seek_row_start=0;
    int seek_row_end=0;
    int seek_col_start=0;
    detect_feature_row_l=0;
    if(left_lost_times!=0 )
    {
        for(int i=MT9V03X_H-40;i>=20;i--)
        {
            int left_line_lost_label=0;
            for(int j=-15;j<=15;j++)
            {
                if(image[i+j][40]==white_point)
                {
                    left_line_lost_label++;
                }     
            }
            if(left_line_list[i]>50 && left_line_list[i]<image_w-51 && (left_line_list[i]-left_line_list[i-1]>row_diff_feature || left_line_list[i]-left_line_list[i-2]>row_diff_feature || left_line_list[i]-left_line_list[i-3]>row_diff_feature || left_line_list[i]-left_line_list[i-4]>row_diff_feature)
                && left_line_lost_label>1 )
            {   
                detect_feature_row=(uint8)i;
                detect_feature_row_l=detect_feature_row;
                feature_raw_l=1;
            }
            if(detect_feature_row<40)
            {
                activate_condition_l=1;
            }
            
            if(feature_raw_l&&activate_condition_l&& (T_index_list[feature_T_index]==-1||T_index_list[feature_T_index]==0))
            {
                if(left_line_list[detect_feature_row]>50 && left_line_list[detect_feature_row]<image_w-51)
                {
                    if(detect_feature_row>20 &&detect_feature_row<60)
                    {
                        seek_col_start=left_line_list[detect_feature_row]-35;
                    }
                    else if(detect_feature_row>=60)
                    {
                        seek_col_start=left_line_list[detect_feature_row]-35;
                    }
                    for(int i=100;i>20;i--)
                    {
                        if(image[i-1][seek_col_start]==white_point && image[i][seek_col_start]==white_point &&image[i+1][seek_col_start]==black_point)
                        {
                            feature_row_down=i;
                            break;
                        }
                    }
                    for(int i=100;i>20;i--)
                    {
                        if(image[i-1][seek_col_start]==black_point && image[i][seek_col_start]==white_point &&image[i+1][seek_col_start]==white_point)
                        {
                            feature_row_up=i;
                            break;
                        }
                    }
                    if(feature_row_down!=0 && feature_row_up!=0 && feature_row_down-feature_row_up<10)
                    {
                        feature_label=1;
                    }
                }
                    // int k=0;
                    // for(int i=seek_col_start;i>seek_col_start-10;i=i-2)
                    // {
                    //     up_row = 0;
                    //     down_row = 0;
                    //     for(int j=seek_row_start;j<seek_row_end;j++)
                    //     {
                    //         if(image[j+1][i]==black_point && image[j][i]==white_point && image[j-1][i]==white_point)
                    //         {
                    //             down_row=j;
                    //         }
                    //         else if(image[j+1][i]==white_point && image[j][i]==white_point && image[j-1][i]==black_point)
                    //         {
                    //             up_row=j;
                    //         }
                    //     }
                    //     if(up_row<up_feature_row)
                    //     {
                    //         up_feature_row=up_row;
                    //     }
                    //     if(up_row!=0 && down_row!=0)
                    //     {
                    //         row_diff[k]=down_row-up_row;
                    //     }
                    //     k++;
                    // }
            }
            if(feature_label==1)
            {
                break;
            }

            
        }
        
    }
    if(right_lost_times!=0 )
    {
        for(int i=MT9V03X_H-40;i>=20;i--)
        {
            int right_line_lost_label=0;
            for(int j=-15;j<=15;j++)
            {
                if(image[i+j][image_w-31]==white_point)
                {
                    right_line_lost_label++;
                }
            }
            if(right_line_list[i]>50 && right_line_list[i]<image_w-51 && (right_line_list[i-1]-right_line_list[i]>row_diff_feature || right_line_list[i-2]-right_line_list[i]>row_diff_feature || right_line_list[i-3]-right_line_list[i]>row_diff_feature || right_line_list[i-4]-right_line_list[i]>row_diff_feature) 
                && right_line_lost_label>1)
            {
                detect_feature_row=(uint8)i;
                feature_raw_r=1;
            }
            if(detect_feature_row<45)
            {
                activate_condition_r=1;
            }
            if(feature_raw_r && activate_condition_r&&(T_index_list[feature_T_index]==1||T_index_list[feature_T_index]==0))
            {
                if(right_line_list[detect_feature_row]>50 && right_line_list[detect_feature_row]<image_w-51)
                {
                    seek_row_start=detect_feature_row-20;
                    seek_row_end=detect_feature_row+30;
                    if(detect_feature_row>20 &&detect_feature_row<60)
                    {
                        seek_col_start=right_line_list[detect_feature_row]+30;
                    }
                    else if(detect_feature_row>=60)
                    {
                    seek_col_start=right_line_list[detect_feature_row]+40;
                    }
                    for(int i=seek_row_end;i>20;i--)
                    {
                        if( image[i][seek_col_start]==white_point&&image[i+1][seek_col_start]==black_point)
                        {
                            feature_row_down=i;
                            break;
                        }
                    }
                    for(int i=seek_row_end;i>20;i--)
                    {
                        if(image[i-1][seek_col_start]==black_point && image[i][seek_col_start]==white_point )
                        {
                            feature_row_up=i;
                            break;
                        }
                    }
                    if(feature_row_down!=0 && feature_row_up!=0 && feature_row_down-feature_row_up<10)
                    {
                        feature_label=1;
                    }
                }
                if(feature_label==1)
                {
                    break;
                }
                    // int k=0;
                    // for(int i=seek_col_start;i<seek_col_start+10;i=i+2)
                    // {
                    //     up_row = 0;
                    //     down_row = 0;
                    //     for(int j=seek_row_start;j<seek_row_end;j++)
                    //     {
                    //         if(image[j+1][i]==black_point && image[j][i]==white_point && image[j-1][i]==white_point)
                    //         {
                    //             down_row=j;
                    //         }
                    //         else if(image[j+1][i]==white_point && image[j][i]==white_point && image[j-1][i]==black_point)
                    //         {
                    //             up_row=j;
                    //         }
                    //     }
                    //     if(up_row<up_feature_row)
                    //     {
                    //         up_feature_row=up_row;
                    //     }
                    //     if(up_row!=0 && down_row!=0)
                    //     {
                    //         row_diff[k]=down_row-up_row;
                    //     }
                    //     k++;
                    // }
                // }
                // if(detect_row_diff(row_diff, 10) &&row_diff[0]!=0 && row_diff[1]!=0 && row_diff[2]!=0 && row_diff[3]!=0 && row_diff[4]!=0)
                // {
                //     feature_label = 1;
                    
            }
        }
        

    }
    // printf("detect_feature_row=%d, feature_raw_l=%d, feature_raw_r=%d, feature_label=%d\n",detect_feature_row, feature_raw_l, feature_raw_r, feature_label);
    

            
            // if(feature_raw_r)
            // {
            //     seek_col_start=right_line_list[detect_feature_row]+30;
            // }
            // if(feature_raw_l)
            // {
            //     seek_col_start=left_line_list[detect_feature_row]-30;
            // }
            // image[seek_row_start][seek_col_start-10]=5;
            // image[seek_row_end][seek_col_start-25]=5;
            // if(seek_col_start>30 && seek_col_start<image_w-30)
            // {
            
                // int up_row=0;
                // int down_row=0;
                // int row_diff[5]={0};

    // printf("detect_feature_row=%d, feature_raw_l=%d, feature_raw_r=%d\n",detect_feature_row, feature_raw_l, feature_raw_r);
    
        // printf("feature_label=%d, up_feature_row=%d\n", feature_label, up_feature_row);

    // feature_label=0;
    // feature_raw_r=0;
    // feature_raw_l=0; 
    // detect_feature_row=0;
    // // printf("left_lost_times=%d, right_lost_times=%d\n",left_lost_times,right_lost_times);
    // if((left_lost_times!=0 || right_lost_times!=0) )
    // {
    //     for(int i=MT9V03X_H-31;i>=feature_raw;i--)
    //     {
    //         if((left_line_list[i]-left_line_list[i-1]>15 || left_line_list[i]-left_line_list[i-2]>15 ||left_line_list[i]-left_line_list[i-3]>15 || left_line_list[i]-left_line_list[i-4]>15)
    //             && (left_line_list[i-5]==0 || left_line_list[i-4]==0 || left_line_list[i-3]==0 || left_line_list[i-2]==0 || left_line_list[i-1]==0 || left_line_list[i]==0 ||left_line_list[i+1]==0 || left_line_list[i+2]==0 || left_line_list[i+3]==0 || left_line_list[i+4]==0 || left_line_list[i+5]==0 
    //                 ||left_line_list[i-6]==0 || left_line_list[i+6]==0 || left_line_list[i-7]==0 || left_line_list[i+7]==0 || left_line_list[i-8]==0 || left_line_list[i+8]==0 || left_line_list[i-9]==0 || left_line_list[i+9]==0 || left_line_list[i-10]==0 || left_line_list[i+10]==0))
    //         {   
    //             feature_label = 1;
    //             feature_raw_l=(uint8)i;
    //             break;
    //         }
    //         else if((right_line_list[i-1]-right_line_list[i]>15 || right_line_list[i-2]-right_line_list[i]>15 || right_line_list[i-3]-right_line_list[i]>15 || right_line_list[i-4]-right_line_list[i]>15) &&
    //                 (right_line_list[i-5]==MT9V03X_W-1 || right_line_list[i-4]==MT9V03X_W-1 || right_line_list[i-3]==MT9V03X_W-1 || right_line_list[i-2]==MT9V03X_W-1 || right_line_list[i-1]==MT9V03X_W-1 || right_line_list[i]==MT9V03X_W-1 || right_line_list[i+1]==MT9V03X_W-1 || right_line_list[i+2]==MT9V03X_W-1 || right_line_list[i+3]==MT9V03X_W-1 || right_line_list[i+4]==MT9V03X_W-1 || right_line_list[i+5]==MT9V03X_W-1 
    //                 ||right_line_list[i-6]==MT9V03X_W-1 || right_line_list[i+6]==MT9V03X_W-1 || right_line_list[i-7]==MT9V03X_W-1 || right_line_list[i+7]==MT9V03X_W-1 || right_line_list[i-8]==MT9V03X_W-1 || right_line_list[i+8]==MT9V03X_W-1 || right_line_list[i-9]==MT9V03X_W-1 || right_line_list[i+9]==MT9V03X_W-1 || right_line_list[i-10]==MT9V03X_W-1 || right_line_list[i+10]==MT9V03X_W-1))
    //         {
    //              feature_label = 1;
    //             feature_raw_r=(uint8)i;
    //             break;
    //         }
    //     }
    // }
    // if(feature_raw_l!=0)
    // {
    //     for(int i=feature_raw_l;i>=5;i--)
    //     {
    //         if(image[i-1][left_line_list[feature_raw_l-1]]==black_point)
    //         {
    //             detect_feature_row=(uint8)i;
    //             break;
    //         }
    //     }
    // }
    // else if(feature_raw_r!=0)
    // {
    //     for(int i=feature_raw_r;i>=5;i--)
    //     {
    //         if(image[i-1][right_line_list[feature_raw_r-1]]==black_point)
    //         {
    //             detect_feature_row=(uint8)i;
    //             break;
    //         }
    //     }
    // } 

}

FeatureDetectResult image_feature;
feature_result result_feature={0,0,0};
int feature_T=0;

int zhuan_left_flag=0;
int zhuan_right_flag=0;
int zhuan_row=0;
int zhuan_condition_left=0;
int zhuan_condition_right=0;
int left_run_flag=0;
int right_run_flag=0;
int run_stop_flag=0;
int run_once_flag=0;
int feature_T_left=0;
int feature_T_right=0;
uint8 condition_T=0;
float angle_T=0.0f;
int feature_condition_error=0;
int row_times=0;
int run_row_detect=0;
int row_detect_least=0;
int activate_condition_straight=0;
int output_feature=0;
int back_condition=0;
int back_condition_row=0;
int run_straight_feature_row=0;
int image_times_53=0;
int image_times_55=0;
int image_53_and_55_judge=0;
int32 encoder_count_image_judge=0;
int tun_straight_state=0;
void feature_square()
{

    if(feature_T_index==48 || feature_T_index==49 )
    {
        ERROR_IMAGE_LINE_2=ERROR_IMAGE_LINE_2_BJT;
    }
    else 
    {
        ERROR_IMAGE_LINE_2=ERROR_IMAGE_LINE_2_D;
    }

    // if(feature_T_index==26)
    // {
    //     ERROR_IMAGE_LINE_2=ERROR_IMAGE_LINE_2_D+15;
    // }
    // // if( feature_T_index==29 ||feature_T_index==30 ||feature_T_index==31 ||feature_T_index==32 
    // //     ||feature_T_index==55 ||feature_T_index==56 ||feature_T_index==57 ||feature_T_index==58 ||feature_T_index==59)
    // // {
    // //     ERROR_IMAGE_LINE_2=ERROR_IMAGE_LINE;
    // // }
    // else
    // {
    //     ERROR_IMAGE_LINE_2=ERROR_IMAGE_LINE_2_D;
    // }
    // if(feature_T_index==AMP_END_INDEX &&condition_T==0 && image_53_and_55_judge==0)
    // {
    //     encoder_count_image_judge=motor_average_distance_count_sum;
    //     image_53_and_55_judge=1;
    // }
    
    if(feature_T_index==9 &&condition_T==0 && image_53_and_55_judge==0)
    {
        encoder_count_image_judge=motor_average_distance_count_sum;
        image_53_and_55_judge=1;
    }
    //第一段惯导完成后转完第一个弯
    if(feature_T_index==63 &&condition_T==0 && image_53_and_55_judge==0)
    {
        encoder_count_image_judge=motor_average_distance_count_sum;
        image_53_and_55_judge=1;
    }
    //由下往上的电容，几型弯前的
    if(feature_T_index==65 &&condition_T==0 && image_53_and_55_judge==0)
    {
        encoder_count_image_judge=motor_average_distance_count_sum;
        image_53_and_55_judge=1;
    }
    //第二段惯导之前的两个直行
    if(feature_T_index==68 &&condition_T==0 && image_53_and_55_judge==0)
    {
        encoder_count_image_judge=motor_average_distance_count_sum;
        image_53_and_55_judge=1;
    }
    //决赛赛道的45cm
        // if(feature_T_index==53 &&condition_T==0 && image_53_and_55_judge==0)
        // {
        //     encoder_count_image_judge=motor_average_distance_count_sum;
        //     image_53_and_55_judge=1;
        // }
    //mos小圈前的直行mos判断
    // if(feature_T_index==31 &&condition_T==0 && image_53_and_55_judge==0)
    // {
    //     encoder_count_image_judge=motor_average_distance_count_sum;
    //     image_53_and_55_judge=1;
    // }


    if(feature_T_index==42 &&condition_T==0 && image_53_and_55_judge==0)
    {
        encoder_count_image_judge=motor_average_distance_count_sum;
        image_53_and_55_judge=1;
    }
    if(feature_T_index==45 &&condition_T==0 && image_53_and_55_judge==0)
    {
        encoder_count_image_judge=motor_average_distance_count_sum;
        image_53_and_55_judge=1;
    }
    
    // if(feature_T_index==45 && condition_T==0 && image_53_and_55_judge==0)
    // {
    //     encoder_count_image_judge=motor_average_distance_count_sum;
    //     image_53_and_55_judge=1;
    // }
    // if(feature_T_index == AMP_START_INDEX && condition_T==0 && image_53_and_55_judge==0)
    // {
    //     encoder_count_image_judge=motor_average_distance_count_sum;
    //     image_53_and_55_judge=1;
    // }

    // if(motor_average_distance_count_sum - encoder_count_image_judge >=encoder_count_image && image_53_and_55_judge==1 &&feature_T_index==AMP_END_INDEX)
    // {
    //     feature_T_index++;
    //     feature_T++;
    //     image_53_and_55_judge=0;
    // }
    if(motor_average_distance_count_sum - encoder_count_image_judge >=encoder_count_image && image_53_and_55_judge==1 &&feature_T_index==9)
    {
        feature_T_index++;
        feature_T++;
        image_53_and_55_judge=0;
    }
    if(motor_average_distance_count_sum - encoder_count_image_judge >=9000 && image_53_and_55_judge==1 &&feature_T_index==63)
    {
        image_53_and_55_judge=0;
    }
    if(motor_average_distance_count_sum - encoder_count_image_judge >=10000 && image_53_and_55_judge==1 &&feature_T_index==65)
    {
        image_53_and_55_judge=0;
    }
    if(motor_average_distance_count_sum - encoder_count_image_judge >=8000 && image_53_and_55_judge==1 &&feature_T_index==42)
    {
        image_53_and_55_judge=0;
    }    
    if(motor_average_distance_count_sum - encoder_count_image_judge >=8000 && image_53_and_55_judge==1 &&feature_T_index==45)
    {
        image_53_and_55_judge=0;
    }
    if(motor_average_distance_count_sum - encoder_count_image_judge >=58000 && image_53_and_55_judge==1 &&feature_T_index==68)
    {
        feature_T_index+=2;
        feature_T+=2;
        image_53_and_55_judge=0;
    }
    if(motor_average_distance_count_sum - encoder_count_image_judge >=24000 && image_53_and_55_judge==1 &&feature_T_index==53)
    {
        feature_T_index++;
        feature_T++;
        image_53_and_55_judge=0;
    }
    // if(motor_average_distance_count_sum - encoder_count_image_judge >=encoder_count_image && image_53_and_55_judge==1 &&feature_T_index==31)
    // {
    //     feature_T_index++;
    //     feature_T++;
    //     image_53_and_55_judge=0;
    // }
    // else if(motor_average_distance_count_sum - encoder_count_image_judge >=encoder_count_image && image_53_and_55_judge==1 &&feature_T_index==45)
    // {
    //     feature_T_index++;
    //     feature_T++;
    //     image_53_and_55_judge=0;
    // }

    //单独处理左侧有直行和十字的上行长直道


    // if(feature_T_index==47 && condition_T==0 )
    // {
    //     feature_T_index++;
    //     feature_T++;
    //     if(feature_T>index_num)
    //     {
    //         feature_T=1;
    //     }
    //     zhuan_left_flag=1;
    //     condition_T=1;
    //     output_feature=1;
    // }
    //路口判断
    if(T_index_list[feature_T_index]==1&& feature_raw_r==1&& condition_T==0 &&feature_label==1)
    {
        if(image_53_and_55_judge==0)
        {
            feature_T++;
            feature_T_index++;
            if(feature_T>index_num)
            {
                feature_T=1;
            }
            zhuan_left_flag=1;
            condition_T=1;
            output_feature=1;
        }

        
    }
    else if(T_index_list[feature_T_index]==-1&& feature_raw_l==1&& condition_T==0 &&feature_label==1)
    {
        if(image_53_and_55_judge==0)
        {
            feature_T++;
            feature_T_index++;
            if(feature_T>index_num)
            {
                feature_T=1;
            }
            zhuan_left_flag=1;
            condition_T=1;
            output_feature=1;
        }
        
    }
    else if(T_index_list[feature_T_index]==0&&feature_label==1 && condition_T==0 && feature_raw_l==1)
    {
        if(image_53_and_55_judge==0)
        {
            feature_T++;
            feature_T_index++;
            if(feature_T>index_num)
            {
                feature_T=1;
            }
            condition_T=1;
            output_feature=1;
            run_row_detect=0;
            feature_T_left=1;
            row_detect_least=0;
        }

    }
    else if(T_index_list[feature_T_index]==0&&feature_label==1 && condition_T==0 && feature_raw_r==1)
    {
        feature_T++;
        feature_T_index++;
        if(feature_T>index_num)
        {
            feature_T=1;
        }
        condition_T=1;
        output_feature=1;
        run_row_detect=0;
        feature_T_right=1;
        row_detect_least=0;
    }
    //停车
    if(feature_T_index==index_num)
    {
        run_stop_flag=1;
        feature_T_index=0;
    }
    if(run_stop_flag == 1 && feature_T_index==STROP_FEATURE_T)
    {
        run_once_flag=1;
    }
    // if(feature_T_index==57)
    // {
    //     run_once_flag=1;
    // }


    //转向行判断
    zhuan_row=0;
    int zhuan_row_try1=0;
    int zhuan_row_try2=0;
    int zhuan_row_temp=0;
    if(condition_T==1 &&zhuan_left_flag==1)
    {
        if(left_line_list[MT9V03X_H-40]<30)
        {
            zhuan_row_temp=0;
        }
        else if(left_line_list[MT9V03X_H-40]<60)
        {
            zhuan_row_temp=30;
        }
        else 
        {
            zhuan_row_temp=left_line_list[MT9V03X_H-40]-45;
        }
        for(int i=MT9V03X_H-50;i>20;i--)
        {
            if( image[i-1][zhuan_row_temp]==white_point && image[i][zhuan_row_temp]==white_point && image[i+1][zhuan_row_temp]==black_point && image[i+2][zhuan_row_temp]==black_point)
            {
                zhuan_row_try1=i;
                break;
            }
        }
        for(int i=MT9V03X_H-50;i>20;i--)
        {
            if( image[i-1][zhuan_row_temp]==black_point && image[i][zhuan_row_temp]==white_point && image[i+1][zhuan_row_temp]==white_point && image[i-2][zhuan_row_temp]==black_point)
            {
                zhuan_row_try2=i;
                break;
            }
        }
        if(zhuan_row_try1!=0 && zhuan_row_try2!=0 && zhuan_row_try1-zhuan_row_try2<10 && zhuan_row_try1>zhuan_row_try2 )
        {
            zhuan_row=zhuan_row_try1;
        }

    }
    else if(condition_T==1 && zhuan_right_flag==1)
    {
        if(right_line_list[MT9V03X_H-40]>image_w-30)
        {
            zhuan_row_temp=image_w-1;
        }
        else if(right_line_list[MT9V03X_H-40]>image_w-60)
        {
            zhuan_row_temp=image_w-30;
        }
        else
        {
            zhuan_row_temp=right_line_list[MT9V03X_H-40]+45;
        }
        for(int i=MT9V03X_H-50;i>10;i--)
        {
            if(  image[i-1][zhuan_row_temp]==white_point && image[i][zhuan_row_temp]==white_point && image[i+1][zhuan_row_temp]==black_point )
            {
                zhuan_row_try1=i;
                break;
            }
        }
        for(int i=MT9V03X_H-50;i>20;i--)
        {
            if( image[i-1][zhuan_row_temp]==black_point && image[i][zhuan_row_temp]==white_point && image[i+1][zhuan_row_temp]==white_point )
            {
                zhuan_row_try2=i;
                break;
            }
        }
        if(zhuan_row_try1!=0 && zhuan_row_try2!=0 && zhuan_row_try1-zhuan_row_try2<15 && zhuan_row_try1>zhuan_row_try2)
        {
            zhuan_row=zhuan_row_try1;
        }
    }


    //路口处理
    if(feature_T!=0)
    {
            //减速标志位判断    
        if(T_index_list[feature_T-1]!=0&&condition_T==1)
        {
            deceleration_label=1;
        }
        if(detect_feature_row>60)
        {
            activate_condition_straight=1;
        }
        if(condition_T==1)
        {
            if(T_index_list[feature_T-1]==-1)
            {
                if((zhuan_row>ERROR_IMAGE_LINE_2 ||detect_feature_row_l>ERROR_IMAGE_LINE_2)&&zhuan_condition_left ==0)
                {
                    zhuan_condition_left=1;
                    angle_T=attitude.yaw;
                }
            }
            else if(T_index_list[feature_T-1]==1)
            {
                if((zhuan_row>ERROR_IMAGE_LINE_2 ||detect_feature_row>ERROR_IMAGE_LINE_2)&&zhuan_condition_right ==0)
                {
                    zhuan_condition_right=1;
                    angle_T=attitude.yaw;
                }
            }
            else if(T_index_list[feature_T-1]==0)
            {
                // if(feature_T==3||feature_T==22)
                // {
                //     Add_Line(93,115,mid_line_list[20],20);
                // }
                // // if(feature_T==7||feature_T==26)
                // // {
                // //     Add_Line(93,115,mid_line_list[20],20);
                // // }
                // else
                // {
                    // if(detect_feature_row>30)
                    // {
                    //     Add_Line(mid_line_list[115],115,mid_line_list[detect_feature_row-25],detect_feature_row-25);
                    // }
                    if(detect_feature_row>40 && detect_feature_row<80)
                    {
                        if(mid_line_list[detect_feature_row+30]<120 && mid_line_list[detect_feature_row+30]>60)
                        {
                            if(mid_line_list[detect_feature_row-25]<120 && mid_line_list[detect_feature_row-25]>60)
                            {
                                // Add_Line(mid_line_list[detect_feature_row+30],detect_feature_row+30,mid_line_list[detect_feature_row-20],detect_feature_row-20);
                            }
                        }
                    }


                    // else if(detect_feature_row==0)
                    // {
                    //     if(feature_row_down<90 &&feature_row_down>30 &&feature_row_up>20 && feature_row_up<70)
                    //     {
                    //         Add_Line(mid_line_list[feature_row_down+20],feature_row_down+20,mid_line_list[feature_row_up-10],feature_row_up-10);
                    //     }
                    // }
                    // else{
                    //     if(mid_line_list[ERROR_IMAGE_LINE]>93+30)
                    //     {
                    //         mid_line_list[ERROR_IMAGE_LINE]=93+30;
                    //     }
                    //     else if(mid_line_list[ERROR_IMAGE_LINE]<93-30)
                    //     {
                    //         mid_line_list[ERROR_IMAGE_LINE]=93-30;
                    //     }
                    // }
                    // if(mid_line_list[ERROR_IMAGE_LINE]>93+15)
                    // {
                    //     mid_line_list[ERROR_IMAGE_LINE]=93+15;
                    // }
                    // else if(mid_line_list[ERROR_IMAGE_LINE]<93-15)
                    // {
                    //     mid_line_list[ERROR_IMAGE_LINE]=93-15;
                    // }
                
            }
        }
        // if(feature_T-1==0)
        // {
        //     regression(0,100,110);
        //     if(fabs(parameterB)<0.3f)
        //     {
        //         Add_Line1(mid_line_list[115],115,mid_line_list[25],25);
        //     }
        // }

    }
    

    // printf("condition_T=%d, feature_T=%d, zhuan_row=%d, zhuan_condition_left=%d, zhuan_condition_right=%d, deceleration_label=%d\n", condition_T, feature_T, zhuan_row, zhuan_condition_left, zhuan_condition_right, deceleration_label);
    //转向退出处理
    if(condition_T==1 )
    {
        if(T_index_list[feature_T-1]!=0&&(zhuan_condition_right==1||zhuan_condition_left==1))
        {
            if(get_angle_err(angle_T) > 68.0f)
            {
                output_feature=0;
            }
            if(get_angle_err(angle_T) > 72.0f)
            {
                condition_T=0;
                deceleration_label=0;
                zhuan_left_flag=0;
                zhuan_right_flag=0;
                zhuan_condition_right=0;
                zhuan_condition_left=0;
                feature_label=0;
                activate_condition_l=0;
                activate_condition_r=0;
                image_53_and_55_judge=0; 
                image_times_53=0;   
            }
            
        }
        else if(T_index_list[feature_T-1]==0)
        {
            if(detect_feature_row_l>detect_feature_row)
            {
                run_straight_feature_row=detect_feature_row_l;
            }
            else 
            {
                run_straight_feature_row=detect_feature_row;
            }
            if(run_straight_feature_row!=0 &&run_straight_feature_row>back_condition_row )
            {
                back_condition_row=run_straight_feature_row;
            }
            if(back_condition_row-run_straight_feature_row>15 &&run_straight_feature_row!=0 )
            {
                back_condition++;
            }
            if(back_condition==2)
            {
                    condition_T=0;
                    left_run_flag=0;
                    feature_T_left=0;
                    feature_T_right=0;
                    activate_condition_l=0;
                    activate_condition_r=0;
                    activate_condition_straight=0;
                    back_condition_row=0;
                    back_condition=0;
            }
            // if(feature_T_left)
            // {
            //     int row_least=0;
            //     for(int i=MT9V03X_H-1;i>10;i--)
            //     {
            //         if(image[i][col_left]==white_point && image[i-1][col_left]==white_point)
            //         {
            //             row_least=i;
            //             break;
            //         }
            //     }
            //     if(row_detect_least==0)
            //     {
            //         for(int i=50;i>10;i--)
            //         {
            //             if(image[i][col_left]==white_point && image[i-1][col_left]==white_point)
            //             {
            //                 row_detect_least=1;
            //                 break;
            //             }
            //         }
            //     }
            //     else if(row_detect_least==1)
            //     {
            //         for(int i=80;i>50;i--)
            //         {
            //             if(image[i][col_left]==white_point && image[i-1][col_left]==white_point)
            //             {
            //                 row_detect_least=2;
            //                 break;
            //             }
            //         }
            //     }
            //     else if(row_detect_least==2)
            //     {
            //         for(int i=110;i>80;i--)
            //         {
            //             if(image[i][col_left]==white_point && image[i-1][col_left]==white_point)
            //             {
            //                 row_detect_least=3;
            //                 break;
            //             }
            //         }
            //     }
            //     if(row_least>90 )
            //     {
            //         row_times++;
            //     }
            //     else
            //     {
            //         row_times=0;
            //     }
            //     // image_test = system_getval_ms()
            //     // if(row_times!=0 && row_detect_least==3 &&activate_condition_straight) 
            //     // int left_lost_count=0;
            //     // for(int i=MT9V03X_H-36;i>10;i--)
            //     // {
            //     //     if(image[i][col_left]==white_point)
            //     //     {
            //     //         left_lost_count++;
            //     //     }
            //     // }
            //     // if(left_lost_count==0)
            //     // {
            //     //     left_run_flag++;
            //     // }
            //     // if(left_run_flag!=0)
            //     // {
            //     //     condition_T=0;
            //     //     left_run_flag=0;
            //     //     feature_T_left=0;
            //     // }
            // }
            // else if( feature_T_right)
            // {
            //     int row_least=0;
            //     for(int i=MT9V03X_H-1;i>70;i--)
            //     {
            //         if(image[i][col_right]==white_point && image[i-1][col_right]==white_point)
            //         {
            //             row_least=i;
            //             break;
            //         }
            //     }
            //     if(row_detect_least==0)
            //     {
            //         for(int i=50;i>10;i--)
            //         {
            //             if(image[i][col_right]==white_point && image[i-1][col_right]==white_point)
            //             {
            //                 row_detect_least=1;
            //                 break;
            //             }
            //         }
            //     }
            //     else if(row_detect_least==1)
            //     {
            //         for(int i=80;i>50;i--)
            //         {
            //             if(image[i][col_right]==white_point && image[i-1][col_right]==white_point)
            //             {
            //                 row_detect_least=2;
            //                 break;
            //             }
            //         }
            //     }
            //     else if(row_detect_least==2)
            //     {
            //         for(int i=110;i>80;i--)
            //         {
            //             if(image[i][col_right]==white_point && image[i-1][col_right]==white_point)
            //             {
            //                 row_detect_least=3;
            //                 break;
            //             }
            //         }
            //     }
            //     if(row_least>90 )
            //     {
            //         row_times++;
            //     }
            //     else
            //     {
            //         row_times=0;
            //     }
            //     if(detect_feature_row!=0 &&detect_feature_row>back_condition_row )
            //     {
            //         back_condition_row=detect_feature_row;
            //     }
            //     if(back_condition_row-detect_feature_row>30 &&detect_feature_row!=0 )
            //     {
            //         back_condition=1;
            //     }
            //     // if((row_times!=0 &&row_detect_least==3 &&activate_condition_straight)||(back_condition==1))
            //     if(back_condition==1)
            //     {
            //         back_condition_row=0;
            //         back_condition=0;
            //         condition_T=0;
            //         right_run_flag=0;
            //         feature_T_right=0;
            //         activate_condition=0;
            //         activate_condition_straight=0;
            //     }
            //     // int right_lost_count=0;
            //     // for(int i=MT9V03X_H-36;i>10;i--)
            //     // {
            //     //     if(image[i][col_right]==white_point)
            //     //     {
            //     //         right_lost_count++;
            //     //     }
            //     // }
            //     // if(right_lost_count==0)
            //     // {
            //     //     right_run_flag++;
            //     // }
            //     // if(right_run_flag!=0)
            //     // {
            //     //     condition_T=0;
            //     //     right_run_flag=0;
            //     //     feature_T_right=0;
            //     // }
            // }   
        }
        
    }
    

    

    if(zhuan_condition_left==1 &&output_feature==1)
    {
        mid_line_list[ERROR_IMAGE_LINE]=0;
    }
    else if(zhuan_condition_right==1 &&output_feature==1)
    {
        mid_line_list[ERROR_IMAGE_LINE]=image_w-1;
    }
    // detect_longest_col();
    // if((condition_T==0||(T_index_list[feature_T-1]==0&&condition_T==1)))
    // {
    //     if(longest_col<50)
    //     {
    //         Add_Line(93,115,mid_line_list[60],60);
    //     }
    //     else
    //     {
    //         Add_Line(93,115,mid_line_list[longest_col+10],longest_col+10);
    //     }   
        
    // }

    // image_feature.left_feature_flag1=0;
    // image_feature.left_feature_flag2=0;
    // image_feature.right_feature_flag1=0;
    // image_feature.right_feature_flag2=0;
    // image_feature.height_feature_flag1=0;
    // image_feature.height_feature_flag2=0;
    // result_feature.left=0;
    // result_feature.right=0;
    // result_feature.height=0;
    // // printf("feature_label=%d, detect_feature_row=%d\n",feature_label,detect_feature_row);
    // if((feature_label==1 && detect_feature_row!=0)||condition_T!=0)
    // {
    //     int detect_row=0;
    //     if(detect_feature_row>15)
    //     {
    //         detect_row=height_start_raw-3;
    //     }
    //     else if(detect_feature_row!=0)
    //     {
    //         detect_row=detect_feature_row-3;
    //     }
    //     else{
    //         detect_row=height_start_raw+10;
    //     }
    //     int left_feature_col_count[3]={0};
    //     for(int j=left_start_col;j<left_start_col+5;j+=2)
    //     {
    //         for(int i=detect_row;i<height_end_raw;i++)
    //         {
    //             //左侧特征：上白下黑
    //             if(image[i-1][j]==black_point && image[i][j]==white_point && image[i+1][j]==white_point )
    //             {
    //                 image_feature.left_feature_flag1=i;
    //             }
    //             else if(image[i-1][j]==white_point && image[i][j]==white_point && image[i+1][j]==black_point)
    //             {
    //                 image_feature.left_feature_flag2=i;
    //             }
    //         }
    //         left_feature_col_count[(j-left_start_col)/2]=image_feature.left_feature_flag2-image_feature.left_feature_flag1;
    //     }
    //     int right_feature_col_count[3]={0};
    //     for(int j=right_start_col-5;j<right_start_col;j+=2)
    //     {
    //         for(int i=detect_row;i<height_end_raw;i++)
    //         {
    //             //右侧特征：上白下黑
    //             if(image[i-1][j]==black_point && image[i][j]==white_point && image[i+1][j]==white_point)
    //             {
    //                 image_feature.right_feature_flag1=i;
    //             }
    //             else if(image[i-1][j]==white_point && image[i][j]==white_point && image[i+1][j]==black_point)
    //             {
    //                 image_feature.right_feature_flag2=i;
    //             }
    //         }
    //         right_feature_col_count[(j-(right_start_col-5))/2]=image_feature.right_feature_flag2-image_feature.right_feature_flag1;
    //     }
    //     if(abs(left_feature_col_count[0]-left_feature_col_count[1])<3 && abs(left_feature_col_count[0]-left_feature_col_count[2])<3 && abs(left_feature_col_count[1]-left_feature_col_count[2])<3 &&left_feature_col_count[0]>2 && left_feature_col_count[1]>2 && left_feature_col_count[2]>2)
    //     {
    //         result_feature.left=1;
    //     }
    //     if(abs(right_feature_col_count[0]-right_feature_col_count[1])<3 && abs(right_feature_col_count[0]-right_feature_col_count[2])<3 && abs(right_feature_col_count[1]-right_feature_col_count[2])<3 && right_feature_col_count[0]>2 && right_feature_col_count[1]>2 && right_feature_col_count[2]>2)
    //     {
    //         result_feature.right=1;
    //     }
    //     for(int i=left_start_col+20;i<right_start_col-20;i++)
    //     {
    //         //上行特征：左白右黑（从左向右）
    //         if(image[detect_row][i-1]==black_point && image[detect_row][i]==white_point && image[detect_row][i+1]==white_point)
    //         {
    //             image_feature.height_feature_flag1=i;
    //             break;
    //         }
    //     }
    //     for(int i=right_start_col-35;i>left_start_col+35;i--)
    //     {
    //         //上行特征：左白右黑（从右向左）
    //         if(image[detect_row][i-1]==white_point && image[detect_row][i]==white_point && image[detect_row][i+1]==black_point)
    //         {
    //             image_feature.height_feature_flag2=i;
    //             break;
    //         }
    //     }
    //     if(image_feature.height_feature_flag2!=0&&image_feature.height_feature_flag1!=0)
    //     {
    //         result_feature.height=1;
    //     }
    // }
}

void turn(uint8 direction)
{
    if(direction==1)
    {
        if(image_feature.left_feature_flag2!=0)
        {
            left_line_offset(image_feature.left_feature_flag2+15);
        }
        else{
            left_line_offset(feature_raw_l+15);
        }
    }
    else if(direction==2)
    {
        if(image_feature.right_feature_flag2!=0)
        {
            right_line_offset(image_feature.right_feature_flag2+15);
        }
        else{
            right_line_offset(feature_raw_r+15);
        }
    }
    else if(direction==0)
    {
        if(image_feature.height_feature_flag1!=0 &&image_feature.height_feature_flag2!=0)
        {
            Add_Line(94,119,(image_feature.height_feature_flag1+image_feature.height_feature_flag2)/2,5);
        }
    }
}

uint8 speed_select_label = 0;
float speed_select_angle = 0.0f;
int T_corner=0;
int times_T=0;
uint8 condition_T_angle=0;

int error_image;
int error_image_last;

void get_error_image()
{
    error_image_last=error_image;

    error_image=93-mid_line_list[ERROR_IMAGE_LINE];
}

void image_draw_pre()
{
    for(int i = 0; i < MT9V03X_H;i++)
    {
        image[i][left_line_list[i]]  = 5;
        image[i][right_line_list[i]]  = 15;
        image[i][mid_line_list[i]]  = 10;        
        image[i][col_left]  = 10;
        image[i][col_right]  = 10;
        image[i][50]  = 10;
        image[i][image_w-51]=10;
    }
    for(int i=0 ;i< MT9V03X_W;i++)
    {
        image[20][i] = 5;
        image[90][i] = 5;
        image[80][i] = 5;
        image[detect_feature_row][i]=15;
    }

}

int white_sheet_points=0;
int judge_white_sheet=0;

uint8 white_sheet_start(uint8 (*source_image)[MT9V03X_W])
{
    image = source_image;
    Binarization();
    int white_all_points=0;
    judge_white_sheet++;
    for(int i=MT9V03X_H-1;i>MT9V03X_H-40;i--)
    {
        for(int j=0;j<MT9V03X_W;j++)
        {
            if(image[i][j]==white_point)
            {
                white_all_points++;
            }
        }
    }
    if(judge_white_sheet==10)
    {
        white_sheet_points=white_all_points;
    }
    else if(judge_white_sheet%150==0)
    {
        if(abs(white_all_points-white_sheet_points)>250)
        {
            image_start_flag=1;
        }
    }
    // image_start_flag = 1;

    return image_start_flag;
}

void detect_threshold(uint8 (*source_image)[MT9V03X_W])
{
    image = source_image;
    Binarization();
}

void ins_image(uint8 (*source_image)[MT9V03X_W])
{
    image = source_image;
    Binarization();
    output_feature=0;
    condition_T=0;
    deceleration_label=0;
    zhuan_left_flag=0;
    zhuan_right_flag=0;
    zhuan_condition_right=0;
    zhuan_condition_left=0;
    feature_label=0;
    activate_condition_r=0;
    activate_condition_l=0;
    image_53_and_55_judge=0;
    image_times_53=0;
    error_image=0;
    error_image_last=0;

    if(ins_current_data_sheet_index == 0)
    {
        feature_T_index=62;     
        feature_T=62;
    }
    if(ins_current_data_sheet_index == 1)
    {
        feature_T_index=72;     
        feature_T=72;
    }
    if(ins_current_data_sheet_index == 2)
    {
        feature_T_index=54 - 4;     
        feature_T=54 - 4;
    }
    if(ins_current_data_sheet_index == 3)
    {
        feature_T_index=62;
        feature_T=62;
    }
    if(ins_current_data_sheet_index ==6) // 4
    {
        feature_T_index=3;     
        feature_T=3;
    }
    if(ins_current_data_sheet_index ==5)
    {
        feature_T_index=13;     
        feature_T=13;
    }

}

void image_process(uint8 (*source_image)[MT9V03X_W])
{
    image = source_image;
    Binarization();
    remove_white_noise_filter();
    search_line();
    detect_feature();
    feature_square();
        // feature_process();
        // image_draw_pre();
    get_error_image();
    
    // remove_white_noise_filter();
    
}
