#include "Util.h"

#include <fstream>

vector<bool> Util::int2BoolVec(int input,unsigned int numBits)
{
    // Most significant bit is returned in index 0 of the vector.
    if (numBits>8*sizeof(int))
        throw (-1);

    int inputMask=1;
    inputMask = inputMask << (numBits-1);

    vector<bool> output(numBits,0);
    int inputShifted = input;
    for (unsigned int ii=0; ii<numBits; ii++){
        if((inputShifted&inputMask)!=0){
            output[ii]=1;
        }
        inputShifted = inputShifted << 1;
    }
    return output;
}

int Util::boolVec2Int(const vector<bool> &input)
{
    // Most significant bit should be located at index 0 of the vector.
    int output=0;
    for (unsigned int ii=0; ii<input.size(); ii++) {
        output*=2;
        output+=int(input[ii]);
    }
    return output;
}

void Util::clearFile(string filename)
{
    ofstream file;
    file.open(filename.c_str(), ios::out | ios::trunc);
    file.close();
}

string Util::buildSplitFilename(string filename, int counter)
{
    stringstream ss;
    ss << filename.substr(0,filename.find_last_of('.')) << "_" << counter << filename.substr(filename.find_last_of('.'));
    return ss.str();
}

string Util::int2BinaryString(int input, unsigned int numBits)
{
    string output = "";
    vector<bool> boolVec = int2BoolVec(input, numBits);
    for (unsigned int ii=0; ii<boolVec.size(); ii++) {
        if (boolVec[ii]) {
            output += '1';
        }
        else {
            output += '0';
        }
    }
    return output;
}

int Util::binaryString2int(string input)
{
    vector<bool> boolVec;
    for (unsigned int ii=0; ii<input.size(); ii++) {
        if (input[ii] == '0') {
            boolVec.push_back(false);
        }
        else {
            boolVec.push_back(true);
        }
    }
    return boolVec2Int(boolVec);
}
