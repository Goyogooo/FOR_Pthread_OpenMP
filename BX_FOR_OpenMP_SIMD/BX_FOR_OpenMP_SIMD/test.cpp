#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <omp.h>
#include <immintrin.h>

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

    // OpenMP 并行区域
#pragma omp parallel
    {
        int num_threads = omp_get_num_threads(); // 获取当前的线程数
        int thread_id = omp_get_thread_num(); // 获取当前线程的ID
        int chunk_size = (length - 1) / num_threads; // 计算每个线程处理的元素数量
        int start_index = thread_id * chunk_size; // 计算每个线程的起始索引
        int end_index = (thread_id == num_threads - 1) ? (length - 1) : (start_index + chunk_size); // 计算每个线程的结束索引


        int temp = (end_index - start_index) % 8;
        __m256 comp, arr1, arr2;
        uint32_t i;
        for (i = start_index + 1; i <= end_index - temp - 7; i += 8)
        {
            arr1 = _mm256_loadu_ps(&floatArray[i - 1]);
            arr2 = _mm256_loadu_ps(&floatArray[i]);
            comp = _mm256_sub_ps(arr2, arr1);//从arr2减去arr1
            _mm256_storeu_ps(&compress[i], comp);
        }
        for (; i <= end_index; i++)
        {
            compress[i] = floatArray[i] - floatArray[i - 1];
        }

  
    }

    auto afterTime = std::chrono::steady_clock::now();
    double time = std::chrono::duration<double>(afterTime - beforeTime).count();
    std::cout << " time=" << time << "seconds" << std::endl;
    std::ofstream f("D:/MyVS/BX_FOR_OpenMP_SIMD/compress.txt", std::ios::app);
    if (!f.is_open()) {
        std::cerr << "无法打开文件" << std::endl;
        return 0;
    }
    for (float value : compress) {
        f << value << ' ';
    }
    f.close();
    std::ofstream f2("D:/MyVS/BX_FOR_OpenMP_SIMD/array.txt", std::ios::app);
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


