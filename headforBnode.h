/*
 * headforBnode.h
 *
 *  Created on: 2014年11月14日
 *      Author: Administrator
 */

#ifndef HEADFORBNODE_H_
#define HEADFORBNODE_H_
#include<iostream>
#include<vector>
#include<utility>

using namespace std;
//异常
class insert_index_error:public exception
{

};

class not_found:public exception{

};

class delete_index_error: public exception
{

};

//////////////////////
class index_location
{
public:
	int blocknum;//块号
	int number; //第几个
	vector<string> record_position;
	index_location(int a,int b,vector<string> s):blocknum(a),number(b),record_position(s){}
};
//位置信息




class search_info
{
public:
	string db_name;
	vector<string> condition;// = or <= or >= or < or >
	vector<string> attribute_name;//attribute name
	vector<string> value;
};

class search_result
{
public:
	vector<string> value;
};//一个search_result即为一条查询结果





#endif /* HEADFORBNODE_H_ */
