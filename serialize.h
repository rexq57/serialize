#pragma once

#include <string>

using namespace std;

class OutEngine;
class InEngine;

// 序列化对象需要实现该接口
class Serializable
{
public:
    virtual void serialize(OutEngine& x) = 0;
    virtual void deserialize(InEngine& x) = 0;
};

class OutEngine
{
public:
    
    OutEngine() {}
    
    virtual ~OutEngine() { freeMem(); }
    
    // 重设缓冲区大小
    void resize(size_t size)
    {
        size_t dstSize = usingSize + size;
        if (dstSize > memSize)
        {
            memSize += dstSize + INS_SIZE;
            void *tmp = malloc(memSize);
            memcpy(tmp, mem, usingSize);
            freeMem();
            mem = (char*)tmp;
        }
    }
    
    // 数据内存
    const void* data() const { return mem; }
    
    // 数据尺寸
    size_t size() const { return usingSize; }
    
    // 清除数据
    void clear()
    {
        memSize = 0;
        usingSize = 0;
        freeMem();
    }
    
    template<typename SerializableType>
    OutEngine& operator << (SerializableType& a);
    
protected:
    
    // 追加数据
    void write(const void* data, size_t size)
    {
        resize(size);
        memcpy(&mem[usingSize], data, size);
        usingSize += size;
    }
    
    template<typename T>
    friend void serialize(OutEngine& x, T& a);
    
private:
    
    void freeMem()
    {
        if (mem)
        {
            free(mem);
            mem = 0;
        }
    }
    
    const int INS_SIZE = sizeof(size_t);
    size_t memSize = 0;
    size_t usingSize = 0;
    char* mem = 0;
};

class InEngine
{
public:
    InEngine(const string& s) {
        reset(s.data(), s.size(), true);
    }
    
    InEngine(const void* data, size_t size) {
        reset(data, size, false);
    }
    
    virtual ~InEngine() {
        freeMem();
    }
    
    void reset(const void* data, size_t size, bool copy)
    {
        if (copy)
        {
            freeMem();
            mem = malloc(size);
            memcpy(mem, data, size);
        }
        else
        {
            mem = (void*)data;
        }
        
        assign = !copy;
        is = (char*)mem;
        n_size = size;
    }
    
    void read(void* dst, size_t size)
    {
        memcpy(dst, is, size);
        is += size;
    }
    
//    inline
//    size_t donesize() const
//    {
//        return n_size-leftsize();
//    }
    
    template<typename SerializableType>
    InEngine& operator >> (SerializableType& a) ;
    
private:
    
//    inline
//    size_t leftsize() const
//    {
//        return n_size - (is - (char*)mem);
//    }
    
    void freeMem()
    {
        if (!assign && mem)
        {
            free(mem);
            mem = 0;
        }
    }
    
    void* mem = 0;
    char* is  = 0;
    size_t n_size = 0;
    bool assign = false;
};

// 泛型匹配类实现
template<typename T>
void serialize(OutEngine& x, T& a)
{ 
    a.serialize(x);
}
template<typename T>
void deserialize(InEngine& x, T* a)
{
    return a->deserialize(x);
}

// 匹配基础类型
/// string
template<>
void serialize(OutEngine& x, string& a)
{
    int len = (int)a.size();
    x.write(&len, sizeof(len));
    x.write(a.data(), a.size());
}
template<>
void deserialize(InEngine& x, string* a)
{
    int len;
    x.read(&len, sizeof(len));
    a->resize(len);
    x.read((void*)a->data(), len);
}

/// Marco definition
#define _LARGETAIL_DATA_SERIALIZE(Type) template<> \
void serialize(OutEngine& x, Type& a) \
{ \
    Type c=htonl(a); \
    x.write((const char*)&c, sizeof(c)); \
}

#define _LARGETAIL_DATA_DESERIALIZE(Type) template<> \
void deserialize(InEngine& x, Type* c) \
{ \
    int cpySize = sizeof(*c); \
    x.read(c, cpySize); \
    *c=ntohl(*c); \
}

#define _NORMAL_DATA_SERIALIZE(Type) template<> \
void serialize(OutEngine& x, Type& a) \
{ \
x.write((const char*)&a,sizeof(a)); \
}
#define _NORMAL_DATA_DESERIALIZE(Type) template<> \
void deserialize(InEngine& x, Type* a)\
{ \
x.read(a, sizeof(*a)); \
}

#define LARGETAIL_DATA_SERIALIZE(Type) _LARGETAIL_DATA_SERIALIZE(Type) _LARGETAIL_DATA_DESERIALIZE(Type)
#define NORMAL_DATA_SERIALIZE(Type) _NORMAL_DATA_SERIALIZE(Type) _NORMAL_DATA_DESERIALIZE(Type)

LARGETAIL_DATA_SERIALIZE(int)
LARGETAIL_DATA_SERIALIZE(long)
NORMAL_DATA_SERIALIZE(double)
NORMAL_DATA_SERIALIZE(float)
NORMAL_DATA_SERIALIZE(char)

template<typename SerializableType>
OutEngine& OutEngine::operator << (SerializableType& a)
{
    ::serialize(*this, a);
    return *this;
}

template<typename SerializableType>
InEngine& InEngine::operator >> (SerializableType& a)
{
    ::deserialize(*this, &a);
    return *this;
}

////////////////////////////////////////////////////////////

#define _SERIALIZE(...) \
virtual void serialize(OutEngine& x) override {__VA_ARGS__;}

#define _DESERIALIZE(...) \
virtual void deserialize(InEngine& x) override {__VA_ARGS__;}

#define _SE_ADD(a) x << a
#define _DES_ADD(a) x >> a

#define SERIALIZE_1(a) _SERIALIZE(_SE_ADD(a)) _DESERIALIZE(_DES_ADD(a))
#define SERIALIZE_2(a, b) _SERIALIZE(_SE_ADD(a), _SE_ADD(b)) _DESERIALIZE(_DES_ADD(a), _DES_ADD(b))
#define SERIALIZE_3(a, b, c) _SERIALIZE(_SE_ADD(a), _SE_ADD(b), _SE_ADD(c)) _DESERIALIZE(_DES_ADD(a), _DES_ADD(b), _DES_ADD(c))
#define SERIALIZE_4(a, b, c, d) _SERIALIZE(_SE_ADD(a), _SE_ADD(b), _SE_ADD(c), _SE_ADD(d)) _DESERIALIZE(_DES_ADD(a), _DES_ADD(b), _DES_ADD(c), _DES_ADD(d))
#define SERIALIZE_5(a, b, c, d, e) _SERIALIZE(_SE_ADD(a), _SE_ADD(b), _SE_ADD(c), _SE_ADD(d), _SE_ADD(e)) _DESERIALIZE(_DES_ADD(a), _DES_ADD(b), _DES_ADD(c), _DES_ADD(d), _DES_ADD(e))
#define SERIALIZE_6(a, b, c, d, e, f) _SERIALIZE(_SE_ADD(a), _SE_ADD(b), _SE_ADD(c), _SE_ADD(d), _SE_ADD(e), _SE_ADD(f)) _DESERIALIZE(_DES_ADD(a), _DES_ADD(b), _DES_ADD(c), _DES_ADD(d), _DES_ADD(e), _DES_ADD(f))
#define SERIALIZE_7(a, b, c, d, e, f, g) _SERIALIZE(_SE_ADD(a), _SE_ADD(b), _SE_ADD(c), _SE_ADD(d), _SE_ADD(e), _SE_ADD(f), _SE_ADD(g)) _DESERIALIZE(_DES_ADD(a), _DES_ADD(b), _DES_ADD(c), _DES_ADD(d), _DES_ADD(e), _DES_ADD(f), _DES_ADD(g))