#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <omp.h>
#include <immintrin.h>

// ��ȡС�˸�ʽ�����ֽ��޷�������
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

// ��ȡһ����������
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
        std::cerr << "�޷����ļ�" << std::endl;
        return 1;
    }
    file.seekg(32832, std::ios::beg);
    std::vector<uint32_t> array = read_array(file);
    std::vector<float> floatArray;
    // ת���������鵽��������
    for (int value : array) {
        floatArray.push_back(static_cast<float>(value));
    }
    uint32_t length = floatArray.size();

    std::vector<float> compress(length);
    compress[0] = floatArray[0];
    auto beforeTime = std::chrono::steady_clock::now();

    // OpenMP ��������
#pragma omp parallel
    {
        int num_threads = omp_get_num_threads(); // ��ȡ��ǰ���߳���
        int thread_id = omp_get_thread_num(); // ��ȡ��ǰ�̵߳�ID
        int chunk_size = (length - 1) / num_threads; // ����ÿ���̴߳����Ԫ������
        int start_index = thread_id * chunk_size; // ����ÿ���̵߳���ʼ����
        int end_index = (thread_id == num_threads - 1) ? (length - 1) : (start_index + chunk_size); // ����ÿ���̵߳Ľ�������


        int temp = (end_index - start_index) % 8;
        __m256 comp, arr1, arr2;
        uint32_t i;
        for (i = start_index + 1; i <= end_index - temp - 7; i += 8)
        {
            arr1 = _mm256_loadu_ps(&floatArray[i - 1]);
            arr2 = _mm256_loadu_ps(&floatArray[i]);
            comp = _mm256_sub_ps(arr2, arr1);//��arr2��ȥarr1
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
        std::cerr << "�޷����ļ�" << std::endl;
        return 0;
    }
    for (float value : compress) {
        f << value << ' ';
    }
    f.close();
    std::ofstream f2("D:/MyVS/BX_FOR_OpenMP_SIMD/array.txt", std::ios::app);
    if (!f2.is_open()) {
        std::cerr << "�޷����ļ�" << std::endl;
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


