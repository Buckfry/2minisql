/*
 * interpreter.h
 *
 *  Created on: 2014年11月5日
 *      Author: Even
 */

#ifndef INTERPRETER_H_
#define INTERPRETER_H_

#include "sql.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cassert>
#include <string>
#include <map>
#include <vector>


void initSQL(SQL &sql);
bool interpret(istringstream& in, SQL& sql);
void initInterpreter();

string getquery(istream& in = cin);
string getLastError(); //输出出错信息



#endif /* INTERPRETER_H_ */
