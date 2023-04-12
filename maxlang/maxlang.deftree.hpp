//
//  maxlang.deftree.hpp
//  maxlang.modulator
//
//  Created by charles on 24/03/2023.
//

#ifndef maxlang_deftree_h
#define maxlang_deftree_h

namespace maxlang {

class modtordef_param;

class modtordef
{
    public :
    
    /* members */
    
    std::map<std::string,modtordef_param> params;
    std::string modtor_class;
    modtor_type_enum modtor_type_e;
    
    std::string modtordef_str;
    
    /* methods */
    
    modtordef(modtor_type_enum _modtor_type_e, std::string _class);
    
    ~modtordef();
    
    bool operator==(modtordef a);
    bool operator<=(modtordef a); /* check if modtorparam are modtor type and same class*/

    //modtordef& operator=(modtordef other);
    
    std::string get_modtordef_str();
    int setparam(std::string name, modtordef_param value);
    modtordef merge_modtordef(modtordef B);

};


class modtordef_param
{
    public :
    
    /* members */
    
    modtor_param_type _type;
    
    double _value_d;
    int _value_i;
    std::vector<double> _list;
    modtordef _modtordef = modtordef(modtor_type_enum::add,"add");;
    std::string _string;
    
    /* methods */
    modtordef_param();
    modtordef_param(double v);
    modtordef_param(int v);
    modtordef_param(std::vector<double> l);
    modtordef_param(modtordef m);
    modtordef_param(std::string s);
    
    ~modtordef_param();
    
    bool operator==(modtordef_param a);
    bool operator<=(modtordef_param a); /* check if modtorparam are modtor type and same class*/
    
    modtordef_param merge_modtordef_param(modtordef_param B);
    
    std::string to_str();
};
    
modtordef_param::modtordef_param()
{
    _type = modtor_param_type::e_double;
    _value_d = 0.;
}
modtordef_param::modtordef_param(double v){
    
    _type = modtor_param_type::e_double;
    _value_d = v;
}
    
modtordef_param::modtordef_param(int v){
    
    _type = modtor_param_type::e_int;
    _value_i = v;
}
    
modtordef_param::modtordef_param(std::vector<double> l){
    
    _type = modtor_param_type::e_list;
    _list = l;
}
    
modtordef_param::modtordef_param(modtordef m){
    
    _type = modtor_param_type::e_modtor;
    _modtordef = m;
}
    
modtordef_param::modtordef_param(std::string s){
    
    _type = modtor_param_type::e_string;
    _string = s;
}
    
modtordef_param::~modtordef_param()
{
    /*
    if(modtor_param_type::e_modtor && _modtordef)
        delete _modtordef;
    if(modtor_param_type::e_list)
        _list.clear();
    */
    
}

bool modtordef_param::operator<=(modtordef_param a)
{
    bool condA = (_type == modtor_param_type::e_modtor) && (a._type == modtor_param_type::e_modtor);
    
    if(condA)
    {
        bool condB = (_modtordef.modtor_type_e == a._modtordef.modtor_type_e);
        return condB;
    }
    else
        return false;
    
}


bool modtordef_param::operator==(modtordef_param a)
{
    if(_type != a._type)
        return false;
    
    if(_type == modtor_param_type::e_double)
    {
        return _value_d == a._value_d;
    }
 
    if(_type == modtor_param_type::e_int )
    {
        return _value_i == a._value_i;
    }
    
    if(_type == modtor_param_type::e_list )
    {
        return _list == a._list;
    }
    if(_type == modtor_param_type::e_string )
    {
        return _string == a._string;
    }
    if(_type == modtor_param_type::e_modtor )
    {
        return _modtordef == a._modtordef;
        
    }
}
    
std::string modtordef_param::to_str()
{
    std::string out_str;
    //_modtordef.
   
    {
        if(_type == modtor_param_type::e_double)
        {
            out_str = std::to_string(_value_d);
        }
            
        if(_type == modtor_param_type::e_int )
        {
            out_str = std::to_string(_value_i);
        }
        
        if(_type == modtor_param_type::e_list )
        {
            unsigned long _size = _list.size();
            out_str = "[ ";
            for(unsigned int i = 0; i < _size; i++)
            {
                  out_str += std::to_string(_list[i]);
                  out_str += " ";
            }
            out_str += "]";
        }
        if(_type == modtor_param_type::e_string )
        {
            out_str = _string;
        }
        if(_type == modtor_param_type::e_modtor )
        {
            out_str = _modtordef.get_modtordef_str();
        }
            
    }
    
    out_str += " ";
    
    return out_str;
}



modtordef::modtordef(modtor_type_enum _modtor_type_e, std::string _class)
{
    /* clean better shortcuts management */
    if(_modtor_type_e == modtor_type_enum::add)
        _class = "add";
    if(_modtor_type_e == modtor_type_enum::minus)
        _class = "minus";
    if(_modtor_type_e == modtor_type_enum::mul)
        _class = "mul";
    if(_modtor_type_e == modtor_type_enum::div)
        _class = "div";
    
    modtor_class = _class;
    modtor_type_e = _modtor_type_e;
    
    /* modtor def */
}

/*modtordef& modtordef::operator=(modtordef other)
{
    return *this;
}*/

modtordef::~modtordef()
{
    
}
    

bool modtordef::operator==(modtordef a)
{
    bool result = true;
    
    if( modtor_type_e != a.modtor_type_e)
        return false;
    
    for (std::map<std::string,modtordef_param>::iterator it=params.begin(); it!=params.end(); ++it)
    {
        if(a.params.count(it->first) > 0)
        {
            result = result && (a.params[it->first] == params[it->first] );
        }
    }
    
    return result;
}


bool modtordef::operator<=(modtordef a)
{
    return (this->modtor_type_e == a.modtor_type_e);
}

std::string modtordef::get_modtordef_str()
{
    modtordef_str = "";
    modtordef_str += modtor_class;
    modtordef_str += "( ";
    
    
    for (std::map<std::string,modtordef_param>::iterator it=params.begin(); it!=params.end(); ++it)
    {
        modtordef_str += it->first;
        modtordef_str += "=";
        modtordef_str += it->second.to_str();

    }
    
    

    modtordef_str += " )";
    return modtordef_str;
}



int modtordef::setparam(std::string name, modtordef_param value)
{
    // check if param is a refname
    if(name=="name")
    {
        //modtor_refname = value->getstring();
        params[name] = value;
        return 1;
    }
    // common seed param
    if(name=="seed")
    {
        params[name] = value;
        return 1;
    }
    // common seed param
    if(name=="sync")
    {
        params[name] = value;
        return 1;
    }
    // else specific modtor param
    if ( params.find(name) == params.end() )
    { // not found
        params[name] = value;
        return 0;
    } else {
        // found
        params.erase(name); // TEST bug
        params[name] = value;
        //std::cout << value._type << std::endl;
    }
    
    return 1;
}




modtordef_param modtordef_param::merge_modtordef_param(modtordef_param B)
{
    if(*this == B)
    {
        return modtordef_param(*this);
    }
    else
    {
        /* check if same modtor type */
        if(*this <= B)
        {
            modtordef out = _modtordef.merge_modtordef(B._modtordef);
            return modtordef_param(out);
            
        }else
        {
            /* do the shit */
            modtordef xfade = modtordef(modtor_type_enum::interpol,"interpol");
            xfade.setparam("a", *this);
            xfade.setparam("b", B);
            xfade.setparam("id",std::string("interp00"));
            
            return modtordef_param(xfade);
            
        }
        
        
        
    }
}
    
modtordef modtordef::merge_modtordef(modtordef B)
{
    std::string _key;
    
    if(*this == B)
    {
        return modtordef(*this);
    }
    else
    {

        /* check if same modtor type */
        if(*this <= B)
        {
            /* out modtordef is copied from this*/
            modtordef out = modtordef(*this);
            modtordef in = modtordef(B);
            
            for (std::map<std::string,modtordef_param>::iterator it=params.begin(); it!=params.end(); ++it)
            {
                _key = it->first;
                if(in.params.count(_key))
                {
                    /* param in B exists so merge the param*/
                    out.params[_key] = params[_key].merge_modtordef_param(in.params[_key]);
                }
            }
            
            return out;
            
        }else
        {
            /* do the shit */
            modtordef xfade = modtordef(modtor_type_enum::interpol,"interpol");
            xfade.setparam("a", *this);
            xfade.setparam("b", B);
            xfade.setparam("id",std::string("interp00"));
            
            return xfade;
            
        }
        
    }
    
}







int deftree_parse_modtor_params(const pegtl::parse_tree::node& n, modtordef *&_modtordef, t_object * m_ob);

int deftree_parse_modtor_operator_argument(const pegtl::parse_tree::node& n, modtordef *&_modtordef,std::string p_name, modtordef_param *&_modtordef_param, t_object * m_ob);

int deftree_parse_modtor_def(const pegtl::parse_tree::node& n, modtordef *&_modtordef, t_object * m_ob);

int deftree_parse_modtor_operator_expression(const pegtl::parse_tree::node& n, modtordef *&_modtordef, t_object * m_ob)
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
    
    /* check and dont create operator modtor */
    modtor * _modtor_dum;
    modtor_type_enum modtor_type_e = modtor_create_fromstring(op_str,_modtor_dum,false);
    
    /* CREATE _modtordef*/
    _modtordef = new modtordef(modtor_type_e,op_str);
    
    /* parse the operator arguments and make modtor_param*/
    maxlang::modtordef_param * value_a, *value_b;
    auto ret_a = deftree_parse_modtor_operator_argument(*arg_a_node,_modtordef,"a",value_a,m_ob);
    auto ret_b = deftree_parse_modtor_operator_argument(*arg_b_node,_modtordef,"b",value_b,m_ob);
    if(!ret_a || !ret_b)
   { object_error(m_ob, "error parsing operator arguments %s",n.string().c_str());
    return 0;}
    
    return 1;
    
}

int deftree_parse_modtor_operator_argument(const pegtl::parse_tree::node& n, modtordef *&_modtordef, std::string p_name, modtordef_param *&_modtordef_param, t_object * m_ob)
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
            _modtordef_param = new modtordef_param(v);
            _modtordef->setparam(p_name, *_modtordef_param);

            return 1;
        }
        else if (value_node->type == "maxlang::int_value")
        {
            int v = stoi(value_node->string());
            _modtordef_param = new modtordef_param(v);
            _modtordef->setparam(p_name, *_modtordef_param);
            return 1;
        }
        else if (value_node->type == "maxlang::modtor_argument_variable")
        {
            /* TODO */

        }else if (value_node->type == "maxlang::modtor_def")
        {
            // new child modtor
            modtordef * child_modtordef = 0;
            if(deftree_parse_modtor_def(*value_node, child_modtordef,m_ob))
            {
                _modtordef_param = new modtordef_param(*child_modtordef);
                _modtordef->setparam(p_name,*_modtordef_param);
                return 1;
            }else
            {
                object_error(m_ob, "unknown modtor_param value type");
                return 0;
            }
        }else if (value_node->type == "maxlang::modtor_operator_expression")
        {
            modtordef * child_modtordef = 0;
            if(deftree_parse_modtor_operator_expression(*value_node, child_modtordef,m_ob))
            {
                _modtordef_param = new modtordef_param(*child_modtordef);
                _modtordef->setparam(p_name,*_modtordef_param);
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


int deftree_parse_modtor_def(const pegtl::parse_tree::node& n, modtordef *&_modtordef, t_object * m_ob)
{
   if( n.children.empty()) {
       return 0;
   }
   
   // parse children (modtor + modtor) modtor_expression_operator
   if( n.children.size() == 1)
   {
       pegtl::parse_tree::node *type_node = n.children[0].get();
       if(type_node->has_content() && type_node->type == "maxlang::modtor_expression_operator")
           return deftree_parse_modtor_operator_expression(*type_node,_modtordef,m_ob);
       
   }
   
   
   // parse children maxlang::modtor_type + maxlang::modtor_arguments

    pegtl::parse_tree::node *type_node = n.children[0].get();
    pegtl::parse_tree::node *args_node = n.children[1].get();
    
    
    // parse children maxlang::modtor_type
    if(type_node->has_content() && type_node->type == "maxlang::modtor_type") {
        std::string name = type_node->string();
        modtor * dum_modtor;
        modtor_type_enum modtor_type_e = modtor_create_fromstring(name,dum_modtor,false);
        if(modtor_type_e == modtor_type_enum::unknown)
        {
            object_error(m_ob, "unknown modtor type %s",name.c_str());
            return 0;
        }else
        {
            _modtordef = new modtordef(modtor_type_e,name);
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
                if(!deftree_parse_modtor_params( *up, _modtordef, m_ob ))
                {
                    delete _modtordef;
                    _modtordef = NULL;
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



int deftree_parse_modtor_param_value(const pegtl::parse_tree::node& n, modtordef *&_modtordef, std::string name, modtordef_param *&_modtordef_param, t_object * m_ob)
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
            _modtordef_param = new modtordef_param(v);
            _modtordef->setparam(name, *_modtordef_param);
            return 1;
        }
        else if (value_node->type == "maxlang::int_value")
        {
            int v = stoi(value_node->string());
            _modtordef_param = new modtordef_param(v);
            _modtordef->setparam(name, *_modtordef_param);
            return 1;
        }
        else if (value_node->type == "maxlang::list_expression")
        {
            // get all children maxlang::double_value or maxlang::int_value
            if( !value_node->children.empty() ) {
                std::vector<double> list;
                
                for( auto& child_v : value_node->children )
                    list.push_back(stod(child_v->string()));
                _modtordef_param = new modtordef_param(list);
                _modtordef->setparam(name, *_modtordef_param);
                return 1;
                
            }else
            {
                object_error(m_ob, "modtor_param list has no value");
            }

        }else if (value_node->type == "maxlang::modtor_def")
        {
            // new child modtor
            modtordef * child_modtordef = 0;
            if(deftree_parse_modtor_def(*value_node, child_modtordef,m_ob))
            {
                //// ****** BUGFGG
                _modtordef_param = new modtordef_param(*child_modtordef);
                _modtordef->setparam(name,*_modtordef_param);
                return 1;
            }else
            {
                object_error(m_ob, "unknown modtor_param value type");
                return 0;
            }
        }else if (value_node->type == "maxlang::modtor_operator_expression")
        {
            modtordef * child_modtordef = 0;
            if(deftree_parse_modtor_operator_expression(*value_node, child_modtordef,m_ob))
            {
                _modtordef_param = new modtordef_param(*child_modtordef);
                _modtordef->setparam(name,*_modtordef_param);
                return 1;
            }else
            {
                object_error(m_ob, "unknown modtor_param value type");
                return 0;
            }
        }else if (value_node->type == "maxlang::lidentifier")
        {
            std::string ident = value_node->string();
            _modtordef_param = new modtordef_param(ident);
            _modtordef->setparam(name, *_modtordef_param);
            
            return 1;
        }
    }
    else {
        object_error(m_ob, "modtor_param has no value");
        return 0;
    }
    return  1;
}

int deftree_parse_modtor_params(const pegtl::parse_tree::node& n, modtordef *&_modtordef, t_object * m_ob)
{
    
    if(n.has_content() && n.type == "maxlang::modtor_argument") {
        
        // parse children maxlang::modtor_argument_name + maxlang::modtor_argument_value
        if( n.children.empty() && n.children.size()<2 ) {
            return 0;
        }
        
        pegtl::parse_tree::node *name_node = n.children[0].get();
        pegtl::parse_tree::node *value_node = n.children[1].get();
        
        std::string name = name_node->string();
        maxlang::modtordef_param * value;
        
        // name : special modtor argument to ref a specific sub-modtor
        if(name == "name")
        {
            
        }
        
        // get into value node
        if(deftree_parse_modtor_param_value(*value_node,_modtordef,name,value,m_ob))
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

// construct the deftree
int deftree_make( const pegtl::parse_tree::node& n, modtordef *&_modtordef, t_object * m_ob)
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
        return deftree_parse_modtor_def(*modtor_node,_modtordef,m_ob);
    }else if (modtor_node->type == "maxlang::modtor_operator_expression")
    {
        return deftree_parse_modtor_operator_expression(*modtor_node,_modtordef,m_ob);
    }
    return 1;
}

// construct the modtor argument value tree
int deftree_make_parameter( const pegtl::parse_tree::node& n, modtordef *&_modtordef, std::string arg_name, t_object * m_ob)
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
    
    maxlang::modtordef_param * value;
    
    if(value_node->type == "maxlang::modtor_argument_value")
    {
        //     int deftree_parse_modtor_param_value(const pegtl::parse_tree::node& n, modtor *&_modtor, std::string name, modtor_param *&_modtor_param, t_object * m_ob)
        return deftree_parse_modtor_param_value(*value_node,_modtordef,arg_name,value,m_ob);
    }
    return 1;
}

    
}



#endif /* maxlang_parsetree_h */
