//
//  maxlang.parsetree.hpp
//  maxlang.modulator
//
//  Created by charles on 23/03/2020.
//

#ifndef maxlang_parsetree_h
#define maxlang_parsetree_h

namespace maxlang {

std::shared_ptr<modtor*> merge_modtor( std::shared_ptr<modtor*> &_modtorA_ptr, std::shared_ptr<modtor*> &_modtorB_ptr, scope* _scope);
    
    int  modtree_parse_modtor_params(const pegtl::parse_tree::node& n, std::shared_ptr<modtor*> &_modtor_ptr, t_object * m_ob, scope * _scope);
    int modtree_parse_modtor_operator_expression(const pegtl::parse_tree::node& n, std::shared_ptr<modtor*> &_modtor_ptr, t_object * m_ob, scope * _scope);
    
    int modtree_parse_modtor_operator_argument(const pegtl::parse_tree::node& n, std::shared_ptr<modtor*> &_modtor_ptr,std::string p_name, std::shared_ptr<modtor_param*> &_modtor_param_ptr, t_object * m_ob, scope * _scope);
    
    int modtree_parse_modtor_def(const pegtl::parse_tree::node& n, std::shared_ptr<modtor*>& _modtor_ptr, t_object * m_ob, scope * modtor_scope);

    int modtree_parse_modtor_operator_expression(const pegtl::parse_tree::node& n, std::shared_ptr<modtor *> &_modtor_ptr, t_object * m_ob, scope * _scope)
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
        std::shared_ptr<modtor *> modtor_ret = modtor_create_fromstring(op_str,_scope);
        _modtor_ptr = modtor_ret;
        
        /* parse the operator arguments and make modtor_param*/
        std::shared_ptr<maxlang::modtor_param *> value_a, value_b;
        
        auto ret_a = modtree_parse_modtor_operator_argument(*arg_a_node,_modtor_ptr,"a",value_a,m_ob,_scope);
        auto ret_b = modtree_parse_modtor_operator_argument(*arg_b_node,_modtor_ptr,"b",value_b,m_ob,_scope);
        if(!ret_a || !ret_b)
       { object_error(m_ob, "error parsing operator arguments %s",n.string().c_str());
        return 0;}
        
        return 1;
        
    }

    int modtree_parse_modtor_operator_argument(const pegtl::parse_tree::node& n, std::shared_ptr<maxlang::modtor *> &_modtor_ptr, std::string p_name, std::shared_ptr<maxlang::modtor_param *> &_modtor_param_ptr, t_object * m_ob, scope * _scope)
    {
        // n : maxlang::modtor_argument_value
        maxlang::modtor_param* _modtor_param;
        maxlang::modtor * _modtor = *_modtor_ptr;
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
                std::shared_ptr<maxlang::modtor_param *> _modtor_param_ptr_int = std::make_shared<modtor_param*>(_modtor_param);
               
                _modtor_param_ptr = _modtor_param_ptr_int;
                _modtor->setparam(p_name, _modtor_param_ptr);

                return 1;
            }
            else if (value_node->type == "maxlang::int_value")
            {
                int v = stoi(value_node->string());
                _modtor_param = new modtor_param(v);
                _modtor_param_ptr = std::make_shared<modtor_param*>(_modtor_param);
                _modtor->setparam(p_name, _modtor_param_ptr);
                return 1;
            }
            else if (value_node->type == "maxlang::modtor_argument_variable")
            {
                /* TODO */

            }else if (value_node->type == "maxlang::modtor_def")
            {
                // new child modtor
                modtor * child_modtor = 0;
                std::shared_ptr<modtor*> child_modtor_ptr;
                
                if(modtree_parse_modtor_def(*value_node, child_modtor_ptr,m_ob, _scope))
                {
                    modtor_param * _modtor_param  = new modtor_param(child_modtor_ptr);
                    _modtor_param_ptr = std::make_shared<modtor_param*>(&_modtor_param);
                    _modtor->setparam(p_name,_modtor_param_ptr);
                    return 1;
                }else
                {
                    object_error(m_ob, "unknown modtor_param value type");
                    return 0;
                }
            }else if (value_node->type == "maxlang::modtor_operator_expression")
            {
                modtor * child_modtor = 0;
                std::shared_ptr<modtor*> child_modtor_ptr;
                if(modtree_parse_modtor_operator_expression(*value_node, child_modtor_ptr,m_ob, _scope))
                {
                    _modtor_param = new modtor_param(child_modtor_ptr);
                    _modtor_param_ptr = std::make_shared<modtor_param*>(_modtor_param);
                    _modtor->setparam(p_name,_modtor_param_ptr);
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

    
int modtree_parse_modtor_def(const pegtl::parse_tree::node& n, std::shared_ptr<modtor *>& _modtor_ptr, t_object * m_ob, scope * _scope)
    {
       if( n.children.empty()) {
           return 0;
       }
        
        if(*_modtor_ptr == nullptr)
        {
            _modtor_ptr = std::make_shared<modtor *>();
        }
       
       // parse children (modtor + modtor) modtor_expression_operator
       if( n.children.size() == 1)
       {
           pegtl::parse_tree::node *type_node = n.children[0].get();
           if(type_node->has_content() && type_node->type == "maxlang::modtor_expression_operator")
               return modtree_parse_modtor_operator_expression(*type_node,_modtor_ptr,m_ob,_scope);
           
       }
       
       
       // parse children maxlang::modtor_type + maxlang::modtor_arguments

        pegtl::parse_tree::node *type_node = n.children[0].get();
        pegtl::parse_tree::node *args_node = n.children[1].get();
        
        
        // parse children maxlang::modtor_type
        if(type_node->has_content() && type_node->type == "maxlang::modtor_type") {
            std::string name = type_node->string();
            /// BUG
            std::shared_ptr<modtor *> m = modtor_create_fromstring(name,_scope);
            _modtor_ptr = m;
            
            if(*m == nullptr)
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
                    if(!modtree_parse_modtor_params( *up, _modtor_ptr, m_ob, _scope ))
                    {
        
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

    
    
    int modtree_parse_modtor_param_value(const pegtl::parse_tree::node& n, std::shared_ptr<modtor *> *_modtor, std::string name, modtor_param *_modtor_param, t_object * m_ob, scope * _scope)
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
                std::shared_ptr<modtor_param *> _modtor_param_ptr = std::make_shared<modtor_param *> (_modtor_param);
                (**_modtor)->setparam(name, _modtor_param_ptr);
                if(name=="seed")
                    (**_modtor)->seed(value_node->string());
                if(name=="sync")
                    (**_modtor)->sync(v);
                return 1;
            }
            else if (value_node->type == "maxlang::int_value")
            {
                int v = stoi(value_node->string());
                _modtor_param = new modtor_param(v);
                std::shared_ptr<modtor_param *> _modtor_param_ptr = std::make_shared<modtor_param *> (_modtor_param);
                (**_modtor)->setparam(name, _modtor_param_ptr);
                if(name=="seed")
                    (**_modtor)->seed(value_node->string());
                if(name=="sync")
                    (**_modtor)->sync(v);
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
                    std::shared_ptr<modtor_param *> _modtor_param_ptr = std::make_shared<modtor_param *> (_modtor_param);
                    (**_modtor)->setparam(name, _modtor_param_ptr);
                    return 1;
                    
                }else
                {
                    object_error(m_ob, "modtor_param list has no value");
                }

            }else if (value_node->type == "maxlang::modtor_def")
            {
                // new child modtor
                modtor * child_modtor = 0;
                std::shared_ptr<modtor *> child_modtor_ptr;
                std::shared_ptr<modtor *> def = modtor_create_fromstring("add", _scope);
                child_modtor_ptr = def ;
                if(modtree_parse_modtor_def(*value_node, child_modtor_ptr,m_ob, _scope))
                {
                    //// ****** BUGFGG
                    _modtor_param = new modtor_param(child_modtor_ptr);
                    std::shared_ptr<modtor_param *> _modtor_param_ptr = std::make_shared<modtor_param *> (_modtor_param);
                    (**_modtor)->setparam(name,_modtor_param_ptr);
                    return 1;
                }else
                {
                    object_error(m_ob, "unknown modtor_param value type");
                    return 0;
                }
            }else if (value_node->type == "maxlang::modtor_operator_expression")
            {
                modtor * child_modtor = 0;
                std::shared_ptr<modtor *> child_modtor_ptr ;
                if(modtree_parse_modtor_operator_expression(*value_node, child_modtor_ptr,m_ob, _scope))
                {
                    _modtor_param = new modtor_param(child_modtor_ptr);
                    std::shared_ptr<modtor_param *> _modtor_param_ptr = std::make_shared<modtor_param *> (_modtor_param);
                    (**_modtor)->setparam(name,_modtor_param_ptr);
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
                std::shared_ptr<modtor_param *> _modtor_param_ptr = std::make_shared<modtor_param *> (_modtor_param);
                (**_modtor)->setparam(name, _modtor_param_ptr);
                if(name=="seed")
                    (**_modtor)->seed(ident);
                return 1;
            }
        }
        else {
            object_error(m_ob, "modtor_param has no value");
            return 0;
        }
        return  1;
    }
    
    int modtree_parse_modtor_params(const pegtl::parse_tree::node& n, std::shared_ptr<modtor *> *_modtor, t_object * m_ob, scope * _scope)
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
            if(modtree_parse_modtor_param_value(*value_node,_modtor,name,value,m_ob,_scope))
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
    int modtree_make( const pegtl::parse_tree::node& n, std::shared_ptr<modtor *> &modtor_ptr, t_object * m_ob, scope * _scope)
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
            return modtree_parse_modtor_def(*modtor_node,modtor_ptr,m_ob,_scope);
        }else if (modtor_node->type == "maxlang::modtor_operator_expression")
        {
            return modtree_parse_modtor_operator_expression(*modtor_node,modtor_ptr,m_ob,_scope);
        }
        return 1;
    }
    
    // construct the modtor argument value tree
    int valtree_make( const pegtl::parse_tree::node& n, std::shared_ptr<modtor *> *modtor, std::string arg_name, t_object * m_ob, scope * _scope)
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
            return modtree_parse_modtor_param_value(*value_node,modtor,arg_name,value,m_ob,_scope);
        }
        return 1;
    }


    

// merge modtor params
std::shared_ptr<modtor_param*> merge_modtor_param(std::shared_ptr<modtor_param*> &paramA_ptr, std::shared_ptr<modtor_param*> &paramB_ptr, scope * _scope)
    {
        maxlang::modtor_param* paramA = *paramA_ptr;
        maxlang::modtor_param* paramB = *paramB_ptr;
        //enum modtor_param_type { e_int, e_double, e_list, e_modtor, e_string };
        if(paramA->_type==paramB->_type && paramA->_type == e_modtor)
        {
            maxlang::modtor * _modtor;
            std::shared_ptr<modtor*>  _modtorA_ptr = paramA->getmodtor();
            maxlang::modtor * _modtorA = *_modtorA_ptr;
            std::shared_ptr<modtor*>  _modtorB_ptr = paramB->getmodtor();
            maxlang::modtor_param * _modtor_param_returned;
            _modtorA->merge_modtor(_modtorB_ptr, _scope);
            _modtor_param_returned = new maxlang::modtor_param(std::make_shared<modtor*>(_modtorA));
            
            std::shared_ptr<maxlang::modtor_param*> returned_param  = std::make_shared<maxlang::modtor_param*>(_modtor_param_returned);
            
            
            return returned_param;
        }
        else
        {
            // RUDE
            /* create operator modtor */
            std::shared_ptr<modtor*> new_modtor_ptr = modtor_create_fromstring("interpolate",_scope);
            
            
            
            (*new_modtor_ptr)->setparam("a", paramA_ptr);
            (*new_modtor_ptr)->setparam("b", paramB_ptr);
            
            //std::shared_ptr<maxlang::modtor_param*> returned_param;
            //std::shared_ptr<maxlang::modtor_param*> returned_param  = std::make_shared<maxlang::modtor_param*>(new modtor_param(new_modtor_ptr));
            
            std::shared_ptr<maxlang::modtor_param*> returned_param;
            return returned_param;
        }
        
        
    }


    // merge modtor
std::shared_ptr<modtor *> merge_modtor( std::shared_ptr<modtor *> &modtor_A_ptr, std::shared_ptr<modtor *> &modtor_B_ptr, scope * _scope)
    {
        maxlang::modtor * _modtor = *modtor_A_ptr ;
    return _modtor->merge_modtor(modtor_B_ptr,_scope);
    }
    
    
    
}

#endif /* maxlang_parsetree_h */
