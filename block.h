//
//  block.h
//  buffer
//
//  Created by Emma Chen on 14-11-11.
//  Copyright (c) 2014年 Emma Chen. All rights reserved.
//

#ifndef buffer_block_h
#define buffer_block_h
#include<iostream>
using namespace std;

#define BLOCK_LEN			4096	// the size of one block

//决定不要filetype了，效率变低了，能实现就好，所以张卉和李屹淳协调下，文件名不要设一样的
class block {
public:
    int blockNum;//block在文件中的顺序
    string fileName;
    char data[BLOCK_LEN];//string or char*
    int iTime;//for LRU
    block();
    virtual ~block();
    void initial();
    //private:
    bool pin;
    bool dirty;
    bool writen;
};



#endif
