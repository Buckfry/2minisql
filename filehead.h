//
//  filehead.h
//  buffer
//
//  Created by Emma Chen on 14-11-11.
//  Copyright (c) 2014年 Emma Chen. All rights reserved.
//

#ifndef __buffer__filehead__
#define __buffer__filehead__
#include <iostream>
#include <string>
#include <stdio.h>

#define OFFSET				16


using namespace std ;
class fileHead{
public:
    string filename;
    int age;
    char fileInfo[OFFSET];
    fileHead(){};
    ~fileHead(){};
    void initFileHead();
    //private:
    bool dirty;
    bool writen;
};

#endif /* defined(__buffer__filehead__) */
