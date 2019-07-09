#pragma once

namespace coding {
    
    class Coder {
    public:
        template<typename A, typename T> inline
        void encode(const T& value, const char* key);
    };
    
    class Decoder {
    public:
        template<typename A, typename T> inline
        T decode(const char* key);
        
        template<typename A, typename T> inline
        T* decodeAsPtr(const char* key);
    };
    
    // 编码解码接口
    class Codable {
        
    public:
        virtual void encodeWithCoder(Coder* coder) const = 0;
        virtual bool initWithCoder(Decoder* decoder) = 0;
    };
    
    //////////////////////////////////////////////////////////////////////////
    // Coder转发实现
    template<typename A,typename T> inline
    void Coder::encode(const T& value, const char* key)
    {
        ((A*)this)->encode(value, key);
    }
    
    template<typename A, typename T> inline
    T Decoder::decode(const char* key)
    {
        return ((A*)this)->template decode<T>(key);
    }
    
    template<typename A, typename T> inline
    T* Decoder::decodeAsPtr(const char* key)
    {
        return ((A*)this)->template decodeAsPtr<T>(key);
    }
    
    //////////////////////////////////////////////////////////////////////////
    
};
