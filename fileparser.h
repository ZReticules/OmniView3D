#ifndef FILEPARSER_H
#define FILEPARSER_H

#include <string>
#include <vector>

using namespace std;

namespace FileParser{

// bool parseStl(vector<shared_ptr<Shape3D>>& items, string fname, bool needClear=false);
// bool parseStlAscii(vector<shared_ptr<Shape3D>>& items, string fname, bool needClear=false);
bool ParseTxtGL(vector<float>& items, wstring fname, bool needClear=false);

bool ParseStlGL(vector<float>& triangles, wstring fname, bool needClear=false);
bool ParseStlAsciiGL(vector<float>& triangles, wstring fname, bool needClear=false);

using ParserFunction = bool(*)(vector<float>& triangles, wstring fname, bool needClear);

};

#endif // FILEPARSER_H
