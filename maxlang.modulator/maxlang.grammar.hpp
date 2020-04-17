//
//  maxlang.grammar.hpp
//  maxlang.modulator
//
//  Created by charles on 23/03/2020.
//

#ifndef maxlang_grammar_h
#define maxlang_grammar_h

#include <string>
#include <iostream>

#include <tao/pegtl.hpp>
#include <tao/pegtl/contrib/parse_tree.hpp>

namespace pegtl = tao::pegtl;


namespace maxlang
{
    
    struct seps : pegtl::star< pegtl::blank > {};
    
    struct key_bool_true : TAO_PEGTL_KEYWORD( "true" ) {};
    struct key_bool_false : TAO_PEGTL_KEYWORD( "false" ) {};
    
    // Values
    struct double_value
    : pegtl::sor<
    pegtl::seq< pegtl::opt< pegtl::one< '+', '-' > >, seps, pegtl::plus< pegtl::digit > , pegtl::one<'.'>, pegtl::plus<pegtl::digit> >,
    pegtl::seq< pegtl::opt< pegtl::one< '+', '-' > >, seps, pegtl::one<'.'>, pegtl::plus<pegtl::digit> >,
    pegtl::seq< pegtl::opt< pegtl::one< '+', '-' > >, seps, pegtl::plus< pegtl::digit > , pegtl::one<'.'> >,
    pegtl::seq< pegtl::opt< pegtl::one< '+', '-' > >, seps, pegtl::plus< pegtl::digit > >
    >{};
    struct int_value : pegtl::seq< pegtl::opt< pegtl::one< '+', '-' > >, seps, pegtl::plus< pegtl::digit > > {};
    struct bool_value
    : pegtl::sor<
    pegtl::seq<key_bool_true>,
    pegtl::seq<key_bool_false>,
    pegtl::one< '0' >,
    pegtl::one< '1' >
    >{};
    struct positive_int_value : pegtl::seq< pegtl::opt< pegtl::one< '+' > >, seps, pegtl::plus< pegtl::digit > > {};
    
    
    struct modtor_argument_value;
    
    struct list_value : pegtl::list<pegtl::sor<double_value,  int_value>, seps> {};
    struct list_expression : pegtl::seq< seps, pegtl::one<'['>,seps, list_value,seps, pegtl::one<']'>, seps > {};
    
    struct litteral : pegtl::plus<pegtl::alpha> {};
    
    struct lidentifier : pegtl::plus<pegtl::sor<pegtl::alnum,pegtl::one<'-'>,pegtl::one<'_'>,pegtl::one<'.'>>> {};
    
    // modtor specific
    struct modtor_expression;
    struct modtor_argument_value : pegtl::sor<double_value,  int_value,  list_expression, bool_value, modtor_expression, lidentifier > {};
    struct modtor_argument_name : litteral {};
    
    struct modtor_type : litteral {};
    
    struct modtor_argument : pegtl::seq< modtor_argument_name, seps, pegtl::one<'='>, seps, modtor_argument_value, seps > {};
    
    struct modtor_arguments : pegtl::seq<pegtl::one<'('>, seps, pegtl::star<modtor_argument>, seps, pegtl::one<')'>> {};
    
    struct modtor_expression : pegtl::seq<modtor_type, seps, modtor_arguments > {};
    
    struct modtor_start : pegtl::must< modtor_expression, seps, pegtl::eolf > {};
    
    
}  // namespace maxlang

#endif /* maxlang_grammar_h */
