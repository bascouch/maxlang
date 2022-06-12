//
//  maxlang.parsetree.hpp
//  maxlang.modulator
//
//  Created by charles on 23/03/2020.
//

#ifndef maxlang_parsetree_h
#define maxlang_parsetree_h

namespace maxlang {
    
    int modtree_parse_modtor_params(const pegtl::parse_tree::node& n, modtor *&_modtor, t_object * m_ob);
    int modtree_parse_modtor_operator_expression(const pegtl::parse_tree::node& n, modtor *&_modtor, t_object * m_ob, double from_value);
    
    int modtree_parse_modtor_operator_argument(const pegtl::parse_tree::node& n, modtor *&_modtor,std::string p_name, modtor_param *&_modtor_param, t_object * m_ob);
    
    int modtree_parse_modtor_def(const pegtl::parse_tree::node& n, modtor *&_modtor, t_object * m_ob, double from_value);

    int modtree_parse_modtor_operator_expression(const pegtl::parse_tree::node& n, modtor *&_modtor, t_object * m_ob, double from_value)
    {
        if( n.children.empty()) {
            return 0;
        }
        maxlang::print_node( n );
            
        if( n.children.size() < 3)
        {
            object_error(m_ob, "modtor_operator_expression needs 3 child nodes : %s",n.string().c_str());
            return 0;
        }
        
        // parse 3 children :  modtor_operator_argument modtor_operator modtor_operator_argument
        
        pegtl::parse_tree::node *arg_a_node = n.children[0].get();
        pegtl::parse_tree::node *oper_node = n.children[1].get(); // always a modtor_operator
        pegtl::parse_tree::node *arg_b_node = n.children[2].get();
        
        std::string op_str = oper_node->string();
        
        /* create operator modtor */
        modtor_type_enum modtor_type_e = modtor_create_fromstring(op_str,_modtor,from_value);
        
        /* parse the operator arguments and make modtor_param*/
        maxlang::modtor_param * value_a, *value_b;
        auto ret_a = modtree_parse_modtor_operator_argument(*arg_a_node,_modtor,"a",value_a,m_ob);
        auto ret_b = modtree_parse_modtor_operator_argument(*arg_b_node,_modtor,"b",value_b,m_ob);
        if(!ret_a || !ret_b)
       { object_error(m_ob, "error parsing operator arguments %s",n.string().c_str());
        return 0;}
        
        return 1;
        
    }

    int modtree_parse_modtor_operator_argument(const pegtl::parse_tree::node& n, modtor *&_modtor, std::string p_name, modtor_param *&_modtor_param, t_object * m_ob)
    {
        // n : maxlang::modtor_argument_value
        if(n.has_content() && n.type == "maxlang::modtor_operator_argument") {
            // parse children maxlang::double_value || maxlang::int_value || maxlang::list_expression || maxlang::modtor_expression
            if( n.children.empty() || n.children.size()<1 ) {
                return 0;
            }
            pegtl::parse_tree::node *value_node = n.children[0].get();

            if(value_node->type == "maxlang::double_value")
            {
                double v = stod(value_node->string());
                _modtor_param = new modtor_param(v);
                _modtor->setparam(p_name, _modtor_param);

                return 1;
            }
            else if (value_node->type == "maxlang::int_value")
            {
                int v = stoi(value_node->string());
                _modtor_param = new modtor_param(v);
                _modtor->setparam(p_name, _modtor_param);
                return 1;
            }
            else if (value_node->type == "maxlang::modtor_argument_variable")
            {
                /* TODO */

            }else if (value_node->type == "maxlang::modtor_def")
            {
                // new child modtor
                modtor * child_modtor = 0;
                if(modtree_parse_modtor_def(*value_node, child_modtor,m_ob, 0.))
                {
                    _modtor_param = new modtor_param(child_modtor);
                    _modtor->setparam(p_name,_modtor_param);
                    return 1;
                }else
                {
                    object_error(m_ob, "unknown modtor_param value type");
                    return 0;
                }
            }else if (value_node->type == "maxlang::modtor_operator_expression")
            {
                modtor * child_modtor = 0;
                if(modtree_parse_modtor_operator_expression(*value_node, child_modtor,m_ob, 0.))
                {
                    _modtor_param = new modtor_param(child_modtor);
                    _modtor->setparam(p_name,_modtor_param);
                    return 1;
                }else
                {
                    object_error(m_ob, "unknown modtor_param value type");
                    return 0;
                }
            }
        }
        else {
            object_error(m_ob, "modtor_param has no value");
            return 0;
        }
        return  1;
    }

    
    int modtree_parse_modtor_def(const pegtl::parse_tree::node& n, modtor *&_modtor, t_object * m_ob, double from_value)
    {
       if( n.children.empty()) {
           return 0;
       }
       
       // parse children (modtor + modtor) modtor_expression_operator
       if( n.children.size() == 1)
       {
           pegtl::parse_tree::node *type_node = n.children[0].get();
           if(type_node->has_content() && type_node->type == "maxlang::modtor_expression_operator")
               return modtree_parse_modtor_operator_expression(*type_node,_modtor,m_ob,from_value);
           
       }
       
       
       // parse children maxlang::modtor_type + maxlang::modtor_arguments

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
                _modtor->setparam(name, _modtor_param);
                if(name=="seed")
                    _modtor->seed(value_node->string());
                if(name=="sync")
                    _modtor->sync(v);
                return 1;
            }
            else if (value_node->type == "maxlang::int_value")
            {
                int v = stoi(value_node->string());
                _modtor_param = new modtor_param(v);
                _modtor->setparam(name, _modtor_param);
                if(name=="seed")
                    _modtor->seed(value_node->string());
                if(name=="sync")
                    _modtor->sync(v);
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
                    _modtor->setparam(name, _modtor_param);
                    return 1;
                    
                }else
                {
                    object_error(m_ob, "modtor_param list has no value");
                }

            }else if (value_node->type == "maxlang::modtor_def")
            {
                // new child modtor
                modtor * child_modtor = 0;
                if(modtree_parse_modtor_def(*value_node, child_modtor,m_ob, 0.))
                {
                    //// ****** BUGFGG
                    _modtor_param = new modtor_param(child_modtor);
                    _modtor->setparam(name,_modtor_param);
                    return 1;
                }else
                {
                    object_error(m_ob, "unknown modtor_param value type");
                    return 0;
                }
            }else if (value_node->type == "maxlang::modtor_operator_expression")
            {
                modtor * child_modtor = 0;
                if(modtree_parse_modtor_operator_expression(*value_node, child_modtor,m_ob, 0.))
                {
                    _modtor_param = new modtor_param(child_modtor);
                    _modtor->setparam(name,_modtor_param);
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
                _modtor->setparam(name, _modtor_param);
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

        if(modtor_node->type == "maxlang::modtor_def")
        {
            return modtree_parse_modtor_def(*modtor_node,modtor,m_ob,from_value);
        }else if (modtor_node->type == "maxlang::modtor_operator_expression")
        {
            return modtree_parse_modtor_operator_expression(*modtor_node,modtor,m_ob,from_value);
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
    
    
    
}

#endif /* maxlang_parsetree_h */
