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
        
        template<typename T, std::enable_if_t<!std::is_base_of<Codable, T>::value, int> = 0> inline
        void encode(const T& value, const char* key){
            _addValue(value, key);
        };
        
        template<typename T, std::enable_if_t<std::is_base_of<Codable, T>::value, int> = 0> inline
        void encode(const T& value, const char* key)
        {
            JsonCoder coder(_doc);
            value.encodeWithCoder(&coder);
            
            obj()->AddMember(Value().SetString(key, (unsigned int)strlen(key)), *coder.obj(), allocator());
            
            _rebuildBuffer = true;
        }
        
        template <>
        void encode(const std::string& str, const char* key)
        {
            _addValue(str.c_str(), key);
        }
        
        const char* string()
        {
            rapidjson::StringBuffer& buffer = _buffer;
            if (_rebuildBuffer)
            {
                buffer.Clear();
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                doc()->Accept(writer);
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
        
        inline Value* obj()
        {
            if (_obj)
                return _obj;
            
            // 当前是root，直接返回doc
            return _doc;
        }
        
        inline Document::AllocatorType& allocator()
        {
            return _doc->GetAllocator();
        }
        
        inline Document* doc()
        {
            return _doc;
        }
        
        template<typename T>
        void _addValue(const T& value, const char* key)
        {
            _getValue(key).Set(value);
            _rebuildBuffer = true;
        }
        
        Value& _getValue(const char* key) {
            Value* o = obj();
            
            if (o->HasMember(key)) assert(!"已经存在！会导致该键重复存在于json中！");
            
            o->AddMember(Value().SetString(key, (unsigned int)strlen(key)), Value(), allocator());
            
            return (*o)[key];
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
            T* value = decodeAsPtr<T>(key);
            return *value;
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
        
        
        template<typename T, std::enable_if_t<std::is_same<const char*, T>::value || std::is_same<std::string, T>::value, int> = 0>
        const char* decode(const char* key)
        {
            return (*_doc)[key].GetString();
        }
        
    private:
        
        JsonDecoder(Document* doc, bool assign=false)
        {
            _doc = doc;
            _assign = assign;
        }
        
        bool _assign = false;
        Document* _doc = 0;
    };
    
//    template<typename T> inline
//    void encode(JsonCoder* x, const T& value, const char* key)
//    {
//        ((JsonCoder*)x)->encode(value, key);
//    }
////
////    template<typename T, typename R = std::shared_ptr<T>> inline
////    R decode(JsonDecoder* x, const char* key)
////    {
////        return ((JsonDecoder*)x)->decode<T>(key);
////    }
//    
//    template<typename A, typename T> inline
//    std::shared_ptr<T> decode(JsonDecoder* x, const char* key)
//    {
//        return 0;
////        return ((A*)x)->template decode<T, R>(key);
//    }
};
