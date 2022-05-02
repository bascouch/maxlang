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
//#include <tao/pegtl/contrib/analyze.hpp>

namespace pegtl = tao::pegtl;
#include "maxlang.utils.hpp"
#include "maxlang.grammar.hpp"



int main(int argc, const char * argv[]) {
    
    std::string name;
    
    
        try {
            pegtl::string_input<> in( std::string(//"rand(name=myrand min=72 max=83 freq=8 varifreq=0 walk=1 seed=DAF)"
                                                  //" env(list=[28. 0.1 57. 0.3 50 0.5 30. 0.3 0.] time=100 varitime=1 loop=0 segcurve=-0.44 play=lfo(name = play-rate mode=2 min=0 max=1 freq=2 varifreq=1 pw=0.9) add=choicei(name = add-rate list=[41 44 46] freq=0.5 varifreq=1) mul=0.5)"
                                                  //"xfade(a=lfo ( freq=randi(freq= 0.5 min=3 max=24) varifreq=0. mode=randi(freq= 0.1 min=1 max=5) min=choice(list=[50 52 47 51 34 31 84] freq=3 add=-12) max=choice(list=[50 52 47 51 34 31 84 61 65] freq=3 add=12) curve=0.74) b= choicei ( freq=3.4 list=[0. 0.1 0.2 0.3 0.9] mul=lfo(min=36 max=64 freq=12) add=36) fade=lfo(mode=2 freq=3))"
                                                  "lfo(freq = (rand() + 1.) mode=4) + (rand() / 34)"
                                                  //"(lfo(min=0 max=1 count=10) * randi())*line(time=10000)"
                                                  //"($fff + rand())"
                                                  //"(lfo(min=0 max=1 count=10) * randi())*line(time=10000)"
                                                  //"$fff + rand()"
                                                  //"rand()+3"
                                                  //"lfo(freq = (rand() + 1.) mode=4) + (rand() / 34)"
                                                  ), "source" );
            if(
               const auto root = pegtl::parse_tree::parse< maxlang::modtor_start, maxlang::store >(in)
               //const auto root = pegtl::parse< maxlang::modtor_start, pegtl::nothing, pegtl::tracer >(in)
               )
            {
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
