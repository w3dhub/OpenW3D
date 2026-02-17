#pragma once

#include "dxvk_wrapper_compat.h"

#define __RPCNDR_H_VERSION__

#include <dxvk/rpc.h>

#define __RPC_FAR
#define __RPC_USER
#define __RPC_STUB

class IRpcStubBuffer;
class IRpcChannelBuffer;
typedef struct RPC_MESSAGE PRPC_MESSAGE;
