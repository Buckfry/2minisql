/*
 * RecordManager.h
 *
 *  Created on: 2014年11月5日
 *      Author: zhanghui2014
 */

#ifndef RECORDMANAGER_H_
#define RECORDMANAGER_H_

#include<iostream>
#include<string>
#include"block.h"
#include"BufferManager.h"
#include"sql.h"
#include"publichead.h"
using namespace std;

#define MAX_ATTR_NUM 18


class record_fileInfo {
public:
	string fileName;		// the name of the file, no longer than 2byte
	vector<string> attribute_name;  //no more than 9 attributes
	vector<int> attribute_type;  // int 2, float 3 or char 1
	int recordAmount;		// the number of record in the file‘s block
	int recordLength;		// the length of the record in the file
//	fileInfo *next;			// the pointer points to the next file
//	block *firstBlock;	// point to the first block within the file
	int currentblocknum;// the number of blocks which are to be used in the file
	int recordcount;   //the number of record in the current block

};


class condition_info
{
public:
	vector<string> conditionattr;
	vector<string> comparedvalue;
	vector<string>  condition;//= , <>, <, >,<= , >=
};
class RecordManager {
private:
	fileHead fh;
	record_fileInfo fi;

	//确认所操作的表
	void focus_current_db(string DB_name);
	//将class fileInfo转换为数组来存储
	char* translate_fileinfo(record_fileInfo fi);
	//更新fi
	void getfileinfo(string fileinfo,attr_info attribute_info,string filename);
	//打印属性值
	void Print(vector<string> attrvalue);
	//打印表头
	void printhead(vector<string> attr);
	//找出所查找的属性在attr中的位置
	vector<int>& findposition(vector<string> attr,vector<string> searchedattr);
	//判断是否符合指定条件
	bool Confirm(vector<string> attr,vector<string> attrvalue,condition_info condition);

public:
	//构造
	RecordManager();
      //析构
	virtual ~RecordManager();
	//释放数据库的内存缓冲区
	void Close_Database(string DB_name);
	//释放表或索引的内存缓冲区
	void Close_File(string DB_name,string filename);
	//建表时初始化表头文件
	void create_table(string DB_name,create_record data);
	//向表中插入元组,同时返回所插入元组的位置
	recordposition& insert_record(string DB_name,string filename,vector<string> attr,attr_info attribute_info);
	//选择语句（无where)
	void Select_No_Where(string DB_name,string filename,vector<string> attr,attr_info attribute_info);
//	//选择语句（无where）有索引
//	void Select_With_Useful_No_Where(string DB_name,string filename,vector<string> attr,vector<recordposition> rp,attr_info attribute_info);
	//有where，但无可用索引的select语句
	void Select_Without_Useful_Cond(string DB_name,string filename,vector<string> attr,condition_info cond,attr_info attribute_info);
	//有where，有索引可用的select语句
	void Select_With_Useful_Cond(string DB_name,string filename,vector<string> attr,vector<recordposition> rp,condition_info cond,attr_info attribute_info);
	//删除语句（无where)
	void Delete_No_Where(string DB_name,string filename,attr_info attribute_info);
	//有where,但无可用索引的delete语句,同时返回删除位置
	vector<vector<string> >& Delete_Without_Useful_Cond(string DB_name,string filename,condition_info cond,attr_info attribute_info,vector<string> indexattr);
	//有where,有可用索引的delete语句,同时返回删除位置
	vector<vector<string> >& Delete_With_Useful_Cond(string DB_name,string filename,vector<recordposition> rp,condition_info cond,attr_info attribute_info,vector<string> indexattr);
	//给index提供键的值和位置
	keyinfo& getkeyinfo(string DB_name, string filename,string keyname,attr_info attribute_info);
    //删除数据库
	void Drop_Table(string DB_name,string filename);
	//退出程序
	void Quit(string DB_Name);
//
//
//	//验证可否插入(prime? unique?)
//	bool Verify_Insertable(string DB_Name,string Table_Name,index_info nodes[32],int count,string Attr);
//

};

#endif /* RECORDMANAGER_H_ */
//class create_record{
//public:
//	string table_name;
//	int attribute_num;
////	vector<char> attribute_name;//属性名
////	vector<char> attribute_type;//属性类型
////	vector<int> attribute_length;//属性长度(每个属性的字节数)
//
//};
//
//class attr_info{
//public:
//		vector<string> attribute_name;//属性名
//		vector<int> attribute_type;//属性类型
//
//};
//
//class keyinfo{
//public:
//	string keyname;
//	vector<pair<char,recordposition>> keys;//存储键值和对应的位置
//};
//
//class recordposition{
//	int    recordnum;
//	int    blocknum;
//};
