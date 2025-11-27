#include<iostream>
#include"unique_ptr.hpp"
#include<cstdio>
#

struct FileCloser {
    void operator()(FILE* f) const {
        std::cout << "FileCloser" << std::endl;
        if (f) std::fclose(f);
    }
};

using FilePtr = unique_ptr<FILE, FileCloser>;

int main()
{
    FilePtr fp(std::fopen("test.txt", "r"));
    return 0;
}