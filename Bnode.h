/*
 * Bnode.h
 *
 *  Created on: 2014年11月11日
 *      Author: Administrator
 */

#ifndef BNODE_H_
#define BNODE_H_



#include "Buffermanager.h"
#include "headforBnode.h"
#include "publichead.h"
#include <math.h>
#include <stdlib.h>
using namespace std;

extern BufferManager datamanager;

class Bnode {
public:
//	BufferManager datamanager;
	Bnode(string name);
	int type;
	virtual ~Bnode();
	void createindex(int length,keyinfo info);
	void InserttoIndex(string value,recordposition info);
	void deleteindex(string value);
	void drop_index(string name)
	{
		datamanager.deleteTable(name);
	}
	vector<recordposition> searchbyindex(string condition,string value)
	{
		vector<recordposition> r;
		try
		{
			index_location temp = search(condition, value);
			for (int i = 0; i < temp.record_position.size(); i++) {
				recordposition tr;
				tr.blocknum = toint(temp.record_position[i].substr(0, 4));
				tr.recordnum = toint(temp.record_position[i].substr(4, 8));
				r.push_back(tr);
			}
		} catch (exception& e) {
			cout << "not found!" << endl;
		}
		return r;
	}

private:
	void insert_index(string value,string loc);
	string tostring(int a);
	int toint(string s);
	index_location search(string condition,string value);
	void split(int position);
	string update(int position,string child,string old,string value);
	bool showresult(string condition,string input,string value);
	void update_value(int position,string value,string child);
	void delete_update(int position);
	int fanout;
	int length;  //节点中value长度
	string index_name;
	int blocknum;
};

#endif /* BNODE_H_ */
