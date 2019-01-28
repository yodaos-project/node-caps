#ifndef TEST_ADDONS_NAPI_7_FACTORY_WRAP_MYOBJECT_H_
#define TEST_ADDONS_NAPI_7_FACTORY_WRAP_MYOBJECT_H_

#include <node_api.h>
#include "caps.h"
#define MAX_STR_LEN 100000
#define STR_MAX_STR_LEN "100000"

enum class ByteOrder {NBO = 0x80 /*network byte order*/, HBO /*host byte order*/};
class CapsObject {
public:
    static napi_status Init(napi_env env, napi_value exports);

    static void Destructor(napi_env env, void *nativeObject, void *finalize_hint);

    static napi_value WriteInt32(napi_env env, napi_callback_info info);

    static napi_value WriteUInt32(napi_env env, napi_callback_info info);

    static napi_value WriteInt64(napi_env env, napi_callback_info info);

    static napi_value WriteUInt64(napi_env env, napi_callback_info info);

    static napi_value WriteFloat(napi_env env, napi_callback_info info);

    static napi_value WriteDouble(napi_env env, napi_callback_info info);

    static napi_value WriteBinary(napi_env env, napi_callback_info info);

    static napi_value WriteString(napi_env env, napi_callback_info info);

    static napi_value WriteCaps(napi_env env, napi_callback_info info);

    static napi_value WriteVoid(napi_env env, napi_callback_info info);

    static napi_value ReadInt32(napi_env env, napi_callback_info info);

    static napi_value ReadUInt32(napi_env env, napi_callback_info info);

    static napi_value ReadInt64(napi_env env, napi_callback_info info);

    static napi_value ReadUInt64(napi_env env, napi_callback_info info);

    static napi_value ReadFloat(napi_env env, napi_callback_info info);

    static napi_value ReadDouble(napi_env env, napi_callback_info info);

    static napi_value ReadBinary(napi_env env, napi_callback_info info);

    static napi_value ReadString(napi_env env, napi_callback_info info);

    static napi_value ReadCaps(napi_env env, napi_callback_info info);

    static napi_value ReadVoid(napi_env env, napi_callback_info info);

    static napi_value Serialize(napi_env env, napi_callback_info info);

    static napi_value Deserialize(napi_env env, napi_callback_info info);

private:
    CapsObject();
    void SetCaps(std::shared_ptr<Caps> _caps);

    ~CapsObject();

    static napi_ref constructor;

    static napi_value New(napi_env env, napi_callback_info info);
    static void DeleteForArray(napi_env env, void* finalize_data, void* finalize_hint);
    std::shared_ptr<Caps> caps;
    napi_env env_;
    napi_ref wrapper_;
};

#endif  // TEST_ADDONS_NAPI_7_FACTORY_WRAP_MYOBJECT_H_
