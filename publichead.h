/*
 * publichead.h
 *
 *  Created on: 2014年11月14日
 *      Author: Administrator
 */

#ifndef PUBLICHEAD_H_
#define PUBLICHEAD_H_
#include "BufferManager.h"
//////////////////////////////////////////////////////////////////
//catalog
//class SQL{
//public:
//	string targetName="";
//	int argc=0;
//	vector<string> argv;//属性名
//	vector<int> type;//属性的类型，int,char,float
//	vector<int> isUnique;//是否primary=1,unique=2,not null=3
//	vector<int> length;
//	vector<string> value;
//	vector<string> result;
//	vector<string> aux;
//	SQL(){}
//	~SQL(){}
//};

////////////////////////////////////////
//公共类

class TableInfo{
public:
	vector<string> attrname;
	vector<int> type;//char=1,int=2,float=3
	vector<int> attrtype;//primary=1,unique=2,not null=3
	vector<int> length;
};
class IndexInfo{
public:
	vector<string> indexname;
	vector<string> attrname;
	TableInfo table;
};
class Index{
public:
	int type;
	int length;
	TableInfo table;
};

//属性既不是primary或者unique时，att=0;
//create index的时候attribute 类型的长度和类型



//////////////////////////////////////////////////////////////////
//调用index函数要用到的类
class recordposition{
public:
	int    recordnum;
	int    blocknum;
};
class keyinfo{
public:
	string keyname;
	vector<pair<string,recordposition> > keys;	//存储键值和对应的位置
};



//////////////////////////////////////////////////////////////////
//调用record函数要用到的类
class create_record{
public:
	string table_name;
	int attribute_num;
//	vector<char> attribute_name;//属性名
//	vector<char> attribute_type;//属性类型
	vector<int> attribute_length;//属性长度(每个属性的字节数)

};

class attr_info{
public:
		vector<string> attribute_name;//属性名
		vector<int> attribute_type;//属性类型
		vector<int> attribute_length;//属性长度(每个属性的字节数)

};
////////////////////////////////////////////////////////////////





#endif /* PUBLICHEAD_H_ */
