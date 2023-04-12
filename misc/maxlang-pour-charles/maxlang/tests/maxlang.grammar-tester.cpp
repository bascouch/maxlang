//
//  main.cpp
//  maxlang.grammar-tester
//
//  Created by charles on 20/03/2020.
//

#include <string>
#include <iostream>

class myclass
{
    public :
    myclass()
    {
        value = "toto";
    }
    
    myclass(std::string tutu)
    {
        value = tutu;
    }
    ~myclass()
    {
        
    }
    
    std::string value;
};

int test_create_class(myclass *&_m)
{
    _m = new myclass("caca");
    return 1;
}

int main(int argc, const char * argv[]) {
    
    myclass * m;
    //std::cout << m->value << std::endl;
    test_create_class(m);
    std::cout << m->value << std::endl;
        return 0;
    }
