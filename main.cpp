/*
 * main.cpp
 *
 *  Created on: 2014年11月14日
 *      Author: Even
 */

#include "sql.h"
#include "Interpreter.h"
#include "APIModule.h"
#include "publichead.h"
#include "BufferManager.h"
#include <iostream>
#include <sstream>
using namespace std;

/*void printSQL(const SQL &sql){
    cout << "method " << sql.method << endl;
    cout << "argc " << sql.argc << endl;
    cout << "targetName " << sql.targetName << endl;
    cout << "argv ";
    for(int i = 0;i<sql.argv.size();i++)
        cout << sql.argv[i] << " ";
    cout << endl;
    cout << "value ";
    for(int i = 0;i<sql.value.size();i++)
        cout << sql.value[i] << " ";
    cout << endl;
    cout << "result(aux) ";
    for(int i = 0;i<sql.result.size();i++)
        cout << sql.result[i] << " ";
    cout << endl;
    cout << "operators ";
    for(int i = 0;i<sql.operators.size();i++)
        cout << sql.operators[i] << " ";
    cout << endl;
    cout << "LogicOperater ";
     for(int i = 0;i<sql.logicalOperators.size();i++)
        cout << sql.logicalOperators[i] << " ";
    cout << endl;
    cout << "types ";
     for(int i = 0;i<sql.types.size();i++)
        cout << sql.types[i] << " ";
    cout << endl;
    cout << "length ";
     for(int i = 0;i<sql.length.size();i++)
        cout << sql.length[i] << " ";
    cout << endl;
    cout << "aux ";
    for(int i = 0;i<sql.aux.size();i++)
        cout << sql.aux[i] << " ";
    cout << ends;
    cout << "isUnique ";
     for(int i = 0;i<sql.isUnique.size();i++)
        cout << sql.isUnique[i] << " ";
    cout << endl;
}
*/

BufferManager datamanager;

int main()
{
    SQL sql;
    string Current_DBname;

     string str;
    initAPI();
    initInterpreter();
    str = getquery();
    istringstream ss(str);
    interpret(ss,sql);
    while(sql.method != QUIT){  //main 里判断了QUIT；
      // printSQL(sql);

        cout << getLastError() << endl;
        APIModule(sql,Current_DBname);
        cout <<"Current_DBname: " <<Current_DBname<<endl;
        str = getquery();
        if (str == "eof") {
            return 1;
        }
        istringstream ss(str);  //将字符串转化为流对象
        interpret(ss,sql);
    }
    APIModule(sql,Current_DBname);
    return 0;
}


