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
#include <tao/pegtl/contrib/tracer.hpp>

namespace pegtl = tao::pegtl;
#include "maxlang.utils.hpp"
#include "maxlang.grammar.hpp"



int main(int argc, const char * argv[]) {
    
    std::string name;
    
    
        try {
            pegtl::string_input<> in( std::string("lfo(freq = rand() + 1. mode=4) + rand() / 34"
                                                  ), "source" );
            if(
               //const auto root = pegtl::parse_tree::parse< maxlang::modtor_start, maxlang::store >(in)
               const auto root = pegtl::parse< maxlang::modtor_start, pegtl::nothing, pegtl::tracer >(in)
               )
            {
                //maxlang::print_node( *root );
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
