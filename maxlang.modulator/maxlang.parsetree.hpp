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
    template<> struct store<double_value> : std::true_type {};
    template<> struct store<bool_value> : std::true_type {};
    template<> struct store<int_value> : std::true_type {};
    template<> struct store<positive_int_value> : std::false_type {};
    template<> struct store<modtor_type> : std::true_type {};
    template<> struct store<modtor_argument_name> : std::true_type {};
    template<> struct store<modtor_argument_value> : std::true_type {};
    template<> struct store<list_expression> : std::true_type {};
    // clang-format on
    
    void print_node( const pegtl::parse_tree::node& n, const std::string& s );
    
    
   /* int modtree_parse_modtor(const pegtl::parse_tree::node& n, modtor * modtor, t_object * m_ob)
    {
        
        if(n.has_content() && n.type == "maxlang::modtor_type") {
            std::string name = n.string();
            modtor_type_enum modtor_type_e = modtor_create_fromstring(name,modtor);
            if(modtor_type_e == modtor_type_enum::unknown)
            {
                object_error(m_ob, "unknown modtor type %s",name.c_str());
                return 0;
            }
            
        }
        else {
         object_error(m_ob, "modtor has no content");
        }
        return  1;
    } */
    
    // construct the modtree
    void modtree_make( const pegtl::parse_tree::node& n, const std::string& s = "" )
    {
        // detect the root node:
        if( n.is_root() ) {
            std::cout << "ROOT" << std::endl;
        }
        
        else {
            if( n.has_content() ) {
                if(n.type == "maxlang::modtor_type")
                {
                    
                }
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
