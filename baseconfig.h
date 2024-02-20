#ifndef BASECONFIG_H
#define BASECONFIG_H
#include <variant>
#include "tools.h"

template <typename TModeType>
struct _handler_instance
{
    using handler_type = typename TModeType::handler_type;
    using type = _handler_instance<TModeType>;
    static typename TModeType::handler_type handler;
};

template <typename TModeType>
typename _handler_instance<TModeType>::handler_type _handler_instance<TModeType>::handler;

template <typename TModeType>
struct ModeTypeHandler
{
    virtual void doSomething(TModeType &modeType) = 0;
};

struct Ex1ModeTypeHandler;

struct Ex1ModeType
{
    using handler_type = Ex1ModeTypeHandler;
};

struct Ex1ModeTypeHandler : ModeTypeHandler<Ex1ModeType>
{
    void doSomething(Ex1ModeType &modeType) override
    {
    }
};

struct Ex2ModeTypeHandler;

struct Ex2ModeType
{
    using handler_type = Ex2ModeTypeHandler;
};

struct Ex2ModeTypeHandler : ModeTypeHandler<Ex2ModeType>
{
    void doSomething(Ex2ModeType &modeType) override
    {
    }
};

template <typename ...Args>
void doSomething(std::variant<Args...> &para)
{
    return std::visit(Overload { 
        [](Args& mt) {  _handler_instance<Args>::handler.doSomething(mt); }... 
    }, para);
}

using ModeTypes = std::variant<Ex1ModeType, Ex2ModeType>;

void test()
{
    ModeTypes a = Ex1ModeType();
    ModeTypes b = Ex2ModeType();

    doSomething(a);
    doSomething(b);
}

#endif