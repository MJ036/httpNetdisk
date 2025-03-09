#include <iostream>
#include "Token.hpp"
using std::cout;
using std::endl;

int main()
{
    Token token("Jackie", "12345678");
    cout << "token:" << token.genToken() << endl;
    return 0;
}


