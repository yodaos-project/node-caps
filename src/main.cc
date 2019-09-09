#include <node_api.h>
#include "capsobject.h"
#include "common.h"

napi_value Init(napi_env env, napi_value exports) {
    if (CapsObject::Init(env, exports) != napi_ok)
        return nullptr;
    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
