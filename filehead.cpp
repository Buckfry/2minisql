//
//  filehead.cpp
//  buffer
//
//  Created by Emma Chen on 14-11-11.
//  Copyright (c) 2014年 Emma Chen. All rights reserved.
//

#include "filehead.h"
void fileHead::initFileHead(){
    age=0;
    dirty=false;
    filename="";
    writen=false;
    for(int j=0;j<OFFSET;j++)
        fileInfo[j]='\0';
    
}

