/*
 * sql.h
 *
 *  Created on: 2014年11月3日
 *      Author:
 */

#ifndef SQL_H_
#define SQL_H_

#include <iostream>
#include <vector>
#include <string>
#include <map>


using namespace std;

enum SqlMethod {
	NOP,
	SELECTALL,
	SELECTWHERE,
	INSERT,
	DELETEALL,
	DELETEWHERE,
	CREATEDATABASE,
	CREATETABLE,
	CREATEINDEX,
	DROPDATABASE,
	DROPTABLE,
	DROPINDEX,
	USEDATABASE,
	HELP,
	QUIT,
	EXECFILE,
};



class SQL {
public:
	SqlMethod       method;			//语句执行的方法

	int             argc;				//语句中参数的数目
    string          targetName;		//语句的目标表名/数据库名/索引名
    vector < int >   length;
	vector < string > argv;			//语句中的参数
	vector < string > value;			//语句中参数对应的数值
	vector < string > result;			//select语句中选择打印的属性
	vector < string > operators;		//where中的符号
	vector < string > logicalOperators;//where中的逻辑，目前只支持and
	vector < int > types;			//create table时的类型参数   int=2,float=3,char=1;
	vector < string > aux;			//create index时tablename 存放地
	vector < int > isUnique;			//create table哪些是unique


} ;




#endif // _SQL_H_








