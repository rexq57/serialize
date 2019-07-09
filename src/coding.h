#pragma once

namespace coding {
    
    class Coder {
    public:
        template<typename A, typename T> inline
        void encode(const T& value, const char* key);
    };
    
    template<typename T> struct is_shared_ptr : std::false_type {};
    template<typename T> struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {};
    
//    template< class T >
//    struct is_shared_ptr : is_shared_ptr2<T> {};
    
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
    
    template<typename A, typename T> inline
    T* decodeAsPtr(Decoder* x, const char* key)
    {
        return ((A*)x)->template decodeAsPtr<T>(key);
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
    
    template<typename A, typename T> inline
    T* Decoder::decodeAsPtr(const char* key)
    {
        return coding::decodeAsPtr<A, T>(this, key);
    }
    
    //////////////////////////////////////////////////////////////////////////
    
};
