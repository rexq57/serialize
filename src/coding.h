#pragma once

namespace coding {
    
    class Coder {
    public:
        template<typename T> inline
        void encode(const T& value, const char* key);
        
    private:
        virtual void e(){}; // 解决:'coding::Coder' is not polymorphic
    };
    
    class Decoder {
    public:
        template<typename T> inline
        T decode(const char* key);
        
        template<typename T> inline
        T* decodeAsPtr(const char* key);
        
    private:
        virtual void e(){}; // 解决:'coding::Coder' is not polymorphic
    };
    
    // 编码解码接口
    class Codable {
        
    public:
        virtual void encodeWithCoder(Coder* coder) const = 0;
        virtual bool initWithCoder(Decoder* decoder) = 0;
    };    
};
