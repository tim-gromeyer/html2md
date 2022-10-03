#include <iostream>
#include <string>

using namespace std;


int main(int argc, char **argv)
{
    string input;
    string outFile;

    cout << "You have entered " << argc
             << " arguments:" << "\n";

    for (int i = 0; i < argc; ++i)
        cout << argv[i] << "\n";

    return 0;
}
