#ifndef UTIL_H
#define UTIL_H

#include <vector>
#include <string>
#include <sstream>
#include <iostream>

using namespace std;

namespace Util {
    vector<bool> int2BoolVec(int input,unsigned int numBits);
    int boolVec2Int(const vector<bool> &input);
    void clearFile(string filename);
    string buildSplitFilename(string filename, int counter);
    string int2BinaryString(int input, unsigned int numBits);
    int binaryString2int(string input);



    // If you modify a template function you need to rebuild the project for that to take effect.
    template <class T>
    string vec2String(const vector<T> &input) {
        stringstream ss;
        for (unsigned int ii=0; ii<input.size(); ii++) {
            if (ii!=0) {
                ss << " ";
            }
            ss << input[ii];
        }
        return ss.str();
    }

    // If you modify a template function you need to rebuild the project for that to take effect.
    template <class T>
    void string2Vec (const string &str, vector<T> &result) {
        T element;
        stringstream ss(str);
        result.clear();
        while (ss >> element) {
            result.push_back(element);
        }
    }
}


#endif // UTIL_H
