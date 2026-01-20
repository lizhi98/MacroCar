#include "stdio.h"
#include "image_process.h"
#include "windows.h"

uint8 source_image[MT9V03X_H][MT9V03X_W];

int main(int argc, char** argv) {
    if(argc < 2) {
        printf("Please provide image data as command line argument.\n");
        return -1;
    }
    // 从文件argv[1]读取图像数据到source_image数组
    FILE * file = fopen(argv[1], "r");
    if(!file) {
        printf("Failed to open file: %s\n", argv[1]);
        return -1;
    }
    for(int i=0;i<MT9V03X_H;i++){
        for(int j=0;j<MT9V03X_W;j++){
            // 检测到文件末尾就退出
            if (fscanf(file, "%d ", &source_image[i][j]) != 1) {
                fclose(file);
                printf("Unexpected end of file or read error.\n");
                return -1;
            }
        }
    }

    fclose(file);

    // 清空屏幕
    system("cls");
    image_process(source_image);
    // 打印处理后的图像
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    WORD color_black = 0;
    WORD color_white = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE;
    WORD color_mid_line = FOREGROUND_RED | BACKGROUND_RED;
    WORD color_side_line_left = FOREGROUND_BLUE | BACKGROUND_BLUE;
    WORD color_side_line_right = FOREGROUND_GREEN | BACKGROUND_GREEN;
    WORD color_image_boundary = FOREGROUND_RED | FOREGROUND_GREEN | BACKGROUND_RED | BACKGROUND_GREEN;

    for(int i=0;i<MT9V03X_H;i++){
        // 最上面边界
        if(i == 0){
            SetConsoleTextAttribute(hConsole, color_image_boundary);
            for(int k=0;k<=MT9V03X_W+1;k++){
                printf("--");
            }
            printf("\n");
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        }
        // 图像内容
        for(int j=0;j<MT9V03X_W;j++){
            // 最左边边界
            if(j == 0){
                SetConsoleTextAttribute(hConsole, color_image_boundary);
                printf("| ");
                SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            }
            // 根据数值判断颜色
            switch (source_image[i][j])
            {
            case 0:
                SetConsoleTextAttribute(hConsole, color_black);
                break;
            case 255:
                SetConsoleTextAttribute(hConsole, color_white);
                break;
            case 10:
                SetConsoleTextAttribute(hConsole, color_mid_line);
                break;
            case 5:
                SetConsoleTextAttribute(hConsole, color_side_line_left);
                break;
            case 15:
                SetConsoleTextAttribute(hConsole, color_side_line_right);
                break;
            default:
                SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
                break;
            }
            printf("%d ",source_image[i][j] == 255 ? 1 : 0);
            // 重置颜色
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            // 最右边边界
            if(j == MT9V03X_W - 1){
                SetConsoleTextAttribute(hConsole, color_image_boundary);
                printf(" |");
                SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            }
        }
        printf("\n");

        // 最下面边界
        if(i == MT9V03X_H - 1){
            SetConsoleTextAttribute(hConsole, color_image_boundary);
            for(int k=0;k<=MT9V03X_W+1;k++){
                printf("--");
            }
            printf("\n");
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        }
    }
    return 0;
}