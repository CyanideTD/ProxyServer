#ifndef _STD_HEADER_H
#define _STD_HEADER_H

#include <iostream>
#include <stdlib.h>
#include <hash_map>
#include <string.h>
#include <sys/types.h>       
#include <map>
#include <base/common/wtsetypedef.h>
#include <svrpublib/BaseProtocol.h>
#include "jdbc/mysql_driver.h"    
#include "jdbc/mysql_connection.h"
#include <jdbc/cppconn/driver.h> 
#include <jdbc/cppconn/prepared_statement.h>
#include <pthread.h>
#include <errno.h>
#include <limits>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>
#include <semaphore.h>
#include <queue>

#define USER "root"
#define URL  "localhost"
#define PASS "Cqh19930423"
#define DATABASE "TEST"

enum EDBPROXYKeySet
{
	//////////////////////////
	// GLOBAL KEY
	EN_GLOBAL_KEY__BEGIN = 0,
	EN_GLOBAL_KEY__RES_CODE,
	EN_GLOBAL_KEY__RES_COST_TIME,
	EN_GLOBAL_KEY__RES_TOTAL_RES_NUM,
	EN_GLOBAL_KEY__RES_CUR_RES_NUM,
	EN_GLOBAL_KEY__RES_BUF,

	// hu <-> hs
	EN_KEY_HU2HS__BEGIN = 5000,
	EN_KEY_HU2HS__REQ_NUM,
	EN_KEY_HU2HS__REQ_BUF,

	EN_KEY_HS2HU__BEGIN = 5100,
	EN_KEY_HS2HU__RES_DB_TYPE,
	EN_KEY_HS2HU__RES_TBL_TYPE,
	EN_KEY_HS2HU__RES_INDEX_OPENTYPE,
	EN_KEY_HS2HU__RES_COMMAND,

	// hu -> lock
	EN_KEY_HU2LOCK__BEGIN = 6000,
	EN_KEY_HU2LOCK__REQ_KEY_NUM,
	EN_KEY_HU2LOCK__REQ_KEY_LIST,
	EN_KEY_HU2LOCK__REQ_TIMEOUT_US,

	// lock -> hu
	EN_KEY_LOCK2HU__BEGIN = 6100,

	EN_KEY_RESOURCE = 6200,
};

enum RelationDataServiceType
{
	// hu <---> hs
	EN_HU2HS__BEGIN = 3000,
	EN_SERVICE_TYPE_HU2HS__SEARCH_REQ,
	EN_SERVICE_TYPE_HU2HS__UPDT_REQ,
	EN_SERVICE_TYPE_RESOURCE_GET,
	EN_SERVICE_TYPE_RESOURCE_COST,

	EN_HS2HU__BEGIN = 3100,
	EN_SERVICE_TYPE_HS2HU__SEARCH_RSP,
	EN_SERVICE_TYPE_HS2HU__UPDT_RSP,

	// hu <---> lock
	EN_HU2LOCK__BEGIN = 4000,
	EN_SERVICE_TYPE_HU2LOCK__GET_REQ,
	EN_SERVICE_TYPE_HU2LOCK__RELEASE_REQ,

	EN_LOCK2HU__BEGIN = 4100,
	EN_SERVICE_TYPE_LOCK2HU__GET_RSP,
	EN_SERVICE_TYPE_LOCK2HU__RELEASE_RSP,
};



#endif