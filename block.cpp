//
//  block.cpp
//  buffer
//
//  Created by Emma Chen on 14-11-11.
//  Copyright (c) 2014年 Emma Chen. All rights reserved.
//

#include <iostream>
#include "block.h"
using namespace std;

block::block() {
    
    this->initial();
}

block::~block() {
    
}
void block::initial(){
    blockNum=-1;
    fileName="";
    iTime=0;
    dirty=false;
    pin=0;
    writen=false;
    for(int i=0;i<BLOCK_LEN;i++)
        data[i]='@';
    data[BLOCK_LEN-1]='\0';
}

