/// 条款2：尽量以const，enum，inline替换#define
// 定义一个class专属常量

class GamePlayer{
private:
    enum {NumTurns = 5};  //"the enum hack"
    int scores[NumTurns];
    ...
};

/// 条款6：若不想使用编译器自动生成的函数，就该明确拒绝
// 定义一个阻止copying动作的base class

class Uncopyable{
protected:  //允许derived对象构造和析构
    Uncopyable() {};
    ~Uncopyable() {};
private:
    Uncopyable(const Uncopyable&);  //但阻止copying
    Uncopyable& operator=(const Uncopyable&);
};  //Boost提供的版本名为noncopyable

// 要阻止自定义的对象被拷贝，只需继承Uncopyable
class HomeForSale: private Uncopyable{ //class不再声明copy构造函数或copy assign操作符
    ...
};


/// 条款10：令operator=返回一个reference to *this
// 这条适用于所有赋值相关运算

class Widget{
public:
    ...
    Widget& operator=(const Widget& rhs)
    {
        ...
        return *this;
    }
    Widget& operator+=(const Widget& rhs)  //这个协议适用于+=，-=，*=，等等。
    {
        ...
        return *this;
    }
    Widget& operator=(int rhs)  //此函数也适用即使此操作符的参数类型不符协定。
    {
        ...
        return *this;
    }
    ...
};

/// 条款11：在operator=中处理“自我赋值”
// 让operator=同时具有“异常安全性”和“自我赋值安全性”

class Bitmap{...};
class Widget{
    ...
    Widget& operator=(const Widget& rhs);
private:
    Bitmap* Pb;
};

Widget& Widget::operator=(const Widget& rhs)
{
    Bitmap* pOrig = Pb;  //记住原先的pb
    Pb = new Bitmap(*rhs.Pb);  //令pb指向*pb的一个副本
    delete pOrig;  //删除原先的pb
    return *this;
}

/// 条款14：在资源管理类中小心copying行为
// 对底层资源祭出“引用计数法”

class Lock{
public:
    explicit Lock(Mutex* pm):mutexPtr(pm, unlock)  //以某个Mutex初始化shared_ptr并以unlock函数为删除器
    {
        lock(mutexPtr.get());
    }
private:
    std::tr1::shared_ptr<Mutex> mutexPtr;
};

/// 条款25：考虑写出一个不抛异常的swap函数

namespace WidgetStuff {
    ...  //模板化的WidgetImpl等等。
    template<typename T>
    class Widget{
    public:
        ...
        void swap(Widget& other)
        {
            using std::swap;
            swap(pImpl, other.pImpl);
        }
        ...
    };
    ...
    template<typename T>
    void swap(Widget<T>& a, Widget<T>& b)
    {
        a.swap(b);
    }
}

/// 条款42：了解typename的双重意义
// 使用关键字typename标识嵌套从属类型名称

template<typename IterT>
void workWithIterator(IterT iter)
{
    typedef typename std::iterator_traits<IterT>::value_type value_type;
    value_type temp(*iter);
    ...
}

/// 条款49：了解new-handler的行为
// 为class添加set_new_handler支持能力

template <typename T>
class NewHandlerSupport {
public:
    static std::new_handler set_new_handler(std::new_handler p) throw();
    static void* operator new(std::size_t size) throw(std::bad_alloc);
    ...
    
private:
    static std::new_handler currentHandler;
};

template <typename T>
std::new_handler
NewHandlerSupport<T>::set_new_handler(std::new_handler p) throw(){
    std::new_handler oldHandler = currentHandler;
    currentHandler = p;
    return oldHandler;
}

template <typename T>
void* NewHandlerSupport<T>::operator new(std::size_t size) throw(std::bad_alloc){
    NewHandlerHolder h(std::set_new_handler(currentHandler));
    return ::operator new(size);
}

//以下将每一个currentHandler初始化为null
template<typename T>
std::new_handler NewHandlerSupport<T>::currentHandler = 0;

//应用：为Widget添加set_new_handler支持能力
class Widget: public NewHandlerSupport<Widget>{
    ...//不必声明set_new_handler或operator new
}

/// 条款52：写了placement new也要写placement delete
// 内含所有正常形式的new和delete的base class
class StandardNewDeleteForms{
public:
    //normal new/delete
    static void* operator new(std::size_t size) throw(std::bad_alloc)
    { return ::operator new(size); }
    static void* operator delete(void* pMemory) throw()
    { ::operator delete(pMemory); }
    //placement new/delete
    static void* operator new(std::size_t size, void* ptr) throw()
    { return ::operator new(size, ptr); }
    static void* operator delete(void* pMemory, void* ptr) throw()
    { return ::operator delete(pMemory, ptr); }
    //nothrow new/delete
    static void* operator new(std::size_t size, const std::nothrow_t& nt) throw()
    { return ::operator new(size, nt); }
    static void* operator delete(void* pMemory, const std::nothrow_t& nt) throw()
    { ::operator delete(pMemory); }
};

//应用：为widget扩充标准形式的new和delete
class Widget: public StandardNewDeleteForms{//继承标准形式
public:
    using StandardNewDeleteForms::operator new;//让这些形式可见
    using StandardNewDeleteForms::operator delete;
    
    static void* operator new(std::size_t size, std::ostream& logStream)
        throw(std::bad_alloc);//添加一个placement new
    static void operator delete(void* pMemory, std::ostream& logStream)
        throw();//添加一个placement delete
    ...
};