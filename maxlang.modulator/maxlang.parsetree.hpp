//
//  maxlang.parsetree.hpp
//  maxlang.modulator
//
//  Created by charles on 23/03/2020.
//

#ifndef maxlang_parsetree_h
#define maxlang_parsetree_h

namespace maxlang {

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
    template<> struct store<double_value> : std::false_type {};
    template<> struct store<bool_value> : std::false_type {};
    template<> struct store<int_value> : std::false_type {};
    template<> struct store<positive_int_value> : std::false_type {};
    template<> struct store<modtor_type> : std::true_type {};
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
}

#endif /* maxlang_parsetree_h */
