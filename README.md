# 一个简单快速的C++序列化实现

不依赖于第三方库，想实现序列化功能，灵感来自于下面的文章

https://blog.csdn.net/Kiritow/article/details/53129096

其代码位于

https://github.com/Kiritow/WarTime-Project/blob/master/frame/serialize.h



考虑到序列化需要高性能代码，于是进行优化，用C语言重写STL部分，使得效率提升了10倍以上

在本人电脑上执行100w次序列化/反序列化，代码如下：

``````c++
cbox box;
box.a=11;
box.b=6.6;
box.str="Hello World";

cbox box3;
box3.a=33;
box3.b=12.5;
box3.str="Yummy Hamburger!";

auto t = std::chrono::steady_clock::now();
int testCount = 1000000;

for (int i=0; i<testCount; i++) {

#ifdef OLD_VERSION
  OutEngine oe;
  oe << box << box3;

  cbox box2, box4;

  const string& b = oe.str();
  InEngine ie(b);
#else
  OutEngine oe;
  oe.resize(1024);
  oe << box << box3;

  cbox box2, box4;

  const void* data = oe.data();
  InEngine ie(data, oe.size());
#endif
	ie >> box2 >> box4;
}

float time = (std::chrono::steady_clock::now() - t).count() / 1.e9;
printf("用时 %f\n", time);
``````

优化前 6.181744s  

优化后 0.457298s



用法比较简单，只需要继承自Serializable即可

``````c++
class cbox : public Serializable
{
public:
    int a;
    double b;
    std::string str;
    
    SERIALIZE_3(a, b, str)
};
``````

你可以使用内建宏 SERIALIZE_#num 来设置序列化成员，或者手写以下两个虚函数的实现

``````
virtual void serialize(OutEngine& x) override
{
	x << a << b << str;
}

virtual void deserialize(InEngine& x) override
{
	x >> a >> b >> str;
}
``````

序列化

``````
OutEngine oe;
oe.resize(1024); // 可选：设置缓冲区大小
oe << box << box3;
``````

反序列化

``````
cbox box2, box4;

const void* data = oe.data();
InEngine ie(data, oe.size());

ie >> box2 >> box4;
``````



如果需要增加更多类型的支持，使用下列代码即可

``````c++
// 自定义类型扩展
template<>
void serialize(OutEngine& x, long& a)
{
    long c = htonl(a);
    x.write((const char*)&c, sizeof(c));
}

template<>
void deserialize(InEngine& x, long* c)
{
    x.read(c, sizeof(*c));
    *c=ntohl(*c);
}
``````