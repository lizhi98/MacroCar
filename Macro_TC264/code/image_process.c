#include "image_process.h"

#define GrayScale 256
#define grayscale 256
uint16 hist[GrayScale]={0};     //灰度值像素点的数量，数值存放，直方图
float P[GrayScale]={0};         //每个灰度级出现的概率
float PK[GrayScale]={0};        //概率累计和
float MK[GrayScale]={0};        //灰度值累加均值
uint8 img_threshold=189;            //输出阈值
float imgsize;                  //图像像素总量

uint8 (*image)[MT9V03X_W];

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

void Binarization()
{

    
    // uint8 threshold= Ostu(image);
    // if(threshold-img_threshold>4)
    // {
    //     img_threshold=img_threshold+4;
    // }
    // else if(img_threshold-threshold>4)
    // {
    //     img_threshold=img_threshold-4;
    // }
    // else
    // {
    //     img_threshold=threshold;
    // }
    // // img_threshold=91;
    img_threshold=Ostu(image);
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
uint8 left_line_list[MT9V03X_H]={0};
uint8 right_line_list[MT9V03X_H]={MT9V03X_W-1};
uint8 mid_line_list[MT9V03X_H];
uint8 left_lost_times = 0;
uint8 right_lost_times = 0;

#define left_end_col 10
#define right_end_col 175

void search_line()
{
    left_lost_times = 0;
    right_lost_times = 0;
    for(int i=MT9V03X_H-1 ;i>=0;i--)
    {
        for(int j=left_end_col;j<right_end_col;j++)
        {
            if(image[i][j-2]==black_point && image[i][j-1]==black_point && image[i][j]==white_point && image[i][j+1]==white_point)
            {
                left_line_list[i]=(uint8)j;
                break;
            }
            else{
                left_line_list[i]=0;
            }
        }
        for(int j=right_end_col;j>=left_end_col ;j--)
        {
            if(image[i][j-1]==white_point && image[i][j]==white_point && image[i][j+1]==black_point && image[i][j+2]==black_point)
            {
                right_line_list[i]=(uint8)j;
                break;
            }
            else{
                right_line_list[i]=MT9V03X_W-1;
            }
        }
        if(left_line_list[i]==0)
        {
            left_lost_times++;
        }
        if(right_line_list[i]==MT9V03X_W-1)
        {
            right_lost_times++;
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
        mid_line_list[i]=MT9V03X_W-1;
    }
}
void Add_Line(int x1,int y1,int x2,int y2)//补中线
{
    int i,max,a1,a2;
    int hx;
    if(x1>=MT9V03X_W-1)//起始点位置校正，排除数组越界的可能
       x1=MT9V03X_W-1;
    else if(x1<=0)
        x1=0;
    if(y1>=MT9V03X_H-1)
        y1=MT9V03X_H-1;
    else if(y1<=0)
        y1=0;
    if(x2>=MT9V03X_W-1)
        x2=MT9V03X_W-1;
    else if(x2<=0)
        x2=0;
    if(y2>=MT9V03X_H-1)
        y2=MT9V03X_H-1;
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
        if(hx>=MT9V03X_W)
            hx=MT9V03X_W;
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
        if(left_line_list[i]==0&&right_line_list[i]==MT9V03X_W-1)
        {
            longest_col++;
        }
        else{
            break;
        }
    }
}

uint8 feature_label;
uint8 feature_raw_l;
uint8 feature_raw_r;
#define feature_raw 20      //20-90
uint8 detect_feature_row=0;
void detect_feature()
{
    feature_label=0;
    feature_raw_r=0;
    feature_raw_l=0;
    detect_feature_row=0;
    if((left_lost_times!=0 || right_lost_times!=0) )
    {
        for(int i=MT9V03X_H-20;i>=feature_raw;i--)
        {
            if(left_line_list[i]-left_line_list[i-1]>15 && left_line_list[i]-left_line_list[i-2]>18 && (left_line_list[i-5]==0 || left_line_list[i-4]==0 || left_line_list[i-3]==0 || left_line_list[i-2]==0 || left_line_list[i-1]==0 || left_line_list[i]==0 ||left_line_list[i+1]==0 || left_line_list[i+2]==0 || left_line_list[i+3]==0 || left_line_list[i+4]==0 || left_line_list[i+5]==0))
            {
                feature_label = 1;
                feature_raw_l=i;
                break;
            }
            else if( right_line_list[i-1]-right_line_list[i]>15 && right_line_list[i-2]-right_line_list[i]>18 && (right_line_list[i-5]==MT9V03X_W-1 || right_line_list[i-4]==MT9V03X_W-1 || right_line_list[i-3]==MT9V03X_W-1 || right_line_list[i-2]==MT9V03X_W-1 || right_line_list[i-1]==MT9V03X_W-1 || right_line_list[i]==MT9V03X_W-1 || right_line_list[i+1]==MT9V03X_W-1 || right_line_list[i+2]==MT9V03X_W-1 || right_line_list[i+3]==MT9V03X_W-1 || right_line_list[i+4]==MT9V03X_W-1 || right_line_list[i+5]==MT9V03X_W-1))
            {
                feature_label = 1;
                feature_raw_r=i;
                break;
            }
        }
    }
    if(feature_raw_l!=0)
    {
        for(int i=feature_raw_l;i>=5;i--)
        {
            if(image[i-1][left_line_list[feature_raw_l-1]]==black_point)
            {
                detect_feature_row=i;
                break;
            }
        }
    }
    else if(feature_raw_r!=0)
    {
        for(int i=feature_raw_r;i>=5;i--)
        {
            if(image[i-1][right_line_list[feature_raw_r-1]]==black_point)
            {
                detect_feature_row=i;
                break;
            }
        }
    }

    
}

FeatureDetectResult image_feature;
feature_result result_feature={0,0,0};
#define height_start_raw 10          
#define height_end_raw 100
#define left_start_col 5
#define right_start_col 183
void feature_square()
{
    image_feature.left_feature_flag1=0;
    image_feature.left_feature_flag2=0;
    image_feature.right_feature_flag1=0;
    image_feature.right_feature_flag2=0;
    image_feature.height_feature_flag1=0;
    image_feature.height_feature_flag2=0;
    result_feature.left=0;
    result_feature.right=0;
    result_feature.height=0;
    if(feature_label==1 && detect_feature_row!=0||condition_T!=0)
    {
        int detect_row=0;
        if(detect_feature_row>15)
        {
            detect_row=height_start_raw;
        }
        else if(detect_feature_row!=0)
        {
            detect_row=detect_feature_row-3;
        }
        else{
            detect_row=height_start_raw+15;
        }
        int left_feature_col_count[3]={0};
        for(int j=left_start_col;j<left_start_col+5;j+=2)
        {
            for(int i=detect_row;i<height_end_raw;i++)
            {
                //左侧特征：上白下黑
                if(image[i-1][j]==black_point && image[i][j]==white_point && image[i+1][j]==white_point )
                {
                    image_feature.left_feature_flag1=i;
                }
                else if(image[i-1][j]==white_point && image[i][j]==white_point && image[i+1][j]==black_point)
                {
                    image_feature.left_feature_flag2=i;
                }
            }
            left_feature_col_count[(j-left_start_col)/2]=image_feature.left_feature_flag2-image_feature.left_feature_flag1;
        }
        int right_feature_col_count[3]={0};
        for(int j=right_start_col-5;j<right_start_col;j+=2)
        {
            for(int i=detect_row;i<height_end_raw;i++)
            {
                //右侧特征：上白下黑
                if(image[i-1][j]==black_point && image[i][j]==white_point && image[i+1][j]==white_point)
                {
                    image_feature.right_feature_flag1=i;
                }
                else if(image[i-1][j]==white_point && image[i][j]==white_point && image[i+1][j]==black_point)
                {
                    image_feature.right_feature_flag2=i;
                }
            }
            right_feature_col_count[(j-(right_start_col-5))/2]=image_feature.right_feature_flag2-image_feature.right_feature_flag1;
        }
        if(abs(left_feature_col_count[0]-left_feature_col_count[1])<3 && abs(left_feature_col_count[0]-left_feature_col_count[2])<3 && abs(left_feature_col_count[1]-left_feature_col_count[2])<3 &&left_feature_col_count[0]>2 && left_feature_col_count[1]>2 && left_feature_col_count[2]>2)
        {
            result_feature.left=1;
        }
        if(abs(right_feature_col_count[0]-right_feature_col_count[1])<3 && abs(right_feature_col_count[0]-right_feature_col_count[2])<3 && abs(right_feature_col_count[1]-right_feature_col_count[2])<3 && right_feature_col_count[0]>2 && right_feature_col_count[1]>2 && right_feature_col_count[2]>2)
        {
            result_feature.right=1;
        }
        for(int i=left_start_col;i<right_start_col;i++)
        {
            //上行特征：左白右黑（从左向右）
            if(image[detect_row][i-1]==black_point && image[detect_row][i]==white_point && image[detect_row][i+1]==white_point)
            {
                image_feature.height_feature_flag1=i;
                break;
            }
        }
        for(int i=right_start_col-1;i>left_start_col+1;i--)
        {
            //上行特征：左白右黑（从右向左）
            if(image[detect_row][i-1]==white_point && image[detect_row][i]==white_point && image[detect_row][i+1]==black_point)
            {
                image_feature.height_feature_flag2=i;
                break;
            }
        }
        if(image_feature.height_feature_flag2!=0&&image_feature.height_feature_flag1!=0)
        {
            result_feature.height=1;
        }
    }
}
uint8 condition_T=0;
uint8 condition_corner=0;
void turn(uint8 direction)
{
    if(direction==1)
    {
        if(image_feature.left_feature_flag1!=0 && image_feature.left_feature_flag2!=0)
        {
            left_line_offset(image_feature.left_feature_flag2);
        }
        else{
            left_line_offset(feature_raw_l);
        }
    }
    else if(direction==2)
    {
        if(image_feature.right_feature_flag1!=0 && image_feature.right_feature_flag2!=0)
        {
            right_line_offset(image_feature.right_feature_flag2);
        }
        else{
            right_line_offset(feature_raw_r);
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


uint8 T_index=0;
#define index_num 6
int T_index_list[index_num]={-1,1,1,-1,0,0};
// int T_index_list[index_num]={-1,1,1,-1};
// #define index_num 12
// int T_index_list[index_num]={1,-1,1,1,1,-1,-1,0,-1,0,1,0};
// #define index_num 11
// int T_index_list[index_num]={0,0,0,1,0,-1,0,-1,1,0,1};
// #define index_num 12
// int T_index_list[index_num]={0,-1,0,1,1,0,-1,-1,1,1,1,-1};
// #define index_num 10
// int T_index_list[index_num]={1,-1,1,1,0,-1,-1,1,-1,1}; //正极出发
// #define index_num 13
// int T_index_list[index_num]={-1,1,1,-1,-1,1,0,1,-1,-1,-1,-1,0}; //正极出发
// #define index_num 12
// int T_index_list[index_num]={-1,1,1,-1,0,0,-1,0,-1,-1,0,0}; //正极出发
// int T_index_list[index_num]={-1,1,1,-1,-1,0,-1,0,0,-1,-1,-1,0}; //正极出发

int T_corner=0;
float angle_T=0.0f;
float angle_corner=0.0f;
void feature_process()
{
    detect_feature();
    feature_square();
    //纯左转
    if(result_feature.left==1  && result_feature.right==0 && result_feature.height==0  &&condition_T==0)
    {
        turn(1);
        // condition_corner=1;
        // angle_corner=attitude.yaw;
    }
    //纯右转
    else if (result_feature.right==1 &&  result_feature.left==0 && result_feature.height==0 && condition_T==0 )
    {
        turn(2);
        // condition_corner=2;
        // angle_corner=attitude.yaw;
    }
    // T
    else if(result_feature.height==0&& result_feature.left==1 && result_feature.right==1 && condition_T==0  )
    {
        T_index++;
        if(T_index>index_num)
        {
            T_index=1;
        }
        condition_T=1;
        angle_T=attitude.yaw;
    }
    else if(result_feature.height==1 && result_feature.left==0 && result_feature.right==1 && condition_T==0)
    {
        T_corner=2;
        T_index++;
        if(T_index>index_num)
        {
            T_index=1;
        }
        condition_T=1;
        angle_T=attitude.yaw;
    }
    else if(result_feature.height==1 && result_feature.left==1 && result_feature.right==0 && condition_T==0)
    {
        T_corner=1;
        T_index++;
        if(T_index>index_num)
        {
            T_index=1;
        }
        condition_T=1;
        angle_T=attitude.yaw;
    }
    // if(condition_corner==1)
    // {
    //     turn(1);
    //     if(get_angle_err(angle_corner) > 40.0f)
    //     {
    //         condition_corner=0;
    //     }
    // }
    // else if(condition_corner==2)
    // {
    //     turn(2);
    //     if(get_angle_err(angle_corner) > 40.0f)
    //     {
    //         condition_corner=0;
    //     }
        
    // }
    if( condition_T==1)
    {
        //直行
        if(T_index_list[T_index-1]==0)
        {
            //暂时不补线，保持原有路线
            turn(0);    
        }
        else if(T_index_list[T_index-1]==-1)
        {
            //左转
            turn(1);
        }
        else if(T_index_list[T_index-1]==1)
        {
            //右转
            turn(2);
        }
        //左右转向退出
        if(T_index_list[T_index-1]!=0 )
        {
            if(get_angle_err(angle_T) > 45.0f)
            {
                condition_T=0;
            }
        }
        //直行退出
        else
        {
            // if(left_lost_times==0 && right_lost_times==0)
            // {
            //     condition_T=0;
            // }
            if(T_corner==1&&left_lost_times==0)
            {
                T_corner=0;
                condition_T=0;
            }
            else if(T_corner==2&&right_lost_times==0)
            {
                T_corner=0;
                condition_T=0;
            }
        }
    }
   
}




#define ERROR_IMAGE_LINE 48     
int error_image;
int error_image_last;
void get_error_image()
{
    error_image_last=error_image;
    error_image=94-mid_line_list[ERROR_IMAGE_LINE];
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
    remove_white_noise_filter();
    search_line();
    feature_process();
    image_draw_pre();
    get_error_image();

}






// void draw_tangle()
// {
//     remove_white_noise_filter() ;
//     for (int i = 0; i < MT9V03X_H; i++)
//     {
//         image[i][0] = 0;
//         image[i][MT9V03X_W - 1] = 0;
//     }
//     for (int j = 0; j < MT9V03X_W; j++)
//     {
//         image[0][j] = 0;
//         image[MT9V03X_H - 1][j] = 0;
//     }
// }


// uint8 left_start_point = 0;
// uint8 right_start_point = MT9V03X_W - 1;
// void seek_start_points(int row)
// {
//     draw_tangle();
//     left_start_point = 0;
//     right_start_point = MT9V03X_W - 1;

//     for (uint8 i = 1; i < MT9V03X_W - 1; i++)
//     {
//         if (image[row][i-1] == 0 && image[row][i] == 255 && image[row][i+1] == 255)
//         {
//             left_start_point = i;
//             break;
//         }

//     }
//      for (uint8 i = 1; i < MT9V03X_W - 1; i++)
//     {
//         if (image[row][MT9V03X_W-i-2] == 255 && image[row][MT9V03X_W-i-1] == 255 && image[row][MT9V03X_W-i] == 0)
//         {
//             right_start_point = MT9V03X_W-i-1;
//             break;
//         }
//     }
// }

// int seek_stop_line(void)
// {
//     int count = 0;
//     for(int i=left_start_point;i<=right_start_point;i++)
//     {
//         int count1=0 ;
//         for(int j=MT9V03X_H-2;j>=0;j--)
//         {
//                 if(image[j][i] == 255)
//                 {
//                     count1++;
//                 }
//                 else
//                 {
//                     break;
//                 }
//         }
//         if(count<count1)
//         {
//             count = count1;
//         }
//         }


//     return count;
// }

// #define MAX_POINTS 600
// uint8 left_points_raw[MAX_POINTS] = {0};
// uint8 left_points_col[MAX_POINTS] = {0};
// uint8 right_points_raw[MAX_POINTS] = {0};
// uint8 right_points_col[MAX_POINTS] = {0};
// int points_count;
// int min_raw_l = MT9V03X_H - 2;
// int min_raw_r = MT9V03X_H - 2;
// int stop_line;
// int seek_line_label=0;
// void seek_line( int height, int width)
// {

//     seek_start_points(MT9V03X_H - 2);
//     points_count = 0;
//     min_raw_l = MT9V03X_H - 2;
//     min_raw_r = MT9V03X_H - 2;
//     uint8 point_l_raw = height - 2;
//     uint8 point_l_col = left_start_point;
//     uint8 point_r_raw = height - 2;
//     uint8 point_r_col = right_start_point;

// //    int dir_l[4] = {0, 1, 2, 3}; // 0为上，1为右，2为下，3为左
// //    int dir_r[4] = {0, 1, 2, 3}; // 0为上，1为左，2为下，3为右
//     int dir_ll = 0;              // 左当前方向
//     int dir_rr = 0;              // 右当前方向
// //    int turn_l = 0;
// //    int turn_r = 0;

//    stop_line = seek_stop_line();

//     //初次扫线判断图像是否有效
//    seek_line_label=0;
//    if(left_start_point!=0 && right_start_point!=MT9V03X_W-1 && stop_line!=0)
//    {
//     seek_line_label=1;
//    }
//    for (int i = 0; i < MAX_POINTS; i++)
//    {
//        // 检查左右点是否接近
//        if (((point_l_raw - point_r_raw) * (point_l_raw - point_r_raw) +
//             (point_l_col - point_r_col) * (point_l_col - point_r_col)) < 5)
//         {
//             break;
//         }
//         //最长白列截止
//         // if(point_l_raw <= MT9V03X_H-2-stop_line || point_r_raw <= MT9V03X_H-2-stop_line)
//         // {
//         //     break;
//         // }
//             // 记录最小行数
//         if (point_l_raw < min_raw_l)
//         {
//             min_raw_l = point_l_raw;
//         }
//         if (point_r_raw < min_raw_r)
//         {
//             min_raw_r = point_r_raw;
//         }
//         // 存储当前点
//         left_points_raw[points_count] = point_l_raw;
//         left_points_col[points_count] = point_l_col;
//         right_points_raw[points_count] = point_r_raw;
//         right_points_col[points_count] = point_r_col;
//         points_count++;

//         /* 左线追踪 */
//         if (dir_ll == 0 && point_l_raw >= point_r_raw )
//         { // 向上
//             if (image[point_l_raw - 1][point_l_col] == 0)
//             {
//                 dir_ll += 1;
//             }
//             else if (image[point_l_raw - 1][point_l_col] == 255 &&
//                     image[point_l_raw - 1][point_l_col - 1] == 255)
//             {
//                 dir_ll -= 1;
//                 point_l_raw = point_l_raw - 1;
//                 point_l_col = point_l_col - 1;
//             }
//             else if (image[point_l_raw - 1][point_l_col] == 255 &&
//                     image[point_l_raw - 1][point_l_col - 1] == 0)
//             {
//                 point_l_raw = point_l_raw - 1;
//             }
//             else
//             {
//                 break;
//             }
//             if (dir_ll < 0)
//                 dir_ll = 3;
//         }
//         else if (dir_ll == 1 && point_l_raw >= point_r_raw )
//         { // 向右
//             if (image[point_l_raw][point_l_col + 1] == 0)
//             {
//                 dir_ll += 1;
//             }
//             else if (image[point_l_raw][point_l_col + 1] == 255 &&
//                     image[point_l_raw - 1][point_l_col + 1] == 255)
//             {
//                 dir_ll -= 1;
//                 point_l_raw = point_l_raw - 1;
//                 point_l_col = point_l_col + 1;
//             }
//             else if (image[point_l_raw][point_l_col + 1] == 255 &&
//                     image[point_l_raw - 1][point_l_col + 1] == 0)
//             {
//                 point_l_col = point_l_col + 1;
//             }
//             else
//             {
//                 break;
//             }
//         }
//         else if (dir_ll == 2 && point_l_raw >= point_r_raw )
//         { // 向下
//             if (image[point_l_raw + 1][point_l_col] == 0)
//             {
//                 dir_ll += 1;
//             }
//             else if (image[point_l_raw + 1][point_l_col] == 255 &&
//                     image[point_l_raw + 1][point_l_col + 1] == 255)
//             {
//                 dir_ll -= 1;
//                 point_l_raw = point_l_raw + 1;
//                 point_l_col = point_l_col + 1;
//             }
//             else if (image[point_l_raw + 1][point_l_col] == 255 &&
//                     image[point_l_raw + 1][point_l_col + 1] == 0)
//             {
//                 point_l_raw = point_l_raw + 1;
//             }
//             else
//             {
//                 break;
//             }
//         }
//         else if (dir_ll == 3 && point_l_raw >= point_r_raw )
//         { // 向左
//             if (image[point_l_raw][point_l_col - 1] == 0)
//             {
//                 dir_ll += 1;
//             }
//             else if (image[point_l_raw][point_l_col - 1] == 255 &&
//                     image[point_l_raw + 1][point_l_col - 1] == 255)
//             {
//                 dir_ll -= 1;
//                 point_l_raw = point_l_raw + 1;
//                 point_l_col = point_l_col - 1;
//             }
//             else if (image[point_l_raw][point_l_col - 1] == 255 &&
//                     image[point_l_raw + 1][point_l_col - 1] == 0)
//             {
//                 point_l_col = point_l_col - 1;
//             }
//             else
//             {
//                 break;
//             }
//             if (dir_ll > 3)
//                 dir_ll = 0;
//         }

//         /* 右线追踪 */
//         if (dir_rr == 0 && point_l_raw < point_r_raw )
//         { // 向上
//             if (image[point_r_raw - 1][point_r_col] == 0)
//             {
//                 dir_rr += 1;
//             }
//             else if (image[point_r_raw - 1][point_r_col] == 255 &&
//                     image[point_r_raw - 1][point_r_col + 1] == 255)
//             {
//                 dir_rr -= 1;
//                 point_r_raw = point_r_raw - 1;
//                 point_r_col = point_r_col + 1;
//             }
//             else if (image[point_r_raw - 1][point_r_col] == 255 &&
//                     image[point_r_raw - 1][point_r_col + 1] == 0)
//             {
//                 point_r_raw = point_r_raw - 1;
//             }
//             else
//             {
//                 break;
//             }
//             if (dir_rr < 0)
//                 dir_rr = 3;
//         }
//         else if (dir_rr == 1 && point_l_raw < point_r_raw )
//         { // 向右（实际上是向左搜索）
//             if (image[point_r_raw][point_r_col - 1] == 0)
//             {
//                 dir_rr += 1;
//             }
//             else if (image[point_r_raw][point_r_col - 1] == 255 &&
//                     image[point_r_raw - 1][point_r_col - 1] == 255)
//             {
//                 dir_rr -= 1;
//                 point_r_raw = point_r_raw - 1;
//                 point_r_col = point_r_col - 1;
//             }
//             else if (image[point_r_raw][point_r_col - 1] == 255 &&
//                     image[point_r_raw - 1][point_r_col - 1] == 0)
//             {
//                 point_r_col = point_r_col - 1;
//             }
//             else
//             {
//                 break;
//             }
//             if (dir_rr > 3)
//                 dir_rr = 0;
//         }
//         else if (dir_rr == 2 && point_l_raw < point_r_raw )
//         { // 向下
//             if (image[point_r_raw + 1][point_r_col] == 0)
//             {
//                 dir_rr += 1;
//             }
//             else if (image[point_r_raw + 1][point_r_col] == 255 &&
//                     image[point_r_raw + 1][point_r_col - 1] == 255)
//             {
//                 dir_rr -= 1;
//                 point_r_raw = point_r_raw + 1;
//                 point_r_col = point_r_col - 1;
//             }
//             else if (image[point_r_raw + 1][point_r_col] == 255 &&
//                     image[point_r_raw + 1][point_r_col - 1] == 0)
//             {
//                 point_r_raw = point_r_raw + 1;
//             }
//             else
//             {
//                 break;
//             }
//             if (dir_rr > 3)
//                 dir_rr = 0;
//         }
//         else if (dir_rr == 3 && point_l_raw < point_r_raw )
//         { // 向左（实际上是向右搜索）
//             if (image[point_r_raw][point_r_col + 1] == 0)
//             {
//                 dir_rr += 1;
//             }
//             else if (image[point_r_raw][point_r_col + 1] == 255 &&
//                     image[point_r_raw + 1][point_r_col + 1] == 255)
//             {
//                 dir_rr -= 1;
//                 point_r_raw = point_r_raw + 1;
//                 point_r_col = point_r_col + 1;
//             }
//             else if (image[point_r_raw][point_r_col + 1] == 255 &&
//                     image[point_r_raw + 1][point_r_col + 1] == 0)
//             {
//                 point_r_col = point_r_col + 1;
//             }
//             else
//             {
//                 break;
//             }
//             if (dir_rr > 3)
//                 dir_rr = 0;
//         }
//     }
// }

// uint8 left_line_list[MT9V03X_H];
// uint8 right_line_list[MT9V03X_H];
// uint8 lost_times_left = 0;
// uint8 lost_times_right = 0;
// void seek_list(int num)
// {
//     lost_times_left = 0;
//     lost_times_right = 0;
//     int max_raw;
//     if(min_raw_l>min_raw_r)
//     {
//         max_raw=min_raw_l;
//     }
//     else
//     {
//         max_raw=min_raw_r;
//     }
//     int j = MT9V03X_H - 2;
//     int k = MT9V03X_H - 2;
//     for (int i = 0; i < MT9V03X_H; i++)
//     {
//         left_line_list[i] = 0;
//         right_line_list[i] = MT9V03X_W - 1;
//     }
//     if(seek_line_label!=0)
//     {
//         for (int i = 0; i < num; i++)
//         {

//             if (left_points_raw[i] == j && j >= max_raw)
//             {
//                 left_line_list[j] = left_points_col[i];
//                 if(left_line_list[j]==1)
//                 {
//                     lost_times_left++;
//                 }
//                 j--;
//             }
//             if (right_points_raw[i] == k && k >= max_raw)
//             {
//                 right_line_list[k] = right_points_col[i];
//                 if(right_line_list[k]==MT9V03X_W-2)
//                 {
//                     lost_times_right++;
//                 }
//                 k--;
//             }
//         }
//     }
    
// }

// void Add_Line(int x1,int y1,int x2,int y2)//右补线,补的是边界
// {
//     int i,max,a1,a2;
//     int hx;
//     if(x1>=MT9V03X_W-1)//起始点位置校正，排除数组越界的可能
//        x1=MT9V03X_W-1;
//     else if(x1<=0)
//         x1=0;
//     if(y1>=MT9V03X_H-1)
//         y1=MT9V03X_H-1;
//     else if(y1<=0)
//         y1=0;
//     if(x2>=MT9V03X_W-1)
//         x2=MT9V03X_W-1;
//     else if(x2<=0)
//         x2=0;
//     if(y2>=MT9V03X_H-1)
//         y2=MT9V03X_H-1;
//     else if(y2<=0)
//          y2=0;
//     a1=y1;
//     a2=y2;
//     if(a1>a2)//坐标互换
//     {
//         max=a1;
//         a1=a2;
//         a2=max;
//     }

    
//     for(i=a1;i<=a2;i++)//根据斜率补线即可
//     {       //-46      //45 //-46 +89
//         hx=(i-y1)*(x2-x1)/(y2-y1)+ x1;
//         if(hx>=MT9V03X_W)
//             hx=MT9V03X_W;
//         else if(hx<=0)
//             hx=0;
//         mid_line_list[i]=(uint8)hx;
//     }
// }




// //--------------------------------------------------------------------------------------------
// //求斜率和方差
// //--------------------------------------------------------------------------------------------

// float parameterA;
// float parameterB;
// void regression(int type, int startline, int endline)//最小二乘法拟合曲线，分别拟合中线，左线，右线,type表示拟合哪几条线
// {
//     int i = 0;
//     int sumlines = endline - startline;
//     int sumX = 0;
//     int sumY = 0;
//     float averageX = 0;
//     float averageY = 0;
//     float sumUp = 0;
//     float sumDown = 0;
//     if (type == 0)      //拟合中线
//     {
//         for (i = startline; i < endline; i++)
//         {
//             sumX += i;
//             sumY += mid_line_list[i];
//         }
//         if (sumlines != 0)
//         {
//             averageX = (float)(sumX / sumlines);     //x的平均值
//             averageY = (float)(sumY / sumlines);     //y的平均值
//         }
//         else
//         {
//             averageX = 0;     //x的平均值
//             averageY = 0;     //y的平均值
//         }
//         for (i = startline; i < endline; i++)
//         {
//             sumUp += (mid_line_list[i] - averageY) * (i - averageX);
//             sumDown += (i - averageX) * (i - averageX);
//         }
//         if (sumDown == 0) parameterB = 0;
//         else parameterB = sumUp / sumDown;
//         parameterA = averageY - parameterB * averageX;
//     }
//     else if (type == 1)//拟合左线
//     {
//         for (i = startline; i < endline; i++)
//         {
//             sumX += i;
//             sumY += left_line_list[i];
//         }
//         if (sumlines == 0) sumlines = 1;
//         averageX = (float)(sumX / sumlines);     //x的平均值
//         averageY = (float)(sumY / sumlines);     //y的平均值
//         for (i = startline; i < endline; i++)
//         {
//             sumUp += (left_line_list[i] - averageY) * (i - averageX);
//             sumDown += (i - averageX) * (i - averageX);
//         }
//         if (sumDown == 0) parameterB = 0;
//         else parameterB = sumUp / sumDown;
//         parameterA = averageY - parameterB * averageX;
//     }
//     else if (type == 2)//拟合右线
//     {
//         for (i = startline; i < endline; i++)
//         {
//             sumX += i;
//             sumY += right_line_list[i];
//         }
//         if (sumlines == 0) sumlines = 1;
//         averageX = (float)(sumX / sumlines);     //x的平均值
//         averageY = (float)(sumY / sumlines);     //y的平均值
//         for (i = startline; i < endline; i++)
//         {
//             sumUp += (right_line_list[i] - averageY) * (i - averageX);
//             sumDown += (i - averageX) * (i - averageX);
//         }
//         if (sumDown == 0) parameterB = 0;
//         else parameterB = sumUp / sumDown;
//         parameterA = averageY - parameterB * averageX;

//     }

// }
// int monileft[MT9V03X_H];
// int moniright[MT9V03X_H];
// int monimiddle[MT9V03X_H];
// void monileftfuzhi(float A, float B, int start_point, int end_point)
// {
//     int m;
//     for (m = start_point; m <= end_point; m++)
//     {
//         if ((B * m + A) >= 255) monileft[m] = 255;
//         if ((B * m + A) <= 0) monileft[m] = 0;
//         else if (0 < (B * m + A) && (B * m + A) < 255) monileft[m] = (int)(B * m + A);
//     }
// }
// void monirightfuzhi(float A, float B, int start_point, int end_point)
// {
//     int m;
//     for (m = start_point; m <= end_point; m++)
//     {
//         if ((B * m + A) >= 255) moniright[m] = 255;
//         if ((B * m + A) <= 0) moniright[m] = 0;
//         else if (0 < (B * m + A) && (B * m + A) < 255) moniright[m] = (int)(B * m + A);
//     }
// }

// void monizhongfuzhi(float A, float B, int start_point, int end_point)
// {
//     int m;
//     for (m = start_point; m <= end_point; m++)
//     {
//          if ((B * m + A) >= 255) monimiddle[m] = 255;
//          if ((B * m + A) <= 0) monimiddle[m] = 0;
//          else if (0 < (B * m + A) && (B * m + A) < 255) monimiddle[m] = (int)(B * m + A);
//     }
// }

// double pianfangleft;
// double pianfangright;
// double pianfangmid;
// void pianfangcal(int begin, int end, int type)
// {
//     int i = 0;
//     if (type == 1)//左线拟合差平方计算
//     {
//         pianfangleft = 0;
//         regression(1, begin, end);
//         monileftfuzhi(parameterA, parameterB, (int)begin, (int)end);
//         for (i = begin; i <= end; i++)
//         {
//             pianfangleft = pianfangleft + (left_line_list[i] - monileft[i]) * (left_line_list[i] - monileft[i]);
//         }
//         pianfangleft = pianfangleft / (end - begin + 1);
//     }
//     if (type == 2)//右线拟合差平方计算
//     {
//         pianfangright = 0;
//         regression(2, begin, end);
//         monirightfuzhi(parameterA, parameterB, (int)begin, (int)end);
//         for (i = begin; i <= end; i++)
//         {
//             pianfangright = pianfangright + (right_line_list[i] - moniright[i]) * (right_line_list[i] - moniright[i]);
//         }
//         pianfangright = pianfangright / (end - begin + 1);
//     }
//     if (type == 0)//中线拟合差平方计算
//     {
//         pianfangmid = 0;
//         regression(0, begin, end);
//         monizhongfuzhi(parameterA, parameterB, (int)begin, (int)end);
//         int fangjun = 0;
//         int junfang = 0;
//         for (i = begin; i <= end; i++)
//         {
//             fangjun = fangjun + (mid_line_list[i]) * (mid_line_list[i]);
//         }
//         fangjun = fangjun / (end - begin + 1);
//         for (i = begin; i <= end; i++)
//         {
//             junfang = junfang + (mid_line_list[i]);
//         }
//         junfang = junfang / (end - begin + 1);
//         junfang = junfang * junfang;
//         pianfangmid = (fangjun - junfang) * 1.0;
//     }

// }


// //--------------------------------------------------------------------------------------------
// //新版特征提取
// //--------------------------------------------------------------------------------------------
// FeatureDetectResult feature_new;
// int feature_corner_l;
// int feature_corner_r;
// void feature_extract()
// {
//     int min_raw;
//     if(min_raw_l>min_raw_r)
//     {
//         min_raw=min_raw_l;
//     }
//     else
//     {
//         min_raw=min_raw_r;
//     }
//     feature_corner_l=0;
//     feature_corner_r=0;
//     feature_new.left_feature_flag=0;
//     feature_new.right_feature_flag=0;
//     for(int i=MT9V03X_H-2; i>min_raw+2;i--)
//     {
//         //左侧特征提取
//         if(left_line_list[i]-left_line_list[i-1]>10 && left_line_list[i]-left_line_list[i-2]>10 )
//         {
//             feature_corner_l=i;
//             feature_new.left_feature_flag=1;
//             break;
//         }
//         //右侧特征提取
//         if(right_line_list[i-1]-right_line_list[i]>10 && right_line_list[i-2]-right_line_list[i]>10 )
//         {
//             feature_corner_r=i;
//             feature_new.right_feature_flag=1;
//             break;
//         }
//     }
//     if(feature_new.left_feature_flag==1 && feature_new.right_feature_flag==0)
//     {
//         if(feature_corner_l>min_raw+5 && feature_corner_l<MT9V03X_H-5)
//         {
//             for(int i=feature_corner_l-3;i<feature_corner_l+3;i++)
//             {
//                 if(right_line_list[i-1]-right_line_list[i]>10 && right_line_list[i-2]-right_line_list[i]>10)
//                 {
//                     feature_corner_r=i;
//                     feature_new.right_feature_flag=1;
//                     break;
//                 }
//             }
//         }

//     }
//     if(feature_new.right_feature_flag==1 && feature_new.left_feature_flag==0)
//     {
//         if(feature_corner_r>min_raw+5 && feature_corner_r<MT9V03X_H-5)
//         {
//             for(int i=feature_corner_r-3;i<feature_corner_r+3;i++)
//             {
//                 if(left_line_list[i]-left_line_list[i-1]>10 && left_line_list[i]-left_line_list[i-2]>10)
//                 {
//                     feature_corner_l=i;
//                     feature_new.left_feature_flag=1;
//                     break;
//                 }
//             }
//         }
        
//     }
// }




// //--------------------------------------------------------------------------------------------
// //老版特征提取
// //--------------------------------------------------------------------------------------------

// FeatureDetectResult image_feature;
// //行特征值
// #define FEATURE_DETECT_HEIGHT 5
// #define detect_left_start_col 30
// #define detect_right_start_col 150
// #define detect_existing_row_min 2
// #define detect_existing_row_max 70

// //列特征值
// #define FEATURE_DETECT_WIDTH_LEFT 15
// #define FEATURE_DETECT_WIDTH_RIGHT 175
// #define detect_height_start_row 10
// #define detect_height_end_row 80
// #define detect_existing_col_min 1
// #define detect_existing_col_max 20

// int left_add_row=0;
// int right_add_row=0;
// FeatureDetectResult detect_feature_line()
// {

//     FeatureDetectResult result={0,0,0};
    

//     //检查左列特征
//     int left_flag_count[5]={0};
//     int left_flag[5]={0};
//     for(int j=FEATURE_DETECT_WIDTH_LEFT+4;j>=FEATURE_DETECT_WIDTH_LEFT;j--)
//     {
//         for(int i=detect_height_start_row;i<detect_height_end_row;i++)
//         {
//             if(image[i-1][j]==0&&image[i][j]==255&&image[i+1][j]==255)                
//             {
//                 left_add_row=i;
//                 left_flag[j-FEATURE_DETECT_WIDTH_LEFT]=1;

//             }
//             if(left_flag[j-FEATURE_DETECT_WIDTH_LEFT]==1)
//             {
//                 if(image[i][j]==255)
//                 {
//                     left_flag_count[j-FEATURE_DETECT_WIDTH_LEFT]++;
//                 }
//             }
//             if(image[i-1][j]==255&&image[i][j]==255&&image[i+1][j]==0)
//             {
//                 left_flag[j-FEATURE_DETECT_WIDTH_LEFT]=0;
//             }
//         }
//     }
//     if( left_flag_count[0]>detect_existing_col_min &&
//         left_flag_count[1]>detect_existing_col_min &&
//         left_flag_count[2]>detect_existing_col_min &&
//         left_flag_count[3]>detect_existing_col_min &&
//         left_flag_count[4]>detect_existing_col_min &&
//         left_flag_count[0]<detect_existing_col_max &&
//         left_flag_count[1]<detect_existing_col_max &&
//         left_flag_count[2]<detect_existing_col_max &&
//         left_flag_count[3]<detect_existing_col_max &&
//         left_flag_count[4]<detect_existing_col_max)
//         {
//             result.left_feature_flag=1;
//         }

//     // left_count=0;
//     // int left_flag=0;
//     // int left_flag_count[5]={0};
//     // for(int i=0;i<MT9V03X_H-1;i++)
//     // {
//     //     if(left_count==0)
//     //     {
//     //         feature_row_l++;
//     //     }
//     //     if(image[i][FEATURE_DETECT_WIDTH_LEFT]==255)
//     //     {
//     //         left_count++;
//     //     }
//     // }
//     // if(left_count>1)
//     // {
//     //     left_flag=1;
//     // }
//     // if(left_flag==1)
//     // {
//     //     for(int j=FEATURE_DETECT_WIDTH_LEFT;j<FEATURE_DETECT_WIDTH_LEFT+5;j++)
//     //     {
//     //         for(int i=detect_height_start_row;i<detect_height_end_row;i++)
//     //         {
//     //             if(image[i][j]==255)
//     //             {
//     //                 left_flag_count[j-FEATURE_DETECT_WIDTH_LEFT]++;
//     //             }
//     //         }
//     //     }
//     //     if( left_flag_count[0]>detect_existing_col_min &&
//     //         left_flag_count[1]>detect_existing_col_min &&
//     //         left_flag_count[2]>detect_existing_col_min &&
//     //         left_flag_count[3]>detect_existing_col_min &&
//     //         left_flag_count[4]>detect_existing_col_min &&
//     //         left_flag_count[0]<detect_existing_col_max &&
//     //         left_flag_count[1]<detect_existing_col_max &&
//     //         left_flag_count[2]<detect_existing_col_max &&
//     //         left_flag_count[3]<detect_existing_col_max &&
//     //         left_flag_count[4]<detect_existing_col_max)
//     //     {
//     //         result.left_feature_flag=1;
//     //     }
//     // }

//     //检查右列特征
//     int right_flag_count[5]={0};
//     int right_flag[5]={0};
//     for(int j=FEATURE_DETECT_WIDTH_RIGHT-5;j<FEATURE_DETECT_WIDTH_RIGHT;j++)
//     {
//         for(int i=detect_height_start_row;i<detect_height_end_row;i++)
//         {
//             if(image[i-1][j]==0&&image[i][j]==255&&image[i+1][j]==255)                
//             {
//                 right_add_row=i;
//                 right_flag[j-(FEATURE_DETECT_WIDTH_RIGHT-5)]=1;
//             }
//             if(right_flag[j-(FEATURE_DETECT_WIDTH_RIGHT-5)]==1)
//             {
//                 if(image[i][j]==255)
//                 {
//                     right_flag_count[j-(FEATURE_DETECT_WIDTH_RIGHT-5)]++;
//                 }
//             }
//             if(image[i-1][j]==255&&image[i][j]==255&&image[i+1][j]==0)
//             {
//                 right_flag[j-(FEATURE_DETECT_WIDTH_RIGHT-5)]=0;
//             }
//         }
//     } 
//     if( right_flag_count[0]>detect_existing_col_min &&
//         right_flag_count[1]>detect_existing_col_min &&
//         right_flag_count[2]>detect_existing_col_min &&
//         right_flag_count[3]>detect_existing_col_min &&
//         right_flag_count[4]>detect_existing_col_min &&
//         right_flag_count[0]<detect_existing_col_max &&
//         right_flag_count[1]<detect_existing_col_max &&
//         right_flag_count[2]<detect_existing_col_max &&
//         right_flag_count[3]<detect_existing_col_max &&
//         right_flag_count[4]<detect_existing_col_max)
//         {
//             result.right_feature_flag=1;
//         }

//     // right_count=0;
//     // int right_flag=0;
//     // int right_flag_count[5]={0};
//     // for(int i=0;i<MT9V03X_H-1;i++)
//     // {
//     //     if(right_count==0)
//     //     {
//     //         feature_row_r++;
//     //     }
//     //     if(image[i][FEATURE_DETECT_WIDTH_RIGHT]==255)
//     //     {
//     //         right_count++;
//     //     }
//     // }
//     // if(right_count>1)
//     // {
//     //     right_flag=1;
//     // }
//     // //待优化
//     // if(right_flag==1)
//     // { 
//     //     for(int j=FEATURE_DETECT_WIDTH_RIGHT-5;j<FEATURE_DETECT_WIDTH_RIGHT;j++)
//     //     {
//     //         for(int i=detect_height_start_row;i<detect_height_end_row;i++)
//     //         {
//     //             if(image[i][j]==255)
//     //             {
//     //                 right_flag_count[j-(FEATURE_DETECT_WIDTH_RIGHT-5)]++;
//     //             }
//     //         }
//     //     }
//     //     if( right_flag_count[0]>detect_existing_col_min &&
//     //         right_flag_count[1]>detect_existing_col_min &&
//     //         right_flag_count[2]>detect_existing_col_min &&
//     //         right_flag_count[3]>detect_existing_col_min &&
//     //         right_flag_count[4]>detect_existing_col_min &&
//     //         right_flag_count[0]<detect_existing_col_max &&
//     //         right_flag_count[1]<detect_existing_col_max &&
//     //         right_flag_count[2]<detect_existing_col_max &&
//     //         right_flag_count[3]<detect_existing_col_max &&
//     //         right_flag_count[4]<detect_existing_col_max)
//     //     {
//     //         result.right_feature_flag=1;
//     //     }

//     // }
//     //检查行特征，确保不会出现断线（如电容），并且区分转角和直线
//     //int height_flag=0;
//     //int height_count_flag=0;
//     int stop_line = 0;
    
//     if(min_raw_l>min_raw_r)
//     {
//         stop_line=min_raw_l;
//     }
//     else
//     {
//         stop_line=min_raw_r;
//     }

//          //检查是否存在白列变短情况,且出现左或右横线
//         if(stop_line >= FEATURE_DETECT_HEIGHT &&result.left_feature_flag==1 ||stop_line >= FEATURE_DETECT_HEIGHT && result.right_feature_flag==1)
//         {
//             result.height_feature_flag = 1;
//         }
//         else
//         {
//             result.height_feature_flag = 0;
//         }
    
   
    
   
//     // if(height_flag==1)
//     // {
//     //     // 检查行特征
//     //     int row_flag_count[5] = {0};
//     //     int row_flag[5] = {0};
//     //     int row_ad_start_col = 0;
//     //     int row_ad_end_col = 0;
//     //     for (int i = FEATURE_DETECT_HEIGHT; i < FEATURE_DETECT_HEIGHT+5; i++)
//     //     {
//     //         for (int j = detect_left_start_col; j < detect_right_start_col; j++)
//     //         {
//     //             if (image[i][j - 1] == 0 && image[i][j] == 255 && image[i][j + 1] == 255)
//     //             {
//     //                 row_ad_start_col = j;
//     //                 row_flag[i - FEATURE_DETECT_HEIGHT] = 1;
//     //             }
//     //             if (row_flag[i - FEATURE_DETECT_HEIGHT] == 1)
//     //             {
//     //                 if (image[i][j] == 255)
//     //                 {
//     //                     row_flag_count[i - FEATURE_DETECT_HEIGHT]++;
//     //                 }
//     //             }
//     //             if (image[i][j - 1] == 255 && image[i][j] == 255 && image[i][j + 1] == 0)
//     //             {
//     //                 row_ad_end_col = j;
//     //                 row_flag[i - FEATURE_DETECT_HEIGHT] = 0;
//     //             }
//     //         }
//     //     }
//     //     if (row_flag_count[0] > detect_existing_row_min &&
//     //     row_flag_count[1] > detect_existing_row_min &&
//     //     row_flag_count[2] > detect_existing_row_min &&
//     //     row_flag_count[3] > detect_existing_row_min &&
//     //     row_flag_count[4] > detect_existing_row_min &&
//     //     row_flag_count[0] < detect_existing_row_max &&
//     //     row_flag_count[1] < detect_existing_row_max &&
//     //     row_flag_count[2] < detect_existing_row_max &&
//     //     row_flag_count[3] < detect_existing_row_max &&
//     //     row_flag_count[4] < detect_existing_row_max)
//     //     {
//     //         height_count_flag = 1;  // 
//     //     }
//     // }
    
   
//     //有元素存在，导致白列变短，并且存在路线（电容之类元素）
//     // if( height_flag == 1 && height_count_flag == 1 )
//     // {
//     //     result.height_feature_flag = 2; // 存在白列变短且有路线（电容之类元素）
//     // }
//     // //存在白列变短，但不存在路线（转角）
//     // else if( height_flag == 1 && height_count_flag == 0 )
//     // {
//     //     result.height_feature_flag = 1; // 白列变短，但不存在路线（转角）
//     // }
//     // //不存在白列变短情况，同时有路线(直线)
//     // else
//     // {
//     //     result.height_feature_flag = 3; // 不存在路线变短情况，但是有路线(直线)
//     // }
//     // //检查是否存在路线
//     // int count_height[5]={0};
//     // for(int i=FEATURE_DETECT_HEIGHT-5;i<FEATURE_DETECT_HEIGHT;i++)
//     // {
//     //    for(int j=detect_left_start_col;j<detect_right_start_col;j++)
//     //     {
//     //         if(image[i][j]==255)
//     //         {
//     //             count_height[i-10]++;
//     //         }
//     //     }
//     // }
//     // //存在白列变短且有路线（电容之类元素）
//     // if( count_height[0]>detect_existing_row_min &&
//     //     count_height[1]>detect_existing_row_min &&
//     //     count_height[2]>detect_existing_row_min &&
//     //     count_height[3]>detect_existing_row_min &&
//     //     count_height[4]>detect_existing_row_min &&
//     //     count_height[0]<detect_existing_row_max &&
//     //     count_height[1]<detect_existing_row_max &&
//     //     count_height[2]<detect_existing_row_max &&
//     //     count_height[3]<detect_existing_row_max &&
//     //     count_height[4]<detect_existing_row_max &&
//     //     height_flag==1 )
//     //     {
//     //         result.height_feature_flag=2;
//     //     }
//     // //白列变短，但不存在路线（转角）
//     // else if( height_flag==1)
//     //     {
//     //         result.height_feature_flag=1;
//     //     }
//     // //不存在路线变短情况，但是有路线(直线)
//     // else if(count_height[0]>detect_existing_row_min &&
//     //     count_height[1]>detect_existing_row_min &&
//     //     count_height[2]>detect_existing_row_min &&
//     //     count_height[3]>detect_existing_row_min &&
//     //     count_height[4]>detect_existing_row_min &&
//     //     count_height[0]<detect_existing_row_max &&
//     //     count_height[1]<detect_existing_row_max &&
//     //     count_height[2]<detect_existing_row_max &&
//     //     count_height[3]<detect_existing_row_max &&
//     //     count_height[4]<detect_existing_row_max &&
//     //     height_flag==0
//     // )
//     // {
//     //     result.height_feature_flag=3;
//     // }

    
//     return result;
// }

// void Add_left_line()
// {
//     int min_raw;
//     if(min_raw_l>min_raw_r)
//     {
//         min_raw=min_raw_l;
//     }
//     else{
//         min_raw=min_raw_r;
//     }
//     if(min_raw>50)
//     {
//         Add_Line(mid_line_list[MT9V03X_H-2],MT9V03X_H-2,0,left_add_row+1);
//     }
//     else
//     {
//         Add_Line(mid_line_list[MT9V03X_H-2-30],MT9V03X_H-32,0,left_add_row+1);
//     }

// }
// void Add_right_line()
// {
//     int min_raw;
//     if(min_raw_l>min_raw_r)
//     {
//         min_raw=min_raw_l;
//     }
//     else{
//         min_raw=min_raw_r;
//     }
//     if(min_raw>50)
//     {
//         Add_Line(mid_line_list[MT9V03X_H-2],MT9V03X_H-2,187,right_add_row+1);
//     }
//     else
//     {   
//         Add_Line(mid_line_list[MT9V03X_H-32],MT9V03X_H-32,187,right_add_row+1);
//     }

// }

// FeatureDetectResult image_plan_feature={0,0,0};

// uint8 T_index=0;
// int T[11]={0,0,0,1,0,-1,0,-1,1,0,1};

// int condition=0;
// float angle_T=0;
// void process_T(float current_angle)
// {

//     //检测T型路口，并将condition置1，记录转向角度
//     if((image_plan_feature.left_feature_flag==1&&condition==0)||(image_plan_feature.right_feature_flag==1&&condition==0)||(image_plan_feature.height_feature_flag==1&&condition==0))
//     {

//         T_index++;
//         if(T_index>11)
//         {
//             T_index=1;
//         }
//         condition=1;
//         angle_T=current_angle;
//     }

//     //在condition为1的情况下，且检测到T型路口情况下执行转向处理函数
//     if(condition==1)
//     {
//         //直行
//         if(T[T_index-1]==0)
//         {
//             //暂时不补线，保持原有路线
//         }
//         else if(T[T_index-1]==-1&&image_feature.left_feature_flag==1)
//         {
//             //左转

//             Add_left_line();
//         }
//         else if(T[T_index-1]==1&&image_feature.right_feature_flag==1)
//         {
//             //右转
//             Add_right_line();
//         }


//         //左右转向退出
//         if(T[T_index-1]!=0 )
//         {
//             if(get_angle_err(angle_T) > 45.0)
//             {
//                 condition=0;
//             }
            
//         }
//         //直行退出
//         else{
//             if(image_feature.left_feature_flag==0 &&image_feature.right_feature_flag==0)
//             {
//                 condition=0;
//             }
//         }
        
//     }

   
// }

// void feature_process(float angle)
// {
//     image_feature = detect_feature_line();
//     //默认左转角
//     if(image_feature.left_feature_flag==1 && image_feature.right_feature_flag==0 &&image_feature.height_feature_flag==1 )
//     {
//         Add_left_line(); 
//     }
//     //默认右转角
//     else if(image_feature.right_feature_flag==1 && image_feature.left_feature_flag==0 && image_feature.height_feature_flag==1 )
//     {
//         Add_right_line();
//     }
//      //正T形路口
//     if(image_feature.left_feature_flag==1 && image_feature.right_feature_flag==1 && image_feature.height_feature_flag==1 && feature_corner_l!=0 && feature_corner_r!=0 && abs(feature_corner_l-feature_corner_r)<10)
//     {
//         image_plan_feature.height_feature_flag=1;
//     }
//     //右倒T型路口
//     else if (image_feature.left_feature_flag==1 && image_feature.right_feature_flag==0 && image_feature.height_feature_flag ==0 && feature_new.left_feature_flag==1 && feature_new.right_feature_flag==0)   
//     {
//         image_plan_feature.right_feature_flag=1;
//     }
//     //左倒T型路口
//     else if (image_feature.left_feature_flag==0 && image_feature.right_feature_flag==1 && image_feature.height_feature_flag ==0 && feature_new.right_feature_flag==1 && feature_new.left_feature_flag==0)
//     {
//         image_plan_feature.left_feature_flag=1;
//     }
//     else
//     {
//         image_plan_feature.left_feature_flag=0;
//         image_plan_feature.right_feature_flag=0;
//         image_plan_feature.height_feature_flag=0;
//     }
//     process_T(angle);
// }

// uint8 mid_line_list[MT9V03X_H];
// int error_image;
// int error_image_last;

// void image_draw(int min_raw)
// {
//     for(int i = min_raw; i < MT9V03X_H - 1;i++)
//     {
//         image[i][mid_line_list[i]]  = 10;
//         image[i][left_line_list[i]]  = 5;
//         image[i][right_line_list[i]]  = 15;
//     }
// }
// void image_draw_pre(int num)
// {
//     for(int i = 0; i < num;i++)
//     {
//         image[left_points_raw[i]][left_points_col[i]]  = 5;
//         image[right_points_raw[i]][right_points_col[i]]  = 15;
//     }
// }

// void get_mid_line()
// {
//     for(int i = 0; i < MT9V03X_H - 1;i++)
//     {
//         mid_line_list[i] = (right_line_list[i] + left_line_list[i]) / 2;
//     }
    
// }

// int get_error_image(void)
// {
//     int min_raw;
//     if(min_raw_l>min_raw_r)
//     {
//         min_raw=min_raw_l;
//     }
//     else
//     {
//         min_raw=min_raw_r;
//     }
//     image_draw(min_raw);
//     int error_image = 0;
//     if(min_raw>50)
//     {
//         error_image=(MT9V03X_W/2-mid_line_list[(MT9V03X_H-1-min_raw)]);
//     }
//     else
//     {
//         error_image=(MT9V03X_W/2-mid_line_list[50]);
//     }
//     error_image=error_image*0.7+error_image_last*0.3;
    
//     error_image_last=error_image;

// //    if(min_raw<55)
// //     {
// //        error_image_last=error_image;
// //        error_image=(MT9V03X_W/2-mid_line_list[60]);
// //
// //    }
//     // if(min_raw<75)
//     // {
//     //     error_image_last=error_image;
//     //     error_image=(MT9V03X_W/2-mid_line_list[(MT9V03X_H-46+min_raw)/2]);
//     // }
//     // else
//     // {
//     //     error_image=(MT9V03X_W/2-mid_line_list[(MT9V03X_H-1+min_raw)/2]);
//     //     
//     // }
//     //error_image=(MT9V03X_W/2-mid_line_list[min_raw+5]);
//     return error_image;
// }


// //图像处理主函数
// void image_process(uint8 (*source_image)[MT9V03X_W],float angle)
// {
//     image = source_image;
//     Binarization(MT9V03X_W, MT9V03X_H);
//     seek_line(MT9V03X_H,MT9V03X_W);
//     seek_list(points_count);
//     get_mid_line();
//     feature_extract();
//     feature_process(angle);
//     // printf(" L:%d\n",feature_new.left_feature_flag);
//     // printf(" R:%d\n",feature_new.right_feature_flag);

//     // printf(" L:%d\n",image_feature.left_feature_flag);
//     // printf(" R:%d\n",image_feature.right_feature_flag);
//     // printf(" H:%d\n",image_feature.height_feature_flag);

//     // printf(" T:%d\n",image_plan_feature.height_feature_flag);
//     // printf(" L:%d\n",image_plan_feature.left_feature_flag);
//     // printf(" R:%d\n",image_plan_feature.right_feature_flag);
//     //image_draw_pre(points_count);
//     error_image=get_error_image();
// }


