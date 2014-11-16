/*
 * Catalog.cpp
 *
 *  Created on: 2014��11��11��
 *      Author: yunshen
 */
#include<windows.h>
#include<stdlib.h>
#include <iosfwd>
#include"Catalog.h"
#include"SQL.h"
#include<iostream>
#include<fstream>
#include<vector>
#include<sstream>
#include"Error.h"
using namespace std;

const string Catalog::DATA_DIR=".\\Mini_Data\\";
const string Catalog::CONFIG=".\\Mini_Data\\MiniSQL.txt";


extern BufferManager datamanager;
Catalog::Catalog()
{//����MiniSQL�����ļ��кʹ�����ݿ���Ϣ����Ϣ�ļ�
	if ((GetFileAttributesA(DATA_DIR.c_str()) == INVALID_FILE_ATTRIBUTES) || !(GetFileAttributesA(DATA_DIR.c_str()) & FILE_ATTRIBUTE_DIRECTORY)){
		CreateDirectory(DATA_DIR.c_str(),NULL);
	}
	if(GetFileAttributesA(CONFIG.c_str())==INVALID_FILE_ATTRIBUTES){
	    ofstream file;
	    file.open(CONFIG.c_str(),ofstream::binary|ofstream::out);
	    file.seekp(0);
	    file.width(16);
	    file<<left<<0;
	    file.close();
	}
}
Catalog::~Catalog(){}
void Catalog::CreateDatabase(SQL *b)
{//�������ݿ�ʱ����������������Ϣ�ļ��в������ݿ��Ƿ���ڣ������ڣ��״�����1����������ɾ���Ժ����µ�ֻ���Ϊ���ڣ���������Ϣ�ļ������¼���Ϣ
	string name=DATA_DIR+b->targetName+"\\";
	if ((GetFileAttributesA(name.c_str()) != INVALID_FILE_ATTRIBUTES) && (GetFileAttributesA(DATA_DIR.c_str()) & FILE_ATTRIBUTE_DIRECTORY)){
			//throw Error(1,"Database already exists!");
		throw Error(1,"Database already exists!");
	}
	fstream file;
	file.open(CONFIG.c_str(),fstream::binary|fstream::out|fstream::in);
	string s;
	int flag,post=-1;
	int count;
	file.seekg(0);
	file.width(16);
	file>>count;//database numbers
	for(int i=0;i<count;i++){
		file.seekg(16+i*36);//���ݿⳤ��Ϊ36
		file.width(4);
		file>>flag;
		file.seekp(16+i*36+4);
		file.width(32);
		file>>s;
		if(flag==0&&s==b->targetName){
			post=16+i*36;
			file.seekp(post);
			file.width(4);
			file<<left<<1;
			break;
		}
	}
	if(post<0){
		file.seekp(16+count*36);
		file.width(4);
		file<<left<<1;
		file.width(32);
		file<<left<<b->targetName;
	}
	file.seekp(0);
	file.width(16);
	file<<left<<count+1;
	file.close();
	//�������ݿ��ļ��У�������������ֵ��ļ�
	CreateDirectory(name.c_str(),NULL);
	string s1=name+"directorydata.txt";
	ofstream tmp;
	tmp.open(s1.c_str(),ofstream::binary|ofstream::out);
    tmp.width(16);
    tmp<<left<<0;//����0��table
    tmp.close();

//    Current_Dataname=b->get_DBname();
}

void Catalog::DropDatabase(SQL *b)
{//���ж��Ƿ��и����ݿ��ļ�������ɾ��Ӧ�����ݿ���ļ��У����������ݿ���Ϣ�ļ�������ɾ��
	string name=DATA_DIR+b->targetName+"\\";
	if ((GetFileAttributesA(name.c_str()) == INVALID_FILE_ATTRIBUTES) || !(GetFileAttributesA(DATA_DIR.c_str()) & FILE_ATTRIBUTE_DIRECTORY)){
				//throw Error(1,"Database already exists!");
			throw Error(4,"Database not exists!");
	}
    fstream file;
    file.open(CONFIG.c_str(),fstream::binary|fstream::out|fstream::in);
    int count;
    file.width(16);
    file>>count;
    string s;
    for(int i=0;i<count;i++){//lazy delete
    	file.seekg(16+i*36+4);
    	file.width(32);
    	file>>s;
    	if(s==b->targetName){
    		file.seekp(0);
    		file<<left<<count-1;
    		file.seekp(16+i*36);
    		file.width(4);
    		file<<left<<0;
    		break;
    	}
    }
    file.close();
    //ɾ�����ݿ��Ӧ�ļ���
    SHFILEOPSTRUCT fileop;
    fileop.hwnd   = NULL;    // no status display
    fileop.wFunc  = FO_DELETE;  // delete operation
    fileop.pFrom  = name.c_str();  // source file name as double null terminated string
    fileop.pTo    = NULL;    // no destination needed
    fileop.fFlags = FOF_NOCONFIRMATION | FOF_SILENT | FOF_NOERRORUI;  // do not prompt the user
    fileop.lpszProgressTitle = NULL;
    int status = SHFileOperation(&fileop);
}

void Catalog::CreateTable(SQL *b,string database)
{//���ұ����ڵ����ݿ��Ƿ���ڣ��ұ��Ƿ���ڣ������ڱ��������������ֵ��ļ��н�����Ϣ,ÿ��table��һ���ļ��У�������index�ļ�
//table�ļ����ｨ������table�Լ�����������
	string name=DATA_DIR+database+"\\";
	if ((GetFileAttributesA(name.c_str()) == INVALID_FILE_ATTRIBUTES) || !(GetFileAttributesA(DATA_DIR.c_str()) & FILE_ATTRIBUTE_DIRECTORY)){
		throw Error(2,"Database not exists!");
	}
	//дһ��Ѱ��table�ĺ���
    if(findtable(database,b->targetName)){
    	throw Error(2,"Table already exists!");
    }
    name+="directorydata.txt";
    fstream file;
    file.open(name.c_str(),fstream::binary|fstream::out|fstream::in);
    int tablecount;
    file.width(16);
    file>>tablecount;
    file.seekp(0);
    file.width(16);
    file<<left<<tablecount+1;
    file.seekp(16+tablecount*2600);
    file.width(32);
    file<<left<<b->targetName;
    file.width(4);
    file<<left<<b->argc;
    file.width(4);
    file<<left<<1;
    vector<string>::iterator an=b->argv.begin();
    vector<int>::iterator at=b->types.begin();
    vector<int>::iterator att=b->isUnique.begin();
    vector<int>::iterator len=b->length.begin();
    string pkname;
    for(int i=0;i<b->argc;i++,an++,at++,att++,len++){
    	file.seekp(16+tablecount*2600+40+i*80);
    	file.width(32);
    	file<<left<<*an;//������
    	file.width(8);
    	file<<left<<*at;//��������
    	file.width(4);
    	file<<left<<*len;//���Գ���
    	file.width(2);
    	file<<left<<*att;//��������,primary=1,unique=2
    	if(*att==1){
    		file.width(2);
    		file<<left<<1;
    		file.width(32);
    		file<<left<<*an;//index name is attribute name
    		pkname=*an;
    	}
    	else{
    		file.width(2);
    		file<<left<<0;
    		file.width(32);
    		file<<left<<"null";
    	}
    }
    file.close();
    //��Ҫ�����ݿ��ļ����н���һ��index�ļ���¼����index��Ϣ,primarykey��index����ɾ��������������������ͬʱ�����Բ������µ������ڵ㣬����������
    name=DATA_DIR+database+"\\"+b->targetName+"\\";
    CreateDirectory(name.c_str(),NULL);
    name+=pkname+".txt";//which attribute has index,then create a file
    ofstream tmp;
    tmp.open(name.c_str(),ofstream::trunc|ofstream::binary|ofstream::out);
    tmp.width(32);
    tmp<<left<<database;
    tmp.width(32);
    tmp<<left<<b->targetName;//table
    tmp.width(32);
    tmp<<left<<pkname;//attribute
    tmp.width(32);
    tmp<<left<<pkname+"_index";//indexname
    tmp.close();
}

IndexInfo& Catalog::DropTable(SQL *b,string database)
{//����������ݿ��Ƿ���ڣ��������table�Ƿ���ڣ����������״������ڣ�ɾ��table�ļ��м��������ļ����������ֵ��ļ�������ɾ����table
	IndexInfo* indexinfo=new IndexInfo();
	string name=DATA_DIR+database+"\\";
		if ((GetFileAttributesA(name.c_str()) == INVALID_FILE_ATTRIBUTES) || !(GetFileAttributesA(DATA_DIR.c_str()) & FILE_ATTRIBUTE_DIRECTORY)){
			throw Error(5,"Database not exists!");
		}
	if(!findtable(database,b->targetName)){
	    	throw Error(5,"Table not exists!");
	}
	name+="directorydata.txt";
	int tablecount;
	string s;
	fstream file;
	file.open(name.c_str(),fstream::binary|fstream::out|fstream::in);
	file.width(16);
	file>>tablecount;
	for(int i=0;i<tablecount;i++){
		file.seekg(16+i*2600);
		file.width(32);
		file>>s;
		if(s==b->targetName){//lazy delete
			file.seekp(16+i*2600+36);
			file.width(4);
			file<<left<<0;
			break;
		}
	}
	file.seekp(0);
	file.width(16);
	file<<left<<tablecount-1;
	int post=findpost(database,b->targetName);
	int attrcount=findattrnum(database,post);
	int flag;
	string iname;
	for(int i=0;i<attrcount;i++){
		file.seekg(post+40+80*i+46);
		file.width(2);
		file>>flag;
		if(flag==1){
			file.seekg(post+40+i*80+48);
			file.width(32);
			file>>iname;
			indexinfo->indexname.push_back(iname);
			file.seekg(post+40+i*80);
			file.width(32);
			file>>iname;
			indexinfo->attrname.push_back(iname);
		}
	}
	file.close();
    //delete table_dir
	name=DATA_DIR+database+"\\"+b->targetName+"\\";
    SHFILEOPSTRUCT fileop;
    fileop.hwnd   = NULL;    // no status display
    fileop.wFunc  = FO_DELETE;  // delete operation
    fileop.pFrom  = name.c_str();  // source file name as double null terminated string
    fileop.pTo    = NULL;    // no destination needed
    fileop.fFlags = FOF_NOCONFIRMATION | FOF_SILENT | FOF_NOERRORUI;  // do not prompt the user
    fileop.lpszProgressTitle = NULL;
    int status = SHFileOperation(&fileop);
    return *indexinfo;
}
Index& Catalog::CreateIndex(SQL *b,string database)//attribute ���͵ĳ��Ⱥ�����
{//��Ӧ���ݿ��Ƿ���ڣ���Ӧtable�Ƿ���ڣ���Ӧ�����Ƿ���ڣ��������ڣ��׳����󣬷����������������Ӧ�����Ѿ��������������״�
	vector<string>::iterator tablename=b->aux.begin();
	vector<string>::iterator attribute=b->argv.begin();
	string name=DATA_DIR+database+"\\";
		if ((GetFileAttributesA(name.c_str()) == INVALID_FILE_ATTRIBUTES) || !(GetFileAttributesA(DATA_DIR.c_str()) & FILE_ATTRIBUTE_DIRECTORY)){
			throw Error(3,"Database not exists!");
		}
	if(!findtable(database,*tablename)){
		    	throw Error(3,"Table not exists!");
	}
	name+="directorydata.txt";
	int tablecount;
	fstream file;
	string s;
	file.open(name.c_str(),fstream::binary|fstream::out|fstream::in);
	file.width(16);
	file>>tablecount;
	for(int i=0;i<tablecount;i++){
		file.seekg(16+i*2600);
		file.width(32);
		file>>s;
		if(s==*tablename){//find table post
			int post=16+i*2600+40;
			string c;
			int o;
			int flag=0;
			for(int j=0;j<b->argc;j++){
			  file.seekg(post+80*j);
			  file.width(32);
			  file>>c;
			  if(c==*attribute){//find attribute name
				  flag=1;
				  file.seekg(post+80*j+46);
				  file.width(2);
				  file>>o;
				  if(o==1){
					  file.close();
					  throw Error(3,"An index has been already created for the column");
				  }
				  else{
					  file.seekg(post+80*j+44);
					  file.width(2);
					  file>>o;
					  if(o==0){
						  file.close();
						  throw Error(3,"Indices must create on unique attributes.");
					  }
					  else{
						 file.seekp(post+80*j+46);
						 file.width(2);
						 file<<left<<1;
						 file.width(32);
						 file<<left<<b->targetName;
						 file.close();
						 break;
					  }
			      }

			  }//�ҵ������� if
			}//inner for
			if(!flag){
				 file.close();
				 throw Error(3,"Column not found!");
			}
		    else break;//extern for
		}
	}//outer for
	//�ڶ�Ӧ����ļ��������������Ϣ,ɾ��index��ʱ��Ҳ������ɾ�������ͬ��������������ͬ�����ֵ�indexʱ���Լ���һ
    name=DATA_DIR+database+"\\"+*tablename+"\\"+*attribute+".txt";
    ofstream tmp;
    tmp.open(name.c_str(),ofstream::trunc|ofstream::binary|ofstream::out);
    tmp.width(32);
    tmp<<left<<database;
    tmp.width(32);
    tmp<<left<<*tablename;
    tmp.width(32);
    tmp<<left<<*attribute;
    tmp.width(32);
    tmp<<left<<b->targetName;
    tmp.close();
    Index* index=new Index();
    int post=findpost(database,*tablename);
    name=DATA_DIR+database+"\\"+"directorydata.txt";
//    fstream file;
    file.open(name.c_str(),fstream::binary|fstream::out|fstream::in);
    file.seekg(post+32);
    int count;
    file.width(4);
    file>>count;
//    string s;
    for(int i=0;i<count;i++){
    	file.seekg(post+40+80*i);
    	file.width(32);
    	file>>s;
    	if(s==*attribute){
    		file.seekg(post+40+80*i+32);
    		file.width(8);
    		file>>index->type;
    		file.width(4);
    		file>>index->length;
    	}
    }
    TableInfo table=ShowTable(b,database);
    index->table=table;
    return *index;
}
string Catalog::DropIndex(SQL *b,string database)//primary index can't delete
{//������Ҫ֪�����ݿ��Ƿ���ڡ����Ƿ���ڣ������Ƿ���ڣ��������Ƿ�������(�����֪��)���������Ƿ�ƥ��//һ�����ݿ��в�ͬ�ı��Ͻ�����ͬ�����������п��ܵ�
	string name=DATA_DIR+database+"\\";
	if ((GetFileAttributesA(name.c_str()) == INVALID_FILE_ATTRIBUTES) || !(GetFileAttributesA(DATA_DIR.c_str()) & FILE_ATTRIBUTE_DIRECTORY)){
								//throw Error(1,"Database already exists!");
		throw Error(6,"Database not exists!");
	}
	//�������ֵ��ļ���ɾ������
	name+="directorydata.txt";
	fstream tmp;
	tmp.open(name.c_str(),fstream::binary|fstream::out|fstream::in);
	int tablecount;
	tmp.width(16);
	tmp>>tablecount;
	int flag;
	bool is=false;
	int type;
	int i;
	string s1,s2,s3;//�ҵ����������ҵ�table��attrname
	for(i=0;i<tablecount;i++){
		int attrcount;
		tmp.seekg(16+2600*i+32);
		tmp.width(4);
		tmp>>attrcount;
		for(int j=0;j<attrcount;j++){
		   tmp.seekg(16+2600*i+40+80*j+46);
		   tmp.width(2);
		   tmp>>flag;
		   tmp.width(32);
		   tmp>>s1;
		   if(flag&&s1==b->targetName){//find
			  tmp.seekg(16+2600*i+40+80*j+44);
			  tmp.width(2);
			  tmp>>type;
			  if(type==1){
				  throw Error(6,"This is the primary index!");
			  }
			  else{
			    tmp.seekp(16+2600*i+40+80*j+46);
			    tmp.width(2);
			    tmp<<left<<0;
			    tmp.seekg(16+2600*i);
			    tmp.width(32);
			    tmp>>s2;//find table
			    tmp.seekg(16+2600*i+40+80*j);
			    tmp.width(32);
			    tmp>>s3;//find attribute
			    tmp.close();
			    is=true;
			    break;
			  }
		   }
		}
		if(is) break;
	}
	if(!is){
		tmp.close();
		throw Error(6,"Index not exists!");
	}
	name=DATA_DIR+database+"\\"+s2+"\\"+s3+".txt";
	DeleteFile(name.c_str());
	return s2;
}
void Catalog::UseDatabase(SQL *b)
{
	string name=DATA_DIR+b->targetName+"\\";
	if ((GetFileAttributesA(name.c_str()) == INVALID_FILE_ATTRIBUTES) || !(GetFileAttributesA(DATA_DIR.c_str()) & FILE_ATTRIBUTE_DIRECTORY)){
									//throw Error(1,"Database already exists!");
			throw Error(10,"Database not exists!");
	}
}
//void Catalog::QuitDatabase(){}


int Catalog::findtable(string database,string tablename)
{

	string name=DATA_DIR+database+"\\"+"directorydata.txt";
	int tablecount,flag;
	string s;
	fstream file;
	file.open(name.c_str(),fstream::binary|fstream::out|fstream::in);
	file.width(16);
	file>>tablecount;
	for(int i=0;i<tablecount;i++){
		file.seekg(16+2600*i);
		file.width(32);
		file>>s;
		file.seekg(16+2600*i+36);
		file.width(4);
		file>>flag;
		if(s==tablename&&flag)
			return 1;
	}
	return 0;
}
TableInfo& Catalog::ShowTable(SQL *b,string database)
{
	TableInfo* tableinfo=new TableInfo();
	string name=DATA_DIR+b->targetName+"\\"+"directorydata.txt";
	fstream file;
	file.open(name.c_str(),fstream::binary|fstream::out|fstream::in);
	int tablecount,flag,post;
	bool is=false;
	string s;
	file.width(16);
	file>>tablecount;
	for(int i=0;i<tablecount;i++){
		file.seekg(16+i*2600);
		file.width(32);
		file>>s;
		file.seekg(16+i*2600+36);
		file.width(4);
		file>>flag;
		if(s==b->targetName&&flag){
			is=true;
			int attrcount;
			file.seekg(16+i*2600+32);
			file.width(4);
			file>>attrcount;
			post=16+i*2600+40;
			string s1;
			int le;
			for(int j=0;j<attrcount;j++){
				file.seekg(post+j*80);
				file.width(32);
				file>>s1;
				tableinfo->attrname.push_back(s1);
				file.width(8);
				file>>le;
				tableinfo->type.push_back(le);
				file.width(4);
				file>>le;
				tableinfo->length.push_back(le);
				file.width(2);
				file>>le;
				tableinfo->attrtype.push_back(le);
			}
			file.close();
			break;
		}
	}
	if(!is){
		file.close();
		throw Error(12,"Table not exists!");
	}
	return *tableinfo;
}
bool Catalog::attrfind(string database,string tablename,string attrname)
{//attribute �Ƿ���ڣ�
	string name=DATA_DIR+database+"\\"+"directorydata.txt";
	int tablecount;
	string s;
	fstream file;
	file.open(name.c_str(),fstream::binary|fstream::out|fstream::in);
	file.width(16);
	file>>tablecount;
	bool is=false;
	for(int i=0;i<tablecount;i++){
		file.seekg(16+i*2600);
		file.width(32);
		file>>s;
		if(s==tablename){
			int attrcount;
			string ch;
			file.seekg(16+i*2600+32);
			file.width(4);
			file>>attrcount;
			for(int j=0;j<attrcount;j++){
				file.seekg(16+i*2600+40+j*80);
				file.width(32);
				file>>ch;
				if(ch==attrname){
					is=true;
					break;
				}
			}
		}
        if(is) break;
	}
	file.close();
    return is;
}

string Catalog::attrindex(string database,string tablename,string attrname)
{//attribute index_name
	int post=findpost(database,tablename);
	string name=DATA_DIR+database+"\\"+"directorydata.txt";
	fstream file;
	file.open(name.c_str(),fstream::binary|fstream::in|fstream::out);
	int attrcount;
	file.seekg(post+32);
	file.width(4);
	file>>attrcount;
	string s;
	int flag;
	for(int i=0;i<attrcount;i++){
		file.seekg(post+40+i*80);
		file.width(32);
		file>>s;
		if(s==attrname){
			file.seekg(post+40+i*80+46);
			file.width(2);
			file>>flag;
			if(flag){
			   file.width(32);
			   file>>s;
			   file.close();
			   break;
			}
			else {s="NULL";break;}
		}
	}
	return s;
}
string Catalog::findprimary(string database,string tablename)
{
	string name=DATA_DIR+database+"\\"+"directorydata.txt";
	fstream file;
	string primary;
	file.open(name.c_str(),fstream::binary|fstream::out|fstream::in);
	int tablecount;
	string s;
	file.width(16);
	file>>tablecount;
	for(int i=0;i<tablecount;i++){
		file.seekg(16+i*2600);
		file.width(32);
		file>>s;
		if(s==tablename){
			file.seekg(16+i*2600+32);
			int indexcount;
			int is;
			file.width(4);
			file>>indexcount;
			for(int j=0;j<indexcount;j++){
				file.seekg(16+i*2600+40+80*j+44);
				file.width(2);
				file>>is;
				if(is==1){
					file.seekg(16+i*2600+40+80*j+48);
					file.width(32);
					file>>primary;
					file.close();
					break;
				}
			}
		}

	}
    return primary;
}
IndexInfo& Catalog::Select(SQL *b,string database)
{//select l1 l2 from table where k1=kk k2=mm
	IndexInfo* indexinfo=new IndexInfo();
	string name=DATA_DIR+database+"\\";
	if ((GetFileAttributesA(name.c_str()) == INVALID_FILE_ATTRIBUTES) || !(GetFileAttributesA(DATA_DIR.c_str()) & FILE_ATTRIBUTE_DIRECTORY)){
			throw Error(8,"Database not exists!");
	}
 //���ݿ��ļ��²����Ƿ���table,table���Ƿ�����Ӧ���ԣ����������������¼��
    name+=b->targetName+"\\";
    if ((GetFileAttributesA(name.c_str()) == INVALID_FILE_ATTRIBUTES) || !(GetFileAttributesA(DATA_DIR.c_str()) & FILE_ATTRIBUTE_DIRECTORY)){
    			throw Error(8,"Table not exists!");
    }
    if(b->result.size()==0&&b->argv.size()==0){//select * from table
         //��Ҫ����primary index
    	//indexinfo->attrname.push_back(findprimary(b->gettargetName()));
    	//indexinfo->indexname.push_back(findprimary(b->gettargetName()));
    }
    else
    if(b->result.size()==0&&b->argv.size()!=0){// select * from table where
    	vector<string>::iterator wt=b->argv.begin();
        for(int i=0;i<b->argv.size();i++,wt++){
        	if(attrfind(database,b->targetName,*wt)){
        		//���������Ƿ���������������д�뷵�صĽṹ��
        	   if(attrindex(database,b->targetName,*wt)!="NULL"){
        		   indexinfo->attrname.push_back(*wt);
        		   indexinfo->indexname.push_back(attrindex(database,b->targetName,*wt));
        	   }
        	}
        	else{
        		throw Error(8,"attribute not exists!");
        	}
        }
        //check value fit the type?
    }
    else
    if(b->result.size()!=0&&b->argv.size()==0){// select ... from table
    	vector<string>::iterator st=b->result.begin();
    	for(int i=0;i<b->result.size();i++,st++){
    	   if(!attrfind(database,b->targetName,*st)){
    	      throw Error(8,"attribute not exists!");
    	   }
    	}
    }
    else
    if(b->result.size()!=0&&b->argv.size()!=0){//select ... from table where ...
    	vector<string>::iterator st=b->result.begin();
    	for(int i=0;i<b->result.size();i++,st++){
    	    if(!attrfind(database,b->targetName,*st)){
    	    	throw Error(8,"attribute not exists!");
    	    }
    	}
    	vector<string>::iterator wt=b->argv.begin();
    	for(int i=0;i<b->argv.size();i++,wt++){
    	    	        		//���������Ƿ���������������д�뷵�صĽṹ��
    	       if(attrfind(database,b->targetName,*wt)){
    	    	   if(attrindex(database,b->targetName,*wt)!="NULL"){
    	    		   indexinfo->attrname.push_back(*wt);
    	    		   indexinfo->indexname.push_back(attrindex(database,b->targetName,*wt));
    	    	   }
    	       }
    	       else{
    	    	   throw Error(8,"attribute not in this table");
    	       }

    	}
    }
    TableInfo table=ShowTable(b,database);
    indexinfo->table=table;
    return *indexinfo;

}
int Catalog::findpost(string database,string tablename)
{
	string name=DATA_DIR+database+"\\"+"directorydata.txt";
	fstream file;
	file.open(name.c_str(),fstream::binary|fstream::out|fstream::in);
	int tablecount;
	string s;
	int post;
	file.width(16);
	file>>tablecount;
	for(int i=0;i<tablecount;i++){
		file.seekg(16+i*2600);
		file.width(32);
		file>>s;
		if(s==tablename){
			post=16+i*2600;
			break;
		}
	}
	file.close();
	return post;
}
IndexInfo& Catalog::Delete(SQL *b,string database)
{//delete from table   delete from table where ....
	IndexInfo* indexinfo=new IndexInfo();
	string name=DATA_DIR+database+"\\";
		if ((GetFileAttributesA(name.c_str()) == INVALID_FILE_ATTRIBUTES) || !(GetFileAttributesA(DATA_DIR.c_str()) & FILE_ATTRIBUTE_DIRECTORY)){
				throw Error(9,"Database not exists!");
	}
	name+=b->targetName+"\\";
	if ((GetFileAttributesA(name.c_str()) == INVALID_FILE_ATTRIBUTES) || !(GetFileAttributesA(DATA_DIR.c_str()) & FILE_ATTRIBUTE_DIRECTORY)){
	    	 throw Error(9,"Table not exists!");
	}
	name=DATA_DIR+database+"\\"+"directorydata.txt";
	fstream file;
	file.open(name.c_str(),fstream::binary|fstream::out|fstream::in);
	int post=findpost(database,b->targetName);
	int attrcount=findattrnum(database,post);
	int flag;
	string iname;
	for(int i=0;i<attrcount;i++){
		file.seekg(post+40+80*i+46);
		file.width(2);
		file>>flag;
		if(flag==1){
			file.seekg(post+40+i*80+48);
			file.width(32);
			file>>iname;
			indexinfo->indexname.push_back(iname);
			file.seekg(post+40+i*80);
			file.width(32);
			file>>iname;
			indexinfo->attrname.push_back(iname);
		}
	}
	file.close();
	if(b->argv.size()==0){//delete from table,
		//indexinfo->attrname.push_back(findprimary(b->gettargetName()));
		//indexinfo->indexname.push_back(findprimary(b->gettargetName()));
	}
	else{//delete from table where ...
		vector<string>::iterator wt=b->argv.begin();
		for(int i=0;i<b->argv.size();i++,wt++){
		  if(attrfind(database,b->targetName,*wt)){
		   /* if(attrindex(database,b->targetName,*wt)!="NULL"){
		    	   indexinfo->attrname.push_back(*wt);
		    	   indexinfo->indexname.push_back(attrindex(database,b->targetName,*wt));
		    }*/
		  }
		  else{
		    	throw Error(9,"attribute not in this table");
		  }
		}
	}
	TableInfo table=ShowTable(b,database);
	indexinfo->table=table;
	return *indexinfo;
}
int Catalog::findattrnum(string database,int tablepost)//table����ʼ��ַ
{
	string name=DATA_DIR+database+"\\"+"directorydata.txt";
	fstream file;
	file.open(name.c_str(),fstream::binary|fstream::in);
	int attrcount;
	file.seekg(tablepost+32);
	file.width(4);
	file>>attrcount;
	return attrcount;
}
int Catalog::findattrtype(string database,int tablepost,int n)
{
	int type;
	string name=DATA_DIR+database+"\\"+"directorydata.txt";
	fstream file;
	file.open(name.c_str(),fstream::binary|fstream::in);
    file.seekg(tablepost+40+n*80+32);
	file.width(8);
	file>>type;
	return type;
}
int Catalog::findattrlength(string database,int tablepost,int n)
{
	int length;
	string name=DATA_DIR+database+"\\"+"directorydata.txt";
	fstream file;
	file.open(name.c_str(),fstream::binary|fstream::in);
	file.seekg(tablepost+40+n*80+40);
	file.width(4);
	file>>length;
	return length;
}
IndexInfo& Catalog::Insert(SQL *b,string database)
{
	IndexInfo* indexinfo=new IndexInfo();
	string name=DATA_DIR+database+"\\";
	if ((GetFileAttributesA(name.c_str()) == INVALID_FILE_ATTRIBUTES) || !(GetFileAttributesA(DATA_DIR.c_str()) & FILE_ATTRIBUTE_DIRECTORY)){
				throw Error(7,"Database not exists!");
	}
	name+=b->targetName+"\\";
	if ((GetFileAttributesA(name.c_str()) == INVALID_FILE_ATTRIBUTES) || !(GetFileAttributesA(DATA_DIR.c_str()) & FILE_ATTRIBUTE_DIRECTORY)){
		    	 throw Error(7,"Table not exists!");
	}
	//��Ҫ�ҳ�table�������ļ��ֵ��е�λ�ã�������Ҫ֪�������ж��٣�Ȼ���ж�ֵ�ø������������״�����ȥ���������Ƿ�Ͷ�Ӧ�����������Ƿ��Ӧ
    //��������Ҫ�ж�ֵ�����ͺͳ����Ƿ�����Ҫ��
	int post=findpost(database,b->targetName);
	int count=findattrnum(database,post);
	if(count!=b->argv.size()){
		throw Error(7,"attribute numbers not right!");
	}
	vector<string>::iterator an=b->argv.begin();
    for(int i=0;i<b->argv.size();i++,an++){
        int type=findattrtype(database,post,i);
        if(type==1){//char
            int length=findattrlength(database,post,i);
            if(length<(*an).length()){
            	throw Error(7,"value doesn't fit!");
            }
        }
        else
        if(type==2){//int
        	int le=(*an).length();
        	int i=0;
        	int length=findattrlength(database,post,i);
        	if((*an)[0]=='-'){le-=1;i=1;}
        	if(length<le){
        	     throw Error(7,"value doesn't fit!");
        	}
        	for(;i<(*an).length();i++){
        		if((*an)[i]<'0'||(*an)[i]>'9'){
        			throw Error(7,"value doesn't fit!");
        		}
        	}
        }
        else{//float
        	int le=(*an).length();
        	int i=0;
        	int length=findattrlength(database,post,i);
        	if((*an)[0]=='-'){le-=1;i=1;}
        	if(length<le){
        		throw Error(7,"value doesn't fit!");
        	}
        	for(;i<(*an).length();i++){
        	     if(((*an)[i]<'0'&&(*an)[i]!='.')||((*an)[i]>'9'&&(*an)[i]!='.')){
        	    	 throw Error(7,"value doesn't fit!");
        	     }
        	}
        }
    }
    name=DATA_DIR+database+"\\"+"directorydata.txt";
    fstream file;
    file.open(name.c_str(),fstream::binary|fstream::out|fstream::in);
    int flag;
    string iname;
    for(int i=0;i<count;i++){
    	file.seekg(post+40+80*i+46);
    	file.width(2);
    	file>>flag;
    	if(flag==1){
    		file.seekg(post+40+i*80+48);
    		file.width(32);
    		file>>iname;
    		indexinfo->indexname.push_back(iname);
    		file.seekg(post+40+i*80);
    		file.width(32);
    		file>>iname;
    		indexinfo->attrname.push_back(iname);
    	}
    }
    TableInfo table=ShowTable(b,database);
    indexinfo->table=table;
    return *indexinfo;//���صĶ�����д
}
void Catalog::execFile(SQL *b,string database)
{
	 string str = b->targetName+".txt";
	 ifstream infile(str.c_str(), ios::in);
	 if(!infile){
		throw(13,"can not open the execfile");
	 }
	 string buffer="";
	 SQL execSql;
	 string s;
	 while(!infile.eof()){
		  getline(infile, s, ';');//����һ��
	 	  if(str=="eof")
	 	  break;
	 	  istringstream ss(str);//ת����istringstream
//	 	  interpret(ss,execSql);//����׼��SQL���ת����������
//	 	  APIModule(execSql,database);//����API����ȡ�ض���ִ�з�ʽ
	 		}
}
void Catalog::helpInfo()
{
	ifstream infile("Help.txt",ios::in);
	if(!infile){
	   throw Error(14,"unable to open help.txt");
	}
	string buffer="";
	while(!infile.eof()){//��help.txt�ļ��е���Ϣ��ʾ����Ļ��
		getline(infile, buffer);
		cout << buffer << endl;
	}
	infile.close();
}




