#pragma once

#include "coding.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include <vector>
#include <map>
#include <string>

namespace coding {
    
    // 使用rapidjson的编码/解码实现
    
    using namespace rapidjson;
    
    // 直接读写
    template< class T >
    struct is_normal
    : std::integral_constant<
    bool,
    std::is_integral<typename std::remove_cv<T>::type>::value ||
    std::is_floating_point<typename std::remove_cv<T>::type>::value ||
    std::is_same<char, typename std::remove_cv<T>::type>::value ||
    std::is_same<char*, typename std::remove_cv<T>::type>::value
    > {};
    
    
    
//    // 数组容器 - 常规
//    template <template<class, class> class A, class B, class C, std::enable_if_t<is_normal<B>::value, int> = 0> inline
//    void ToValue(const A<B, C>& val, Value* value)
//    {
//        Value arr(kArrayType);
//        for (auto& v : vec)
//        {
//            _pushBack(&arr, v);
//        }
//        _addValue(node(), arr, key);
//    }
//    
//    // 数组容器 - 可编码
//    template <template<class, class> class A, class B, class C, std::enable_if_t<std::is_base_of<Codable, B>::value, int> = 0> inline
//    void encode(const A<B, C>& vec, const char* key)
//    {
//        Value arr(kArrayType);
//        for (auto& v : vec)
//        {
//            JsonCoder coder(_doc);
//            v.encodeWithCoder(&coder);
//            
//            _pushBack(&arr, *coder.node());
//        }
//        _addValue(node(), arr, key);
//    }
    
    
    
    
    
    class JsonCoder: public Coder {
        
    public:
        JsonCoder() {
            _doc = new Document();
            _doc->SetObject(); // 必须设置为Object，Object = Dict
        };
        
        ~JsonCoder()
        {
            if (_obj) { delete _obj; }
            else { if (_doc) delete _doc; }
        }
        
        // 常规类型
        template<typename T, std::enable_if_t<is_normal<T>::value, int> = 0> inline
        void encode(const T& value, const char* key){
            _addValue(node(), value, key);
        };
        
        // 可编码
        template<typename T, std::enable_if_t<std::is_base_of<Codable, T>::value, int> = 0> inline
        void encode(const T& value, const char* key)
        {
            _addValue(node(), value, key);
        }
        
        // std字符串
        inline
        void encode(const std::string& str, const char* key)
        {
            _addValue(node(), str, key);
        }
        
        // 数组容器 - 常规
        template <template<class, class> class A, class B, class C, std::enable_if_t<is_normal<B>::value, int> = 0> inline
        void encode(const A<B, C>& vec, const char* key)
        {
            _addValue(node(), vec, key);
        }
        
        // 数组容器 - 可编码
        template <template<class, class> class A, class B, class C, std::enable_if_t<std::is_base_of<Codable, B>::value, int> = 0> inline
        void encode(const A<B, C>& vec, const char* key)
        {
            _addValue(node(), vec, key);
        }
        
        // 字典容器 - 常规
        template <template<class, class, class, class> class A, class B, class C, class D, class E>
        inline
        void encode(const A<B, C, D, E>& map, const char* key)
        {
            _addValue(node(), map, key);
        }

        // 字典容器 - 可编码
//        template <template<class, class, class, class> class A, class B, class C, class D, class E, std::enable_if_t<std::is_base_of<Codable, B>::value, int> = 0>
//        inline
//        void encode(const A<B, C, D, E>& map, const char* key)
//        {
//            _addValue(node(), map, key);
//        }
        
        const char* string()
        {
            rapidjson::StringBuffer& buffer = _buffer;
            if (_rebuildBuffer)
            {
                buffer.Clear();
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                d()->Accept(writer);
                _rebuildBuffer = false;
            }
            
            return buffer.GetString();
        }
        
    private:
        
        template <typename T, std::enable_if_t<std::is_integral<T>::value || std::is_floating_point<T>::value, int> = 0>
        void ToKeyString(const T& value, Value* ret)
        {
            ret->Set(std::to_string(value).c_str(), allocator());
        }
        
        void ToKeyString(const std::string& value, Value* ret)
        {
            ret->Set(value.c_str(), allocator());
        }
        
        // 常规类型
        template<typename T, std::enable_if_t<is_normal<T>::value, int> = 0> inline
        void ToValue(const T& val, Value* value)
        {
            value->Set(val);
        };
        
        // 可编码
        template<typename T, std::enable_if_t<std::is_base_of<Codable, T>::value, int> = 0> inline
        void ToValue(const T& val, Value* value)
        {
            JsonCoder coder(_doc);
            val.encodeWithCoder(&coder);
            
            *value = *coder.node();
        }
        
        // std字符串
        inline
        void ToValue(const std::string& val, Value* value)
        {
            value->Set(val.c_str());
        }
        
        // 数组容器 - 常规
        template <template<class, class> class A, class B, class C, std::enable_if_t<is_normal<B>::value, int> = 0> inline
        void ToValue(const A<B, C>& vec, Value* value)
        {
            value->SetArray();
            for (auto& v : vec)
            {
                _pushBack(value, v);
            }
        }
        
        // 数组容器 - 可编码
        template <template<class, class> class A, class B, class C, std::enable_if_t<std::is_base_of<Codable, B>::value, int> = 0> inline
        void ToValue(const A<B, C>& vec, Value* value)
        {
            value->SetArray();
            for (auto& v : vec)
            {
                JsonCoder coder(_doc);
                v.encodeWithCoder(&coder);
                
                _pushBack(value, *coder.node());
            }
        }
        
        // 字典容器 - 常规
        template <template<class, class, class, class> class A, class B, class C, class D, class E>
        inline
        void ToValue(const A<B, C, D, E>& map, Value* value)
        {
            value->SetObject();
            for (auto& it : map)
            {
                _addValue(value, it.second, it.first);
            }
        }
        
        // 字典容器 - 可编码
//        template <template<class, class, class, class> class A, class B, class C, class D, class E, std::enable_if_t<std::is_base_of<Codable, B>::value, int> = 0>
//        inline
//        void encode(const A<B, C, D, E>& map, Value* value)
//        {
//            for (auto& it : map)
//            {
//                JsonCoder coder(_doc);
//                it.second.encodeWithCoder(&coder);
//
//                _addValue(value, *coder.node(), it.first);
//            }
//        }
        
        JsonCoder(Document* doc) {
            assert(doc);
            _doc = doc;
            _obj = new Value(kObjectType);
        };
        
        inline Document::AllocatorType& allocator()
        {
            return _doc->GetAllocator();
        }
        
        inline Value* node()
        {
            if (_obj)
                return _obj;
            
            // 当前是root，直接返回doc
            return _doc;
        }
        
        inline Document* d() { return _doc; }
        
        // value
        template<typename T, typename T1>
        void _addValue(Value* obj, const T& val, const T1& key)
        {
            // 只支持kStringType
            Value keyValue;
            ToKeyString(key, &keyValue);
            
            if (obj->HasMember(keyValue)) assert(!"已经存在！会导致该键重复存在于json中！");
            
            Value value;
            ToValue(val, &value);
            
            obj->AddMember(keyValue, value, allocator());
            // 未立即存储字符串，如果是临时字符串，会导致引用错误
//            obj->AddMember(Value().SetString(key, (unsigned int)strlen(key)), value, allocator());
//            obj->AddMember(Value().Set(key), value, allocator());
            _rebuildBuffer = true;
        }

        template<typename T>
        void _pushBack(Value* arr, T& value)
        {
            arr->PushBack(value, allocator());
        }
        
        bool _rebuildBuffer = true;
        StringBuffer _buffer = 0;
        Document* _doc = 0;
        Value* _obj = 0;
    };
    
    class JsonDecoder: public Decoder {
        
    public:
        
        JsonDecoder(const char* json) : JsonDecoder(new Document()) {
            _doc->Parse(json);
        }
        
        virtual ~JsonDecoder()
        {
            if (!_assign && _doc)
                delete _doc;
        }
        
        // 常规类型
        template<typename T, std::enable_if_t<is_normal<T>::value, int> = 0> inline
        void decode(const char* key, T* ret)
        {
            *ret = (*_doc)[key].Get<T>();
        }
        
        // 可编码
        template<typename T, std::enable_if_t<std::is_base_of<Codable, T>::value, int> = 0> inline
        void decode(const char* key, T* ret)
        {
            if (!_doc->HasMember(key))
            {
                assert(!"不存在该成员！");
                return;
            }

            Value& doc = (*_doc)[key];
            JsonDecoder decoder((Document*)&doc, true);

            if (!ret->initWithCoder(&decoder)) {
                assert(!"error");
                return;
            }
        }
        
        // std字符串
        void decode(const char* key, std::string* ret)
        {
            *ret = (*_doc)[key].GetString();
        }
        
        // 数组容器 - 常规
        template <template<class, class> class A, class B, class C, std::enable_if_t<is_normal<B>::value, int> = 0> inline
        void decode(const char* key, A<B, C>* ret)
        {
            const auto& arr = (*_doc)[key].GetArray();
            auto size = arr.Size();
            ret->resize(size);
            for (int i=0; i<size; i++)
            {
                (*ret)[i] = arr[i].Get<B>();
            }
        }
        
        // 数组容器 - 可编码
        template <template<class, class> class A, class B, class C, std::enable_if_t<std::is_base_of<Codable, B>::value, int> = 0> inline
        void decode(const char* key, A<B, C>* ret)
        {
            const auto& arr = (*_doc)[key].GetArray();
            auto size = arr.Size();
            ret->resize(size);
            for (int i=0; i<size; i++)
            {
                Value& o = arr[i]; // object
                // assert(o.IsObject());
                
                JsonDecoder decoder((Document*)&o, true);
                if (!(*ret)[i].initWithCoder(&decoder)) {
                    assert(!"error");
                    return;
                }
            }
        }
        
        // 字典容器 - 常规
        template <template<class, class, class, class> class A, class B, class C, class D, class E>
        inline
        void decode(const char* key,  A<B, C, D, E>* ret)
        {
            const auto& map = (*_doc)[key].GetObject();
            for (auto& it : map)
            {
                std::pair<B, C> p;
                ToValue(it.name, &p.first);
                ToValue(it.value, &p.second);
                ret->insert(p);
            }
        }
        
    private:
        
        template<typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0> inline
        void ToValue(Value& value, T* ret)
        {
            if (value.IsString())
                *ret = std::stoi(value.GetString());
            else
                *ret = value.Get<T>();
        }
        
        template<typename T, std::enable_if_t<std::is_floating_point<T>::value, int> = 0> inline
        void ToValue(Value& value, T* ret)
        {
            if (value.IsString())
                *ret = std::stof(value.GetString());
            else
                *ret = value.Get<T>();
        }
        
        void ToValue(Value& value, std::string* ret)
        {
            *ret = value.Get<const char*>();
        }
        
        JsonDecoder(Document* doc, bool assign=false)
        {
            _doc = doc;
            _assign = assign;
        }
        
        bool _assign = false;
        Document* _doc = 0;
    };
    
    //////////////////////////////////////////////////////////////////////////
    // Coder & Decoder 转发实现
    template<typename T> inline
    void Coder::encode(const T& value, const char* key)
    {
        JsonCoder* coder = dynamic_cast<JsonCoder*>(this);
        if (coder)
        {
            coder->encode(value, key);
            return;
        }
        
        assert(!"error!");
    }
    
    template<typename T> inline
    void Decoder::decode(const char* key, T* ret)
    {
        JsonDecoder* coder = dynamic_cast<JsonDecoder*>(this);
        if (coder)
        {
            coder->decode(key, ret);
            return;
        }
        
        assert(!"error!");
    }
};
