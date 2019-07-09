#pragma once

#include "coding.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include <vector>
#include <map>
#include <string>

namespace coding {
    
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
    
    class JsonCoder: public Coder {
        
    public:
        JsonCoder() {
            _doc = new Document();
            _doc->SetObject(); // 必须设置
        };
        
        ~JsonCoder()
        {
            if (_obj)
            {
                delete _obj;
            }
            else
            {
                if (_doc)
                    delete _doc;
            }
        }
        
        // 常规类型
        template<typename T, std::enable_if_t<is_normal<T>::value, int> = 0> inline
        void encode(const T& value, const char* key){
            _addValue(node(), Value().Set(value), key);
        };
        
        // 可编码
        template<typename T, std::enable_if_t<std::is_base_of<Codable, T>::value, int> = 0> inline
        void encode(const T& value, const char* key)
        {
            JsonCoder coder(_doc);
            value.encodeWithCoder(&coder);
            
            _addValue(node(), *coder.node(), key);
        }
        
        // std字符串
        inline
        void encode(const std::string& str, const char* key)
        {
            _addValue(node(), Value().Set(str.c_str()), key);
        }
        
        // 数组容器 - 常规
        template <template<class, class> class A, class B, class C, std::enable_if_t<is_normal<B>::value, int> = 0> inline
        void encode(const A<B, C>& vec, const char* key)
        {
            Value arr(kArrayType);
            for (auto& v : vec)
            {
                _pushBack(&arr, v);
            }
            _addValue(node(), arr, key);
        }
        
        // 数组容器 - 可编码
        template <template<class, class> class A, class B, class C, std::enable_if_t<std::is_base_of<Codable, B>::value, int> = 0> inline
        void encode(const A<B, C>& vec, const char* key)
        {
            Value arr(kArrayType);
            for (auto& v : vec)
            {
                JsonCoder coder(_doc);
                v.encodeWithCoder(&coder);
                
                _pushBack(&arr, *coder.node());
            }
            _addValue(node(), arr, key);
        }
        
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
        
        inline Document* d()
        {
            return _doc;
        }
        
        // value
        template<typename T>
        void _addValue(Value* obj, T& value, const char* key)
        {
            if (obj->HasMember(key)) assert(!"已经存在！会导致该键重复存在于json中！");
            
            obj->AddMember(Value().SetString(key, (unsigned int)strlen(key)), value, allocator());
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
        
        template<typename T, std::enable_if_t<!std::is_base_of<Codable, T>::value
        && !std::is_same<const char*, T>::value
        && !std::is_same<std::string, T>::value
        , int> = 0> inline
        T decode(const char* key)
        {
            return (*_doc)[key].Get<T>();
        }
        
        template<typename T, std::enable_if_t<std::is_base_of<Codable, T>::value, int> = 0> inline
        T decode(const char* key)
        {
            auto value = std::shared_ptr<T>(decodeAsPtr<T>(key));
            return *value.get();
        }
        
        template<typename T, std::enable_if_t<!std::is_base_of<Codable, T>::value
        && !std::is_same<const char*, T>::value
        && !std::is_same<std::string, T>::value
        , int> = 0> inline
        T* decodeAsPtr(const char* key)
        {
            T* value = new T();
            *value = (*_doc)[key].Get<T>();
            return value;
        }
        
        template<typename T, std::enable_if_t<std::is_base_of<Codable, T>::value, int> = 0> inline
        T* decodeAsPtr(const char* key)
        {
            if (!_doc->HasMember(key))
                return 0;
            
            T* obj = new T();
            
            Value& doc = (*_doc)[key];
            JsonDecoder decoder((Document*)&doc, true);
            
            if (!obj->initWithCoder(&decoder)) {assert(!"error"); return 0;}
            
            return obj;
        }
        
        template<typename T, std::enable_if_t<std::is_same<const char*, T>::value || std::is_same<std::string, T>::value, int> = 0> inline
        const char* decode(const char* key)
        {
            return (*_doc)[key].GetString();
        }
        
        template <template<class, class> class A, class B, class C> inline
        A<B, C> decode(const char* key)
        {
            auto value = std::shared_ptr<A<B,C>>(decodeAsPtr<A<B,C>>(key));
            return *value.get();
        }
        
//        template <template<class, class> class A, class B, class C> inline
//        A<B, C>* decodeAsPTR(const char* key)
//        {
//            A<B, C>* ret = new A<B, C>();
//
//            const auto& arr = (*_doc)[key].GetArray();
//            auto size = arr.Size();
//            ret.resize(size);
//            for (int i=0; i<size; i++)
//            {
//                ret[i] = arr[0];
//            }
//
//            return ret;
//        }
        
    private:
        
        JsonDecoder(Document* doc, bool assign=false)
        {
            _doc = doc;
            _assign = assign;
        }
        
        bool _assign = false;
        Document* _doc = 0;
    };
    
    //////////////////////////////////////////////////////////////////////////
    // Coder转发实现
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
    T Decoder::decode(const char* key)
    {
        JsonDecoder* coder = dynamic_cast<JsonDecoder*>(this);
        if (coder)
        {
            return coder->decode<T>(key);
        }
        
        assert(!"error!");
        return T();
    }
    
    template<typename T> inline
    T* Decoder::decodeAsPtr(const char* key)
    {
        JsonDecoder* coder = dynamic_cast<JsonDecoder*>(this);
        if (coder)
        {
            return coder->decodeAsPtr<T>(key);
        }
        
        assert(!"error!");
        return 0;
    }
    //////////////////////////////////////////////////////////////////////////
};
