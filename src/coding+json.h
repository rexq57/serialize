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
            _doc.SetObject(); // 必须设置
        };
        
        template<typename T, std::enable_if_t<!std::is_base_of<Codable, T>::value, int> = 0> inline
        void encode(const T& value, const char* key){
            _addValue(value, key);
        };
        
        template<typename T, std::enable_if_t<std::is_base_of<Codable, T>::value, int> = 0> inline
        void encode(const T& obj, const char* key)
        {
            JsonCoder coder;
            obj.encodeWithCoder(&coder);
            Document& doc = coder._doc;
            
            _doc.AddMember(Value().SetString(key, (unsigned int)strlen(key)), doc, _doc.GetAllocator());
            
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
                _doc.Accept(writer);
                _rebuildBuffer = false;
            }
            
            
            return buffer.GetString();
        }
        
    private:
        
        template<typename T>
        void _addValue(const T& value, const char* key)
        {
            _getValue(key).Set(value);
            _rebuildBuffer = true;
        }
        
        Value& _getValue(const char* key) {
            Document& d = _doc;
            
            if (d.HasMember(key)) assert(!"已经存在！会导致该键重复存在于json中！");
            
            Value v;
            
            d.AddMember(Value().SetString(key, (unsigned int)strlen(key)), v, d.GetAllocator());
            
            return d[key];
        }
        
        bool _rebuildBuffer = true;
        rapidjson::StringBuffer _buffer = 0;
        rapidjson::Document _doc;
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
        
        template<typename T, std::enable_if_t<!std::is_base_of<Codable, T>::value, int> = 0> inline
        T decode(const char* key)
        {
            return (*_doc)[key].Get<T>();
        }
        
        template<typename T, std::enable_if_t<std::is_base_of<Codable, T>::value, int> = 0> inline
        std::shared_ptr<T> decode(const char* key)
        {
            T* obj = new T();
            
            Value& doc = (*_doc)[key];
            JsonDecoder decoder((Document*)&doc, true);
            
            if (!obj->initWithCoder(&decoder)) return 0;
            
            return std::shared_ptr<T>(obj);
        }
        
        template<>
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
};
