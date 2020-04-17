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
    template<> struct store<modtor_expression> : std::true_type {};
    template<> struct store<lidentifier> : std::true_type {};
    template<> struct store<modtor_type> : std::true_type {};
    template<> struct store<modtor_arguments> : std::true_type {};
    template<> struct store<modtor_argument> : std::true_type {};
    template<> struct store<modtor_argument_name> : std::true_type {};
    template<> struct store<modtor_argument_value> : std::true_type {};
    template<> struct store<list_expression> : std::true_type {};
    // clang-format on
    
    void print_node( const pegtl::parse_tree::node& n, const std::string& s );
    
    int modtree_parse_modtor_params(const pegtl::parse_tree::node& n, modtor *&_modtor, t_object * m_ob);
    
   int modtree_parse_modtor(const pegtl::parse_tree::node& n, modtor *&_modtor, t_object * m_ob, double from_value)
    {
        // parse children maxlang::modtor_type + maxlang::modtor_arguments
        if( n.children.empty() || n.children.size()<2 ) {
            return 0;
        }
        
        pegtl::parse_tree::node *type_node = n.children[0].get();
        pegtl::parse_tree::node *args_node = n.children[1].get();
        
        
        // parse children maxlang::modtor_type
        if(type_node->has_content() && type_node->type == "maxlang::modtor_type") {
            std::string name = type_node->string();
            modtor_type_enum modtor_type_e = modtor_create_fromstring(name,_modtor,from_value);
            if(modtor_type_e == modtor_type_enum::unknown)
            {
                object_error(m_ob, "unknown modtor type %s",name.c_str());
                return 0;
            }
        }
        else {
         object_error(m_ob, "modtor has no content");
            return 0;
        }
        
        // parse children maxlang::modtor_arguments
        if(args_node->has_content() && args_node->type == "maxlang::modtor_arguments") {
            if( !args_node->children.empty() ) {
                for( auto& up : args_node->children ) {
                    if(!modtree_parse_modtor_params( *up, _modtor, m_ob ))
                    {
                        delete _modtor;
                        _modtor = NULL;
                        return 0;
                    }
                }
            }
        }
        else {
            object_error(m_ob, "modtor has no arguments");
            return 0;
        }
        return  1;
    }
    
    int modtree_parse_modtor_param_value(const pegtl::parse_tree::node& n, modtor *&_modtor, std::string name, modtor_param *&_modtor_param, t_object * m_ob)
    {
        // n : maxlang::modtor_argument_value
        if(n.has_content() && n.type == "maxlang::modtor_argument_value") {
            // parse children maxlang::double_value || maxlang::int_value || maxlang::list_expression || maxlang::modtor_expression
            if( n.children.empty() || n.children.size()<1 ) {
                return 0;
            }
            pegtl::parse_tree::node *value_node = n.children[0].get();

            if(value_node->type == "maxlang::double_value")
            {
                double v = stod(value_node->string());
                _modtor_param = new modtor_param(v);
                _modtor->setparam(name, *_modtor_param);
                if(name=="seed")
                    _modtor->seed(value_node->string());
                return 1;
            }
            else if (value_node->type == "maxlang::int_value")
            {
                int v = stoi(value_node->string());
                _modtor_param = new modtor_param(v);
                _modtor->setparam(name, *_modtor_param);
                if(name=="seed")
                    _modtor->seed(value_node->string());
                return 1;
            }
            else if (value_node->type == "maxlang::list_expression")
            {
                // get all children maxlang::double_value or maxlang::int_value
                if( !value_node->children.empty() ) {
                    std::vector<double> list;
                    
                    for( auto& child_v : value_node->children )
                        list.push_back(stod(child_v->string()));
                    _modtor_param = new modtor_param(list);
                    _modtor->setparam(name, *_modtor_param);
                    return 1;
                    
                }else
                {
                    object_error(m_ob, "modtor_param list has no value");
                }

            }else if (value_node->type == "maxlang::modtor_expression")
            {
                // new child modtor
                modtor * child_modtor = 0;
                if(modtree_parse_modtor(*value_node, child_modtor,m_ob, 0.))
                {
                    _modtor_param = new modtor_param(child_modtor);
                    _modtor->setparam(name, *_modtor_param);
                    return 1;
                }else
                {
                    object_error(m_ob, "unknown modtor_param value type");
                    return 0;
                }
            }else if (value_node->type == "maxlang::lidentifier")
            {
                std::string ident = value_node->string();
                _modtor_param = new modtor_param(ident);
                _modtor->setparam(name, *_modtor_param);
                if(name=="seed")
                    _modtor->seed(ident);
                return 1;
            }
        }
        else {
            object_error(m_ob, "modtor_param has no value");
            return 0;
        }
        return  1;
    }
    
    int modtree_parse_modtor_params(const pegtl::parse_tree::node& n, modtor *&_modtor, t_object * m_ob)
    {
        
        if(n.has_content() && n.type == "maxlang::modtor_argument") {
            
            // parse children maxlang::modtor_argument_name + maxlang::modtor_argument_value
            if( n.children.empty() && n.children.size()<2 ) {
                return 0;
            }
            
            pegtl::parse_tree::node *name_node = n.children[0].get();
            pegtl::parse_tree::node *value_node = n.children[1].get();
            
            std::string name = name_node->string();
            maxlang::modtor_param * value;
            
            // name : special modtor argument to ref a specific sub-modtor
            if(name == "name")
            {
                
            }
            
            // get into value node
            if(modtree_parse_modtor_param_value(*value_node,_modtor,name,value,m_ob))
            {
               // modtor->setparam(name, *value);
                return 1;
            }else
            {
                object_error(m_ob, "error parsing param value for %s",name.c_str());
                return 0;
            }
            
        }
        else {
            object_error(m_ob, "modtor has no content");
            return 0;
        }
        return  1;
    }
    
    // construct the modtree
    int modtree_make( const pegtl::parse_tree::node& n, modtor *&modtor, t_object * m_ob, double from_value)
    {
        // detect the root node:
        if( !n.is_root() ) {
            return 0;
        }
        
        // get into child which should be maxlang::modtor_expression
        if( n.children.empty() || n.children.size()<1 ) {
            return 0;
        }
        
        pegtl::parse_tree::node *modtor_node = n.children[0].get();

        if(modtor_node->type == "maxlang::modtor_expression")
        {
            return modtree_parse_modtor(*modtor_node,modtor,m_ob,from_value);
        }
        return 1;
    }
    
    // construct the modtor argument value tree
    int valtree_make( const pegtl::parse_tree::node& n, modtor *&modtor, std::string arg_name, t_object * m_ob, double from_value)
    {
        // detect the root node:
        if( !n.is_root() ) {
            return 0;
        }
        
        // get into child which should be maxlang::modtor_expression
        if( n.children.empty() || n.children.size()<1 ) {
            return 0;
        }
        
        pegtl::parse_tree::node *value_node = n.children[0].get();
        
        maxlang::modtor_param * value;
        
        if(value_node->type == "maxlang::modtor_argument_value")
        {
            //     int modtree_parse_modtor_param_value(const pegtl::parse_tree::node& n, modtor *&_modtor, std::string name, modtor_param *&_modtor_param, t_object * m_ob)
            return modtree_parse_modtor_param_value(*value_node,modtor,arg_name,value,m_ob);
        }
        return 1;
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
