#include <iostream>
#include "JSONLoader.h"

json JSONLoader::Load(std::string filename) {
    std::string path = jsonPath + filename;
    std::ifstream i;
    json j;

    i.open(path, std::ios::in | std::ios::binary);
    if (i.good()) {
        i >> j;
        i.close();
    } else {
        if (i.bad()) {
            //could not read file
            //might have read wrong char
            std::cerr << "error i/o opp" << std::endl;
        } else if (i.eof()) {
            //eof reached
            std::cerr << "error eof" << std::endl;
        } else {
            std::cerr << "error read/write opp" << std::endl;
        }
    }

    return j;
}
