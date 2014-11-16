/*
 * Bnode.cpp
 *
 *  Created on: 2014年11月11日
 *      Author: Administrator
 */

#include "Bnode.h"


Bnode::Bnode(string name) {
	index_name = name;
	// TODO Auto-generated constructor stub

}

Bnode::~Bnode() {
	// TODO Auto-generated destructor stub
}
void Bnode::createindex(int current_length,keyinfo info)
{
	string head="1000";//第一个空块位置,前面的1表示空块尚未建立
	head+="9999";//第一个叶节点位置,此时设为无效
	cout << "create index: current_length is: " << current_length <<endl;
	cout << "create index: current_length to string is: " << tostring(current_length) <<endl;
	head+=tostring(current_length);//value的长度 总长度为value+8
	fanout = 4096/(current_length+8+20);
	head+=tostring(fanout);//扇出数

	///////////////////////
	//create函数
	char* c = new char(16);
	for(int i=0;i<16;i++)
	{
		c[i] = head.at(i);
	}
	strcpy(c,head.c_str());
	cout <<"create index's head: " << c<<endl;;
	datamanager.createTable(index_name,c);
	//////////////////////
	for(int i=0;i<info.keys.size();i++)
	{
		try
		{
			string loc = tostring(info.keys[i].second.recordnum);
			loc+=tostring(info.keys[i].second.blocknum);
			insert_index(info.keys[i].first,loc);
		}
		catch(exception & e)
		{
			cout<<"exception: insert_index error when create the index" <<endl;
		}
	}
}

void Bnode::InserttoIndex(string value, recordposition info)
 {
		try {
			string loc = tostring(info.recordnum);
			loc += tostring(info.blocknum);
			insert_index(value, loc);
		} catch (exception & e) {
			cout << "exception: insert error!!!!!"
					<< endl;
		}
}

void Bnode::insert_index(string value,string loc)
{
	string head;
	///////////////////
	//获得头文件
	head = datamanager.getFileInfo(index_name);
	//////////////////
	fanout = toint(head.substr(12,4));  //扇出数
	length = toint(head.substr(8,4));   //一个value的长度

	//前面自动补齐
//	string temp;
//	temp.append(length-value.length(),'0');
//	value = temp + value;
	if(type ==3)
		{
			int i = 0;
			for (i = 0; i < value.length(); i++) {
				if (value.at(i) == '.')
					break;
			}
			if (i == value.length())
				value += ".0";
		}
	//补齐长度
	value = value.append(length-value.size(),'0');

	if(head.substr(4,4)=="9999")//第一个叶节点位置为空，此时创建root
	{
		string block="1";  //表明这是叶节点
		block+="001";  //记录数
		block+="9999"; //父节点，此时表明该节点即为root
		block+="0000"; //块号
		block+="0000";  //下一空块位置，只在空块中和头文件中有用
		block+="9999"; //下一叶节点块号，此时表明并无下一叶节点
		block+=loc;
		block+=value;
		block.append(4095-length-8-20,'0');
		char* c = new char(16);
		for(int i=0;i<16;i++)
			c[i]=head.at(i);
		strcpy(c,block.c_str());
		datamanager.insertBlock(index_name,0,c);//插入到第0块中
		head.replace(4,4,"0000");  //更新第一个叶节点位置
		head.replace(0,4,"1001");  //更新第一个空块位置
		char* c2 = new char(16);
		for(int i=0;i<16;i++)
			c2[i] =head.at(i);
		strcpy(c2,block.c_str());
		datamanager.updateFileInfo(index_name,c2);
	}
	else
	{
		int position;
		/////////////////////
		//找到块号position
//		index_location i_location = search("",value);
//		position = i_location.blocknum;
		////////////////////
		string a;
		string b = value;
		try
		{
			index_location l =  search(a,b);
			position = l.blocknum;
		}
		catch(exception& e)
		{
			cout <<"the value is already in the index!" << endl;
			throw new insert_index_error;
		}
		block* blk = datamanager.readBlock(index_name, position);
		string content = blk->data;
		int number = toint(content.substr(1, 3));
		if (number < fanout - 1) {					//叶节点未满
			int start;
			int num = 0;
			for (; num < number; num++) {
				start = num * (length + 8) + 20;
				if (showresult(">",content.substr(start + 8, length ),value)) {
									//content.substr(start + 8, length ) > value
					break;
				}
			}
			if (num == 0 && (content.substr(4, 4) != "9999")
					&& head.substr(4, 4) != content.substr(8, 4))//需要更新父节点的value值
							{
				int father = toint(content.substr(4, 4));
				update_value(father, value, content.substr(8, 4)); //更新父节点的value值
			}
			string temp = loc + value;

			//插入新值
			start = num * (length + 8) + 20;
			content.insert(start, temp);
			content.replace(1, 3, tostring(number + 1).substr(1,3));
			content = content.substr(0,4095);
			strcpy(blk->data, content.c_str());
			datamanager.updateBlock(index_name, position);
		} else	//叶节点已满
		{
			split(position);
			insert_index(value, loc);
		}
	}
}

void Bnode::split(int number)  //分裂叶节点
{
	block* blk = datamanager.readBlock(index_name,number);
	string content = blk->data;
	string head = datamanager.getFileInfo(index_name);
	int empty_block_num = toint(head.substr(0,4));
	bool build = false;
	if(empty_block_num>1000)
	{
		empty_block_num -=1000;
		build = true;
	}
	if(build)  //下一空块尚未建立
	{
		head.replace(0,4,tostring(empty_block_num+1+1000));   //指向下一空块
		string blank;
		blank.append(4095,'0');
		char* temp = new char(blank.length()+1);
		strcpy(temp,blank.c_str());
		datamanager.insertBlock(index_name,empty_block_num,temp);

		//更新头文件
		char* headtemp = new char(16);
		for(int i=0;i<16;i++)
			headtemp[i] = head.at(i);
		strcpy(headtemp,head.c_str());
		datamanager.updateFileInfo(index_name,headtemp);
	}
	block* empty = datamanager.readBlock(index_name,empty_block_num);
	string newblock;
	string old = content.substr(8,4);  //当前节点块号
	int value_num = toint(content.substr(1,3));  //叶节点中的value个数
	int father = toint(content.substr(4,4));    //父节点位置
	int num = (fanout-1)/2;
	//更新新增的节点
	newblock="2";
	newblock+=tostring(fanout-1-num).substr(1,3);
	newblock+="0000";          //暂时赋值，稍后更新
	newblock+=tostring(empty_block_num); //当前块号
	newblock+="0000";  //下一空块号
	newblock+=content.substr(16,4); //下一叶节点块号
	newblock+=content.substr(num*(length+8)+20,content.size()-num*(length+8)+20);
	newblock.append(4095-newblock.size(),'0');
	strcpy(empty->data,newblock.c_str());
	datamanager.updateBlock(index_name,empty_block_num);

	//更新当前叶节点
	content.replace(1,3,tostring(num).substr(1,3));
	content.replace(16,4,tostring(empty_block_num));//更新下一叶节点位置
	content  = content.substr(0,num*(length+8)+20);
	content.append(4095-content.size(),'0');
	strcpy(blk->data,content.c_str());
	datamanager.updateBlock(index_name,number);

	//更新父节点
	string temp =update(father,tostring(empty_block_num),old,newblock.substr(28,length));   //新增节点的值
	newblock.replace(4,4,temp);
	strcpy(empty->data,newblock.c_str());
	datamanager.updateBlock(index_name,empty_block_num);

}

string Bnode::update(int position,string child,string old,string new_value)  //是否分裂父节点
//child为新增叶节点块号，old为之前叶节点块号，new_value为新增节点最左的值
{
	string head = datamanager.getFileInfo(index_name);
	int empty_block_num = toint(head.substr(0,4));
	bool build = false;
	if(empty_block_num>1000)//下一空块尚未建立
	{
		empty_block_num-=1000;
		build = true;
	}

	if(position==9999)    //父节点为空，即之前的节点为根
	{
		if(build)
		{
			head.replace(0, 4, tostring(empty_block_num + 1 + 1000));   //指向下一空块
			string blank;
			blank.append(4095, '0');
			char* temp = new char(blank.length()+1);
			strcpy(temp, blank.c_str());
			datamanager.insertBlock(index_name, empty_block_num, temp);
			//更新头文件
			char* headtemp = new char(16);
			for(int i=0;i<16;i++)
				headtemp[i] = head.at(i);
			strcpy(headtemp,head.c_str());
			datamanager.updateFileInfo(index_name,headtemp);
		}
		block* oldchild = datamanager.readBlock(index_name,toint(old));//旧节点

		//把原本占据根节点的节点搬到其他地方去，使得第一块始终为根节点
		block* empty = datamanager.readBlock(index_name,empty_block_num);
		string temp = empty->data;
		if(oldchild->data[0]=='2')  //之前搬运的为第一个叶节点
		{
			head.replace(4,4,tostring(empty_block_num));
			char* headtemp = new char(16);
			for(int i=0;i<16;i++)
				headtemp[i] = head.at(i);
			strcpy(headtemp,head.c_str());
			datamanager.updateFileInfo(index_name,headtemp);

		}
		string old_child_data= oldchild->data;
		old_child_data.replace(4,4,"0000");
		strcpy(empty->data,old_child_data.c_str());
		datamanager.updateBlock(index_name,empty_block_num);//更新搬运后的节点


		//建立根节点
		string old_child_value = oldchild->data;
		string root ="1";
		root +=tostring(2).substr(1,3);
		root +="9999";  //父节点块号
		root +="0000";  //当前块号
		root +="0000";
		root +=old;
		////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////
		//别忘了要更新根节点的值
		/////////////////////////////////////////////
		root +=new_value;
		root +=child;
		root.append(4095-root.size(),'0');
		strcpy(oldchild->data,root.c_str());
		datamanager.updateBlock(index_name,0);
		return "0000";
	}
	else
	{
		block* father =  datamanager.readBlock(index_name,position);
		string content = father->data;    //content为当前节点内容
		int number =  toint(content.substr(1,3));
		if(number < fanout - 1)      //节点未满
		{
			int i = 0;
			for(;i < number ;i ++)
			{
				int start = i*(length+4)+16;
				if(content.substr(start,4)==old)
				{
					content.insert(start+4,new_value+child);
					int temp =number++;
					content.replace(1,3,tostring(temp).substr(1,3)); //更新记录数
					content = content.substr(0,4095);
					strcpy(father->data,content.c_str());		//更新
					datamanager.updateBlock(index_name,position);
//					if(i==0)        //需要更新父节点的value
//					{
//						update_value(toint(content.substr(4,4)),content.substr(20,length),content.substr(16,4));
//					}
					return content.substr(8,4); //返回父节点块号
				}
			}
		}
		else   //节点已满
		{
			if (build) {
				head.replace(0, 4, tostring(empty_block_num + 1 + 1000)); //指向下一空块
				string blank;
				blank.append(4095, '0');
				char* temp = new char(blank.length()+1);
				strcpy(temp, blank.c_str());
				datamanager.insertBlock(index_name, empty_block_num, temp);
				//更新头文件
				char headtemp[16];
				for(int i=0;i<16;i++)
					headtemp[i]=head.at(i);
//				strcpy(headtemp, head.c_str());
				datamanager.updateFileInfo(index_name, headtemp);
			}
//			int i;
//			int record_num = toint(content.substr(1,3));
//			bool isleft = false;
//			for(i=0;i<record_num;i++)
//			{
//				int start = i*(length+4)+16;
//				if(i>=record_num/2)
//				{
//					break;
//				}
//				else if(content.substr(start,4)==old)
//				{
//					isleft = true;
//					break;
//				}
//			}
			int num =(fanout-1)/2;
//			if (isleft) {
//				num = record_num / 2;
//			} else
//				num = record_num / 2 + record_num%2;
			////////////////////////////////////////
//			head.replace(0, 4, newblock.substr(12, 16));
			//更新新增的节点
			string newblock = "1";
			newblock += tostring(fanout - 1 - num).substr(1, 3);
			newblock += "0000";          //父节点暂时赋值，稍后更新
			newblock += tostring(empty_block_num);
			newblock += "0000";
			int i = 0;
			for (; i < number; i++) {
				int start = i * (length + 4) + 16;
				if (content.substr(start, 4) == old) {
					content.insert(start + 4, new_value + child);
//					int temp = number++;
//					content.replace(1, 3, tostring(temp).substr(1, 3)); //更新记录数
//					content = content.substr(0, 4095);
//					strcpy(father->data, content.c_str());		//更新
//					datamanager.updateBlock(index_name, position);
				}
			}
			newblock += content.substr(num * (length + 4) + 16+4+length,
					content.size()-(num * (length + 4) + 16+4+length));  //搬过去！
//			newblock +=child;
//			newblock +=new_value;
			newblock.append(4095 - newblock.size(), '0');

			//更新当前叶节点
			content.replace(1, 3, tostring(num).substr(1, 3));
			content = content.substr(0, num * (length + 4) + 16+4);
			content.append(4095 - content.size(), '0');
			strcpy(father->data, content.c_str());
			datamanager.updateBlock(index_name, position);

			 //更新分裂出去的块
			block* empty = datamanager.readBlock(index_name, //取出之前懒惰删除的块
									empty_block_num);
			string temp_empty = empty->data;
			strcpy(empty->data, newblock.c_str());
			datamanager.updateBlock(index_name, empty_block_num);

			//更新父节点
			string temp = update(toint(content.substr(4,4)),newblock.substr(16,4),content.substr(16,4),
					newblock.substr(20,length));
			newblock.replace(4,4,temp);
			strcpy(empty->data, newblock.c_str());
			datamanager.updateBlock(index_name, empty_block_num);

			return newblock.substr(8,4);
		}
	}
}
void Bnode::update_value(int position,string value,string child)
{
	block* father = datamanager.readBlock(index_name, position);
	string content = father->data;
	int num = 0;
	int number = toint(content.substr(1,3));
	for(;num<number;num++)
	{

		int start = num*(length+4)+16;
		if(content.substr(start,4)==child)
		{
			if(num!=0)
				content.replace(start+4,length,value);
			else
				update_value(toint(content.substr(4,4)),value,content.substr(8,4));
			strcpy(father->data,content.c_str());    //更新到这个block
			datamanager.updateBlock(index_name,position);
			return;
		}
	}
}


index_location Bnode::search(string condition,string value)
{
	string head = datamanager.getFileInfo(index_name);
	length = toint(head.substr(8,4));
	//第一个叶节点位置不存在
	if(head.substr(4,4)=="9999")
	{
		throw not_found();
	}

	if(type ==3)
	{
		int i = 0;
		for (i = 0; i < value.length(); i++) {
			if (value.at(i) == '.')
				break;
		}
		if (i == value.length())
			value += ".0";
	}
	//补齐长度
	value = value.append(length-value.size(),'0');

	int blocknum=0; //在index中的第几块
	int number=0;  //在index中具体位置
	vector<string> record_postion; //记录指针

	if (condition.size() == 0 || condition == "=")    //搜索为=时
	{
		block* blk = datamanager.readBlock(index_name, 0); //从根节点开始找
		string content = blk->data;
		while (content[0] == '1')  //为非叶节点时
		{
			int value_number = toint(content.substr(1, 3));
			int j, temp;
			for (j = 0; j < value_number; j++) {
				temp = j * (length + 4) + 16;
				if (showresult(">",content.substr(temp + 4, length),value)) {//content.substr(temp + 4, length) > value
					blk = datamanager.readBlock(index_name,
							toint(content.substr(temp, 4)));
					content = blk->data;
					break;
				}
				else if(content.substr(temp + 4, length) == value)
				{
					blk = datamanager.readBlock(index_name,toint(content.substr(temp+4+length,4)));
					content = blk->data;
					break;
				}
			}
		}

		blocknum = toint(content.substr(8, 4));
		int j, temp;
		int value_number = toint(content.substr(1, 3));
		if (condition.size() == 0)   //表明这是为insert的插入
		{
			for (j = 0; j < value_number; j++) {
				temp = j * (8 + length) + 20;
				if (content.substr(temp + 8, length) == value) //表明插入失敗，原值已存在
					throw insert_index_error();         //抛出异常
				if (showresult(">",content.substr(temp + 8, length),value)) {
					break;//content.substr(temp + 8, length) > value
				}
			}
			number = j;
		}

		else            //等值查询
		{
			for (j = 0; j < value_number; j++) {
				temp = j * (8 + length) + 20;
				if (content.substr(temp + 8, length) == value) //找到了
						{
					number = j;
					record_postion.push_back(content.substr(temp, 8));
					break;
				}
			}
			if (j == value_number) {
				throw not_found();
			}
		}
	} else       //非等值查询
	{
		int leaf_num = toint(head.substr(4, 4));
		while (leaf_num != 9999) { //
			block* blk = datamanager.readBlock(index_name, leaf_num);
			string content = blk->data;
			int j, temp;
			int value_number = toint(content.substr(1, 3));
			for (j = 0; j < value_number; j++) {
				temp = j * (8 + length) + 20;
				if (showresult(condition,
						content.substr(temp + 8, length),value)) {
					record_postion.push_back(content.substr(temp, 8));
				}
			}
			leaf_num = toint(content.substr(16, 4));        //找下一临近的叶节点
		}
	}
	index_location result(blocknum,number,record_postion);
	return result;
}

bool Bnode::showresult(string condition,string input,string value)
{
	if(condition =="<")
	{
		if(type == 1) //为char的时候
			return input<value;
		else
			return atof(input.c_str())<atof(value.c_str());
	}
	else if(condition== "<=")
	{
		if(type ==1)
			return input <= value;
		else
			return atof(input.c_str())<=atof(value.c_str());
	}
	else if(condition == ">")
	{
		if(type == 1)
			return input > value;
		else
			return atof(input.c_str())>atof(value.c_str());
	}
	else if(condition == ">=")
	{
		if(type==1)
			return input >=value;
		else
			return atof(input.c_str())>=atof(value.c_str());
	}
	else if(condition =="<>")
	{
		if(type==1)
			return input !=value;
		else
			return atof(input.c_str())!=atof(value.c_str());

	}
	else
	{
		cout <<"error in showresult" <<endl;
		throw exception();
	}
}

string Bnode::tostring(int a)
{
	string s;
	if(a<10)
	{
		s="000"+a;
	}
	else if(a<100)
	{
		s="00"+a;
	}
	else if(a<1000)
	{
		s="0"+a;
	}
	else
	{
		s=""+a;
	}
	return s;
}

int Bnode::toint(string s)
{
	int answer=0;
	for(int i=0;i<s.size();i++)
	{
		if(s[i]!='0'&&s[i-1]=='0')
		{
			for(;i<s.size();i++)
			{
				answer = answer * 10+s[i]-'0';
			}
			return answer;
		}
	}
	return 0;
}
