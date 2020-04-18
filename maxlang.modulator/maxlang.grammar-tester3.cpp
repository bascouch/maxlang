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



int main(int argc, const char * argv[]) {
    
    std::string name;
    
    
        try {
            pegtl::string_input<> in( std::string("coco32"
                                                  ), "source" );
            if( const auto root = pegtl::parse_tree::parse< maxlang::modtor_argument_value_start, maxlang::store >(in) ) {
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
