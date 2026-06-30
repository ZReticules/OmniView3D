#include "fileparser.h"

#include <fstream>
#include <filesystem>
#include <sstream>
#include <cstdint>
#include <QDebug>

using namespace std;


#define STL_HEADER_SIZE 80
#define STL_TRIANGLE_SIZE 50

struct __attribute__((packed)) Point
{
    float x;
    float y;
    float z;
};

struct __attribute__((packed)) Triangle
{
    Point normal;
    Point points[3];
    int16_t attr;

    Point& operator [](size_t index) { return points[index]; };
};

bool FileParser::ParseStlGL(vector<float>& triangles, wstring path, bool needClear)
{
    size_t dataSize = filesystem::file_size(path.c_str());
    ifstream data(path.c_str(), ios::binary);
    if(!data.is_open())
        return false;
    int trianglesCount = 0;
    union {
        Triangle triangle;
        char header[STL_HEADER_SIZE];
    };
    data.read(header, STL_HEADER_SIZE);
    data.read((char*)&trianglesCount, 4);
    if(trianglesCount * STL_TRIANGLE_SIZE + STL_HEADER_SIZE + 4 != dataSize)
        return false;
    if(needClear)
        triangles.clear();
    triangles.reserve(triangles.size() + trianglesCount * 9);
    // qDebug() << trianglesCount;
    for(int i = 0; i < trianglesCount; i++)
    {
        data.read((char*)&triangle, STL_TRIANGLE_SIZE);
        for(int j = 0; j < 3; j++)
        {
            // pointBuf.x() = triangleBuf[3 + i * 3 + 0];
            // pointBuf.z() = -triangleBuf[3 + i * 3 + 1];
            // pointBuf.y() = triangleBuf[3 + i * 3 + 2];
            // qDebug() << triangleBuf[3 + i * 3 + 0] << " " << triangleBuf[3 + i * 3 + 1] << " " << triangleBuf[3 + i * 3 + 2];
            triangles.push_back(triangle[j].x);
            triangles.push_back(triangle[j].z);
            triangles.push_back(-triangle[j].y);
        }
    }
    return true;
}

bool FileParser::ParseStlAsciiGL(vector<float>& triangles, wstring path, bool needClear)
{
    ifstream data(path.c_str(), ios::binary);
    if(!data.is_open())
        return false;
    if(needClear)
        triangles.clear();
    string line;
    getline(data, line);
    while(getline(data, line), getline(data, line), !data.eof())
    {
        for(int i = 0; i < 3; i++)
        {
            data >> ws;
            getline(data, line);
            float x, y, z;
            sscanf(line.c_str(), "vertex %f %f %f", &x, &y, &z);
            triangles.push_back(x);
            triangles.push_back(z);
            triangles.push_back(-y);
        }
        getline(data, line);
        getline(data, line);
    }
    return true;
}

bool FileParser::ParseTxtGL(vector<float> &items, wstring path, bool needClear)
{
    ifstream data(path.c_str());
    if(!data.is_open())
        return false;
    if(needClear)
        items.clear();
    stringstream ss;
    string strBuf;
    float coordVal;
    while(data >> coordVal)
        items.push_back(coordVal);
    return true;
}
