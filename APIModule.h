/*
 * APIModule.h
 *
 *  Created on: 2014年11月5日
 *      Author: Even
 */

#ifndef APIMODULE_H_
#define APIMODULE_H_

#include "publichead.h"
#include "sql.h"
#include <vector>
#include <string>
#include <map>

void initAPI();
void APIModule(SQL &sql,string& Current_DBname);



#endif /* APIMODULE_H_ */
