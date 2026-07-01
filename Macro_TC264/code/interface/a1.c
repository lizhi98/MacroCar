#include "image_process.h"

#define ERROR_IMAGE_LINE 45
// #define index_num 6
// int T_index_list[index_num]={-1,-1,-1,-1,-1,-1};
// int T_index_list[index_num]={1,-1,-1,1,1,1};
// #define index_num 17
// int T_index_list[index_num]={1,1,-1,-1,1,1,1,-1,-1,0,-1,-1,-1,0,1,0,1};

// #define index_num 12
// int T_index_list[index_num]={-1,-1,0,1,1,0,-1,-1,-1,0,0,-1};

// #define index_num 21
// int T_index_list[index_num]={1,0,1,1,0,0,-1,-1,-1,1,0,1,1,1,1,1,0,1,0,0,1};

// #define index_num 23
// int T_index_list[index_num]={-1,-1,1,1,0,-1,-1,0,-1,-1,-1,1,0,1,0,0,1,1,-1,-1,1,-1,-1};

#define index_num 23
int T_index_list[index_num]={-1,-1,1,0,0,1,1,1,-1,-1,0,-1,1,1,0,0,1,1,-1,-1,1,-1,-1};

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
        if(img_threshold-threshold>5)
        {
            img_threshold=img_threshold-5;
        }
        else if(threshold-img_threshold>5)
        {
            img_threshold=img_threshold+5;
        }
        else
        {
            img_threshold=threshold;
        }
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
            if(image[i][0]==white_point)
            {
                left_lost_times++;
            }
            if(image[i][image_w-1]==white_point)
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
int longest_col=0;
void detect_longest_col()
{
    longest_col=0;
    for(int i=0;i<MT9V03X_H;i++)
    {
        if(left_line_list[i]==0&&right_line_list[i]==image_w-1)
        {
            longest_col++;
        }
        else{
            break;
        }
    }
}

uint8 feature_label=0;
uint8 deceleration_label=0;
uint8 feature_raw_l=0;
uint8 feature_raw_r=0;
int feature_T_index=0;
int feature_T_left=0;
int feature_T_right=0;
#define feature_raw 10    
uint8 detect_feature_row=0;
int up_feature_row=MT9V03X_H-1;
int feature_row_run=0;
void detect_feature()
{
    printf("left_lost_times=%d, right_lost_times=%d\n", left_lost_times, right_lost_times);
    feature_raw_l=0;
    feature_raw_r=0;
    detect_feature_row=0;
    up_feature_row=MT9V03X_H-1;
    if(feature_label==1)
    {
        if(T_index_list[feature_T_index-1]==0 &&feature_T_left==1)
        {
            for(int i=MT9V03X_H-11;i>=feature_raw;i--)
            {
                detect_feature_row=0;
                if(left_line_list[i]-left_line_list[i-1]>10 || left_line_list[i]-left_line_list[i-2]>10 || left_line_list[i]-left_line_list[i-3]>10 || left_line_list[i]-left_line_list[i-4]>10)
                {
                    detect_feature_row=(uint8)i;
                }
                if(detect_feature_row >20)
                {
                    int seek_row_start=detect_feature_row-20;
                    int seek_row_end=detect_feature_row+10;
                    int seek_col_start=left_line_list[detect_feature_row]-10;;
                    if(seek_col_start>30 && seek_col_start<image_w-30)
                    {
                        int up_row=0;
                        int down_row=0;
                        int row_diff[5]={0};
                        if(feature_raw_l)
                        {
                            int k=0;
                            for(int i=seek_col_start-10;i>seek_col_start-25;i=i-3)
                            {
                                up_row = 0;
                                down_row = 0;
                                for(int j=seek_row_start;j<seek_row_end;j++)
                                {
                                    if(image[j+1][i]==black_point && image[j][i]==white_point && image[j-1][i]==white_point)
                                    {
                                        down_row=j;
                                    }
                                    else if(image[j+1][i]==white_point && image[j][i]==white_point && image[j-1][i]==black_point)
                                    {
                                        up_row=j;
                                    }
                                }
                                if(up_row<up_feature_row && up_row!=0)
                                {
                                    up_feature_row=up_row;
                                }
                                if(up_row!=0 && down_row!=0)
                                {
                                    row_diff[k]=down_row-up_row;
                                }
                                k++;
                            }
                            if(detect_row_diff(row_diff, 10) &&row_diff[0]!=0 && row_diff[1]!=0 && row_diff[2]!=0 && row_diff[3]!=0 && row_diff[4]!=0)
                            {
                                feature_row_run=detect_feature_row;
                                break;
                            }
                        }
                    }
                }
            }
        }
        up_feature_row=MT9V03X_H-1;
        if(T_index_list[feature_T_index]==0&&feature_T_right==1)
        {
            for(int i=MT9V03X_H-11;i>=feature_raw;i--)
            {
                detect_feature_row=0;
                if(right_line_list[i-1]-right_line_list[i]>10 || right_line_list[i-2]-right_line_list[i]>10 || right_line_list[i-3]-right_line_list[i]>10 || right_line_list[i-4]-right_line_list[i]>10)
                {
                    detect_feature_row=(uint8)i;
                    feature_raw_r=1;
                }
                if(detect_feature_row >20)
                {
                    int seek_row_start=detect_feature_row-20;
                    int seek_row_end=detect_feature_row+10;
                    int seek_col_start=right_line_list[detect_feature_row]+10;;
                    if(seek_col_start>30 && seek_col_start<image_w-30)
                    {
                        int up_row=0;
                        int down_row=0;
                        int row_diff[5]={0};
                        if(feature_raw_r)
                        {
                            int k=0;
                            for(int i=seek_col_start;i<seek_col_start+15;i=i+3)
                            {
                                up_row = 0;
                                down_row = 0;
                                for(int j=seek_row_start;j<seek_row_end;j++)
                                {
                                    if(image[j+1][i]==black_point && image[j][i]==white_point && image[j-1][i]==white_point)
                                    {
                                        down_row=j;
                                    }
                                    else if(image[j+1][i]==white_point && image[j][i]==white_point && image[j-1][i]==black_point)
                                    {
                                        up_row=j;
                                    }
                                }
                                if(up_row<up_feature_row && up_row!=0)
                                {
                                    up_feature_row=up_row;
                                }
                                if(up_row!=0 && down_row!=0)
                                {
                                    row_diff[k]=down_row-up_row;
                                }
                                k++;
                            }
                            if(detect_row_diff(row_diff, 10) &&row_diff[0]!=0 && row_diff[1]!=0 && row_diff[2]!=0 && row_diff[3]!=0 && row_diff[4]!=0)
                            {
                                feature_row_run=detect_feature_row;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    else if(feature_label==0)
    {
        if((T_index_list[feature_T_index]==-1||T_index_list[feature_T_index]==0)&&left_lost_times>=3)
        {
            
            for(int i=MT9V03X_H-41;i>=feature_raw;i--)
            {
                detect_feature_row=0;
                if(left_line_list[i]-left_line_list[i-1]>10 || left_line_list[i]-left_line_list[i-2]>10 || left_line_list[i]-left_line_list[i-3]>10 || left_line_list[i]-left_line_list[i-4]>10)
                {
                    detect_feature_row=(uint8)i;
                    feature_raw_l=1;
                }
                if(detect_feature_row >20)
                {
                    int seek_row_start=detect_feature_row-20;
                    int seek_row_end=detect_feature_row+15;
                    int seek_col_start=left_line_list[detect_feature_row]-10;;
                    if(seek_col_start>30 && seek_col_start<image_w-30)
                    {
                        int up_row=0;
                        int down_row=0;
                        int row_diff[5]={0};
                        if(feature_raw_l)
                        {
                            int k=0;
                            for(int i=seek_col_start-10;i>seek_col_start-25;i=i-3)
                            {
                                up_row = 0;
                                down_row = 0;
                                for(int j=seek_row_start;j<seek_row_end;j++)
                                {
                                    if(image[j+1][i]==black_point && image[j][i]==white_point && image[j-1][i]==white_point)
                                    {
                                        down_row=j;
                                    }
                                    else if(image[j+1][i]==white_point && image[j][i]==white_point && image[j-1][i]==black_point)
                                    {
                                        up_row=j;
                                    }
                                }
                                if(up_row<up_feature_row && up_row!=0)
                                {
                                    up_feature_row=up_row;
                                }
                                if(up_row!=0 && down_row!=0)
                                {
                                    row_diff[k]=down_row-up_row;
                                }
                                k++;
                            }
                            if(detect_row_diff(row_diff, 10) &&row_diff[0]!=0 && row_diff[1]!=0 && row_diff[2]!=0 && row_diff[3]!=0 && row_diff[4]!=0)
                            {
                                feature_row_run=detect_feature_row;
                                feature_label = 1;
                                break;
                            }
                        }
                    }
                }
            }
        }
        up_feature_row=MT9V03X_H-1;
        if((T_index_list[feature_T_index]==1||T_index_list[feature_T_index]==0)&&right_lost_times>=3)
        {
            for(int i=MT9V03X_H-41;i>=feature_raw;i--)
            {
                detect_feature_row=0;
                if(right_line_list[i-1]-right_line_list[i]>10 || right_line_list[i-2]-right_line_list[i]>10 || right_line_list[i-3]-right_line_list[i]>10 || right_line_list[i-4]-right_line_list[i]>10)
                {
                    detect_feature_row=(uint8)i;
                    feature_raw_r=1;
                }
                if(detect_feature_row >20)
                {
                    int seek_row_start=detect_feature_row-20;
                    int seek_row_end=detect_feature_row+15;
                    int seek_col_start=right_line_list[detect_feature_row]+10;;
                    if(seek_col_start>30 && seek_col_start<image_w-30)
                    {
                        int up_row=0;
                        int down_row=0;
                        int row_diff[5]={0};
                        if(feature_raw_r)
                        {
                            int k=0;
                            for(int i=seek_col_start;i<seek_col_start+15;i=i+3)
                            {
                                up_row = 0;
                                down_row = 0;
                                for(int j=seek_row_start;j<seek_row_end;j++)
                                {
                                    if(image[j+1][i]==black_point && image[j][i]==white_point && image[j-1][i]==white_point)
                                    {
                                        down_row=j;
                                    }
                                    else if(image[j+1][i]==white_point && image[j][i]==white_point && image[j-1][i]==black_point)
                                    {
                                        up_row=j;
                                    }
                                }
                                if(up_row<up_feature_row && up_row!=0)
                                {
                                    up_feature_row=up_row;
                                }
                                if(up_row!=0 && down_row!=0)
                                {
                                    row_diff[k]=down_row-up_row;
                                }
                                k++;
                            }
                            if(detect_row_diff(row_diff, 10) &&row_diff[0]!=0 && row_diff[1]!=0 && row_diff[2]!=0 && row_diff[3]!=0 && row_diff[4]!=0)
                            {
                                feature_row_run=detect_feature_row;
                                feature_label = 1;
                                break;
                            }
                                
                            
                        }
                    }
                }
            }
        }
    }

    printf("detect_feature_row=%d, feature_raw_l=%d, feature_raw_r=%d,feature_label=%d,feature_row_run=%d\n",detect_feature_row, feature_raw_l, feature_raw_r, feature_label, feature_row_run);
    // if(left_lost_times!=0 ||right_lost_times!=0)
    // {
    //     for(int i=MT9V03X_H-31;i>=feature_raw;i--)
    //     {
    //         int left_line_lost_label=0;
    //         int right_line_lost_label=0;
    //         for(int j=-8;j<=8;j++)
    //         {
    //             if(image[i+j][0]==white_point)
    //             {
    //                 left_line_lost_label++;
    //             }
    //             if(image[i+j][image_w-1]==white_point)
    //             {
    //                 right_line_lost_label++;
    //             }
    //         }
    //         if((left_line_list[i]-left_line_list[i-1]>8 || left_line_list[i]-left_line_list[i-2]>10 || left_line_list[i]-left_line_list[i-3]>20 || left_line_list[i]-left_line_list[i-4]>15)
    //             && left_line_lost_label>=1 && (T_index_list[feature_T_index]==-1||T_index_list[feature_T_index]==0))
    //         {   
    //             detect_feature_row=(uint8)i;
    //             feature_raw_l=1;
    //             break;
    //         }
    //         if((right_line_list[i-1]-right_line_list[i]>5 || right_line_list[i-2]-right_line_list[i]>8 || right_line_list[i-3]-right_line_list[i]>10 || right_line_list[i-4]-right_line_list[i]>7) 
    //             && right_line_lost_label>=1&&(T_index_list[feature_T_index]==1||T_index_list[feature_T_index]==0))
    //         {
    //             detect_feature_row=(uint8)i;
    //             feature_raw_r=1;
    //             break;
    //         }
    //     }
    // }
    // printf("detect_feature_row=%d, feature_raw_l=%d, feature_raw_r=%d\n",detect_feature_row, feature_raw_l, feature_raw_r);
    // int seek_row_start=0;
    // int seek_row_end=0;
    // if(detect_feature_row>20)
    // {
    //     seek_row_start=detect_feature_row-20;
    //     seek_row_end=detect_feature_row+15;
    // }
    // int seek_col_start=0;
    // if(feature_raw_r)
    // {
    //     seek_col_start=right_line_list[detect_feature_row]+10;
    // }
    // if(feature_raw_l)
    // {
    //     seek_col_start=left_line_list[detect_feature_row]-10;
    // }
    // if(seek_col_start>30 && seek_col_start<image_w-30)
    // {
    //     int up_row=0;
    //     int down_row=0;
    //     int row_diff[5]={0};
    //     if(feature_raw_l)
    //     {
    //         int k=0;
    //         for(int i=seek_col_start-10;i>seek_col_start-25;i=i-3)
    //         {
    //             up_row = 0;
    //             down_row = 0;
    //             for(int j=seek_row_start;j<seek_row_end;j++)
    //             {
    //                 if(image[j+1][i]==black_point && image[j][i]==white_point && image[j-1][i]==white_point)
    //                 {
    //                     down_row=j;
    //                 }
    //                 else if(image[j+1][i]==white_point && image[j][i]==white_point && image[j-1][i]==black_point)
    //                 {
    //                     up_row=j;
    //                 }
    //             }
    //             if(up_row<up_feature_row)
    //             {
    //                 up_feature_row=up_row;
    //             }
    //             if(up_row!=0 && down_row!=0)
    //             {
    //                 row_diff[k]=down_row-up_row;
    //             }
    //             k++;
    //         }
    //     }
    //     if(feature_raw_r)
    //     {
    //         int k=0;
    //         for(int i=seek_col_start;i<seek_col_start+15;i=i+3)
    //         {
    //             up_row = 0;
    //             down_row = 0;
    //             for(int j=seek_row_start;j<seek_row_end;j++)
    //             {
    //                 if(image[j+1][i]==black_point && image[j][i]==white_point && image[j-1][i]==white_point)
    //                 {
    //                     down_row=j;
    //                 }
    //                 else if(image[j+1][i]==white_point && image[j][i]==white_point && image[j-1][i]==black_point)
    //                 {
    //                     up_row=j;
    //                 }
    //             }
    //             if(up_row<up_feature_row)
    //             {
    //                 up_feature_row=up_row;
    //             }
    //             if(up_row!=0 && down_row!=0)
    //             {
    //                 row_diff[k]=down_row-up_row;
    //             }
    //             k++;
    //         }
    //     }
    //     if(detect_row_diff(row_diff, 10) &&row_diff[0]!=0 && row_diff[1]!=0 && row_diff[2]!=0 && row_diff[3]!=0 && row_diff[4]!=0)
    //     {
    //         feature_label = 1;
    //     }
    // }
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
uint8 condition_T=0;
float angle_T=0.0f;
void feature_square()
{
    //路口判断
    if(feature_raw_r==1&& condition_T==0 &&feature_label==1)
    {
        feature_T++;
        feature_T_index++;
        if(feature_T_index==index_num)
        {
            run_stop_flag=1;
            feature_T_index=0;
        }
        if(feature_T>index_num)
        {
            feature_T=1;
        }
        if(run_stop_flag == 1 && feature_T_index==1)
        {
            run_once_flag=1;
        }
        // if(feature_T_index==13)
        // {
        //     run_once_flag=1;
        // }
        zhuan_right_flag=1;
        condition_T=1;
        // angle_T=attitude.yaw;
    }
    else if(T_index_list[feature_T_index]==-1&& feature_raw_l==1&& condition_T==0 &&feature_label==1)
    {
        feature_T++;
        feature_T_index++;
        if(feature_T_index==index_num)
        {
            run_stop_flag=1;
            feature_T_index=0;
        }
        if(feature_T>index_num)
        {
            feature_T=1;
        }
        if(run_stop_flag == 1 && feature_T_index==1)
        {
            run_once_flag=1;
        }
        // if(feature_T_index==13)
        // {
        //     run_once_flag=1;
        // }
        zhuan_left_flag=1;
        condition_T=1;
        // angle_T=attitude.yaw;
    }
    else if(T_index_list[feature_T_index]==0&&feature_label==1 && condition_T==0 && feature_raw_l==1)
    {
        feature_T++;
        feature_T_index++;
        if(feature_T_index==index_num)
        {
            run_stop_flag=1;
            feature_T_index=0;
        }
        if(feature_T>index_num)
        {
            feature_T=1;
        }
        if(run_stop_flag == 1 && feature_T_index==1)
        {
            run_once_flag=1;
        }
        // if(feature_T_index==13)
        // {
        //     run_once_flag=1;
        // }
        condition_T=1;
        feature_T_left=1;
    }
    else if(T_index_list[feature_T_index]==0&&feature_label==1 && condition_T==0 && feature_raw_r==1)
    {
        feature_T++;
        feature_T_index++;
        if(feature_T_index==index_num)
        {
            run_stop_flag=1;
            feature_T_index=0;
        }
        if(feature_T>index_num)
        {
            feature_T=1;
        }
        if(run_stop_flag == 1 && feature_T_index==1)
        {
            run_once_flag=1;
        }
        // if(feature_T_index==13)
        // {
        //     run_once_flag=1;
        // }
        condition_T=1;
        feature_T_right=1;
    }
    //转向行判断
    zhuan_row=0;
    if(condition_T==1 &&zhuan_left_flag==1)
    {
        for(int i=MT9V03X_H-31;i>5;i--)
        {
            if(image[i-2][20]==black_point && image[i-1][20]==black_point && image[i][20]==white_point && image[i+1][20]==white_point)
            {
                zhuan_row=i;
                break;
            }
        }
    }
    else if(condition_T==1 && zhuan_right_flag==1)
    {
        for(int i=MT9V03X_H-31;i>5;i--)
        {
            if(image[i-2][image_w-21]==black_point && image[i-1][image_w-21]==black_point && image[i][image_w-21]==white_point && image[i+1][image_w-21]==white_point)
            {
                zhuan_row=i;
                break;
            }
        }
    }
    //减速标志位判断    
    if(T_index_list[feature_T-1]!=0&&condition_T==1)
    {
        deceleration_label=1;
    }
    //路口处理
    if(feature_T!=0)
    {
        if(T_index_list[feature_T-1]==-1)
        {
            if(zhuan_row>ERROR_IMAGE_LINE)
            {
                zhuan_condition_left=1;
            }
        }
        else if(T_index_list[feature_T-1]==1)
        {
            if(zhuan_row>ERROR_IMAGE_LINE)
            {
                zhuan_condition_right=1;
            }
        }
        else if(T_index_list[feature_T-1]==0)
        {
            Add_Line(mid_line_list[115],115,mid_line_list[5],5);
        }
    }

    if(zhuan_condition_left==1)
    {
        mid_line_list[ERROR_IMAGE_LINE]=0;
    }
    else if(zhuan_condition_right==1)
    {
        mid_line_list[ERROR_IMAGE_LINE]=image_w-1;
    }
    printf("condition_T=%d, feature_T=%d, zhuan_row=%d, zhuan_condition_left=%d, zhuan_condition_right=%d, deceleration_label=%d\n", condition_T, feature_T, zhuan_row, zhuan_condition_left, zhuan_condition_right, deceleration_label);
    //转向退出处理
    if(condition_T==1)
    {
        if(T_index_list[feature_T-1]!=0)
        {
            // if(get_angle_err(angle_T) > 52.5f)
            // {
            //     condition_T=0;
            //     deceleration_label=0;
            //     zhuan_left_flag=0;
            //     zhuan_right_flag=0;
            //     zhuan_condition_right=0;
            //     zhuan_condition_left=0;
            //     feature_label=0;    
            // }
        }
        else if(T_index_list[feature_T-1]==0)
        {
            if(feature_row_run>=MT9V03X_H-30)
            {
                condition_T=0;
                right_run_flag=0;
                feature_T_right=0;
                feature_T_left=0;
                feature_label=0;
            }
            // if(feature_T_left==1 && feature_T_right==0)
            // {
            //     // if(lost_row_down_l>90)
            //     // {
            //     //     feature_T_left=0;
            //     //     condition_T=0;
            //     // }
            //     // 原退出方案
            //     if(left_lost_times==0)
            //     {
            //         left_run_flag++;
            //     }
            //     if(left_run_flag>2)
            //     {
            //         condition_T=0;
            //         left_run_flag=0;
            //         feature_T_left=0;
            //         feature_label=0;
            //     }
            // }
            // else if(feature_T_left==0 && feature_T_right==1)
            // {
            //     // if(lost_row_down_r>90)
            //     // {
            //     //     feature_T_right=0;
            //     //     condition_T=0;
            //     // }
            //     // 原退出方案
            //     if(right_lost_times==0)
            //     {
            //         right_run_flag++;
            //     }
            //     if(right_run_flag>2)
            //     {
            //         condition_T=0;
            //         right_run_flag=0;
            //         feature_T_right=0;
            //         feature_label=0;
            //     }
            // }   
        }
    }

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
    // printf("index=%d\n",T_index);
    // printf("condition=%d",condition_T);
    // printf("direction=%d\n",direction);
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
        // printf("right_feature_flag2=%d, feature_raw_r=%d\n",image_feature.right_feature_flag2,feature_raw_r);
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




// #define index_num 6
// int T_index_list[index_num]={1,-1,-1,1,0,0};
// int T_index_list[index_num]={-1,1,1,-1};
// 科目三方案1
// #define index_num 12
// int T_index_list[index_num]={1,-1,1,1,1,-1,-1,0,-1,0,1,0};
// 科目三方案2
// #define index_num 13
// int T_index_list[index_num]={0,0,0,1,0,-1,-1,1,1,1,0,0,1};
// 校赛1 科目3
// 校赛1 科目3 
// #define index_num 16
// int T_index_list[index_num]={1,-1,1,0,1,-1,1,-1,-1,-1,0,0,1,0,0,0};
// 校赛2 科目3 
//#define index_sum_num 6
//int T_index_list[index_sum_num]={0,0,0,-1,-1,0};
// #define index_sum_num 6
// int T_index_list[index_sum_num]={0,1,1,0,0,0};
// int * T_index_list = {0};


uint8 speed_select_label = 0;
float speed_select_angle = 0.0f;
int T_corner=0;
int times_T=0;
uint8 condition_T_angle=0;
// void feature_process()
// {
//     detect_feature();
//     feature_square();
//     // printf("feature: left=%d, right=%d, height=%d\n",result_feature.left,result_feature.right,result_feature.height);
//     //纯左转
//     if(result_feature.left==1  && result_feature.right==0 && result_feature.height==0  &&condition_T==0)
//     {
//         turn(1);
//         speed_select_label=1;
//         // condition_corner=1;
//         // angle_corner=attitude.yaw;
//     }
//     //纯右转
//     else if (result_feature.right==1 &&  result_feature.left==0 && result_feature.height==0 && condition_T==0 )
//     {
//         turn(2);
//         speed_select_label=1;
//         // condition_corner=2;
//         // angle_corner=attitude.yaw;
//     }
//     // T
//     else if(result_feature.height==0&& result_feature.left==1 && result_feature.right==1 && condition_T==0  )
//     {       
//         T_index++;
//         if(T_index>index_num)
//         {
//             T_index=1;
//         }
//         condition_T=1;
//     }
//     else if(result_feature.height==1 && result_feature.left==0 && result_feature.right==1 && condition_T==0)
//     {     
//         T_corner=2;
//         T_index++;
//         if(T_index>index_num)
//         {
//             T_index=1;
//         }
//         condition_T=1;
//     }
//     else if(result_feature.height==1 && result_feature.left==1 && result_feature.right==0 && condition_T==0)
//     { 
//         T_corner=1;
//         T_index++;
//         if(T_index>index_num)
//         {
//             T_index=1;
//         }
//         condition_T=1;
//     }
//     // if(condition_corner==1)
//     // {
//     //     turn(1);
//     //     if(get_angle_err(angle_corner) > 40.0f)
//     //     {
//     //         condition_corner=0;
//     //     }
//     // }
//     // else if(condition_corner==2)
//     // {
//     //     turn(2);
//     //     if(get_angle_err(angle_corner) > 40.0f)
//     //     {
//     //         condition_corner=0;
//     //     }
//     // }
//     if( condition_T==1)
//     {
//         if(T_index_list[T_index-1]==0)
//         {
//             //直行
//             turn(0);    
//         }
//         else if(T_index_list[T_index-1]==-1)
//         {
//             speed_select_label=1;
//             if(condition_T_angle==0 && (image_feature.left_feature_flag2>ERROR_IMAGE_LINE-15||feature_raw_l>ERROR_IMAGE_LINE-15))
//             {
//                 condition_T_angle=1;
//                 angle_T=attitude.yaw;
//             }
//             //左转
//             turn(1);
//         }
//         else if(T_index_list[T_index-1]==1)
//         {
//             speed_select_label=1;
//             if(condition_T_angle==0 && (image_feature.right_feature_flag2>ERROR_IMAGE_LINE-15||feature_raw_r>ERROR_IMAGE_LINE-15))
//             {
//                 condition_T_angle=1;
//                 angle_T=attitude.yaw;
//             }
//             //右转
//             turn(2);
//         }
//         //左右转向退出
//         if(T_index_list[T_index-1]!=0 && condition_T_angle==1)
//         {
//             if(get_angle_err(angle_T) > 60.0f)
//             {
//                 condition_T=0;
//                 condition_T_angle=0;
//             }
//         }
//         //直行退出
//         else
//         {
//             times_T++;
//             if(T_corner==1&&left_lost_times==0 &&times_T>3)
//             {
//                 T_corner=0;
//                 condition_T=0;
//                 times_T=0;
//             }
//             else if(T_corner==2&&right_lost_times==0 &&times_T>3)
//             {
//                 T_corner=0;
//                 condition_T=0;
//                 times_T=0;
//             }
//         }
//     }
// }





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
    }
}

void image_process(uint8 (*source_image)[MT9V03X_W])
{
    image = source_image;
    Binarization();
    // remove_white_noise_filter();
    search_line();
    detect_feature();
    feature_square();
    // feature_process();
    image_draw_pre();
    get_error_image();
}
