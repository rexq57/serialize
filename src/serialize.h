#pragma once

#include <vector>
#include <map>
#include <string>

namespace serialize {
    
    
    
    class OutEngine
    {
    public:
        
        OutEngine() {}
        
        virtual ~OutEngine() { freeMem(); }
        
        // 重设缓冲区大小
        void resize(size_t size)
        {
            clear();
            adaptSize(size);
        }
        
        // 数据内存
        const void* data() const { return mem; }
        
        // 数据尺寸
        size_t size() const { return usingSize; }
        
        // 清除缓冲区
        void clear()
        {
            memSize = 0;
            usingSize = 0;
            freeMem();
        }
        
        // 追加数据
        void write(const void* data, size_t size)
        {
            adaptSize(size);
            memcpy(&mem[usingSize], data, size);
            usingSize += size;
        }
        
    private:
        
        // 适应附加的尺寸
        void adaptSize(size_t size)
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
        
        void freeMem()
        {
            if (mem)
            {
                free(mem);
                mem = 0;
            }
        }
        
        const int INS_SIZE = sizeof(size_t) * 2;
        size_t memSize = 0;
        size_t usingSize = 0;
        char* mem = 0;
    };
    
    class InEngine
    {
    public:
        
        InEngine(const void* data, size_t size, bool copy=false) {
            reset(data, size, copy);
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
        
        //    inline
        //    size_t donesize() const
        //    {
        //        return n_size-leftsize();
        //    }
        
        void read(void* dst, size_t size)
        {
            memcpy(dst, is, size);
            is += size;
        }
        
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
    
    // 序列化对象需要实现该接口
    class Serializable
    {
    public:
        virtual void serialize(OutEngine& x) const = 0;
        virtual void deserialize(InEngine& x) = 0;
    };
    
    // 符号重载 (声明)
    
    template<typename SerializableType>
    OutEngine& operator << (OutEngine& ths, const SerializableType& a);
    
    template<typename SerializableType>
    InEngine& operator >> (InEngine& ths, SerializableType& a);
    
    //////////////////////////////////////////////////////////////////////////
    // 支持的数据类型匹配
    
    namespace detect {
        template<typename T> struct is_shared_ptr : std::false_type {};
        template<typename T> struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {};
    };
    
#ifdef _WIN32
    //        little = 0,
    //        big    = 1,
    //        native = little
#define IS_LITTLE_ENDIAN (native == 0)
#else
    //        little = __ORDER_LITTLE_ENDIAN__,
    //        big    = __ORDER_BIG_ENDIAN__,
    //        native = __BYTE_ORDER__
#define IS_LITTLE_ENDIAN (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#endif
    
#if IS_LITTLE_ENDIAN
    template< class T >
    struct is_little_type
    : std::integral_constant<
    bool,
    std::is_integral<typename std::remove_cv<T>::type>::value &&
    !std::is_same<char, typename std::remove_cv<T>::type>::value
    > {};

    // 小端序处理
    template<typename T, std::enable_if_t<is_little_type<T>::value, int> = 0> inline
    void serialize(OutEngine& x, const T& a)
    {
        T c=htonl(a);
        x.write((const char*)&c, sizeof(c));
    }
    template<typename T, std::enable_if_t<is_little_type<T>::value, int> = 0> inline
    void deserialize(InEngine& x, T* p)
    {
        x.read(p, sizeof(*p));
        *p=ntohl(*p);
    }
#endif
    
    // 直接读写
    template< class T >
    struct is_normal
    : std::integral_constant<
    bool,
#if !IS_LITTLE_ENDIAN
    std::is_integral<typename std::remove_cv<T>::type>::value ||
#endif
    std::is_floating_point<typename std::remove_cv<T>::type>::value ||
    std::is_same<char, typename std::remove_cv<T>::type>::value
    > {};
    
    template<typename T, std::enable_if_t<is_normal<T>::value, int> = 0> inline
    void serialize(OutEngine& x, const T& a)
    {
        x.write((const char*)&a,sizeof(a));
    }
    template<typename T, std::enable_if_t<is_normal<T>::value, int> = 0> inline
    void deserialize(InEngine& x, T* p)
    {
        x.read(p, sizeof(*p));
    }
    
    //////////////////////////////////////////////////////////////////////////
    // 类
    
    // string
    inline
    void serialize(OutEngine& x, const std::string& a)
    {
        int len = (int)a.size();
        x << len;
        x.write(a.data(), a.size());
    }
    
    inline
    void deserialize(InEngine& x, std::string* a)
    {
        int len;
        x >> len;
        a->resize(len);
        x.read((void*)a->data(), len);
    }
    
    // Serializable
    template< class T >
    struct is_Serializable : std::is_base_of<Serializable, T> {};
    
    template<typename T, std::enable_if_t<is_Serializable<T>::value, int> = 0> inline
    void serialize(OutEngine& x, const T& a)
    {
        a.serialize(x);
    }
    template<typename T, std::enable_if_t<is_Serializable<T>::value, int> = 0> inline
    void deserialize(InEngine& x, T* a)
    {
        a->deserialize(x);
    }
    
    // 可编码指针
    template<typename T, std::enable_if_t<
    (std::is_pointer<T>::value && std::is_base_of<Serializable, typename std::remove_pointer<T>::type>::value)  // 常规指针
        || (std::is_base_of<Serializable, typename T::element_type>::value && detect::is_shared_ptr<T>::value)  // 智能指针
    , int> = 0> inline
    void serialize(OutEngine& x, const T ptr)
    {
        // 写入指针是否有效的标记，1字节
        bool mark = ptr != 0 && ptr.get() != 0;
        x.write(&mark, 1);
        
        if (!mark) // 不操作NULL指针
            return;
        
        ptr->serialize(x);
    }
    
    template <typename T, std::enable_if_t<std::is_base_of<Serializable, T>::value, int> = 0> // 智能指针
    void deserialize(InEngine& x, std::shared_ptr<T>* ptr)
    {
        // 检查是否有效，无效，则跳过
        bool mark;
        x.read(&mark, 1);
        if (!mark)
            return;
        
        // 对智能指针做重写入
        //*ptr = std::shared_ptr<typename T::element_type>(new typename T::element_type());
        *ptr = std::shared_ptr<T>(new T());

        (*ptr)->deserialize(x);
    }
    
    //////////////////////////////////////////////////////////////////////////
    // 容器
    
    // pair
    template<class B, class C>
    inline
    void serialize(OutEngine& x, const std::pair<B, C>& a)
    {
        x << a.first << a.second;
    }
    
    template<class B, class C>
    inline
    void deserialize(InEngine& x, std::pair<B, C>* a)
    {
        x >> a->first >> a->second;
    }
    
    // map
    template <template<class, class, class, class> class A, class B, class C, class D, class E>
    inline
    void serialize(OutEngine& x, const A<B, C, D, E>& a) {
        x << (int)a.size();
        for (auto& t : a) {
            x << t;
        }
    }
    
    template <template<class, class, class, class> class A, class B, class C, class D, class E>
    inline
    void deserialize(InEngine& x, A<B, C, D, E>* c) {
        int size;
        x >> size;
        for (int i=0; i<size; i++) {
            std::pair<B, C> p;
            x >> p;
            c->insert(p);
        }
    }
    
    // vector    
    template <template<class, class> class A, class B, class C, std::enable_if_t<!is_normal<B>::value, int> = 0>
    inline
    void serialize(OutEngine& x, const A<B, C>& a) {
        x << (int)a.size();
        for (auto& t : a) {
            x << t;
        }
    }

    template <template<class, class> class A, class B, class C, std::enable_if_t<!is_normal<B>::value, int> = 0>
    inline
    void deserialize(InEngine& x, A<B, C>* c) {
        int size;
        x >> size;
        c->resize(size);
        for (int i=0; i<size; i++) {
            x >> (*c)[i];
        }
    }

    // vector 常规数据，直接连续读写
    template <template<class, class> class A, class B, class C, std::enable_if_t<is_normal<B>::value, int> = 0>
    inline
    void serialize(OutEngine& x, const A<B, C>& a) {
        x << (int)a.size();
        x.write(&a[0], a.size() * sizeof(a[0]));
    }

    template <template<class, class> class A, class B, class C, std::enable_if_t<is_normal<B>::value, int> = 0>
    inline
    void deserialize(InEngine& x, A<B, C>* c) {
        int size;
        x >> size;
        c->resize(size);
        x.read(&(*c)[0], size * sizeof((*c)[0]));
    }
    
    //////////////////////////////////////////////////////////////////////////
    // 符号重载（凡是实现过的支持类型，都可以通过 >> << 符号来调用其实现过程）
    
    template<typename SerializableType>
    OutEngine& operator << (OutEngine& ths, const SerializableType& a)
    {
        serialize::serialize(ths, a);
        return ths;
    }
    
    template<typename SerializableType>
    InEngine& operator >> (InEngine& ths, SerializableType& a)
    {
        serialize::deserialize(ths, &a);
        return ths;
    }
    
    //////////////////////////////////////////////////////////////////////////
    // 内建宏
    
#define _SERIALIZE(...) \
virtual void serialize(OutEngine& x) const override {__VA_ARGS__;}
    
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
#define SERIALIZE_8(a, b, c, d, e, f, g, h) _SERIALIZE(_SE_ADD(a), _SE_ADD(b), _SE_ADD(c), _SE_ADD(d), _SE_ADD(e), _SE_ADD(f), _SE_ADD(g), _SE_ADD(h)) _DESERIALIZE(_DES_ADD(a), _DES_ADD(b), _DES_ADD(c), _DES_ADD(d), _DES_ADD(e), _DES_ADD(f), _DES_ADD(g), _DES_ADD(h))
#define SERIALIZE_9(a, b, c, d, e, f, g, h, i) _SERIALIZE(_SE_ADD(a), _SE_ADD(b), _SE_ADD(c), _SE_ADD(d), _SE_ADD(e), _SE_ADD(f), _SE_ADD(g), _SE_ADD(h), _SE_ADD(i)) _DESERIALIZE(_DES_ADD(a), _DES_ADD(b), _DES_ADD(c), _DES_ADD(d), _DES_ADD(e), _DES_ADD(f), _DES_ADD(g), _DES_ADD(h), _DES_ADD(i))
};
