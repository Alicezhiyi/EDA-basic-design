#include "Head.h"
#include <iostream>
#include <string>
int main(int argc, char** argv) {
    if(argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " filename" << std::endl;
        return -1;
    }
    std::string fileName = argv[1];
    // std::string fileName = "test.ll";
    Function f(fileName);
    f.schedule();
    f.bind();

    RTL::Module m(f);
    m.generateRTL();
    return 0;
}