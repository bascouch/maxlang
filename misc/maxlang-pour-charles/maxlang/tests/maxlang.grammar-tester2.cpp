//
//  main.cpp
//  maxlang.grammar-tester
//
//  Created by charles on 20/03/2020.
//

#include <string>
#include <iostream>

#include <tao/pegtl.hpp>
#include <tao/pegtl/contrib/parse_tree.hpp>

namespace pegtl = tao::pegtl;
#include "maxlang.utils.hpp"
#include "maxlang.grammar.hpp"
#include "maxlang.modtree.hpp"
#include "maxlang.parsetree.hpp"

int test_create_modtor(maxlang::modtor * _m)
{
    _m = new maxlang::m_lfo();
    return 1;
}

int main(int argc, const char * argv[]) {
    
    std::string name;
    
    maxlang::modtor * modtor;
    test_create_modtor(modtor);
    
    printf("test %f\n", modtor->params[0]._value_d);
    
    
        try {
            pegtl::string_input<> in( std::string("lfo(cacR=.4 pipi=[34 35 rando(min=3 max=5)] coco=45 pink=randi(min=0 max=1))\n"
                                                  "lfo(c=rad(c=45))\n"
                                                  "lfo(caca=3.4 pipi=false coco=434232.1 pink=54)\n"
                                                  "lfo(caca=3.4 pipi=false coco=434232.1)\n"
                                                  
                                                  "caca(param= 3)\n"
                                                  
                                                  
                                                  "lfo(test=3)\n"
                                                  "lfo(caca=3.4 pipi=false coco=434232.1 pink=randi(min=0 max=1))\n"
                                                  "lfo( )\n"
                                                  ), "source" );
            if( const auto root = pegtl::parse_tree::parse< maxlang::modtor_start, maxlang::store >(in) ) {
                maxlang::print_node( *root );
            }
            else {
                std::cout << "PARSE FAILED" << std::endl;
            }
        }
        catch( const std::exception& e ) {
            std::cout << "PARSE FAILED WITH EXCEPTION: " << e.what() << std::endl;
        }
        return 0;
    }
