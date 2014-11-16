/*
b * interpreter.cpp
 *
 *  Created on: 2014年11月5日
 *      Author: Even
 */


#include "Interpreter.h"

#include "sql.h"


#define STRINGEND ";"

typedef void (*function)(istringstream& in, SQL& sql);  //定义function 是一个函数指针类型，指向一个函数，它的参数列表为istringstream& in, SQL& sql，返回类型为void

map < string, function > allFunctions;
map < string, function > createFunctions;
map < string, function > dropFunctions;
map < string, function > :: iterator it;

map < string, string > operators;
map < string, string> :: iterator itOperator;
map < string, string > logicalOperators;
map < string, string > :: iterator itLogicalOperator;
map < string, int > types;
map < string, int > :: iterator itType;

string unexpectedError;

void initSQL(SQL &sql) {   //初始化SQL结构
	sql.method = NOP;
	sql.argc = 0;
    sql.targetName.clear();
    sql.length.clear();
    sql.argv.clear();
    sql.value.clear();
    sql.result.clear();
	sql.types.clear();
    sql.operators.clear();
    sql.logicalOperators.clear();
    sql.isUnique.clear();
	sql.aux.clear();
}

bool interpret(istringstream& in, SQL& sql) {
	string temp;
	bool flag = true;  //判断是否出错
	unexpectedError.clear();
	try{
		if (in.str().find(';') == string::npos) {   //没有以;结尾
			throw ("The query must end with \';\' !");
		}
		initSQL(sql);  //确保每次SQL已经初始化
		in >> temp;  //取in的第一个字符串
		if ((it = allFunctions.find(temp)) == allFunctions.end())  //没有找到对应的操作
			throw ("There isn't any query start with \"" + temp + "\" !");
		it->second(in, sql);  //找到着进行第一步检验，并初始化sql.method
		if (sql.method != SELECTALL && sql.method != SELECTWHERE && sql.method != DELETEALL && sql.method != DELETEWHERE) {
			in >> temp;
			if (temp != STRINGEND)
				throw ("Expected \';\' after a query !");
		}
	}
	catch (const char* e) {
	    unexpectedError = e;
	    flag = false;
	    initSQL(sql);
	    }
    catch (string& e) {
        unexpectedError = e;
         flag = false;
          initSQL(sql);
         }
    return flag;
}

void interpretCreates(istringstream& in, SQL& sql) {   //检验Create 语句
	string temp;
	in >> temp;   //读入第二个字符
	if ((it = createFunctions.find(temp)) == createFunctions.end())
		throw ("There's no query to create a  \"" + temp + "\" !");
	it->second(in, sql);
}

void interpretDrops(istringstream& in, SQL& sql) {  //检验Drop语句
	string temp;
	in >> temp;
	if ((it = dropFunctions.find(temp)) == dropFunctions.end())
		throw ("There's no query to drop a \"" + temp + "\" !");
	it->second(in, sql);
}

void interpretWhere(istringstream& in, SQL& sql) {
	string strName, strOp, strValue, strLogicalOp;
	int nArgc = 0;  //记录参数个数
	while (true) {
		in >> strName;  //读入第五个字符串属性

		if (strName == STRINGEND)
			throw ("Comparative value or variable missing before comparison operator !");
		in >> strOp;  //读入比较符
		if (strOp == STRINGEND)
			throw ("Comparison operator missing after \"" + strName + "\" !");
		if ((itOperator = operators.find(strOp)) == operators.end())  //检测比较符是否存在
			throw ("Can't resolve \"" + strOp + "\" as a comparison operator !");
		in >> strValue;  //读入比较值
		if (strValue == STRINGEND)
			throw ("Comparative value or variable missing after \"" + strOp + "\" !");

		sql.argv.push_back(strName);
		sql.operators.push_back(itOperator->second);
		sql.value.push_back(strValue);

		++nArgc;
		in >> strLogicalOp;  //读入逻辑比较值
		if (strLogicalOp == STRINGEND) {
			break;
		}
		if ((itLogicalOperator = logicalOperators.find(strLogicalOp)) == logicalOperators.end())  //检测是否是and 或 or
			throw ("Can't resolve \"" + strLogicalOp + "\" as a logical operator !");
		sql.logicalOperators.push_back(itLogicalOperator->second);
	}
	sql.argc = nArgc;
}

void interpretDelete(istringstream& in, SQL& sql) {  //检验delete语句
	string strName;
	in >> strName;   //读入第二个字符串
	if (strName != "from")
		throw ("\"From\" missing after \"delete\" !");
	in >> strName;   //读入第三个字符串
	if (strName == STRINGEND)
		throw ("Table name missing after \"from\" !");
	sql.targetName = strName;
	in >> strName;  //读入第四个字符串
	if (strName == STRINGEND) {
		sql.method = DELETEALL;
	} else if (strName == "where") {
		sql.method = DELETEWHERE;
		interpretWhere(in, sql);
	} else
		throw ("Syntax error after table name \"" + sql.targetName + "\" !");
}

void interpretExecfile(istringstream& in, SQL& sql) {   //检验并解析execfile
	string strName;
	sql.method = EXECFILE;
	in >> strName;  //读入第二个字符串
	if (strName == STRINGEND)
		throw ("File name missing after \"execfile\" !");
	sql.targetName = strName;
}

void interpretHelp(istringstream& in, SQL& sql) {   //检验并解析help
	sql.method = HELP;
}

void interpretQuitDatabase(istringstream& in, SQL& sql) {   //检验并解析quit
	sql.method = QUIT;
}

void interpretUseDatabase(istringstream& in, SQL& sql) {   //检验并解析use
	string strName;
	sql.method = USEDATABASE;
	in >> strName;
	if (strName == STRINGEND)
		throw ("Database missing after \"use\" !");
	sql.targetName = strName;
}

void interpretInsert(istringstream& in, SQL& sql) {   //检测insert语句
	string strName, strTemp;
	string strChar;
	int nArgc = 0;
	sql.method = INSERT;
	in >> strTemp;   //读入第二个字符串
	if (strTemp != "into")
		throw ("\"Into\" missing after \"insert\" !");
	in >> strName;  //读入第三个字符串
	if (strName == STRINGEND)
		throw ("Table name missing after \"into\" !");
	sql.targetName = strName;
	in >> strTemp;  //读入第四个字符串
	if (strTemp != "values")
		throw ("\"Values\" missing after \"" + strName +"\" !");
	in >> strChar;  //读入第五个字符串
	if (strChar != "(")
		throw ("\'(\' missing after \"values\" !");
	in >> strName; //读入第六个字符串
	while (strName != ")") {
		if (strName == STRINGEND)
			throw ("Syntax error in \"values\" !");
		sql.argv.push_back(strName);
		++nArgc;
		in >> strChar;
		if (strChar == ")") {
			strName = strChar;
		} else if (strChar != "," )
			throw ("\',\' missing after value \"" + strName + "\" !");
		else {
			in >> strName;
		}
	}
	sql.argc = nArgc;
}



void interpretSelect(istringstream& in, SQL& sql) {
	string strName;
	string strChar;
	in >> strName;  //读入第二个字符串
	if (strName == "*") {
		in >> strName;;   // 读入第三个字符串
	} else {
		while (strName != "from") {
			if (strName == STRINGEND)
				throw ("Syntax error after \"select\" !");
			sql.result.push_back(strName);
			in >> strChar;
			if (strChar == "from") {
				strName = strChar;
			} else if (strChar != "," )
				throw ("\',\' missing after value \"" + strName + "\" !");
			else {
				in >> strName;
			}
		}
	}
	if (strName != "from")
		throw ("\"From\" missing after column names !");
	in >> strName;
	if (strName == STRINGEND)
		throw ("Table name missing after \"from\" !");
	sql.targetName = strName;
	in >> strName;
	if (strName == STRINGEND) {
		sql.method = SELECTALL;
	} else if (strName == "where") {
		sql.method = SELECTWHERE;
		interpretWhere(in, sql);
	} else
		throw ("Syntax error after table name !");
}



void interpretCreateDatabase(istringstream& in, SQL& sql) {  //检测解析createdatabase语句
	string strName;
	sql.method = CREATEDATABASE;
	sql.argc = 0;
	in >> strName;
	if (strName == STRINGEND)
		throw ("Database name missing in query \"create database\" !");
	if (strName.find('.') != string::npos)
		throw ("Database name can't contain \'.\' !");
	sql.targetName = strName;
}

void interpretCreateTable(istringstream& in, SQL& sql) {
	string strName, strType,strTemp;
	string strChar;

	int nArgc = 0, nTemp;
	map < string, int > keys;
	sql.method = CREATETABLE;
	in >> strName;  //读入第三个字符串
	if (strName == STRINGEND)
		throw ("Syntax error after \"table\" !");
	if (strName.find('.') != string::npos)  //tablename中不能包含.
		throw ("Table name can't contain \'.\' !");
	sql.targetName = strName;
	in >> strChar;
	if (strChar != "(")  //找到（
		throw ("\'(\' missing after table name !");
	in >> strName >> strType;
	while (strName != "primary" || strType != "key") {
		if (strName == STRINGEND || (itType = types.find(strType)) == types.end()) //检测type是否合法
			throw ("Syntax error in defining columns !");
		sql.argv.push_back(strName);
		keys.insert(pair < string, int> (strName, nArgc));  //保存属性是第几个
		sql.types.push_back(itType->second);
		if (itType->second ==1) {  //是有参数的类型
			in >> strChar;  //读入（
			if (strChar != "(")
				throw ("\'(\' missing after data type \"" + strType + "\" !");
			in >> nTemp;  //读入字符串长度n
			if (!in)
				throw ("Syntax error in defining data type of column \"" + strName + "\", an integer is needed !");

			sql.length.push_back(nTemp);
			in >> strChar;  //读入）
			if (strChar != ")")
				throw ("\')\' missing in defining data type \"" + strType + "\" !");
		}
		else {
            sql.length.push_back(255);
		}
		in >> strTemp;
		if (strTemp == "unique") {   //检测属性是否为unique
			sql.isUnique.push_back(2);
			in >> strTemp;
		} else {
			sql.isUnique.push_back(0);
		}
		++nArgc;
		if (strTemp != ",")
			throw ("\',\' missing after definition of column \"" + strName + "\" !");
		in >> strName >> strType; //读入下一个
	}
	sql.argc = nArgc;
	in >> strChar; //读到primary key之后
	if (strChar != "(")
		throw ("'(' missing after \"primary key\" !");
	in >> strName;  //读入primary key 里的属性
	while (strName != ")") {
		if (strName == STRINGEND)
			throw ("Syntax error in primary keys !");
        if(keys.find(strName)!=keys.end()){
		nTemp = keys.find(strName)->second;  //找到primary key 所在第几个
		sql.isUnique[nTemp] = 1;  //primary key 的属性一定是unique
        }
        else {
            throw ("primary key does not exsist !");
        }
		in >> strChar;
		if (strChar == ")") {
			strName = strChar;
		} else if (strChar != "," )  //primary key 可以有好几个组成
			throw ("\',\' missing after primary key \"" + strName + "\" !");
		else {
			in >> strName;
		}
	}
	in >> strChar;
	if (strChar != ")")
		throw ("\')\' missing after table column specification !");
}

void interpretCreateIndex(istringstream& in, SQL& sql) {
	string strName, strTemp;
	string strChar;
	sql.method = CREATEINDEX;
	in >> strName; //读入第三个字符串
	if (strName == STRINGEND)
		throw ("Index name missing after \"index\" !");
	if (strName.find('.') != string::npos)
		throw ("Index name can't contain \'.\' !");
	sql.targetName = strName;
	in >> strTemp;
	if (strTemp != "on")
		throw ("\"On\" missing after index name !");
	in >> strName;
	if (strName == STRINGEND)
		throw ("Table name missing after \"on\" !");
	sql.aux.push_back(strName);
	in >> strChar; //读入（
	if (strChar != "(")
		throw ("'(' missing after table name !");
	in >> strName;  //读入属性名
	if (strName == STRINGEND)
		throw ("Column name missing after \'(\' !");
	sql.argv.push_back(strName);
	in >> strChar;
	if (strChar != ")") {
		throw ("\',\' missing after column name \"" + strName + "\" !");
	}
}

void interpretDropDatabase(istringstream& in, SQL& sql) {
	string strName;
	sql.method = DROPDATABASE;
	in >> strName;
	if (strName == STRINGEND)
		throw ("Database name missing after \"database\" !");
	sql.targetName = strName;
}

void interpretDropTable(istringstream& in, SQL& sql) {
	string strName;
	sql.method = DROPTABLE;
	in >> strName;
	if (strName == STRINGEND)
		throw ("Table name missing after \"table\" !");
	sql.targetName = strName;
}

void interpretDropIndex(istringstream& in, SQL& sql) {
	string strName, strChar;
	sql.method = DROPINDEX;
	in >> strName;  //读入indexname
	if (strName == STRINGEND)
		throw ("Index name missing after \"index\" !");
	sql.targetName = strName;
}


void initInterpreter() {
	allFunctions.insert(pair < string, function > ("create", interpretCreates));
	allFunctions.insert(pair < string, function > ("drop", interpretDrops));
	allFunctions.insert(pair < string, function >("insert", interpretInsert));
	allFunctions.insert(pair < string, function > ("delete", interpretDelete));
	allFunctions.insert(pair < string, function >("select", interpretSelect));
	allFunctions.insert(pair < string, function > ("execfile", interpretExecfile));
	allFunctions.insert(pair < string, function > ("help", interpretHelp));
	allFunctions.insert(pair < string, function > ("quit", interpretQuitDatabase));
	allFunctions.insert(pair < string, function > ("use", interpretUseDatabase));

	createFunctions.insert(pair < string, function > ("database", interpretCreateDatabase));
	createFunctions.insert(pair < string, function > ("table", interpretCreateTable));
	createFunctions.insert(pair < string, function > ("index", interpretCreateIndex));

	dropFunctions.insert(pair < string, function > ("database", interpretDropDatabase));
	dropFunctions.insert(pair < string, function > ("table", interpretDropTable));
	dropFunctions.insert(pair < string, function > ("index", interpretDropIndex));


	operators.insert(pair < string, string> ("=", "="));
	operators.insert(pair < string, string> ("<>", "<>"));
	operators.insert(pair < string, string> (">", ">"));
	operators.insert(pair < string, string> ("<", "<"));
	operators.insert(pair < string, string> (">=", ">="));
	operators.insert(pair < string, string> ("<=", "<="));

    logicalOperators.insert(pair < string, string > ("or", "or"));
	logicalOperators.insert(pair < string, string > ("and", "and"));

	types.insert(pair < string, int > ("char", 1));
    types.insert(pair < string, int > ("int", 2));
    types.insert(pair < string, int > ("float", 3));


}

void reconstruct(string& str) {
	int nIndex;
	int i;



	struct jointSymbol {
		jointSymbol(string _old) : symbolOld(_old), symbolNew(" " + _old + " ") {
		}
		jointSymbol(string _old, string _new) : symbolOld(_old), symbolNew(_new) {
		}
		bool isValid() {
			return (symbolOld != "~");
		}
		    string symbolOld;
			string symbolNew;
	}

	replacedSymbol[] = {
		string(","), string("("), string(")"), string(";"), string(">"), string("<"), string("="), jointSymbol("<  >", "<>"), jointSymbol(">  =", ">="), jointSymbol("<  =", "<="), string("~"), string("."),
	};

	// 在符号两边加空格规范化命令格式
	i = -1;
	while (replacedSymbol[++i].isValid()) {
		jointSymbol mTemp(replacedSymbol[i]);
		for (nIndex = str.find(mTemp.symbolOld, 0); nIndex != str.npos;) {
			str.replace(nIndex, mTemp.symbolOld.length(), mTemp.symbolNew);
			nIndex += 2;
			nIndex = str.find(mTemp.symbolOld, nIndex);
		}
	}

	// 移除多余的空格
	for (nIndex = str.find("  ", 0); nIndex != str.npos;)
	{
		str.replace(nIndex, 2, " ");
		nIndex = str.find("  ", nIndex);
	}
	// trim
	str = str.substr(1, str.size() - 2);

	// 转换成小写格式
	transform(str.begin(), str.end(), str.begin(), (int (*)(int))tolower);
	if (str[0] == 'd' && str[1] == 'r' && str[5] == 'i') {
		jointSymbol mTemp(replacedSymbol[i + 1]);
		for (nIndex = str.find(mTemp.symbolOld, 0); nIndex != str.npos;) {
			str.replace(nIndex, mTemp.symbolOld.length(), mTemp.symbolNew);
			nIndex += 2;
			nIndex = str.find(mTemp.symbolOld, nIndex);
		}
	}
}

string getquery(istream& in) {
	string line("");
	string strWord;
	do{
		do{
			if(in.eof()){  //输入流结束
				return "eof";
			}
			in >> strWord;
		} while (strWord.empty());
		line += " " + strWord;
	} while (strWord[strWord.length ()- 1] != ';');
	reconstruct(line);
	return line;
}

string getLastError() {
	return unexpectedError;
}
