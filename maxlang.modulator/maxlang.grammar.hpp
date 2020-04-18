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
    
    struct lidentifier : pegtl::plus<pegtl::sor<pegtl::alnum,pegtl::one<'-'>,pegtl::one<'_'>>> {};
    
    // modtor specific
    struct modtor_expression;
    struct modtor_argument_value : pegtl::sor<double_value,  int_value,  list_expression, bool_value, modtor_expression, lidentifier > {};
    struct modtor_argument_name : litteral {};
    
    struct modtor_type : litteral {};
    
    struct modtor_argument : pegtl::seq< modtor_argument_name, seps, pegtl::one<'='>, seps, modtor_argument_value, seps > {};
    
    struct modtor_arguments : pegtl::seq<pegtl::one<'('>, seps, pegtl::star<modtor_argument>, seps, pegtl::one<')'>> {};
    
    struct modtor_expression : pegtl::seq<modtor_type, seps, modtor_arguments > {};
    
    struct modtor_start : pegtl::must< modtor_expression, seps, pegtl::eolf > {};
    
    struct modtor_argument_value_start : pegtl::must< modtor_argument_value, seps, pegtl::eolf > {};
    
    
    
    // Rules for constructing the parse tree
    //
    
    template< typename Rule >
    struct action
    {};
    
    // Specialisation of the user-defined action to do
    // something when the 'name' rule succeeds; is called
    // with the portion of the input that matched the rule.
    
    template<>
    struct action< modtor_argument >
    {
        template< typename Input >
        static void apply( const Input& in, std::string& v )
        {
            v += " " + in.string();
            post("modtor_argument : %s",in.string().c_str());
        }
    };
    
    template<>
    struct action< modtor_type >
    {
        template< typename Input >
        static void apply( const Input& in, std::string& v )
        {
            v += " " + in.string();
            post("modtor_type : %s",in.string().c_str());
        }
    };
    
    template<>
    struct action< modtor_argument_value >
    {
        template< typename Input >
        static void apply( const Input& in, std::string& v )
        {
            v += " " + in.string();
            post("modtor_argument_value : %s",in.string().c_str());
        }
    };
    
    template<>
    struct action< modtor_argument_name >
    {
        template< typename Input >
        static void apply( const Input& in, std::string& v )
        {
            v += " " + in.string();
            post("modtor_argument_name : %s",in.string().c_str());
        }
    };
    
    template<>
    struct action< list_value >
    {
        template< typename Input >
        static void apply( const Input& in, std::string& v )
        {
            v += " " + in.string();
            post("modtor_list_value : %s",in.string().c_str());
        }
    };
    
    
    
    // by default, nodes are not generated/stored
    template< typename > struct store : std::false_type {};
    // select which rules in the grammar will produce parse tree nodes:
    template<> struct store<double_value> : std::true_type {};
    template<> struct store<bool_value> : std::true_type {};
    template<> struct store<int_value> : std::true_type {};
    template<> struct store<positive_int_value> : std::false_type {};
    template<> struct store<modtor_expression> : std::true_type {};
    template<> struct store<lidentifier> : std::true_type {};
    template<> struct store<modtor_type> : std::true_type {};
    template<> struct store<modtor_arguments> : std::true_type {};
    template<> struct store<modtor_argument> : std::true_type {};
    template<> struct store<modtor_argument_name> : std::true_type {};
    template<> struct store<modtor_argument_value> : std::true_type {};
    template<> struct store<list_expression> : std::true_type {};
    // clang-format on
    
    void print_node( const pegtl::parse_tree::node& n, const std::string& s = "" )
    {
        // detect the root node:
        if( n.is_root() ) {
            std::cout << "ROOT" << std::endl;
        }
        
        else {
            if( n.has_content() ) {
                std::cout << s << n.type << " \"" << n.string() << "\" at " << n.begin() << " to " << n.end() << std::endl;
            }
            else {
                std::cout << s << n.source << " at " << n.begin() << std::endl;
            }
        }
        // print all child nodes
        if( !n.children.empty() ) {
            const auto s2 = s + "  ";
            for( auto& up : n.children ) {
                print_node( *up, s2 );
            }
        }
    }
    
}  // namespace maxlang

#endif /* maxlang_grammar_h */
