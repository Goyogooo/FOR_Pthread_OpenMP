#include <iostream>
#include <fstream>
#include <vector>
#include<chrono>
#include <thread>

// 读取小端格式的四字节无符号整数
uint32_t read_uint32_le(std::ifstream& stream) {
    uint32_t value;
    char bytes[4];
    stream.read(bytes, 4);
    value = (static_cast<uint32_t>(static_cast<unsigned char>(bytes[3])) << 24) |
        (static_cast<uint32_t>(static_cast<unsigned char>(bytes[2])) << 16) |
        (static_cast<uint32_t>(static_cast<unsigned char>(bytes[1])) << 8) |
        static_cast<uint32_t>(static_cast<unsigned char>(bytes[0]));
    return value;
}

// 读取一个整数数组
std::vector<uint32_t> read_array(std::ifstream& stream) {
    uint32_t length = read_uint32_le(stream);
    std::vector<uint32_t> array(length);
    for (uint32_t i = 0; i < length; ++i) {
        array[i] = read_uint32_le(stream);
    }
    return array;
}
void compressSegment(const std::vector<float>& array, std::vector<float>& compress, int start, int end) {
    for (int i = start + 1; i <= end; ++i) {
        compress[i] = array[i] - array[i - 1];
    }
}


int main() {
    //for (int j = 0; j < 1000; j++) {
    std::ifstream file("D:/MyVS/BXFOR/ExpIndex", std::ios::binary);
    if (!file) {
        std::cerr << "无法打开文件" << std::endl;
        return 1;
    }
    file.seekg(32832, std::ios::beg);
    std::vector<uint32_t> array = read_array(file);
    std::vector<float> floatArray;
    // 转换整数数组到浮点数组
    for (int value : array) {
        floatArray.push_back(static_cast<float>(value));
    }
    uint32_t length = floatArray.size();

    std::vector<float> compress(length);
    compress[0] = floatArray[0];
    auto beforeTime = std::chrono::steady_clock::now();
  
    // 确定线程数量
    int numThreads = std::thread::hardware_concurrency();  // 获取系统支持的并发线程数
    std::vector<std::thread> threads;

    // 计算每个线程的处理范围
    int chunk_size = (length-1) / numThreads;
    for (int t = 0; t < numThreads; ++t) {
        int start_index = t * chunk_size;
        int end_index = (t == numThreads - 1) ? (length - 1) : (start_index + chunk_size);

        // 创建并启动线程
        threads.emplace_back(compressSegment, std::cref(floatArray), std::ref(compress), start_index, end_index);
    }

    // 等待所有线程完成
    for (auto& th : threads) {
        th.join();
    }

    auto afterTime = std::chrono::steady_clock::now();
    double time = std::chrono::duration<double>(afterTime - beforeTime).count();
    std::cout << " time=" << time << "seconds" << std::endl;
    std::ofstream f("D:/MyVS/BX_FOR_Pthread/compress.txt", std::ios::app);
    if (!f.is_open()) {
        std::cerr << "无法打开文件" << std::endl;
        return 0;
    }
    for (float value : compress) {
        f << value << ' ';
    }
    f.close();
    std::ofstream f2("D:/MyVS/BX_FOR_Pthread/array.txt", std::ios::app);
    if (!f2.is_open()) {
        std::cerr << "无法打开文件" << std::endl;
        return 0;
    }
    for (uint32_t value : array) {
        f2 << value << ' ';
    }
    f2.close();

    file.close();
    // }
    return 0;
}