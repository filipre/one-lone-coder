#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#include <iostream>
#include <vector>
#include <string>

using namespace std;

int main()
{
    vector<string> msg {"Hello", "C++", "World", "from", "VS Code", "and the C++ extension!"};
    int a = 3;
    msg.push_back("test");
    for (const string& word : msg)
    {
        cout << word << " ";
    }
    cout << endl;
    a = 4;
    cout << "test" << endl;
}