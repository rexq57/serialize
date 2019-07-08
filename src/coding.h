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
    };
    
    // 编码解码接口
    class Codable {
        
    public:
        virtual void encodeWithCoder(Coder* coder) const = 0;
        virtual bool initWithCoder(Decoder* decoder) = 0;
    };
    
    template<typename A, typename T> inline
    void encode(Coder* x, const T& value, const char* key)
    {
        ((A*)x)->encode(value, key);
    }
    
    template<typename A, typename T> inline
    T decode(Decoder* x, const char* key)
    {
        return ((A*)x)->template decode<T>(key);
    }
    
    //////////////////////////////////////////////////////////////////////////
    // Coder转发实现
    template<typename A,typename T> inline
    void Coder::encode(const T& value, const char* key)
    {
        coding::encode<A>(this, value, key);
    }
    
    template<typename A, typename T> inline
    T Decoder::decode(const char* key)
    {
        return coding::decode<A, T>(this, key);
    }
    
    //////////////////////////////////////////////////////////////////////////
    
};
