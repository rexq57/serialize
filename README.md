# 一个简单快速的C++序列化实现

不依赖于第三方库，想实现序列化功能，灵感来自于下面的文章

https://blog.csdn.net/Kiritow/article/details/53129096

其代码位于

https://github.com/Kiritow/WarTime-Project/blob/master/frame/serialize.h



考虑到序列化需要高性能代码，于是进行优化，用C语言重写STL部分，使得效率提升了30倍(对比boost也是如此)

在本人电脑上执行100w次序列化/反序列化对比，代码如下：

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

  cbox box2, box4;

#if TEST_VERSION == 0 // STL版本
  OutEngine oe;
  oe << box << box3;

  const string& b = oe.str();
  InEngine ie(b);

#elif TEST_VERSION == 1 // 优化版本
  OutEngine oe;
  oe.resize(1024); // 可选：预设缓冲区
  oe << box << box3;

  const void* data = oe.data();
  InEngine ie(data, oe.size());

#elif TEST_VERSION == 2 // boost版本
  std::ostringstream os;
  boost::archive::binary_oarchive oa(os);
  oa << box << box3;

  const std::string& content = os.str();

  std::istringstream is(content);
  boost::archive::binary_iarchive ie(is);
#endif
  ie >> box2 >> box4;
}

float time = (std::chrono::steady_clock::now() - t).count() / 1.e9;
printf("用时 %f\n", time);
``````

Debug

优化前 6.181744s  

优化后 0.457298s (预设缓冲区) 0.703428s(不预设缓冲区)

boost 4.910393s 

Release

优化前 3.367076s

优化后 0.115770s (预设缓冲区) 0.368046s(不预设缓冲区)

boost 3.255476s

在预设缓冲区的情况下可以达到30倍性能提升！相当于依次对每个已定义的数据类型成员调用memcpy，所以速度快。而boost由于实现复杂，性能也比较差。当然，以上测试代码包含重复创建临时变量，而使用istringstream、string等STL模块的初始化和拷贝是造成性能底下的原因，完全使用C函数就不会有这个问题，在性能要求高的地方尽可能避免使用STL。



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
oe << box << box3;
``````

反序列化

``````
cbox box2, box4;

const void* data = oe.data();
InEngine ie(data, oe.size());

ie >> box2 >> box4;
``````

（可选）预设缓冲区，定义一个足够大的缓冲区，以避免序列化过程中的重复内存申请和拷贝

``````
OutEngine oe;
oe.resize(1024);
``````

如果需要增加更多类型的支持，重新实现下列函数即可

``````c++
// 自定义类型扩展
template<>
void serialize(OutEngine& x, Any& a)

template<>
void deserialize(InEngine& x, Any* c)
``````