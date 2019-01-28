#include <string.h>
#include <iostream>
#include "capsobject.h"
#include "common.h"
/*
#define CAPS_SUCCESS 0
#define CAPS_ERR_INVAL -1  // 参数非法
#define CAPS_ERR_CORRUPTED -2  // parse数据格式不正确
#define CAPS_ERR_VERSION_UNSUPP -3  // 不支持parse数据的版本(数据版本高于反序列化工具版本)
#define CAPS_ERR_WRONLY -4  // read的caps对象不可读(通过caps_create创建的caps对象只写)
#define CAPS_ERR_RDONLY -5  // write的caps对象不可写(通过caps_parse创建的caps对象只读)
#define CAPS_ERR_INCORRECT_TYPE -6  // read类型不匹配
#define CAPS_ERR_EOO -7  // 没有更多的成员变量了，read结束(End of Object)

 */

static const char *const ErrorMessage[] =
        {"success", "invalid data",
         "corruped", "version unsupported",
         "this caps object is writeonly",
         "this caps object is readonly",
         "invalid argument", "no more data to read"};
napi_ref CapsObject::constructor;

CapsObject::CapsObject() : env_(nullptr), wrapper_(nullptr) {
    caps = Caps::new_instance();
}


void CapsObject::SetCaps(std::shared_ptr<Caps> _caps) {
    caps = _caps;
}

CapsObject::~CapsObject() {
    napi_delete_reference(env_, wrapper_);
}

void CapsObject::Destructor(napi_env env,
                            void *nativeObject,
                            void * /*finalize_hint*/) {
    CapsObject *obj = static_cast<CapsObject *>(nativeObject);
    delete obj;
}


napi_status CapsObject::Init(napi_env env, napi_value exports) {

    napi_status status;
    napi_value nbo, hbo;
    status = napi_create_uint32(env, 0x80, &nbo);
    if (status != napi_ok)
        return status;
    status = napi_create_uint32(env, 0x81, &hbo);
    if (status != napi_ok)
        return status;
    napi_property_descriptor properties[] = {
            DECLARE_NAPI_PROPERTY("writeInt32", WriteInt32),
            DECLARE_NAPI_PROPERTY("writeUInt32", WriteUInt32),
            DECLARE_NAPI_PROPERTY("writeInt64", WriteInt64),
            DECLARE_NAPI_PROPERTY("writeUInt64", WriteUInt64),
            DECLARE_NAPI_PROPERTY("writeFloat", WriteFloat),
            DECLARE_NAPI_PROPERTY("writeDouble", WriteDouble),
            DECLARE_NAPI_PROPERTY("writeBinary", WriteBinary),
            DECLARE_NAPI_PROPERTY("writeString", WriteString),
            DECLARE_NAPI_PROPERTY("writeCaps", WriteCaps),
            DECLARE_NAPI_PROPERTY("writeVoid", WriteVoid),
            DECLARE_NAPI_PROPERTY("readInt32", ReadInt32),
            DECLARE_NAPI_PROPERTY("readUInt32", ReadUInt32),
            DECLARE_NAPI_PROPERTY("readUInt64", ReadUInt64),
            DECLARE_NAPI_PROPERTY("readInt64", ReadInt64),
            DECLARE_NAPI_PROPERTY("readFloat", ReadFloat),
            DECLARE_NAPI_PROPERTY("readDouble", ReadDouble),
            DECLARE_NAPI_PROPERTY("readBinary", ReadBinary),
            DECLARE_NAPI_PROPERTY("readString", ReadString),
            DECLARE_NAPI_PROPERTY("readCaps", ReadCaps),
            DECLARE_NAPI_PROPERTY("readVoid", ReadVoid),
            DECLARE_NAPI_PROPERTY("serialize", Serialize),
            DECLARE_NAPI_PROPERTY("deserialize", Deserialize),
            {"NetWorkByteOrder", nullptr, nullptr, nullptr, nullptr, nbo, napi_enumerable, nullptr},
            {"HostByteOrder",  nullptr, nullptr, nullptr, nullptr, hbo, napi_enumerable, nullptr},
            {"NBO", nullptr, nullptr, nullptr, nullptr, nbo, napi_enumerable, nullptr},
            {"HBO", nullptr, nullptr, nullptr, nullptr, hbo, napi_enumerable, nullptr}
    };

    napi_value cons;
    status = napi_define_class(
            env, "Caps", NAPI_AUTO_LENGTH, New, nullptr,
            sizeof(properties) / sizeof(napi_property_descriptor), properties, &cons);

    if (status != napi_ok)
        return status;

    status = napi_create_reference(env, cons, 1, &constructor);
    if (status != napi_ok)
        return status;


    status = napi_set_named_property(env, exports, "Caps", cons);
    if (status != napi_ok)
        return status;

    return napi_ok;
}

napi_value CapsObject::New(napi_env env, napi_callback_info info) {
    napi_value _this;
    //get call stack
    size_t argc = 1;
    napi_value args[1];
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, nullptr));
    CapsObject *obj = new CapsObject();
    if (argc == 1) {
        void *obj_input;
        NAPI_CALL(env, napi_unwrap(env, args[0], reinterpret_cast<void**>(&obj_input)));
        obj->SetCaps(*((std::shared_ptr<Caps>*)obj_input));
    }
    obj->env_ = env;
    NAPI_CALL(env, napi_wrap(env,
                             _this,
                             obj,
                             CapsObject::Destructor,
                             nullptr, /* finalize_hint */
                             &obj->wrapper_));

    return _this;
}

napi_value CapsObject::WriteInt32(napi_env env, napi_callback_info info) {
    napi_value _this;

    size_t argc = 1;
    napi_value args[1];
    //get call stack
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, nullptr));

    NAPI_ASSERT(env, argc > 0, "Wrong number of arguments");
    if (argc == 0) {
        NAPI_CALL(env, napi_throw_type_error(env, "write int32 error", "Wrong number of arguments, Expects 1"));
        return nullptr;
    }

    //check arguments type & count
    napi_valuetype valuetype;
    NAPI_CALL(env, napi_typeof(env, args[0], &valuetype));

    NAPI_ASSERT(env, valuetype == napi_number, "Wrong type of arguments, Expects number");
    if (valuetype != napi_number)
        NAPI_CALL(env, napi_throw_type_error(env, "write int32 error", "Wrong type of arguments, Expects number"));

    //get argument
    int32_t v;
    NAPI_CALL(env, napi_get_value_int32(env, args[0], &v));

    //get obj
    CapsObject *obj;
    NAPI_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void **>(&obj)));

    //do write
    int32_t rst = obj->caps->write(v);
    if (rst != CAPS_SUCCESS) {
        if (rst < 0 && rst >= CAPS_ERR_EOO)
            NAPI_CALL(env, napi_throw_error(env, "write int32 error", ErrorMessage[-rst]));
        else
            NAPI_CALL(env, napi_throw_error(env, "write int32 error", "Unknown Error"));
    }
    return nullptr;
}

napi_value CapsObject::WriteUInt32(napi_env env, napi_callback_info info) {
    napi_value _this;

    size_t argc = 1;
    napi_value args[1];
    //get call stack
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, nullptr));

    NAPI_ASSERT(env, argc > 0, "Wrong number of arguments");

    if (argc == 0)
        NAPI_CALL(env, napi_throw_type_error(env, "write uint32 error", "Wrong number of arguments, Expects 1"));
    //check arguments type & count
    napi_valuetype valuetype;
    NAPI_CALL(env, napi_typeof(env, args[0], &valuetype));

    NAPI_ASSERT(env, valuetype == napi_number, "Wrong type of arguments, Expects number");
    if (valuetype != napi_number)
        NAPI_CALL(env, napi_throw_type_error(env, "write uint32 error", "Wrong type of arguments, Expects number"));

    //get argument
    uint32_t v;
    NAPI_CALL(env, napi_get_value_uint32(env, args[0], &v));

    //get obj
    CapsObject *obj;
    NAPI_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void **>(&obj)));

    //do write
    int32_t rst = obj->caps->write(v);
    if (rst != CAPS_SUCCESS) {
        if (rst < 0 && rst >= CAPS_ERR_EOO)
            NAPI_CALL(env, napi_throw_error(env, "write uint32 error", ErrorMessage[-rst]));
        else
            NAPI_CALL(env, napi_throw_error(env, "write uint32 error", "Unknown Error"));
    }
    return nullptr;
}

napi_value CapsObject::WriteInt64(napi_env env, napi_callback_info info) {
    napi_value _this;

    size_t argc = 1;
    napi_value args[1];
    //get call stack
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, nullptr));

    NAPI_ASSERT(env, argc > 0, "Wrong number of arguments");

    if (argc == 0)
        NAPI_CALL(env, napi_throw_type_error(env, "write int64 error", "Wrong number of arguments, Expects 1"));
    //check arguments type & count
    napi_valuetype valuetype;
    NAPI_CALL(env, napi_typeof(env, args[0], &valuetype));

    NAPI_ASSERT(env, valuetype == napi_number, "Wrong type of arguments, Expects number");
    if (valuetype != napi_number)
        NAPI_CALL(env, napi_throw_type_error(env, "write int64 error", "Wrong type of arguments, Expects number"));

    //get argument
    int64_t v;
    NAPI_CALL(env, napi_get_value_int64(env, args[0], &v));

    //get obj
    CapsObject *obj;
    NAPI_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void **>(&obj)));

    //do write
    int32_t rst = obj->caps->write(v);
    if (rst != CAPS_SUCCESS) {
        if (rst < 0 && rst >= CAPS_ERR_EOO)
            NAPI_CALL(env, napi_throw_error(env, "write int64 error", ErrorMessage[-rst]));
        else
            NAPI_CALL(env, napi_throw_error(env, "write int64 error", "Unknown Error"));
    }
    return nullptr;
}


napi_value CapsObject::WriteUInt64(napi_env env, napi_callback_info info) {
  napi_value _this;

  size_t argc = 1;
  napi_value args[1];
  //get call stack
  NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, nullptr));

  NAPI_ASSERT(env, argc > 0, "Wrong number of arguments");

  if (argc == 0)
    NAPI_CALL(env, napi_throw_type_error(env, "write int64 error", "Wrong number of arguments, Expects 1"));
  //check arguments type & count
  napi_valuetype valuetype;
  NAPI_CALL(env, napi_typeof(env, args[0], &valuetype));

  NAPI_ASSERT(env, valuetype == napi_number, "Wrong type of arguments, Expects number");
  if (valuetype != napi_number)
    NAPI_CALL(env, napi_throw_type_error(env, "write int64 error", "Wrong type of arguments, Expects number"));

  //get argument
  double v;
  NAPI_CALL(env, napi_get_value_double(env, args[0], &v));

  //get obj
  CapsObject* obj;
  NAPI_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&obj)));

  //do write
  int32_t rst = obj->caps->write((uint64_t)v);
  if (rst != CAPS_SUCCESS)
  {
    if (rst < 0 && rst >= CAPS_ERR_EOO)
      NAPI_CALL(env, napi_throw_error(env, "write int64 error", ErrorMessage[-rst]));
    else
      NAPI_CALL(env, napi_throw_error(env, "write int64 error", "Unknown Error"));
  }
  return nullptr;
}

napi_value CapsObject::WriteFloat(napi_env env, napi_callback_info info) {
    napi_value _this;

    size_t argc = 1;
    napi_value args[1];
    //get call stack
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, nullptr));

    NAPI_ASSERT(env, argc > 0, "Wrong number of arguments");

    if (argc == 0)
        NAPI_CALL(env, napi_throw_type_error(env, "write float error", "Wrong number of arguments, Expects 1"));
    //check arguments type & count
    napi_valuetype valuetype;
    NAPI_CALL(env, napi_typeof(env, args[0], &valuetype));

    NAPI_ASSERT(env, valuetype == napi_number, "Wrong type of arguments, Expects number");
    if (valuetype != napi_number)
        NAPI_CALL(env, napi_throw_type_error(env, "write float error", "Wrong type of arguments, Expects number"));

    //get argument
    double v;
    NAPI_CALL(env, napi_get_value_double(env, args[0], &v));
    float f = static_cast<float>(v);
    //get obj
    CapsObject *obj;
    NAPI_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void **>(&obj)));

    //do write
    int32_t rst = obj->caps->write(f);
    if (rst != CAPS_SUCCESS) {
        if (rst < 0 && rst >= CAPS_ERR_EOO)
            NAPI_CALL(env, napi_throw_error(env, "write float error", ErrorMessage[-rst]));
        else
            NAPI_CALL(env, napi_throw_error(env, "write float error", "Unknown Error"));
    }
    return nullptr;
}

napi_value CapsObject::WriteDouble(napi_env env, napi_callback_info info) {
    napi_value _this;

    size_t argc = 1;
    napi_value args[1];
    //get call stack
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, nullptr));

    NAPI_ASSERT(env, argc > 0, "Wrong number of arguments");

    if (argc == 0)
        NAPI_CALL(env, napi_throw_type_error(env, "write double error", "Wrong number of arguments, Expects 1"));
    //check arguments type & count
    napi_valuetype valuetype;
    NAPI_CALL(env, napi_typeof(env, args[0], &valuetype));

    NAPI_ASSERT(env, valuetype == napi_number, "Wrong type of arguments, Expects number");
    if (valuetype != napi_number)
        NAPI_CALL(env, napi_throw_type_error(env, "write double error", "Wrong type of arguments, Expects number"));

    //get argument
    double v;
    NAPI_CALL(env, napi_get_value_double(env, args[0], &v));
    //get obj
    CapsObject *obj;
    NAPI_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void **>(&obj)));

    //do write
    int32_t rst = obj->caps->write(v);
    if (rst != CAPS_SUCCESS) {
        if (rst < 0 && rst >= CAPS_ERR_EOO)
            NAPI_CALL(env, napi_throw_error(env, "write double error", ErrorMessage[-rst]));
        else
            NAPI_CALL(env, napi_throw_error(env, "write double error", "Unknown Error"));
    }
    return nullptr;
}

napi_value CapsObject::WriteBinary(napi_env env, napi_callback_info info) {
    napi_value _this;

    size_t argc = 1;
    napi_value args[1];
    //get call stack
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, nullptr));

    NAPI_ASSERT(env, argc > 0, "Wrong number of arguments");

    if (argc == 0)
        NAPI_CALL(env, napi_throw_type_error(env, "write binary error", "Wrong number of arguments, Expects 1"));
    //get obj
    CapsObject *obj;
    NAPI_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void **>(&obj)));

    //check arguments type & count
    bool istypedarray;
    NAPI_CALL(env, napi_is_typedarray(env, args[0], &istypedarray));

    int32_t w_rst;
    if (istypedarray) {
        //get argument Uint8Array
        napi_typedarray_type type;
        napi_value in_array_buffer;
        size_t byte_offset;
        size_t length;
        void *datap;
        NAPI_CALL(env, napi_get_typedarray_info(
                env, args[0], &type, &length, &datap, &in_array_buffer, &byte_offset));
        if (type != napi_uint8_array)
            NAPI_CALL(env,
                      napi_throw_type_error(env, "argument type error", "Wrong type of arguments, Expects Uint8Array"));

        //do write
        w_rst = obj->caps->write(datap, (uint32_t) length);
        if (w_rst != CAPS_SUCCESS) {
            if (w_rst < 0 && w_rst >= CAPS_ERR_EOO)
                NAPI_CALL(env, napi_throw_error(env, "write binary error", ErrorMessage[-w_rst]));
            else
                NAPI_CALL(env, napi_throw_error(env, "write binary error", "Unknown Error"));
        }
    } else {
        NAPI_CALL(env, napi_throw_type_error(env, "write binary error", "Wrong type of arguments, Expects Uint8Array"));
    }
    return nullptr;
}

napi_value CapsObject::WriteString(napi_env env, napi_callback_info info) {
    napi_value _this;

    size_t argc = 1;
    napi_value args[1];
    //get call stack
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, nullptr));

    NAPI_ASSERT(env, argc > 0, "Wrong number of arguments");

    if (argc == 0)
        NAPI_CALL(env, napi_throw_type_error(env, "write string error", "Wrong number of arguments, Expects 1"));
    //get obj
    CapsObject *obj;
    NAPI_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void **>(&obj)));

    //check arguments type & count
    napi_valuetype valuetype;
    NAPI_CALL(env, napi_typeof(env, args[0], &valuetype));

    if (valuetype == napi_string) {
        size_t strlen = 0;
        NAPI_CALL(env, napi_get_value_string_utf8(env, args[0], nullptr, 0, &strlen));
        //check string length
        if (strlen > MAX_STR_LEN) {
            NAPI_CALL(env, napi_throw_error(env, "write string error", "String length too large, Max length "
                    STR_MAX_STR_LEN));
            return nullptr;
        }
        char *paramstr = new char[strlen + 1];
        napi_status status = napi_get_value_string_utf8(env, args[0], paramstr, strlen + 1, nullptr);

        if (status != napi_ok) {
            delete[] paramstr;
            GET_AND_THROW_LAST_ERROR(env);
            return nullptr;
        } else {
            //do write
            int32_t wrst = obj->caps->write(paramstr);
            delete[] paramstr;
            if (wrst != CAPS_SUCCESS) {
                if (wrst < 0 && wrst >= CAPS_ERR_EOO)
                    NAPI_CALL(env, napi_throw_error(env, "write string error", ErrorMessage[-wrst]));
                else
                    NAPI_CALL(env, napi_throw_error(env, "write string error", "Unknown Error"));
            }
        }

    } else
        NAPI_CALL(env, napi_throw_type_error(env, "write string error", "Wrong type of arguments, Expects string"));
    return nullptr;
}

napi_value CapsObject::WriteCaps(napi_env env, napi_callback_info info) {
    napi_value _this;
    size_t argc = 1;
    napi_value args[1];
    //get call stack
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, nullptr));

    if (argc == 0)
        NAPI_CALL(env, napi_throw_type_error(env, "write caps error", "Wrong number of arguments, Expects 1"));
    //get obj
    CapsObject *obj_this;
    NAPI_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void **>(&obj_this)));
    //check arguments type & count
    bool isCaps;
    napi_value cons_local;
    napi_get_reference_value(env, constructor, &cons_local);
    NAPI_CALL(env, napi_instanceof(env, args[0], cons_local, &isCaps));

    if (isCaps) {
        CapsObject *obj_input;
        NAPI_CALL(env, napi_unwrap(env, args[0], reinterpret_cast<void **>(&obj_input)));
        //do write
        int32_t wrst = obj_this->caps->write(obj_input->caps);
        if (wrst != CAPS_SUCCESS) {
            if (wrst < 0 && wrst >= CAPS_ERR_EOO)
                NAPI_CALL(env, napi_throw_error(env, "write caps error", ErrorMessage[-wrst]));
            else
                NAPI_CALL(env, napi_throw_error(env, "write caps error", "Unknown Error"));
        }
    } else
        NAPI_CALL(env, napi_throw_type_error(env, "write caps error", "Wrong type of arguments, Expects Caps"));
    return nullptr;

}

napi_value CapsObject::WriteVoid(napi_env env, napi_callback_info info) {
    napi_value _this;

    //get call stack
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &_this, nullptr));

    //get obj
    CapsObject *obj;
    NAPI_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void **>(&obj)));

    //do write
    int32_t rst = obj->caps->write();
    if (rst != CAPS_SUCCESS) {
        if (rst < 0 && rst >= CAPS_ERR_EOO)
            NAPI_CALL(env, napi_throw_error(env, "write void error", ErrorMessage[-rst]));
        else
            NAPI_CALL(env, napi_throw_error(env, "write int32 error", "Unknown Error"));
    }
    return nullptr;
}

napi_value CapsObject::ReadInt32(napi_env env, napi_callback_info info) {
    napi_value _this;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &_this, nullptr));

    //get obj
    CapsObject *obj;
    NAPI_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void **>(&obj)));

    //do read
    int32_t v;
    int32_t rst = obj->caps->read(v);
    if (rst != CAPS_SUCCESS) {
        if (rst < 0 && rst >= CAPS_ERR_EOO)
            NAPI_CALL(env, napi_throw_error(env, "read error", ErrorMessage[-rst]));
        else
            NAPI_CALL(env, napi_throw_error(env, "read error", "Unknown Error"));
        return nullptr;
    } else {
        napi_value result;
        NAPI_CALL(env, napi_create_int32(env, v, &result));
        return result;
    }
}

napi_value CapsObject::ReadUInt32(napi_env env, napi_callback_info info) {
    napi_value _this;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &_this, nullptr));

    //get obj
    CapsObject *obj;
    NAPI_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void **>(&obj)));

    //do read
    uint32_t v;
    int32_t rst = obj->caps->read(v);
    if (rst != CAPS_SUCCESS) {
        if (rst < 0 && rst >= CAPS_ERR_EOO)
            NAPI_CALL(env, napi_throw_error(env, "read error", ErrorMessage[-rst]));
        else
            NAPI_CALL(env, napi_throw_error(env, "read error", "Unknown Error"));
        return nullptr;
    } else {
        napi_value result;
        NAPI_CALL(env, napi_create_uint32(env, v, &result));
        return result;
    }
}

napi_value CapsObject::ReadInt64(napi_env env, napi_callback_info info) {
    napi_value _this;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &_this, nullptr));

    //get obj
    CapsObject *obj;
    NAPI_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void **>(&obj)));

    //do read
    int64_t v;
    int32_t rst = obj->caps->read(v);
    if (rst != CAPS_SUCCESS) {
        if (rst < 0 && rst >= CAPS_ERR_EOO)
            NAPI_CALL(env, napi_throw_error(env, "read error", ErrorMessage[-rst]));
        else
            NAPI_CALL(env, napi_throw_error(env, "read error", "Unknown Error"));
        return nullptr;
    } else {
        napi_value result;
        NAPI_CALL(env, napi_create_int64(env, v, &result));
        return result;
    }
}

napi_value CapsObject::ReadUInt64(napi_env env, napi_callback_info info) {
  napi_value _this;
  NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, 0, &_this, nullptr));

  //get obj
  CapsObject* obj;
  NAPI_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&obj)));

  //do read
  uint64_t v;
  int32_t rst = obj->caps->read(v);
  if (rst != CAPS_SUCCESS)
  {
    if (rst < 0 && rst >= CAPS_ERR_EOO)
      NAPI_CALL(env, napi_throw_error(env, "read error", ErrorMessage[-rst]));
    else
      NAPI_CALL(env, napi_throw_error(env, "read error", "Unknown Error"));
    return nullptr;
  }
  else
  {
    napi_value result;
    if (v < INT64_MAX)
    {

      NAPI_CALL(env, napi_create_int64(env, static_cast<int64_t>(v), &result));
      return result;
    }
    else
    {
      NAPI_CALL(env, napi_create_double(env, static_cast<double>(v), &result));
      return result;
    }
  }
}

napi_value CapsObject::ReadFloat(napi_env env, napi_callback_info info) {
    napi_value _this;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &_this, nullptr));

    //get obj
    CapsObject *obj;
    NAPI_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void **>(&obj)));

    //do read
    float v;
    int32_t rst = obj->caps->read(v);
    if (rst != CAPS_SUCCESS) {
        if (rst < 0 && rst >= CAPS_ERR_EOO)
            NAPI_CALL(env, napi_throw_error(env, "read error", ErrorMessage[-rst]));
        else
            NAPI_CALL(env, napi_throw_error(env, "read error", "Unknown Error"));
        return nullptr;
    } else {
        napi_value result;
        NAPI_CALL(env, napi_create_double(env, (double) v, &result));
        return result;
    }
}

napi_value CapsObject::ReadDouble(napi_env env, napi_callback_info info) {
    napi_value _this;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &_this, nullptr));

    //get obj
    CapsObject *obj;
    NAPI_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void **>(&obj)));

    //do read
    double v;
    int32_t rst = obj->caps->read(v);
    if (rst != CAPS_SUCCESS) {
        if (rst < 0 && rst >= CAPS_ERR_EOO)
            NAPI_CALL(env, napi_throw_error(env, "read double error", ErrorMessage[-rst]));
        else
            NAPI_CALL(env, napi_throw_error(env, "read double error", "Unknown Error"));
        return nullptr;
    } else {
        napi_value result;
        NAPI_CALL(env, napi_create_double(env, v, &result));
        return result;
    }
}

napi_value CapsObject::ReadBinary(napi_env env, napi_callback_info info) {
    napi_value _this;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &_this, nullptr));

    //get obj
    CapsObject *obj;
    NAPI_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void **>(&obj)));
    //do read
    const void *v;
    uint32_t len;
    int32_t rst = obj->caps->read(v, len);
    if (rst != CAPS_SUCCESS) {
        if (rst < 0 && rst >= CAPS_ERR_EOO)
            NAPI_CALL(env, napi_throw_error(env, "read binary error", ErrorMessage[-rst]));
        else
            NAPI_CALL(env, napi_throw_error(env, "read binary error", "Unknown Error"));
        return nullptr;
    } else {
        napi_value output_buffer;
        napi_value result;
        void *datap = new char[len];
        memcpy(datap, v, len);
        NAPI_CALL(env, napi_create_external_arraybuffer(
                env, datap, (size_t) len, DeleteForArray, nullptr, &output_buffer));
        NAPI_CALL(env, napi_create_typedarray(env, napi_uint8_array, len, output_buffer, 0, &result));
        return result;
    }
}

napi_value CapsObject::ReadString(napi_env env, napi_callback_info info) {
    napi_value _this;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &_this, nullptr));

    //get obj
    CapsObject *obj;
    NAPI_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void **>(&obj)));

    //do read
    const char *v;
    int32_t rst = obj->caps->read(v);
    if (rst != CAPS_SUCCESS) {
        if (rst < 0 && rst >= CAPS_ERR_EOO)
            NAPI_CALL(env, napi_throw_error(env, "read error", ErrorMessage[-rst]));
        else
            NAPI_CALL(env, napi_throw_error(env, "read error", "Unknown Error"));
        return nullptr;
    } else {
        napi_value result;
        NAPI_CALL(env, napi_create_string_utf8(env, v, NAPI_AUTO_LENGTH, &result));
        return result;
    }
}

napi_value CapsObject::ReadCaps(napi_env env, napi_callback_info info) {
    napi_value _this;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &_this, nullptr));

    //get obj
    CapsObject *obj;
    NAPI_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void **>(&obj)));

    //do read
    std::shared_ptr<Caps> v;
    int32_t rst = obj->caps->read(v);
    if (rst != CAPS_SUCCESS) {
        if (rst < 0 && rst >= CAPS_ERR_EOO)
            NAPI_CALL(env, napi_throw_error(env, "read caps error", ErrorMessage[-rst]));
        else
            NAPI_CALL(env, napi_throw_error(env, "read caps error", "Unknown Error"));
        return nullptr;
    } else {
        napi_value cons, newCaps;
        NAPI_CALL(env, napi_get_reference_value(env, constructor, &cons));
        NAPI_CALL(env, napi_new_instance(env, cons, 0, nullptr, &newCaps));
        CapsObject *obj_new;
        NAPI_CALL(env, napi_unwrap(env, newCaps, reinterpret_cast<void **>(&obj_new)));
        obj_new->SetCaps(v);
        return newCaps;
    }
}

napi_value CapsObject::ReadVoid(napi_env env, napi_callback_info info) {
    napi_value _this;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &_this, nullptr));

    //get obj
    CapsObject *obj;
    NAPI_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void **>(&obj)));

    //do read
    int32_t rst = obj->caps->read();
    if (rst != CAPS_SUCCESS) {
        if (rst < 0 && rst >= CAPS_ERR_EOO)
            NAPI_CALL(env, napi_throw_error(env, "read error", ErrorMessage[-rst]));
        else
            NAPI_CALL(env, napi_throw_error(env, "read error", "Unknown Error"));
    }
    napi_value result;
    napi_get_undefined(env, &result);
    return result;
}

napi_value CapsObject::Serialize(napi_env env, napi_callback_info info) {

    napi_value _this;
    size_t argc = 1;
    napi_value args[1];
    //get call stack
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, nullptr));
    enum ByteOrder bo;
    if (argc == 0)
        bo = ByteOrder::NBO;
    else
    {
        napi_valuetype t;
        NAPI_CALL(env, napi_typeof(env, args[0], &t));
        if (t == napi_number)
        {
            int32_t n;
            NAPI_CALL(env, napi_get_value_int32(env, args[0], &n));
            bo = n == 0x80 ? ByteOrder::NBO : ByteOrder::HBO;
        }
        else
            bo = ByteOrder::NBO;
    }
    //get obj
    CapsObject *obj;
    NAPI_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void **>(&obj)));

    //do serialize
    int32_t size = obj->caps->serialize(nullptr, 0);
    auto buff = new uint8_t[size];

    int rst = obj->caps->serialize(buff, static_cast<uint32_t>(size), static_cast<uint32_t>(bo));
    if (size != rst) {
        //delete[] reinterpret_cast<char*>(buff);
        if (rst < 0 && rst >= CAPS_ERR_EOO)
            NAPI_CALL(env, napi_throw_error(env, "serialize error", ErrorMessage[-rst]));
        else
            NAPI_CALL(env, napi_throw_error(env, "serialize error", "Unknown Error"));
        return nullptr;
    } else {
        napi_value result;
        napi_value output_buffer;
        if (napi_create_external_arraybuffer(
                env, buff, (size_t) size, DeleteForArray, nullptr, &output_buffer) != napi_ok) {
            GET_AND_THROW_LAST_ERROR(env);
            return nullptr;
        } else {
            NAPI_CALL(env,
                      napi_create_typedarray(env, napi_uint8_array, size / sizeof(uint8_t), output_buffer, 0, &result));
            return result;
        }
    }

}

napi_value CapsObject::Deserialize(napi_env env, napi_callback_info info) {
    napi_value _this;

    size_t argc = 1;
    napi_value args[1];
    //get call stack
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, nullptr));

    NAPI_ASSERT(env, argc > 0, "Wrong number of arguments");

    //get obj
    CapsObject *obj;
    NAPI_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void **>(&obj)));

    //check arguments type & count
    bool istypedarray;
    NAPI_CALL(env, napi_is_typedarray(env, args[0], &istypedarray));

    int32_t ds_rst;
    if (istypedarray) {
        //get argument Uint8Array
        napi_typedarray_type type;
        napi_value in_array_buffer;
        size_t byte_offset = 0;
        size_t length;
        void *datap;
        NAPI_CALL(env, napi_get_typedarray_info(
                env, args[0], &type, &length, &datap, &in_array_buffer, &byte_offset));
        if (type != napi_uint8_array)
            NAPI_CALL(env,
                      napi_throw_type_error(env, "argument type error", "Wrong type of arguments, Expects Uint8Array"));

        //do write
        ds_rst = Caps::parse(datap, (uint32_t) length, obj->caps);

        if (ds_rst != CAPS_SUCCESS) {
            if (ds_rst < 0 && ds_rst >= CAPS_ERR_EOO)
                NAPI_CALL(env, napi_throw_error(env, "deserialize error", ErrorMessage[-ds_rst]));
            else
                NAPI_CALL(env, napi_throw_error(env, "deserialize error", "Unknown Error"));
        }
    } else {
        NAPI_CALL(env,
                  napi_throw_type_error(env, "argument type error", "Wrong type of arguments, Expects Uint8Array"));
    }
    return nullptr;
}

void CapsObject::DeleteForArray(napi_env env, void *finalize_data, void *) {
    auto *deldata = reinterpret_cast<uint8_t *>(finalize_data);
    delete[] deldata;
    std::cout << "array data deleted" << std::endl;
}

