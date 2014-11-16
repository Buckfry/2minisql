//
//  BufferManager.cpp
//  buffer
//
//  Created by Emma Chen on 14-11-11.
//  Copyright (c) 2014年 Emma Chen. All rights reserved.
//

#include <vector>
#include <fstream>
#include <sstream>
#include "BufferManager.h"
using namespace std;


//BufferManager
BufferManager::BufferManager(){
    this->initBuffer();
}


BufferManager::BufferManager(string DB_Name){
    DBname="./Mini_Data/"+DB_Name;
    this->initBuffer();
}

BufferManager::~BufferManager(){
    this->closeDatabase();
}
void BufferManager::initBuffer(){
    for(int i=0;i<MAX_BLOCK;i++)
        Buffer[i].initial();
    for(int i=0;i<MAX_FILE_ACTIVE;i++)
        fileHlist[i].initFileHead();
}

//inner operation
block* BufferManager::findFreeBlock(){//clear block
    int toplace=0;
    int Tm=Buffer[0].iTime;
    for(int i=0;i<MAX_BLOCK;i++){
        if(!Buffer[i].writen){//buffer is not full
            toplace=i;
            Buffer[toplace].initial();//clear block
            return &Buffer[toplace];
        }
        if(Buffer[i].iTime>Tm&&!Buffer[i].pin)
            toplace=i;
    }
    //buffer is full
    this->writeBlock(&Buffer[toplace]);
    Buffer[toplace].initial();
    return &Buffer[toplace];
}

fileHead* BufferManager::findFreeFileHead(){//clear block
    int toplace=0;
    int Tm=fileHlist[0].age;
    for(int i=0;i<MAX_FILE_ACTIVE;i++){
        if(!fileHlist[i].writen){
            toplace=i;
            fileHlist[toplace].initFileHead();
            return &fileHlist[toplace];
        }
        if(fileHlist[i].age>Tm)
            toplace=i;
    }
    this->Write_File_Info_Disk(&fileHlist[toplace]);
    fileHlist[toplace].initFileHead();
    return &fileHlist[toplace];
}


void BufferManager::writeBlock(block * w_block){//to disk,not clear buf
    if(!w_block->dirty) return;
    string path=DBname+"/"+directory+"/"+w_block->fileName+".txt";
    fstream fout;
    fout.open(path.c_str(),ios::in|ios::out);
    if(!fout){
        cout<<"file open fail(write block())"<<endl;
    }
    fout.seekp(BLOCK_LEN*w_block->blockNum+OFFSET,fout.beg);
    fout.write(w_block->data,BLOCK_LEN);
    fout.close();//not clear buffer
    w_block->dirty=false;
}

void BufferManager::Write_File_Info_Disk(fileHead *fileH){//to disk,not clear buf
    if(!(fileH->dirty)) return;
    string path=DBname+"/"+directory+"/"+fileH->filename+".txt";
    cout <<"in Write_File_Info_Disk: path" << path <<endl;
    fstream fout;
    fout.open(path.c_str(),ios::in|ios::out);
    if(!fout){
        cout<<"file open fail(Write_File_Info_Disk())"<<endl;
    }
    fout.seekp(0,fout.beg);
    fout.write(fileH->fileInfo,OFFSET);
    fout.close();
}

void BufferManager::Get_Block_Disk(string File_Name,int blockNum,char data[]){//from disk
    //char tmp[BLOCK_LEN];
    string path=DBname+"/"+directory+"/"+File_Name+".txt";
    fstream fin;
    fin.open(path.c_str(),ios::in);
    if(!fin){
        cout<<"file open fail(Get_Block_Disk())"<<endl;
    }
    fin.seekg(OFFSET+BLOCK_LEN*blockNum,fin.beg);
    fin.read(data,BLOCK_LEN);
    fin.close();
}
void BufferManager::Get_File_Info_Disk(string File_Name,char getInfo[]){//from disk
    //char tmp[OFFSET];
    string path=DBname+"/"+directory+"/"+File_Name+".txt";
    fstream fin;
    fin.open(path.c_str(),ios::in);
    if(!fin){
        cout<<"file open fail(Get_File_Info_Disk())"<<endl;
    }
    fin.seekg(0,fin.beg);
    fin.read(getInfo,OFFSET);
    fin.close();
}

//interface
string BufferManager::getDatabase(){
    return DBname;
}
void BufferManager::setDatabase(string DB_Name){
    DBname="./Mini_Data/"+DB_Name;
}

void BufferManager::closeFile(string fileName){
    for(int i=0;i<MAX_FILE_ACTIVE;i++){
        if(fileHlist[i].writen&&fileHlist[i].filename==fileName){//in buffer
            fileHlist[i].writen=false;
            for(int j=0;j<MAX_BLOCK;j++){//write block
                if(Buffer[j].writen&&Buffer[j].fileName==fileName){
                    writeBlock(&Buffer[j]);
                    Buffer[j].initial();//clear buf
                }
            }
            Write_File_Info_Disk(&fileHlist[i]);//write filehead
            return;
        }
    }
    return;
}

void BufferManager::closeDatabase(){//close files, clear buffer
    for(int i=0;i<MAX_FILE_ACTIVE;i++){
        if(fileHlist[i].writen){//in buffer
            this->closeFile(fileHlist[i].filename);
        }
        fileHlist[i].initFileHead();//clear all
    }
    this->initBuffer();
}

string  BufferManager::getFileInfo(string fileName){
    string ret;
    for(int i=0;i<MAX_FILE_ACTIVE;i++)//get from buffer
        if(fileHlist[i].writen&&fileHlist[i].filename==fileName){
            fileHlist[i].age=0;//LRU
            for(int j=0;j<MAX_FILE_ACTIVE;j++){
                if(i!=j&&fileHlist[j].writen)
                    fileHlist[j].age++;
            }
            ret=fileHlist[i].fileInfo;
            return ret.substr(0,16);
            
        }
    //char ret[OFFSET];
    char ret1[OFFSET];
    this->Get_File_Info_Disk(fileName,ret1);//get from disk
    fileHead * tmp=this->findFreeFileHead();//write to buffer
    tmp->dirty=true;
    tmp->writen=false;
    for(int j=0;j<OFFSET;j++)
        tmp->fileInfo[j]=ret[j];
    for(int k=0;k<MAX_FILE_ACTIVE;k++){//LRU
        if(tmp!=(&fileHlist[k])&&fileHlist[k].writen)
            fileHlist[k].age++;
    }
    ret=ret1;
    return ret;
}

void BufferManager::updateFileInfo(string fileName,char* fileInfo){
    for(int i=0;i<MAX_FILE_ACTIVE;i++){//must in buffer because record manager and index manager must getFileInfo()
        //before using this method
        if(fileHlist[i].writen&&fileHlist[i].filename==fileName){
            for(int j=0;j<OFFSET;j++)//update
                fileHlist[i].fileInfo[j]=fileInfo[j];
            fileHlist[i].dirty=true;//change state
            fileHlist[i].age=0;
            for(int k=0;k<MAX_FILE_ACTIVE;k++){
                if(i!=k&&fileHlist[k].writen)
                    fileHlist[k].age++;
            }
            return;
        }
    }
    
}


void BufferManager::createTable(string tableName,char* fileInfo){
    fileHead* fileHptr=findFreeFileHead();
    fileHptr->age=0;//set fileHead
    fileHptr->dirty=true;
    fileHptr->writen=true;
    fileHptr->filename=tableName;
    for(int i=0;i<OFFSET;i++){
        fileHptr->fileInfo[i]=fileInfo[i];
    }
    for(int k=0;k<MAX_FILE_ACTIVE;k++){//LRU
        if(fileHptr!=(&fileHlist[k])&&fileHlist[k].writen)
            fileHlist[k].age++;
    }
    //create file
    string path=DBname+"/"+directory+"/"+tableName+".txt";
    cout <<"path is " << path <<endl;
    fstream fcreate;
    fcreate.open(path.c_str(),ios::out);
    if(!fcreate){
        cout<<tableName<<"create fail!(createTable())"<<endl;
        return;}
    fcreate.close();
    this->Write_File_Info_Disk(fileHptr);
}

void BufferManager::deleteTable(string tableName){
    for(int i=0;i<MAX_FILE_ACTIVE;i++){//clear buffer
        if(fileHlist[i].writen&&fileHlist[i].filename==tableName)
            fileHlist[i].initFileHead();
    }
    for(int i=0;i<MAX_BLOCK;i++){//clear buffer
        if(Buffer[i].writen&&Buffer[i].fileName==tableName)
            Buffer[i].initial();
    }
    string path=DBname+"/"+directory+"/"+tableName+".txt";//remove file
    remove(path.c_str());
}

block* BufferManager::readBlock(string fileName, int blockNum){
    for(int i=0;i<MAX_BLOCK;i++)//in buffer
        if(Buffer[i].writen&&Buffer[i].fileName==fileName&&Buffer[i].blockNum==blockNum){
            Buffer[i].iTime=0;
            for(int j=0;j<MAX_BLOCK;j++)
                if(i!=j&&Buffer[j].writen)
                    Buffer[j].iTime++;
            return &Buffer[i];
        }
    //not in buffer
    block*tmp=findFreeBlock();
    tmp->writen=true;//set block
    tmp->blockNum=blockNum;
    tmp->fileName=fileName;
    tmp->iTime=0;
    Get_Block_Disk(fileName,blockNum,tmp->data);
    for(int j=0;j<MAX_BLOCK;j++)//for LRU
        if(tmp!=(&Buffer[j])&&Buffer[j].writen)
            Buffer[j].iTime++;
    getFileInfo(fileName);//update fileHlist
    return tmp;
}

void BufferManager::updateBlock(string fileName,int blockNum,char newdata[]){
    for(int i=0;i<MAX_BLOCK;i++){//must in buffer
        if(Buffer[i].writen&&Buffer[i].fileName==fileName&&Buffer[i].blockNum==blockNum){
            for(int j=0;j<BLOCK_LEN;j++)//update
                Buffer[i].data[j]=newdata[j];//[j]=newdata[j];
            Buffer[i].dirty=true;
            Buffer[i].iTime=0;
            for(int k=0;k<MAX_BLOCK;k++){
                if(i!=k&&Buffer[k].writen)
                    Buffer[k].iTime++;
            }
            return;
        }
        
    }
}

void BufferManager::updateBlock(string fileName,int blockNum){
    for(int i=0;i<MAX_BLOCK;i++){//must in buffer
        if(Buffer[i].writen&&Buffer[i].fileName==fileName&&Buffer[i].blockNum==blockNum){
            Buffer[i].dirty=true;
            Buffer[i].iTime=0;
            for(int k=0;k<MAX_BLOCK;k++){
                if(i!=k&&Buffer[k].writen)
                    Buffer[k].iTime++;
            }
            return;
        }
        
    }


}

void BufferManager::insertBlock(string fileName,int blockNum,char*data){
    block*tmp=findFreeBlock();
    tmp->writen=true;//set block
    tmp->dirty=true;
    tmp->blockNum=blockNum;
    tmp->fileName=fileName;
    tmp->iTime=0;
    for(int i=0;i<BLOCK_LEN;i++)
        tmp->data[i]=data[i];
    for(int j=0;j<MAX_BLOCK;j++)//for LRU
        if(tmp!=(&Buffer[j])&&Buffer[j].writen)
            Buffer[j].iTime++;
    this->getFileInfo(fileName);//update fileHlist
    this->writeBlock(tmp);//write to file
}
