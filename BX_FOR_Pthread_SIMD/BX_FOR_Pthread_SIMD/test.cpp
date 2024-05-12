#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <thread>
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
void compressSegment(const std::vector<float>& array, std::vector<float>& compress, int start, int end) {
    int temp = (end - start) % 8;
    __m256 comp, arr1, arr2;
    uint32_t i;
    for (i = start + 1; i <= end - temp - 7; i += 8)
    {
        arr1 = _mm256_loadu_ps(&array[i - 1]);
        arr2 = _mm256_loadu_ps(&array[i]);
        comp = _mm256_sub_ps(arr2, arr1);//��arr2��ȥarr1
        _mm256_storeu_ps(&compress[i], comp);
    }
    for (; i <= end; i++)
    {
        compress[i] = array[i] - array[i - 1];
    }
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

    // ȷ���߳�����
    int numThreads = std::thread::hardware_concurrency();  // ��ȡϵͳ֧�ֵĲ����߳���
    std::vector<std::thread> threads;

    // ����ÿ���̵߳Ĵ�����Χ
    int chunk_size = (length - 1) / numThreads;
    for (int t = 0; t < numThreads; ++t) {
        int start_index = t * chunk_size;
        int end_index = (t == numThreads - 1) ? (length - 1) : (start_index + chunk_size);

        // �����������߳�
        threads.emplace_back(compressSegment, std::cref(floatArray), std::ref(compress), start_index, end_index);
    }

    // �ȴ������߳����
    for (auto& th : threads) {
        th.join();
    }


    auto afterTime = std::chrono::steady_clock::now();
    double time = std::chrono::duration<double>(afterTime - beforeTime).count();
    std::cout << " time=" << time << "seconds" << std::endl;
    std::ofstream f("D:/MyVS/BX_FOR_Pthread_SIMD/compress.txt", std::ios::app);
    if (!f.is_open()) {
        std::cerr << "�޷����ļ�" << std::endl;
        return 0;
    }
    for (float value : compress) {
        f << value << ' ';
    }
    f.close();
    std::ofstream f2("D:/MyVS/BX_FOR_Pthread_SIMD/array.txt", std::ios::app);
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