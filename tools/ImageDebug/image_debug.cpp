#include <iostream>
#include <sstream>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

#define MT9V03X_W  (188)
#define MT9V03X_H  (120)

int main(int argc, char** argv) {

    std::string imagePath = (argc > 1) ? argv[1] : "test.bmp";

    cv::Mat img = cv::imread(imagePath, cv::IMREAD_GRAYSCALE);
    if (img.empty()) {
        std::cerr << "Failed to load image" << std::endl;
        return -1;
    }

    int rows = std::min(MT9V03X_H, img.rows);
    int cols = std::min(MT9V03X_W, img.cols);

    std::stringstream source_image_array_str;

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            uchar pixel = img.at<uchar>(i, j);
            source_image_array_str << static_cast<int>(pixel);
            source_image_array_str << " ";
        }
    }

    std::string result = source_image_array_str.str();

    // 将result转换成文件txt
    std::string outputPath =  "args.txt";
    std::ofstream ofs(outputPath);
    if (!ofs) {
        std::cerr << "failed to open args.txt\n";
        return -1;
    }

    ofs << result;
    ofs.close();

    // 检查当前目录下是否存在image_process.c和image_process.h文件
    std::string code_path   = ".\\image_process.c";
    std::string header_path = ".\\image_process.h";
    
    if (!fs::exists(fs::path(code_path)) || !fs::exists(fs::path(header_path))) {
        system("copy \"..\\..\\Macro_TC264\\code\\image_process.c\" .");
        system("copy \"..\\..\\Macro_TC264\\code\\image_process.h\" .");
        std::cout << "Copied image_process.c and image_process.h to current directory." << std::endl;
        system("gcc -D_VSCODE image_process_main.c image_process.c -o image_process_debug.exe");
        std::cout << "Compiled image_process_debug.exe." << std::endl;
    }
    system("pause");
    system("image_process_debug.exe args.txt");
    // 删除文件
    system("del image_process.c");
    system("del image_process.h");
    
    return 0;
}
