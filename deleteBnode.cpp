/*
 * delete.cpp
 *
 *  Created on: 2014年11月13日
 *      Author: Administrator
 */
#include "Bnode.h"

void Bnode::deleteindex(string value)
{
	string head = datamanager.getFileInfo(index_name);
	length = toint(head.substr(8,4));
	fanout = toint(head.substr(12,4));
	int position;   //找到子节点块号
	string a;
	string b;
	a = "=";
	b = value;
	int value_position;
	try {
		index_location l = search(a, b);
		position = l.blocknum;
		////////////////////////////////////////////////////////////////////////
		value_position = l.number;
	} catch (exception& e) {
		cout << "delete error: the value isn't in the index" << endl;
		throw new delete_index_error;
	}
	block* blk = datamanager.readBlock(index_name,position);
	string content = blk->data;
	content.erase(value_position*(length+8)+20,length+8);
	content.append(4095-content.size(),'0');
	if(value_position==0)        //更新父节点的value
	{
		update_value(toint(content.substr(4,4)),content.substr(28,length),content.substr(8,4));
	}
	int value_num = toint(content.substr(1,3));
	value_num--;
	content.replace(1,3,tostring(value_num).substr(1,3));
	strcpy(blk->data,content.c_str());
	datamanager.updateBlock(index_name,toint(content.substr(8,4)));
	if(content.substr(8,4)!="0001"&&value_num<(ceil((fanout-1)/2.0)))  //叶节点记录数太少了，需要合并
	{
		delete_update(toint(content.substr(8,4)));
	}

}

void Bnode::delete_update(int position)
{
	block* blk = datamanager.readBlock(index_name,position);
	string content = blk->data;
	block* next;
	int pointlength;
	int initial;
	bool isleft = false;
	if(content[0]=='2') //此节点为叶节点
	{
		pointlength =8;
		initial = 20;
		if(content.substr(16,4)!="0000")	//非最后一个叶节点
		{
			next = datamanager.readBlock(index_name,toint(content.substr(16,4)));
		}
		else
		{
			block* atemp = datamanager.readBlock(index_name,toint(content.substr(4,4)));  //取出父节点
			string temp_content = atemp->data;
			int record_num = toint(temp_content.substr(1,3));
			for(int i=0;i<record_num;i++)
			{
				if(temp_content.substr(i*(4+length)+16,4)==content.substr(8,4))//找到对应
				{
					i--;
					isleft = true;
					string rnext = temp_content.substr(i*(4+length)+16,4);
					next = datamanager.readBlock(index_name,toint(rnext));
				}
			}
		}
	}
	else     //该节点为非叶节点
	{
		pointlength =4;
		initial = 16;
		block* atemp = datamanager.readBlock(index_name,toint(content.substr(4,4)));  //取出父节点
		string temp_content = atemp->data;
		int record_num = toint(temp_content.substr(1,3));
		for (int i = 0; i < record_num; i++) {
			if (temp_content.substr(i * (4 + length) + 16,4) == content.substr(8, 4))  //找到对应
			{
				if(i>=1)   //该非叶节点有左邻点
				{
					isleft = true;
					i--;
				}
				else   //该非叶节点由右邻点
				{
					i++;
				}
				string rnext = temp_content.substr(i * (4 + length) + 16,4);
				next = datamanager.readBlock(index_name, toint(rnext));
			}
		}
	}
	string nextcontent = next->data;
	int number_next = toint(nextcontent.substr(1,3));
	if(number_next>ceil((fanout-1)/2.0))  //相邻点容量较大
	{
		if(isleft)   //邻点为左邻点
		{
			//把左邻点的最右边value+指针值给当前点
			int temp = (number_next-1)*(pointlength+length)+initial;
			string leftpoint = nextcontent.substr(temp+pointlength+length,pointlength); //指针值
			string leftvalue = nextcontent.substr(temp+pointlength,length);   //左邻点value值
			nextcontent.erase(temp+pointlength,length+pointlength);
			nextcontent.append(4095-nextcontent.size(),'0');
			//更新邻点的记录数（-1）
			string new_number_1 = tostring(number_next-1).substr(1,3);
			nextcontent.replace(1,3,new_number_1);
			strcpy(next->data,nextcontent.c_str());
			datamanager.updateBlock(index_name,toint(nextcontent.substr(8,4)));

			//更新父节点
			block* father = datamanager.readBlock(index_name,toint(nextcontent.substr(4,4)));
			string fathercontent = father->data;
			for(int i=0;i<toint(fathercontent.substr(1,3));i++)
			{
				if(fathercontent.substr(i*(4+length)+16,4)==nextcontent.substr(8,4))
				{
					leftpoint+=leftvalue;;
					fathercontent.replace(i*(4+length)+20,length,leftvalue);
					break;
				}
			}
			strcpy(father->data,fathercontent.c_str());
			datamanager.updateBlock(index_name,toint(fathercontent.substr(8,12)));

			//更新当前节点
			content.insert(initial,leftpoint);
			content = content.substr(0,4095);
			string new_number = tostring(ceil((fanout-1)/2.0)).substr(1,3);
			strcpy(blk->data,content.c_str());

			//更新父节点的value
//			update_value(toint(content.substr(4,4)),leftdata.substr(pointlength,length),content.substr(8,4));
		}
		else
		{	//把右邻点的最左边指针+value值给当前点
			string rightpoint = nextcontent.substr(initial, pointlength); //指针值
			string rightvalue = nextcontent.substr(initial+pointlength,length);//value值
			string fathervalue = nextcontent.substr(initial+length+pointlength*2,length);
			nextcontent.erase(initial, length+pointlength);
			nextcontent.append(4095 - nextcontent.size(), '0');
			//更新邻点的记录数（-1）
			string new_number_1 = tostring(number_next - 1).substr(1, 3);
			nextcontent.replace(1, 3, new_number_1);
			strcpy(next->data, nextcontent.c_str());
			datamanager.updateBlock(index_name,
					toint(nextcontent.substr(8, 4)));

			//更新父节点
			block* father = datamanager.readBlock(index_name,
					toint(nextcontent.substr(4, 4)));
			string fathercontent = father->data;
			for (int i = 0; i < toint(fathercontent.substr(1, 3)); i++) {
				if (fathercontent.substr(i * (4 + length) + 16, 4)
						== content.substr(8, 4))
				{
					rightpoint = rightvalue+rightpoint;
					fathercontent.replace(i * (4 + length) + 20, length,
							fathervalue);  //父亲节点更新为右邻点第二小的值
					break;
				}
			}
			strcpy(father->data, fathercontent.c_str());
			datamanager.updateBlock(index_name,
					toint(fathercontent.substr(8, 12)));

			//更新当前节点
			int current_num = toint(content.substr(1,3));	//当前记录数；
			current_num = (current_num)*(length+pointlength)+initial;
			content.insert(current_num, rightpoint);
			content = content.substr(0,4095);
			string new_number =
					tostring(ceil((fanout-1)/2.0)).substr(1, 3);
			strcpy(blk->data, content.c_str());
			datamanager.updateBlock(index_name,toint(content.substr(8,4)));
		}
	}
	else  //邻点大小等于(fanout)/2+(fanout)%2，此时进行合并
	{
		string lefti;
		string righti;
		if(isleft)
		{
			lefti = nextcontent;
			righti = content;
		}
		else
		{
			lefti = content;
			righti =nextcontent;
		}

		if(isleft)  //邻点为左邻点
		{
			block* father = datamanager.readBlock(index_name,toint(righti.substr(4,4)));
			string fathercontent = father->data;
			string data;
			int i;
			for (i = 0; i < toint(fathercontent.substr(1, 3)); i++) {
				if (fathercontent.substr(i * (4 + length) + 16, 4)
						== righti.substr(8, 4)) {
					fathercontent.erase(i*(4+length)+16-length,length+4);
					fathercontent.append(4095-fathercontent.size(),'0');
					int father_number = toint(fathercontent.substr(1,3));
					fathercontent.replace(1,3,tostring(father_number-1).substr(1,3));
					break;
				}
			}
			strcpy(father->data,fathercontent.c_str());
			datamanager.updateBlock(index_name,toint(fathercontent.substr(8,4)));
			if(righti[0]!='2') //为非叶节点
			{
				data = fathercontent.substr(i * (4 + length) + 16 - length,
											length) + righti.substr(initial, 4096);
				lefti.replace(1,3,tostring(fanout-1).substr(1,3));
			}
			else  //为叶节点
			{
				data = righti.substr(initial,4096);
				lefti.replace(1,3,tostring(fanout-2).substr(1,3));
			}

			//更新左邻点
			lefti.insert(number_next*(pointlength+length)+initial,data);
			lefti = lefti.substr(0,4095);
			strcpy(next->data, lefti.c_str());
			datamanager.updateBlock(index_name,
					toint(lefti.substr(8, 4)));

			//当前点变为空块
			string head = datamanager.getFileInfo(index_name);
			righti.replace(12,4,head.substr(0,4));
			head.replace(0,4,righti.substr(8,4));
			char* c = new char(head.length());
			strcpy(c,head.c_str());
			datamanager.updateFileInfo(index_name,c);
			strcpy(blk->data, righti.c_str());
			datamanager.updateBlock(index_name,toint(righti.substr(8,4)));

			//检查父节点
			if(toint(fathercontent.substr(1,3))<ceil((fanout-1)/2.0))
				delete_update(toint(fathercontent.substr(8,4)));
		}
	}
}


