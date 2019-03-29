#ifndef FUNC_HPP
#define FUNC_HPP
template <typename UnusedType>
class function; // в этом файле просто лежит реализация function, которая должна бы уже быть в этой версии
// но её нет, так что скопируем её откуда-то с просторов интернета
// и дошлифуем чуть чуть под баги компилятора.
// А ещё тут везве auto_ptr
// ыыыы......
// Потому что тогда ешё не было нормальных умных указателей.

template <typename ReturnType, typename ... ArgumentTypes>
class function <ReturnType (ArgumentTypes ...)>
{
public:

    typedef ReturnType signature_type (ArgumentTypes ...);

    function()
        : mInvoker()
    {
    }


    template <typename FunctionT>

    function(FunctionT f)
        : mInvoker(new free_function_holder<FunctionT>(f))
    {
    }

    template <typename FunctionType, typename ClassType>
    function(FunctionType ClassType::* f)
        : mInvoker(new member_function_holder<FunctionType, ArgumentTypes ...>(f))
    {
    }


    function(const function & other)
        : mInvoker(other.mInvoker->clone())
    {
    }

    function operator = (const function & other)
    {
        auto t = (other.mInvoker->clone());
        // собственно, это костыль
        // никакие языковые особенности не должны мешать компилятору тут принять сразу результат ф-ции
        // но приходится его сначала сохронить во временную переменную, а только потом присвоить
        // просто потому, что иначе компилятор не съест
        mInvoker = (t);
        return *this;
    }

    ReturnType operator ()(ArgumentTypes ... args)
    {
        return mInvoker->invoke(args ...);
    }


private:


    class function_holder_base
    {
    public:

        function_holder_base()
        {
        }

        virtual ~function_holder_base()
        {
        }

        virtual ReturnType invoke(ArgumentTypes ... args) = 0;

        virtual std::auto_ptr<function_holder_base> clone() = 0;

    private:
        function_holder_base(const function_holder_base & );
        void operator = (const function_holder_base &);
    };


    typedef std::auto_ptr<function_holder_base> invoker_t;


    template <typename FunctionT>
    class free_function_holder : public function_holder_base
    {
    public:

        free_function_holder(FunctionT func)
            : function_holder_base(),
              mFunction(func)
        {

        }

        virtual ReturnType invoke(ArgumentTypes ... args)
        {
            return mFunction(args ...);
        }


        virtual invoker_t clone()
        {
            return invoker_t(new free_function_holder(mFunction));
        }

    private:

        FunctionT mFunction;
    };


    template <typename FunctionType, typename ClassType, typename ... RestArgumentTypes>
    class member_function_holder : public function_holder_base
    {
    public:

        typedef FunctionType ClassType::* member_function_signature_t;

        member_function_holder(member_function_signature_t f)
            : mFunction(f)
        {}

        virtual ReturnType invoke(ClassType obj, RestArgumentTypes ... restArgs)
        {
            return (obj.*mFunction)(restArgs ...);
        }

        virtual invoker_t clone()
        {
            return invoker_t(new member_function_holder(mFunction));
        }

    private:
        member_function_signature_t mFunction;
    };

    invoker_t mInvoker;
};

#endif
