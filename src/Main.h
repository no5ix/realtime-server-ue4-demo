

#ifndef MAIN_H_
#define MAIN_H_

#include "Pre.h"
#include "profile_config.h"

#include "io_service_pool.h"
#include "Worker.h"
#include "server.h"
//using namespace bas;

typedef bas::server<bas::Worker, bas::Worker_Allocator> GameServer;
typedef bas::service_handler_pool<bas::Worker, bas::Worker_Allocator> server_handler_pool;

#ifdef WIN32
#include <windows.h>
bool signal_handle(DWORD dwCtrlType);
#else
bool signal_handle();
#endif

int load_conf(bool reload = false);
int set_dbs();
int parse_db_conf(const char * path, vector<string> * results);

#endif /* MAIN_H_ */
