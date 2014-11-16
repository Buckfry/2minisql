/*
 * RecordManager.cpp
 *
 *  Created on: 2014年11月5日
 *      Author: zhanghui2014
 */

#include "Recordmanager.h"
#include"sql.h"
#include"block.h"
#include"string.h"
#include"BufferManager.h"
#include<iterator>
#include<algorithm>
#include<bitset>
#include<utility>
#include<sstream>
#include"APIModule.h"
#include<stdlib.h>
#include<stdio.h>


extern BufferManager datamanager;

RecordManager::RecordManager() {
	// TODO Auto-generated constructor stub

}

RecordManager::~RecordManager() {
	// TODO Auto-generated destructor stub
}

void RecordManager::focus_current_db(string DB_name)
{
	if(DB_name!=datamanager.DBname)
					datamanager.setDatabase(DB_name);
}
//释放数据库的内存缓冲区
void RecordManager::Close_Database(string DB_name)
{
	this->focus_current_db(DB_name);
        datamanager.closeDatabase();
}
//释放表或索引的内存缓冲区
void RecordManager::Close_File(string DB_name, string filename)
{
	if(DB_name!=datamanager.DBname)
					datamanager.setDatabase(DB_name);
	datamanager.closeFile(filename);
}
//把fileinfo转为char*存储
char* RecordManager::translate_fileinfo(record_fileInfo fi)
{
     stringstream tp;
     tp<<fi.recordAmount<<fi.recordLength<<fi.recordcount<<fi.currentblocknum;
     char * temp = new char(16);
     for(int i=0;i<16;i++)
    	 temp[i] = tp.str().at(i);
     cout <<"temp in translate_fileinfo: " << temp << " length:" <<sizeof(temp)<< endl;
//     strcpy(temp,(tp.str()).c_str());
      return  temp;
}
//更新fi
void RecordManager::getfileinfo(string fileinfo,attr_info attribute_info,string filename)
{
	fi.attribute_name=attribute_info.attribute_name;
	fi.attribute_type=attribute_info.attribute_type;
	fi.fileName=filename;
    stringstream tp;
    string dump;
    tp<<fileinfo;
    tp>>dump>>fi.recordAmount>>dump>>fi.recordLength>>dump>>fi.recordcount>>dump>>fi.currentblocknum;
}
//在建表的时候初始化它的头文件信息
void RecordManager::create_table(string DB_name,create_record data)
{
	this->focus_current_db(DB_name);

	fi.recordLength=0;
	for(vector<int>::iterator it=data.attribute_length.begin();it!=data.attribute_length.end();it++,fi.recordLength+=*it);
    fi.recordAmount=BLOCK_LEN/fi.recordLength;
    fi.currentblocknum=1;
    fi.recordcount=0;
    cout << "before record createTable" << endl;
    datamanager.createTable(data.table_name,translate_fileinfo(this-> fi));

}
//向表中插入元组,同时返回插入位置
recordposition& RecordManager::insert_record(string DB_name,string filename,vector<string> attrvalue,attr_info attribute_info)
{
	this->focus_current_db(DB_name);
    getfileinfo(datamanager.getFileInfo(filename),attribute_info,filename);
    block datablock;
    recordposition rp;

     if(fi.recordcount>fi.recordAmount)
     {
    	 if(fi.currentblocknum>MAX_BLOCK)
    		 cout<<("This table is filled, you cann't add this tulpe into it!")<<endl;
    	 else
    	 {
    		 fi.currentblocknum++;
    		 fi.recordcount=0;
    	 }

     }


    int head=fi.recordcount*fi.recordLength;
    datablock=(*(datamanager.readBlock(filename,(fi.currentblocknum-1))));
    string data(datablock.data,head);
    int focus_attr=0;
    for(vector<string>::iterator it=attrvalue.begin(); it!=attrvalue.end();it++,focus_attr++)
    {
    	data+=*it;
    	for(vector<string>::size_type i=(*it).size();
    			(int) i< attribute_info.attribute_length[focus_attr];
    			data+="/0",i++);
    }

    fi.recordcount++;
    strcpy(datablock.data,data.substr(0,4095).c_str());
//    datablock.data=data.c_str();
    char* p=new char(4096);
    strcpy(p,datablock.data);
    datamanager.insertBlock(filename,(fi.currentblocknum-1),p);
    datamanager.updateFileInfo(filename,this->translate_fileinfo(fi));
    rp.blocknum=(fi.currentblocknum-1);
    rp.recordnum=fi.recordcount;
    return rp;
}
//打印属性值
void RecordManager::Print(vector<string> attr)
{
	for(vector<string>::iterator it=attr.begin();it!=attr.end();it++)
	{
		cout<<"\t"<<*it;
	}
	cout<<endl;
}
//打印属性名
void RecordManager::printhead(vector<string> attr)
{
	for(vector<string>::iterator it=attr.begin();it!=attr.end();it++)
	{
		cout<<"\t"<<*it;
	}
	cout<<endl;
}
//找出所查找的属性在所有属性中的位置
vector<int>& RecordManager::findposition(vector<string>attr,vector<string>searchedattr)
{
	vector<int> selected_attr;
    for(vector<string>::iterator it=searchedattr.begin();it!=searchedattr.end();it++)
    {
    	   vector<string>::iterator findit = find(attr.begin(),attr.end(),*it);
    	   int num=0;
           for(vector<string>::size_type n=findit-attr.begin();n--!=0;num++)
           {
        	   selected_attr.push_back(num);
           }
    }
    return selected_attr;
}
//选择语句（无where)
void RecordManager::Select_No_Where(string DB_name,string filename,vector<string> attr,attr_info attribute_info)
{
	this->focus_current_db(DB_name);
	 getfileinfo(datamanager.getFileInfo(filename),attribute_info,filename);
	block datablock;
	vector<int> attrnum(findposition(fi.attribute_name,attr));
	this->printhead(attr);
	for(int searchblock=0;searchblock!=fi.currentblocknum;searchblock++)
	    {
	    	datablock=(*(datamanager.readBlock(filename,searchblock)));
	    	for(int searchrecord=0;searchrecord!=fi.recordAmount;searchrecord++)
	    	{
	    		vector<string> attrvalue;
	    		for(vector<int>::iterator it=attrnum.begin();it!=attrnum.end();it++)
	    		{
	    			int attrlength=attribute_info.attribute_length[*it];
	    			int headlength=0;
	       			for(int n=*it,i=0;n!=0;headlength+=attribute_info.attribute_length[i++],n--);
	    			string temp1(datablock.data,(searchrecord*fi.recordLength));
	   	   			string temp2(temp1,headlength,headlength+attrlength);
	       			attrvalue.push_back(temp2);
	    		}
	    		this->Print(attrvalue);
	    	}
	    }
}

//给index提供keyinfo
keyinfo& RecordManager::getkeyinfo(string DB_name,string filename,string keyname,attr_info attribute_info)
{
	this->focus_current_db(DB_name);
	 getfileinfo(datamanager.getFileInfo(filename),attribute_info,filename);
	vector<string> temp;
	temp.push_back(keyname);
	keyinfo keyinformation;
	vector<pair<string,recordposition> > keys;
	pair<string,recordposition> key;

	block datablock;
	vector<int> attrnum(findposition(fi.attribute_name,temp));
	for(int searchblock=0;searchblock!=fi.currentblocknum;searchblock++)
	    {
	    	datablock=(*(datamanager.readBlock(filename,searchblock)));
	    	for(int searchrecord=0;searchrecord!=fi.recordAmount;searchrecord++)
	    	{
	    		for(vector<int>::iterator it=attrnum.begin();it!=attrnum.end();it++)
	    		{
	    			int attrlength=attribute_info.attribute_length[*it];
	    			int headlength=0;
	    			for(int n=*it,i=0;n!=0;headlength+=attribute_info.attribute_length[i++],n--);
	    			string temp1(datablock.data,searchrecord*fi.recordLength);
	    			string temp2(temp1,headlength,headlength+attrlength);
	    			key.first=temp2;
	    			key.second.blocknum=searchblock;
	    			key.second.recordnum=searchrecord;
	    			keys.push_back((key));
	    		}
	    	}
	    }
	keyinformation.keyname=keyname;
	keyinformation.keys=keys;
	return keyinformation;
}
//删除语句（无where)
void RecordManager::Delete_No_Where(string DB_name,string filename,attr_info attribute_info)
{
	this->focus_current_db(DB_name);
	 getfileinfo(datamanager.getFileInfo(filename),attribute_info,filename);
	block datablock;
	for(int searchblock=0;searchblock!=fi.currentblocknum;searchblock++)
	    {
	    	datablock=(*(datamanager.readBlock(filename,searchblock)));
	    	for(int searchrecord=0;searchrecord!=fi.recordAmount;searchrecord++)
	    	{
	    		for(int i =0;i!=fi.recordLength;i++)
	    		{
	    			datablock.data[searchrecord*fi.recordLength+i]='\0';
	    		}
	    	}
	        char* p=new char(4096);
	        strcpy(p,datablock.data);
	        datamanager.updateBlock(filename,searchblock,p);
	    }
	fi.currentblocknum=1;
	fi.recordcount=0;
    datamanager.updateFileInfo(filename,this->translate_fileinfo(fi));
}
//判断是否符合条件condition
bool RecordManager::Confirm(vector<string> attr,vector<string> attrvalue, condition_info condition)
{
	 vector<int> conditionattrnum(findposition(attr,condition.conditionattr));
	 vector<int> conditionattrtypenum(findposition(fi.attribute_name,condition.conditionattr));
	 vector<int>::iterator it;
	 vector<int>::iterator itype;
	 int i=0;
	for (it = conditionattrnum.begin(), itype = conditionattrtypenum.begin();
			it != conditionattrnum.end(); it++, i++, itype++) {
		//得到所比较属性的类型fi.attribute_type[*it]
		//得到左比较值attr[*it]，进行比较
		if (condition.condition[i] == "=")		// ‘=’
				{
			if (fi.attribute_type[*itype] == 2) {
				int i1 = atoi( attr[*it].c_str());
				int i2 = atoi((condition.comparedvalue[i]).c_str());
				if (i1 == i2)
					continue;
				else
					return 0;
			} else if (fi.attribute_type[*itype] == 1) {
				const char *p1 = attr[*it].c_str();
				const char *p2 = (condition.comparedvalue[i]).c_str();
				if (!strcmp(p1, p2))
					continue;
				else
					return 0;
			} else if (fi.attribute_type[*itype] == 3) {
				float f1 = (float) atof(attr[*it].c_str());
				float f2 = (float) atof(
						(condition.comparedvalue[i]).c_str());
				if (f1 == f2)
					continue;
				else
					return 0;
			}

		} else if (condition.condition[i] == "<>") {
			if (fi.attribute_type[*itype] == 2) {
				int i1 = atoi( attr[*it].c_str());
				int i2 = atoi ((condition.comparedvalue[i]).c_str());
				if (i1 != i2)
					continue;
				else
					return 0;
			} else if (fi.attribute_type[*itype] == 1) {
				const char *p1 = attr[*it].c_str();
				const char *p2 = (condition.comparedvalue[i]).c_str();
				if (strcmp(p1, p2))
					continue;
				else
					return 0;
			} else if (fi.attribute_type[*itype] == 3) {
				float f1 = (float) atof( attr[*it].c_str());
				float f2 = (float) atof(
						(condition.comparedvalue[i]).c_str());
				if (f1 != f2)
					continue;
				else
					return 0;
			}

		} else if (condition.condition[i] == "<") // '<'
				{
			if (fi.attribute_type[*itype] == 2) {
				int i1 = atoi( attr[*it].c_str());
				int i2 = atoi( (condition.comparedvalue[i]).c_str());
				if (i1 < i2)
					continue;
				else
					return 0;
			} else if (fi.attribute_type[*itype] == 1) {
				const char *p1 = attr[*it].c_str();
				const char *p2 = (condition.comparedvalue[i]).c_str();
				if (strcmp(p1, p2) < 0)
					continue;
				else
					return 0;
			} else if (fi.attribute_type[*itype] == 3) {
				float f1 = (float) atof(attr[*it].c_str());
				float f2 = (float) atof(
						(condition.comparedvalue[i]).c_str());
				if (f1 < f2)
					continue;
				else
					return 0;
			}

		} else if (condition.condition[i] == ">") // '>'
				{
			if (fi.attribute_type[*itype] == 2) {
				int i1 = atoi( attr[*it].c_str());
				int i2 = atoi( (condition.comparedvalue[i]).c_str());
				if (i1 > i2)
					continue;
				else
					return 0;
			} else if (fi.attribute_type[*itype] == 1) {
				const char *p1 = attr[*it].c_str();
				const char *p2 = (condition.comparedvalue[i]).c_str();
				if (strcmp(p1, p2) > 0)
					continue;
				else
					return 0;
			} else if (fi.attribute_type[*itype] == 3) {
				float f1 = (float) atof(attr[*it].c_str());
				float f2 = (float) atof(
						(condition.comparedvalue[i]).c_str());
				if (f1 > f2)
					continue;
				else
					return 0;
			}

		} else if (condition.condition[i] == "<=") // '<='
				{
			if (fi.attribute_type[*itype] == 2) {
				int i1 = atoi( attr[*it].c_str());
				int i2 = atoi( (condition.comparedvalue[i]).c_str());
				if (i1 <= i2)
					continue;
				else
					return 0;
			} else if (fi.attribute_type[*itype] == 1) {
				const char *p1 = attr[*it].c_str();
				const char *p2 = (condition.comparedvalue[i]).c_str();
				if (strcmp(p1, p2) <= 0)
					continue;
				else
					return 0;
			} else if (fi.attribute_type[*itype] == 3) {
				float f1 = (float) atof(attr[*it].c_str());
				float f2 = (float) atof(
						(condition.comparedvalue[i]).c_str());
				if (f1 <= f2)
					continue;
				else
					return 0;
			}

		} else if (condition.condition[i] == ">=") // '>='
				{
			if (fi.attribute_type[*itype] == 2) {
				int i1 = atoi( attr[*it].c_str());
				int i2 = atoi( (condition.comparedvalue[i]).c_str());
				if (i1 >= i2)
					continue;
				else
					return 0;
			} else if (fi.attribute_type[*itype] == 1) {
				const char *p1 = attr[*it].c_str();
				const char *p2 = (condition.comparedvalue[i]).c_str();
				if (strcmp(p1, p2) >= 0)
					continue;
				else
					return 0;
			} else if (fi.attribute_type[*itype] == 3) {
				float f1 = (float) atof( attr[*it].c_str());
				float f2 = (float) atof(
						(condition.comparedvalue[i]).c_str());
				if (f1 >= f2)
					continue;
				else
					return 0;
			}

		}
			else
			{cout<<"An error in record_200+ condition"<<endl;
			exit(1);}

	}
		return 1;
}
//有where，但无可用索引的select语句
void RecordManager::Select_Without_Useful_Cond(string DB_name,string filename,vector<string> attr,condition_info cond,attr_info attribute_info)
{
	this->focus_current_db(DB_name);
	 getfileinfo(datamanager.getFileInfo(filename),attribute_info,filename);
    block datablock;
    vector<int> attrnum(findposition(fi.attribute_name,attr));
    this->printhead(attr);
    for(int searchblock=0;searchblock!=fi.currentblocknum;searchblock++)
    {
    	datablock=(*(datamanager.readBlock(filename,searchblock)));
    	for(int searchrecord=0;searchrecord!=fi.recordAmount;searchrecord++)
    	{
    		vector<string> attrvalue;
    		for(vector<int>::iterator it=attrnum.begin();it!=attrnum.end();it++)
    		{
    			int attrlength=attribute_info.attribute_length[*it];
    			int headlength=0;
  	       		for(int n=*it,i=0;n!=0;headlength+=attribute_info.attribute_length[i++],n--);
    		    string temp1(datablock.data,(searchrecord*fi.recordLength));
    			string temp2(temp1,headlength,headlength+attrlength);
    			attrvalue.push_back(temp2);
    		}
    		if(this->Confirm(attr,attrvalue,cond))
    		this->Print(attrvalue);
    	}
    }

}
//有where,但无可用索引的delete语句
vector<vector<string> >& RecordManager::Delete_Without_Useful_Cond(string DB_name,string filename,condition_info cond,attr_info attribute_info,vector<string> indexattr)
{
	this->focus_current_db(DB_name);
	 getfileinfo(datamanager.getFileInfo(filename),attribute_info,filename);
	 vector<string> indexvalue;
	 vector<vector<string> > indexvalues;
	    block datablock;
	    vector<int> attrnum(findposition(fi.attribute_name,cond.conditionattr));


	    for(int searchblock=0;searchblock!=fi.currentblocknum;searchblock++)
	    {
	    	datablock=(*(datamanager.readBlock(filename,searchblock)));
	    	for(int searchrecord=0;searchrecord!=fi.recordAmount;searchrecord++)
	    	{
	    		vector<string> attrvalue;
	    		for(vector<int>::iterator it=attrnum.begin();it!=attrnum.end();it++)
	    		{
	    			int attrlength=attribute_info.attribute_length[*it];
	    			int headlength=0;
	    			for(int n=*it,i=0;n!=0;headlength+=attribute_info.attribute_length[i++],n--);
	    			string temp1(datablock.data,(searchrecord*fi.recordLength));
	    			string temp2(temp1,headlength,headlength+attrlength);
	    			attrvalue.push_back(temp2);
	    		}
	    		if(this->Confirm(cond.conditionattr,attrvalue,cond))
	    		{
	    			for(int i =0; i!=fi.recordLength;i++)
	    			{
	    				datablock.data[searchrecord*fi.recordLength+i]='\0';//已删除的record仍旧占用空间，不更新fileinfo
	    			}
	    			vector<int> indexattrnum(findposition(fi.attribute_name,indexattr));
	    			for(vector<int>::iterator j=indexattrnum.begin();j!=indexattrnum.end();j++)
	    			{
	    				indexvalue.push_back(attrvalue[*j]);
	    			}
	    			indexvalues.push_back(indexvalue);
	    		}
	    	}
	        char* p=new char(4096);
	        strcpy(p,datablock.data);
	        datamanager.updateBlock(filename,searchblock,p);
	    }
	    return indexvalues;
}
//有where，有索引可用的select语句
void RecordManager::Select_With_Useful_Cond(string DB_name,string filename,vector<string> attr,vector<recordposition> rp,condition_info cond,attr_info attribute_info)
{
	this->focus_current_db(DB_name);
	 getfileinfo(datamanager.getFileInfo(filename),attribute_info,filename);
    block datablock;
    vector<int> attrnum(findposition(fi.attribute_name,attr));
    this->printhead(attr);
    for(vector<recordposition>::iterator po=rp.begin();po!=rp.end();po++)
    {
    	datablock=(*(datamanager.readBlock(filename,(*po).blocknum)));

   		vector<string> attrvalue;
   		for(vector<int>::iterator it=attrnum.begin();it!=attrnum.end();it++)
   		{
   			int attrlength=attribute_info.attribute_length[*it];
   		    int headlength=0;
   			for(int n=*it,i=0;n!=0;headlength+=attribute_info.attribute_length[i++],n--);
   		    string temp1(datablock.data,(((*po).recordnum)*fi.recordLength));
   		    string temp2(temp1,headlength,headlength+attrlength);
   			attrvalue.push_back(temp2);
   		}
   		if(this->Confirm(attr,attrvalue,cond))
    	this->Print(attrvalue);
    }
}
//有where,有可用索引的delete语句,同时返回删除位置
vector<vector<string> >& RecordManager::Delete_With_Useful_Cond(string DB_name,string filename,vector<recordposition> rp,condition_info cond,attr_info attribute_info,vector<string> indexattr)
{
	this->focus_current_db(DB_name);
	 getfileinfo(datamanager.getFileInfo(filename),attribute_info,filename);
	 vector<string> indexvalue;
	 vector<vector<string> > indexvalues;
	    block datablock;
	    vector<int> attrnum(findposition(fi.attribute_name,cond.conditionattr));


	    for(vector<recordposition>::iterator po=rp.begin();po!=rp.end();po++)
	    {
	    	datablock=(*(datamanager.readBlock(filename,(*po).blocknum)));
	   		vector<string> attrvalue;
	    	for(vector<int>::iterator it=attrnum.begin();it!=attrnum.end();it++)
	    	{
	    		int attrlength=attribute_info.attribute_length[*it];
	    	    int headlength=0;
	    		for(int n=*it,i=0;n!=0;headlength+=attribute_info.attribute_length[i++],n--);
	    		string temp1(datablock.data,(((*po).recordnum)*fi.recordLength));
	    	    string temp2(temp1,headlength,headlength+attrlength);
	    		attrvalue.push_back(temp2);
	    	}
	    	if(this->Confirm(cond.conditionattr,attrvalue,cond))
	    	{
	    		for(int i =0; i!=fi.recordLength;i++)
	    		{
	    			datablock.data[((*po).recordnum)*fi.recordLength+i]='\0';//已删除的record仍旧占用空间，不更新fileinfo
	    		}
	    		vector<int> indexattrnum(findposition(fi.attribute_name,indexattr));
	    	    for(vector<int>::iterator j=indexattrnum.begin();j!=indexattrnum.end();j++)
	    		{
	    			indexvalue.push_back(attrvalue[*j]);
	    		}
	    		 indexvalues.push_back(indexvalue);
	    	}
	       char* p=new char(4096);
	       strcpy(p,datablock.data);
	       datamanager.updateBlock(filename,(*po).blocknum,p);
	   }
	    return indexvalues;
}
//drop table
void RecordManager::Drop_Table(string DB_name, string filename)
{
	this->focus_current_db(DB_name);
	datamanager.deleteTable(filename);
}
//退出数据库
void RecordManager::Quit(string DB_name)
{
	this->focus_current_db(DB_name);
	datamanager.closeDatabase();
}









