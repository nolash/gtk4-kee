#ifndef _KEE_RPC_H
#define _KEE_RPC_H

#include <gtk/gtk.h>

#ifndef RPC_BUFFER_SIZE
#define RPC_BUFFER_SIZE	4096
#endif

#ifndef RPC_COMMAND_SIZE
#define RPC_COMMAND_SIZE 1048576
#endif


struct kee_rpc {
	GApplication *gap;
}

#endif // _KEE_RPC_H
