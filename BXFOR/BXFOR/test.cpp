#include <iostream>
#include <fstream>
#include <vector>
#include<chrono>
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
    for (int j = 0; j < 1000; j++) {
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
        for (uint32_t i = 1; i < length; i++)
        {
            compress[i] = array[i] - array[i - 1];
        }
        auto afterTime = std::chrono::steady_clock::now();
        double time = std::chrono::duration<double>(afterTime - beforeTime).count();
        std::cout << " time=" << time << "seconds" << std::endl;
        /*std::ofstream f("D:/MyVS/BXFOR/compress.txt", std::ios::app);
        if (!f.is_open()) {
            std::cerr << "�޷����ļ�" << std::endl;
            return 0;
        }
        for (float value : compress) {
            f << value << ' ';
        }
        f.close();*/
        /*std::ofstream f("D:/MyVS/BXFOR/array.txt", std::ios::app);
        if (!f.is_open()) {
            std::cerr << "�޷����ļ�" << std::endl;
            return 0;
        }
        for (uint32_t value : array) {
            f << value << ' ';
        }
        f.close();*/

        file.close();
    }
    return 0;
}