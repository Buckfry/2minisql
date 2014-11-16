/*
 * Error.cpp
 *
 *  Created on: 2014年11月11日
 *      Author: yunshen
 */
#include "Error.h"

Error::Error(int code,string text)
{
	message = text;
    switch (code){
      	case 1:data="Error 1 : Can't create database : "+text;break;
      	case 2:data="Error 2 : Can't create table : "+text;break;
      	case 3:data="Error 3 : Can't create index : "+text;break;
      	case 4:data="Error 4 : Can't drop database : "+text;break;
      	case 5:data="Error 5 : Can't drop table : "+text;break;
      	case 6:data="Error 6 : Can't drop index : "+text;break;
      	case 7:data="Error 7 : Can't insert data : "+text;break;
      	case 8:data="Error 8 : Can't select data : "+text;break;
      	case 9:data="Error 9 : Can't delete data : "+text;break;
      	case 10:data="Error 10 : Can't use database : "+text;break;
      	case 11:data="Error 11 : Can't quit MiniSQL : "+text;break;
      	case 12:data="Error 12 : Can't show database : "+text;break;
      	case 13:data="Error 13 : Can't execute file : "+text;break;
      	case 14:data="Error 14 : Can't find helpinfo : "+text;break;
      	default:data="Error 15 : Unknown error : "+text;break;
    }
}

string Error::getMessage()
{
	return message;
}

void Error::printError() const throw(){
	cout<<data<<endl;
}



