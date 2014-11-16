//
//  BufferManager.h
//  buffer
//
//  Created by Emma Chen on 14-11-11.
//  Copyright (c) 2014年 Emma Chen. All rights reserved.
//

#ifndef __buffer__BufferManager__
#define __buffer__BufferManager__

#include <string>
#include <set>
#include <stack>
#include <vector>
#include "block.h"
#include "filehead.h"
using namespace std;



#define MAX_FILE_ACTIVE 	5		// limit the active files in the buffer
#define MAX_BLOCK			40		// the max number of the blocks

/*class fileHead{
public:
    string filename;
    int age;
    char fileInfo[OFFSET];
    fileHead();
    ~fileHead();
    void initFileHead();
    //private:
    bool dirty;
    bool writen;
};*/

class BufferManager{
public:
	string DBname;
	string directory;
    block Buffer[MAX_BLOCK];
    fileHead fileHlist[MAX_FILE_ACTIVE];
    
    BufferManager(string DB_Name);//initial BufferManager with a DB_name
    BufferManager();
    ~BufferManager();
    //interface
    void setDatabase(string DB_Name);
    string getDatabase();
    
    void closeFile(string fileName);
    void closeDatabase();
    
    string  getFileInfo(string fileName);
    void updateFileInfo(string fileName,char* fileInfo);
    
    void createTable(string tableName,char* fileInfo);
    void deleteTable(string tableName);
    
    
    
    block* readBlock(string fileName, int blockNum);
//    void updateBlock(string fileName,int blockNum);
    void updateBlock(string fileName,int blockNum,char*newdata);
    void updateBlock(string fileName,int blockNum);
    void insertBlock(string fileName,int blockNum,char*data);
    
private:
    void initBuffer();
    
    block* findFreeBlock();
    fileHead* findFreeFileHead();
    
    void writeBlock(block * w_block);//to disk
    void Get_Block_Disk(string File_Name,int blockNum,char data[]);//from disk
    void Get_File_Info_Disk(string File_Name,char getInfo[]);//from disk
    void Write_File_Info_Disk(fileHead *fileH);//to disk
    
    
};



#endif /* defined(__buffer__BufferManager__) */
