#pragma once

namespace coding {
    
    // 编码类
    class Coder {
    public:
        template<typename T> inline
        void encode(const T& value, const char* key);
        
    private:
        virtual void e(){}; // 解决:'coding::Coder' is not polymorphic
    };
    
    // 解码类
    class Decoder {
    public:
        template<typename T> inline
        void decode(const char* key, T* ret);
        
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
