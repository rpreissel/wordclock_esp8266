#ifndef tools_h
#define tools_h

#include <WString.h>

template<typename ... Ts>                                                 // (7) 
struct Overload : Ts ... { 
    using Ts::operator() ...;
};
template<class... Ts> Overload(Ts...) -> Overload<Ts...>;

String split(String s, char parser, int index);

#endif