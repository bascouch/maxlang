#include "maxcpp/maxcpp6.h"
#include "ext_strings.h"
#include "ext_dictobj.h"

#include <string>
#include <iostream>
#include <chrono>

#include <tao/pegtl.hpp>
#include <tao/pegtl/contrib/parse_tree.hpp>

namespace pegtl = tao::pegtl;


#include "maxlang.utils.hpp"
#include "maxlang.grammar.hpp"
#include "maxlang.modtree.hpp"
#include "maxlang.parsetree.hpp"
#include "maxlang.macrotree.hpp"

class maxlang_modulator : public MaxCpp6<maxlang_modulator> {
public:
    
    // members
    std::chrono::high_resolution_clock::time_point std_prevTick = std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::time_point std_tack;
    std::chrono::high_resolution_clock::time_point std_tmpTick;
    
    double prevTick;
    double tack;
    double tmpTick;
    
    int use_system_clock=0;
    
    int n_chans = 1;
    bool const_mode = true;
    t_atom * outlist;
    t_atom * outsample;
    t_atom * outstring;
    double * lastval;
    
    std::vector<std::shared_ptr<maxlang::modtor *>> modtor_vector;
    std::vector<std::string> modtor_sources;
    std::vector<std::string> modtor_sources_param;
    std::vector<std::map<std::string,std::shared_ptr<maxlang::modtor *>>> named_modtor_ref_vector;
    t_systhread_mutex mutx;
    
    maxlang::scope * _scope;
    
    
    std::vector<std::shared_ptr<maxlang::modtor *>> modtor_vector_destination;
    double interpolate_coeff = 0;
    
    
    
    
    // internal clock // disabled
    void *m_clock;
    double m_interval;
    
    int m_verbose=1;
    
    
    // functions
    
    
	maxlang_modulator(t_symbol * sym, long ac, t_atom * av) {
		setupIO(1, 4); // inlets / outlets
        long v;
        
        // num channels
        if(ac> 0)
            switch(av[0].a_type)
            {
                case A_LONG:
                    v = av[0].a_w.w_long;
                    n_chans = (v > 1)? (long)v : 1;
                    break;
                case A_FLOAT:
                    break;
                case A_SYM:
                    break;
            }
        
        // fill modtor vector with null pointers
        modtor_vector.resize(n_chans, 0);
        modtor_vector_destination.resize(n_chans, 0);
        modtor_sources.resize(n_chans);
        modtor_sources_param.resize(n_chans);
        
       
        
        _scope = new maxlang::scope;
        
        for(int i=0; i<n_chans; i++)
        {
            modtor_vector[i] = maxlang::modtor_create_fromstring("const",this->_scope);
            modtor_vector_destination[i] = maxlang::modtor_create_fromstring("const",this->_scope);
         }
        
        std::map<std::string,std::shared_ptr<maxlang::modtor*>> modtor_ref;
        named_modtor_ref_vector.resize(n_chans, modtor_ref);
        
        outlist = new t_atom[n_chans];
        outsample= new t_atom[4096];
        outstring = new t_atom[2];
        lastval = new double[n_chans];
        
        systhread_mutex_new(&mutx, SYSTHREAD_MUTEX_NORMAL);

        
	}
	~maxlang_modulator() {
        modtor_vector.clear();
        modtor_vector_destination.clear();
        delete outlist;

    }
	
	// methods:
	void bang(long inlet) {
        int k=0;
        double time = getTack();
        // mutex lock
        systhread_mutex_lock(mutx);
        for(auto &modtor_head : modtor_vector)
        {
            if(modtor_head)
            {
                lastval[k] = (*modtor_head)->get(time);
            }else
                lastval[k] = 0.;
            atom_setfloat(outlist+k,lastval[k]);
            k++;
        }
        // mutex unlock
        systhread_mutex_unlock(mutx);
        
        pushTack();
        outlet_float(m_outlets[1],time);
        outlet_list(m_outlets[0],0L,n_chans,outlist);
	}
    
    void parameter(long inlet, t_symbol * s, long ac, t_atom * av) {
        
        double value;
        
        if(ac< 2)
        {
            object_error(&m_ob,"parameter mess needs at least 2 args");
            return;
        }
        // name parsing :
        // • direct root modtor parameter
        // • sub-modtor by name.parameter
        
        std::string name = av[0].a_w.w_sym->s_name;
        std::shared_ptr<maxlang::modtor *> modtor_ = modtor_vector[0];
        
        if(!modtor_)
        {
            object_error(&m_ob,"parameter mess: no modulator yet defined");
            return;
        }
            
        std::vector<std::shared_ptr<maxlang::modtor*>> modtor_ref;
        std::vector<std::shared_ptr<maxlang::modtor*>> modtor_ref_current;
        modtor_ref_current.resize(n_chans);

        
        std::string modtor_name;
        
        std::vector<std::string> nodes;
        bool delim_found = false;
        
        std::string delimiter = ".";
        

        size_t pos = 0;
        
        while ((pos = name.find(delimiter)) != std::string::npos) {
            nodes.push_back(name.substr(0, pos));
            //std::cout << tokens.end() << std::endl;
            name.erase(0, pos + delimiter.length());
            delim_found = true;
        }
        
        if( delim_found ) // '.' found
        {
            // check if first node is found in named modtor ref
            modtor_name = nodes[0];
            if(named_modtor_ref_vector[0].find(modtor_name)!=named_modtor_ref_vector[0].end())
            {
                for(int i=0; i< n_chans; i++)
                    modtor_ref_current[i] = named_modtor_ref_vector[i][modtor_name];
                // remove first node
                nodes.erase(nodes.begin());
            }else
            {
                for(int i=0; i< n_chans; i++)
                    modtor_ref_current[i] = modtor_vector[i];
               
            }
            
            while(nodes.size()>0)
            {
                modtor_name = nodes[0];
                std::shared_ptr<maxlang::modtor *> tmp_modtor;
                for(int i=0; i<n_chans; i++)
                {
                    if((tmp_modtor = (*modtor_ref_current[i])->get_param_modtor(modtor_name)) != nullptr)
                    {
                        modtor_ref_current[i] = tmp_modtor;
                    }
                    else
                    {
                        object_error(&m_ob, "modtor node %s not found",modtor_name.c_str());
                        return;
                    }
                }
                
                nodes.erase(nodes.begin());
                
            }
            
            // fill with modtor_ref_current
            for(int i=0; i< n_chans; i++)
                modtor_ref.push_back(modtor_ref_current[i]);
        }
        else
        {
            // fill with root modtor
            for(int i=0; i< n_chans; i++)
                modtor_ref.push_back(modtor_vector[i]);
        }
        
        // CHECK FOR any of the arguments are symbol => modtor parameter parsing
        
        int k=1,flag=0;
        while(k<ac)
        {
            if(av[k].a_type == A_SYM)
            {
                flag=1;
                break;
            }
            k++;
        }
        
        if(flag) // modtor parameter parsing
        {
            // pop first paramter : name
            av++;
            ac--;
            parse_parameter(ac,av,modtor_ref,name);
            return;
        }
        
        if (ac==2)
        {
            switch(av[1].a_type)
            {
                case A_LONG:
                    value = av[1].a_w.w_long;
                    break;
                case A_FLOAT:
                    value = av[1].a_w.w_float;
                    break;
                case A_SYM:
                    break;
            }
            systhread_mutex_lock(mutx);
            if(modtor_)
            {
                for(auto modtor_v : modtor_ref)
                if((*modtor_v)->setparam(name, std::make_shared<maxlang::modtor_param*>(new maxlang::modtor_param(value)))==0)
                    object_error(&m_ob, "parameter %s not found",name.c_str());
                    
            }
            systhread_mutex_unlock(mutx);
        }
        if (ac>2)
        {
            std::vector<double> input_list;
            for(int i=1; i<ac; i++)
            {
                switch(av[i].a_type)
                {
                    case A_LONG:
                        value = av[i].a_w.w_long;
                        input_list.push_back(value);
                        break;
                    case A_FLOAT:
                        value = av[i].a_w.w_float;
                        input_list.push_back(value);
                        break;
                    case A_SYM:
                        break;
                }
                
            }
            
            if(input_list.size()>1)
            {
                systhread_mutex_lock(mutx);
                if(modtor_)
                {
                    for(auto modtor_v : modtor_ref)
                    if((*modtor_v)->setparam(name, std::make_shared<maxlang::modtor_param*>(new maxlang::modtor_param(input_list)))==0)
                        object_error(&m_ob, "parameter %s not found",name.c_str());
                        
                }
                systhread_mutex_unlock(mutx);
            }else
                object_error(&m_ob, "input list is too short for %s",name.c_str());
            
        }
        
    }
    
    void variable(long inlet, t_symbol * s, long ac, t_atom * av) {
        
        double value;
        
        if(ac< 2)
        {
            object_error(&m_ob,"variable mess needs at least 2 args");
            return;
        }
        // name parsing :
        // • direct root modtor parameter
        // • sub-modtor by name.parameter
        
        std::string name = av[0].a_w.w_sym->s_name;
        std::shared_ptr<maxlang::modtor *> modtor_ = modtor_vector[0];
        
        if(!modtor_)
        {
            object_error(&m_ob,"variable mess: no modulator yet defined");
            return;
        }
            
        
        // CHECK FOR any of the arguments are symbol => modtor parameter parsing
        

        if (ac==2)
        {
            switch(av[1].a_type)
            {
                case A_LONG:
                    value = av[1].a_w.w_long;
                    break;
                case A_FLOAT:
                    value = av[1].a_w.w_float;
                    break;
                case A_SYM:
                    break;
            }
            systhread_mutex_lock(mutx);
            if(_scope)
            {
                object_post(&m_ob, "set variable: parsed %s %f",name.c_str(),value);
                _scope->setvariable(name, value);
                    
            }
            systhread_mutex_unlock(mutx);
        
        
            
        }
        
    }
    
    
    void sync(long inlet, t_symbol * s, long ac, t_atom * av) {
        
        double phase=-0.00001;
        if(ac> 0)
            switch(av[0].a_type)
            {
                case A_LONG:
                    phase = av[0].a_w.w_long;
                    break;
                case A_FLOAT:
                    phase = av[0].a_w.w_float;
                    break;
                case A_SYM:
                    break;
            }
        
        systhread_mutex_lock(mutx);
        for(auto &modtor_head : modtor_vector)
            (*modtor_head)->sync(phase);
        systhread_mutex_unlock(mutx);
    }
	
	void test(long inlet, t_symbol * s, long ac, t_atom * av) {

	}
    
    void verbose(long inlet, t_symbol * s, long ac, t_atom * av) {
        if(ac>=1 && av[0].a_type == A_LONG)
        {
            m_verbose = av[0].a_w.w_long;
        }
    }
    
    void floatin(long inlet, double in)
    {
        post("bang\n");
        
        for(int i=0; i< n_chans; i++)
        {
            if(modtor_vector[i] && (*modtor_vector[i])->modtor_classname == "const")
            {
                (*modtor_vector[i])->setparam("val", std::make_shared<maxlang::modtor_param*>(new maxlang::modtor_param(in)));
            }
            else
            {
                modtor_create_fromstring("const",_scope);
                std::shared_ptr<maxlang::modtor*> p = modtor_create_fromstring("const",_scope) ;
                modtor_vector[i].swap(p);
                (*modtor_vector[i])->setparam("val", std::make_shared<maxlang::modtor_param*>(new maxlang::modtor_param(in)));
            }
                
        }
    }
    
    void parse(long inlet, t_symbol * s, long ac, t_atom * av) {
        std::string name;
        std::string atoms;
        bool atoms_set = false;
        
        /* if only float or long parameter do a parse const*/
        if (ac==1)
        {
            double value;
            std::string value_str;
            switch(av[0].a_type)
            {
                case A_LONG:
                    value = av[0].a_w.w_long;
                    break;
                case A_FLOAT:
                    value = av[0].a_w.w_float;
                    break;
                case A_SYM:
                    try
                    {
                        std::string value_str (av[0].a_w.w_sym->s_name);
                        //object_post(&m_ob, "parsing float from %s",av[0].a_w.w_sym->s_name);
                        std::string::size_type sz;     // alias of size_t

                        value = std::stod (value_str,&sz);
                        
                    }
                    catch (std::invalid_argument const& ex)
                    {
                        //object_error(&m_ob, "single float parsing %s in %s",ex.what(),atoms.c_str());
                        //return;
                    }
                    
                    break;
            }
            
            atoms = "const( val=" + std::to_string(value) + " )";
            atoms_set = true;
            //object_post(&m_ob, "parsing %s",atoms.c_str());
            
        }
        
        if(!atoms_set)
            for(int i=0; i<ac; i++)
                switch(av[i].a_type)
                {
                    case A_SYM:
                        atoms += (av[i].a_w.w_sym->s_name);
                        atoms += " ";
                        break;
                    case A_LONG:
                        atoms += std::to_string(av[i].a_w.w_long);
                        atoms += " ";
                        break;
                    case A_FLOAT:
                        atoms += std::to_string(av[i].a_w.w_float);
                        atoms += " ";
                        break;
                }
            
            
        if(m_verbose)
            object_post(&m_ob, "parsing %s",atoms.c_str());
        
        /* do macro replace*/
        
        {
            try {
                if(!maxlang::macro_parse_and_apply(atoms, n_chans, modtor_sources, &m_ob))
                {
                    object_error(&m_ob, "macro parse and apply error in %s",atoms.c_str());
                    return;
                }
                    
            }
            catch( const std::exception& e ) {
                object_error(&m_ob, "macro error %s in %s",e.what(),atoms.c_str());
                return;
                }
        }
        
        // parse * n_chans
        try {
            for(int i=0; i< n_chans; i++)
            {
                pegtl::string_input input( modtor_sources[i], std::string("input"));
            
                if( const auto root = pegtl::parse_tree::parse< maxlang::modtor_start, maxlang::store >(input) ) {
                    if(m_verbose)
                        maxlang::print_node( *root );
                    systhread_mutex_lock(mutx);

                    /* dispose old modtree if any */
                    if(*modtor_vector[i])
                    {
                        modtor_vector_destination[i] = modtor_vector[i];
                    }
                    
                    int ret = maxlang::modtree_make(*root, &modtor_vector[i], &m_ob, _scope);
                    systhread_mutex_unlock(mutx);
                    
                    if(!ret)
                    {
                        object_error(&m_ob, "error making modtree : %s",atoms.c_str());
                        return;
                    }
                    else
                    {
                        // ALL GOOD -> get refnames
                        named_modtor_ref_vector[i].clear();
                        (*modtor_vector[i])->traverse_for_ref(named_modtor_ref_vector[i]);
                    }
                }
                else {
                    object_error(&m_ob, "error parsing %s",atoms.c_str());
                    return;
                }
                
                atom_setlong(outstring,i);
                atom_setsym(outstring+1,gensym(modtor_sources[i].c_str()));
                outlet_list(m_outlets[2], 0L, 2,outstring);
                
                
            }
    }
    catch( const std::exception& e ) {
        object_error(&m_ob, "parse error %s in %s",e.what(),atoms.c_str());
        return;
        }
        
    }
    
    void merge(long inlet, t_symbol * s, long ac, t_atom * av)
    {
        std::string name;
        std::string atoms;
        bool atoms_set = false;
        
        /* if only float or long parameter do a parse const*/
        if (ac==1)
        {
            if(m_verbose)
                object_post(&m_ob, "entering one argument case %s");
            double value;
            std::string value_str;
            switch(av[0].a_type)
            {
                case A_LONG:
                    value = av[0].a_w.w_long;
                    break;
                case A_FLOAT:
                    value = av[0].a_w.w_float;
                    break;
                case A_SYM:
                    try
                    {
                        std::string value_str (av[0].a_w.w_sym->s_name);
                        //object_post(&m_ob, "parsing float from %s",av[0].a_w.w_sym->s_name);
                        std::string::size_type sz;     // alias of size_t

                        value = std::stod (value_str,&sz);
                        
                    }
                    catch (std::invalid_argument const& ex)
                    {
                        //object_error(&m_ob, "single float parsing %s in %s",ex.what(),atoms.c_str());
                        //return;
                    }
                    
                    break;
            }
            
            atoms = "const( val=" + std::to_string(value) + " )";
            atoms_set = true;
            //object_post(&m_ob, "parsing %s",atoms.c_str());
            
        }
        
        if(!atoms_set)
            for(int i=0; i<ac; i++)
                switch(av[i].a_type)
                {
                    case A_SYM:
                        atoms += (av[i].a_w.w_sym->s_name);
                        atoms += " ";
                        break;
                    case A_LONG:
                        atoms += std::to_string(av[i].a_w.w_long);
                        atoms += " ";
                        break;
                    case A_FLOAT:
                        atoms += std::to_string(av[i].a_w.w_float);
                        atoms += " ";
                        break;
                }
            
            
        if(m_verbose)
            object_post(&m_ob, "parsing %s",atoms.c_str());
        
        
        // macro replace
        
        try {
            if(!maxlang::macro_parse_and_apply(atoms, n_chans, modtor_sources, &m_ob))
            {
                object_error(&m_ob, "macro parse and apply error in %s",atoms.c_str());
                return;
            }
                
        }
        catch( const std::exception& e ) {
            object_error(&m_ob, "macro error %s in %s",e.what(),atoms.c_str());
            return;
            }
        
        // parse * n_chans
        try {
            for(int i=0; i< n_chans; i++)
            {
                pegtl::string_input input( modtor_sources[i], std::string("input"));
            
                if( const auto root = pegtl::parse_tree::parse< maxlang::modtor_start, maxlang::store >(input) ) {
                    if(m_verbose)
                        maxlang::print_node( *root );
                    systhread_mutex_lock(mutx);

                    int ret = maxlang::modtree_make(*root, &modtor_vector_destination[i], &m_ob, _scope);
                    systhread_mutex_unlock(mutx);
                    
                    if(!ret)
                    {
                        object_error(&m_ob, "error making modtree : %s",atoms.c_str());
                        return;
                    }
                    else
                    {
                        // ALL GOOD
                        // merge with current graph
                        systhread_mutex_lock(mutx);
                        modtor_vector[i] = merge_modtor(
                                                        &modtor_vector[i],
                                                        &modtor_vector_destination[i],
                                                        _scope);
                        systhread_mutex_unlock(mutx);

                        
                        // &É"'(§È!ÇÀÀÇ!È§('"É&&É"'(§È!
                        
                        //
                        (*modtor_vector[i])->traverse_for_ref(named_modtor_ref_vector[i]);
                    }
                }
                else {
                    object_error(&m_ob, "error parsing %s",atoms.c_str());
                    return;
                }
                
                atom_setlong(outstring,i);
                atom_setsym(outstring+1,gensym(modtor_sources[i].c_str()));
                outlet_list(m_outlets[2], 0L, 2,outstring);
                
                
            }
    }
    catch( const std::exception& e ) {
        object_error(&m_ob, "parse error %s in %s",e.what(),atoms.c_str());
        return;
        }
        
    }
    
    void parse_parameter( long ac, t_atom * av, std::vector<std::shared_ptr<maxlang::modtor*>> modtor_ref_ptr, std::string arg_name) {
        std::string name;
        std::string atoms;
        
        for(int i=0; i<ac; i++)
            switch(av[i].a_type)
        {
            case A_SYM:
                atoms += (av[i].a_w.w_sym->s_name);
                atoms += " ";
                break;
            case A_LONG:
                atoms += std::to_string(av[i].a_w.w_long);
                atoms += " ";
                break;
            case A_FLOAT:
                atoms += std::to_string(av[i].a_w.w_float);
                atoms += " ";
                break;
        }
        if(m_verbose)
            object_post(&m_ob, "parsing %s",atoms.c_str());
        try {
            if(!maxlang::macro_parse_and_apply(atoms, n_chans, modtor_sources_param, &m_ob))
            {
                object_error(&m_ob, "macro parse and apply error in %s",atoms.c_str());
                return;
            }
                
        }
        catch( const std::exception& e ) {
            object_error(&m_ob, "macro error %s in %s",e.what(),atoms.c_str());
            return;
            }
        
        try {
            for(int i=0; i< n_chans; i++)
            {
                pegtl::string_input input( atoms, std::string("input"));
                
                if( const auto root = pegtl::parse_tree::parse< maxlang::modtor_argument_value_start, maxlang::store >(input) ) {
                    if(m_verbose)
                        maxlang::print_node( *root );
                    systhread_mutex_lock(mutx);
                    
                    int ret = maxlang::valtree_make(*root, &modtor_ref_ptr[i], arg_name ,&m_ob, _scope);
                    systhread_mutex_unlock(mutx);
                    
                    if(!ret)
                    {
                        object_error(&m_ob, "error making valtree : %s",atoms.c_str());
                        return;
                    }
                    else
                    {
                        named_modtor_ref_vector[i].clear();
                        (*modtor_vector[i])->traverse_for_ref(named_modtor_ref_vector[i]);
                    }
                }
                else {
                    object_error(&m_ob, "error parsing %s",atoms.c_str());
                    return;
            }}
        }
        catch( const std::exception& e ) {
            object_error(&m_ob, "parse error %s in %s",e.what(),atoms.c_str());
            return;
        }
        
    }
    
    
    
    std::shared_ptr<maxlang::modtor*> dictionary_parse(t_dictionary *d)
    {
        t_symbol * modtor_key = gensym("modtor");
        t_symbol * param_key = gensym("param");
        
        std::shared_ptr<maxlang::modtor*> _modtor_ptr;
        maxlang::modtor* _modtor;
        
        if(dictionary_hasentry(d,modtor_key) )
        {
            const char * modtor_type;
            dictionary_getstring(d,modtor_key, &modtor_type);
            /* create operator modtor */
            maxlang::modtor_type_enum modtor_type_e ;
            
            _modtor_ptr = maxlang::modtor_create_fromstring(modtor_type,_scope);
            
            
            if((_modtor = *_modtor_ptr) == nullptr || _modtor->modtor_class == maxlang::modtor_type_enum::unknown)
            {
                object_error(&m_ob, "unknown modtor type %s",modtor_type);
                return NULL;
            }
            //object_post(&m_ob, "new modtor %s", modtor_type);
            
        }else
        {
            object_error(&m_ob, "dictionary has no modtor key");
            return NULL;
        }
        
        if(dictionary_hasentry(d,param_key) )//&& dictionary_entryisdictionary(d, param_key))
        {
            t_dictionary    *sub_dict;
            dictionary_getdictionary(d,param_key, (t_object**) &sub_dict);
            t_symbol **keys = NULL;
            long numkeys = 0;
            long i;
            long numatoms;
            t_atom* atoms;
            std::string name;
            dictionary_getkeys(sub_dict, &numkeys, &keys);
            
            for(i=0; i<numkeys; i++){
                // do something with the keys...
                name = std::string(keys[i]->s_name);
                maxlang::modtor_param * _modtor_param;
                dictionary_getatoms(sub_dict, keys[i], &numatoms, &atoms);
                
                if(numatoms==1)
                {
                    if(atoms[0].a_type == A_FLOAT)
                    {
                        double v = atoms[0].a_w.w_float;
                        (*_modtor_ptr)->setparam(name, std::make_shared<maxlang::modtor_param*>(new maxlang::modtor_param(v)));
                        
                        if(name=="seed")
                            (*_modtor_ptr)->seed(std::to_string(v));
                        if(name=="sync")
                            (*_modtor_ptr)->sync(v);
                    }
                    else if (atoms[0].a_type == A_LONG)
                    {
                        int v = atoms[0].a_w.w_long;
                        (*_modtor_ptr)->setparam(name, std::make_shared<maxlang::modtor_param*>(new maxlang::modtor_param(v)));
                        if(name=="seed")
                            (*_modtor_ptr)->seed(std::to_string(v));
                        if(name=="sync")
                            (*_modtor_ptr)->sync(v);

                    }else if (atoms[0].a_type == A_SYM)
                    {
                        char * s = atoms[0].a_w.w_sym->s_name;
                        if(s)
                        {
                            (*_modtor_ptr)->setparam(name, std::make_shared<maxlang::modtor_param*>(new maxlang::modtor_param(s)));
                            
                            if(name=="seed")
                                (*_modtor_ptr)->seed(std::string(s));
                        }
                        
                    }else if (atoms[0].a_type == A_OBJ)
                    {
                        t_dictionary * dchild;
                        std::shared_ptr<maxlang::modtor*> modtorchild_ptr;
                        
                        dictionary_getdictionary(sub_dict, keys[i], (t_object**)&dchild);
                        modtorchild_ptr = dictionary_parse(dchild);
                        
                        if(modtorchild_ptr)
                        {
                            (*_modtor_ptr)->setparam(name, std::make_shared<maxlang::modtor_param*>(new maxlang::modtor_param(modtorchild_ptr)));
                        }
                    }
                }else
                {
                    std::vector<double> retlist;
                    for(int j=0; j<numatoms; j++)
                    {
                        if(atoms[j].a_type == A_FLOAT)
                        {
                            double v = atoms[j].a_w.w_float;
                            retlist.push_back(v);
                        }
                        else if (atoms[j].a_type == A_LONG)
                        {
                            int v = atoms[j].a_w.w_long;
                            retlist.push_back((double)v);

                        }
                    }
                    
                    (*_modtor_ptr)->setparam(name, std::make_shared<maxlang::modtor_param*>(new maxlang::modtor_param(retlist)));
                    
                }
            }
            if(keys)
                dictionary_freekeys(d, numkeys, keys);
            
            
        }
        
        return _modtor_ptr;
    }
    
    
    // set modtor by dictionary
    void dictionary(long inlet, t_symbol * s, long ac, t_atom * av) {
        t_dictionary    *d;
        if(ac==1 && av[0].a_type==A_SYM)
        {
            t_symbol * dict_id = av[0].a_w.w_sym;
            d = dictobj_findregistered_retain(dict_id);
            if (!d) {
                object_error(&m_ob, "unable to reference dictionary named %s", s);
                return;
            }
            for(int i=0; i< n_chans; i++)
            {
                std::shared_ptr<maxlang::modtor *> modtor_ptr = dictionary_parse(d);
                if(modtor_ptr)
                {
                    if(modtor_vector[i])
                        modtor_vector[i].reset();
                    modtor_vector[i] = modtor_ptr;
                    named_modtor_ref_vector[i].clear();
                    (*modtor_vector[i])->traverse_for_ref(named_modtor_ref_vector[i]);
                    return;
                }
                
            }
        }
        else
        {
            object_error(&m_ob, "dictionary needs sym arg");
            return;
        }
        
        object_error(&m_ob, "error when parsing dictionary");
        return;
        
    }
    
    void clear(long inlet)
    {
        systhread_mutex_lock(mutx);
        for(int i=0; i<n_chans; i++)
        {
            if(modtor_vector[i]){
                modtor_vector[i].reset();
                named_modtor_ref_vector[i].clear();
            }
        }
        systhread_mutex_unlock(mutx);
        
    }
    
    void sample(long inlet, t_symbol * s, long ac, t_atom * av) {
        long points=256;
        double duration = 5000;
        
        maxlang::modtor * _modtor = *modtor_vector[0];
        
        /*if(ac>=1 && av[0].a_type == A_FLOAT)
        {
            duration = av[0].a_w.w_float;
            object_post(&m_ob, "duration set %d",duration);
            
        }
        if(ac>=2 && av[1].a_type == A_LONG)
        {
            points = (av[1].a_w.w_long > 2)? av[1].a_w.w_long : 2;
            object_post(&m_ob, "points set %d",points);
        }
         */
        
        if(_modtor)
        {
            
            
            systhread_mutex_lock(mutx);
            //double time = 0;
            double time_inc = duration / points;
            for(int i=0; i<points; i++)
                atom_setfloat(outsample+i,_modtor->get(time_inc));

            // mutex unlock
            systhread_mutex_unlock(mutx);

            outlet_list(m_outlets[3],0L,points,outsample);
        }
    
    }
    
    
    
    // TIMING
    double tick()
    {
        tmpTick = prevTick;
        if(use_system_clock)
        {
            std_prevTick = std::chrono::high_resolution_clock::now();
            
            auto duration = std_prevTick - std_tmpTick;
            return  std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
        }
        else
        {
            prevTick = (double) gettime();
            return prevTick - tmpTick;
        }
    }
    
    double getTack()
    {
        if(use_system_clock)
            return  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - std_tack).count()/1000.;
        else
            return gettime() - tack;
    }
    
    void pushTack()
    {
        if(use_system_clock)
            std_tack = std::chrono::high_resolution_clock::now();
        else
            tack = gettime();
    }
    
    
    
};

C74_EXPORT int main(void) {
	// create a class with the given name:
	maxlang_modulator::makeMaxClass("maxlang.modulator");
    post("maxlang.modulator - charles bascou - contac@charlesbascou.com");
    post("build %s %s",__DATE__,__TIME__);
    
	REGISTER_METHOD(maxlang_modulator, bang);
	REGISTER_METHOD_GIMME(maxlang_modulator, test);
    REGISTER_METHOD_GIMME(maxlang_modulator, verbose);
    REGISTER_METHOD_GIMME(maxlang_modulator, parse);
    REGISTER_METHOD_GIMME(maxlang_modulator, merge);
    REGISTER_METHOD_GIMME(maxlang_modulator, parameter);
    REGISTER_METHOD_GIMME(maxlang_modulator, variable);
    REGISTER_METHOD_GIMME(maxlang_modulator, sync);
    REGISTER_METHOD_GIMME(maxlang_modulator, dictionary);
    REGISTER_METHOD_FLOAT(maxlang_modulator, floatin);
    REGISTER_METHOD_GIMME(maxlang_modulator, sample);
    REGISTER_METHOD(maxlang_modulator, clear);
	

}
