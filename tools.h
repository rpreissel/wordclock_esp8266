#ifndef tools_h
#define tools_h

#include <WString.h>
#include <variant>

template <typename... Ts> // (7)
struct Overload : Ts...
{
    using Ts::operator()...;
};
template <class... Ts>
Overload(Ts...) -> Overload<Ts...>;

template <typename T, typename... Args>
struct concatenator;

template <typename... Args0, typename... Args1>
struct concatenator<std::variant<Args0...>, Args1...>
{
    using type = std::variant<Args0..., Args1...>;
};

String split(String s, char parser, int index);

#endif