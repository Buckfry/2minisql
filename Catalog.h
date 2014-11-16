/*
 * Catalog.h
 *
 *  Created on: 2014��11��11��
 *      Author: yunshen
 */

#ifndef CATALOG_H_
#define CATALOG_H_
#include"SQL.h"
#include"publichead.h"
#include<string>
using namespace std;
class Catalog{
public:
	Catalog();
	~Catalog();
	void CreateDatabase(SQL *b);
	void DropDatabase(SQL *b);
	void CreateTable(SQL *b,string database);
	IndexInfo& DropTable(SQL *b,string database);
	Index& CreateIndex(SQL *b,string database);
	string DropIndex(SQL *b,string database);
	void UseDatabase(SQL *b);
	//void QuitDatabase();
	IndexInfo& Select(SQL *b,string database);
    IndexInfo& Delete(SQL *b,string database);
    IndexInfo& Insert(SQL *b,string database);
	TableInfo& ShowTable(SQL *b,string databse);
	void execFile(SQL *b,string database);
	void helpInfo();
	int findtable(string tablename,string databse);//set public
private:
	//int findtable(SQL *b,string tablename);//set public
	bool attrfind(string database,string tablename,string attrname);//���������Ƿ���ڱ���
	string attrindex(string database,string tablename,string attrname);//���������Ƿ���index,��ʱ����indexname�����򷵻�NULL
	string findprimary(string database,string tablename);//Ѱ��������
	int findpost(string database,string tablename);//���Ҷ�Ӧ��table�������ֵ��ļ��ľ���λ��
	int findattrnum(string database,int tablepost);//�������Ե�����
	int findattrtype(string database,int tablepost,int n);//�������Ե�����
	int findattrlength(string database,int tablepost,int n);//�������Եĳ���
	static const string DATA_DIR;
	static const string CONFIG;
};





#endif /* CATALOG_H_ */
