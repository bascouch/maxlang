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
#include "maxlang.deftree.hpp"


class maxlang_modulator : public MaxCpp6<maxlang_modulator> {
public:
    
    /* members */
    
    std::chrono::high_resolution_clock::time_point std_prevTick = std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::time_point std_tack;
    std::chrono::high_resolution_clock::time_point std_tmpTick;
    
    double prevTick;
    double tack;
    double tmpTick;
    
    int use_system_clock=0;
    
    int n_chans = 1;
    t_atom * outlist;
    t_atom * outsample;
    t_atom * outstring;
    double * lastval;
    
    bool const_mode = true;
  
    
    std::vector<maxlang::modtor *> modtor_vector;
    std::vector<std::string> modtor_sources;
    std::vector<std::string> modtor_sources_param;
    std::vector<std::map<std::string,maxlang::modtor*>> named_modtor_ref_vector;
    t_systhread_mutex mutx;
    
    std::unique_ptr<pegtl::parse_tree::node> modtor_parse_tree;
    
    maxlang::modtordef ** defTreeA_list;
    maxlang::modtordef ** defTreeB_list;
    
    
    // internal clock // disabled
    void *m_clock;
    double m_interval;
    
    int m_verbose=1;
    
    
    
    /* methods */
    
	maxlang_modulator(t_symbol * sym, long ac, t_atom * av) {
		setupIO(1, 4); // inlets / outlets
        long v;
        
        // num channels
        if(ac> 0)
            switch(av[0].a_type)
            {
                case A_LONG:
                    v = av[0].a_w.w_long;
                    n_chans = (v > 1)? v : 1;
                    break;
                case A_FLOAT:
                    break;
                case A_SYM:
                    break;
            }
        
        // fill modtor vector with null pointers
        modtor_vector.resize(n_chans, 0);
        modtor_sources.resize(n_chans);
        modtor_sources_param.resize(n_chans);
        
        defTreeA_list = new maxlang::modtordef*[n_chans];
        defTreeB_list = new maxlang::modtordef*[n_chans];
        
        for(int i=0; i<n_chans; i++)
        {
            defTreeA_list[i] = new maxlang::modtordef(maxlang::modtor_type_enum::add,"add");
            defTreeB_list[i] = new maxlang::modtordef(maxlang::modtor_type_enum::add,"add");
        }
        
        
        std::map<std::string,maxlang::modtor*> modtor_ref;
        named_modtor_ref_vector.resize(n_chans, modtor_ref);
        
        outlist = new t_atom[n_chans];
        outstring = new t_atom[2];
        lastval = new double[n_chans];
        outsample= new t_atom[4096];

        
        systhread_mutex_new(&mutx, SYSTHREAD_MUTEX_NORMAL);

        
	}
	~maxlang_modulator() {
        modtor_vector.clear();
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
                lastval[k] = modtor_head->get(time);
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
        maxlang::modtor * modtor_ = modtor_vector[0];
        
        if(!modtor_)
        {
            object_error(&m_ob,"parameter mess: no modulator yet defined");
            return;
        }
            
        std::vector<maxlang::modtor*> modtor_ref;
        std::vector<maxlang::modtor*> modtor_ref_current;
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
                maxlang::modtor * tmp_modtor;
                for(int i=0; i<n_chans; i++)
                {
                    if(tmp_modtor = modtor_ref_current[i]->get_param_modtor(modtor_name))
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
                for(auto &modtor_v : modtor_ref)
                if(modtor_v->setparam(name, new maxlang::modtor_param(value))==0)
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
                    for(auto &modtor_v : modtor_ref)
                    if(modtor_v->setparam(name, new maxlang::modtor_param(input_list))==0)
                        object_error(&m_ob, "parameter %s not found",name.c_str());
                        
                }
                systhread_mutex_unlock(mutx);
            }else
                object_error(&m_ob, "input list is too short for %s",name.c_str());
            
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
            modtor_head->sync(phase);
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
    
    void parse(long inlet, t_symbol * s, long ac, t_atom * av) {
        std::string name;
        std::string atoms;
        bool atoms_set = false;
        bool double_param = false;
        double double_value = 0;
        
        /* if only float or long parameter do a parse const*/
        if (ac==1)
        {
            
            std::string value_str;
            switch(av[0].a_type)
            {
                case A_LONG:
                    double_value = av[0].a_w.w_long;
                    break;
                case A_FLOAT:
                    double_value = av[0].a_w.w_float;
                    break;
                case A_SYM:
                    try
                    {
                        std::string value_str (av[0].a_w.w_sym->s_name);
                        //object_post(&m_ob, "parsing float from %s",av[0].a_w.w_sym->s_name);
                        std::string::size_type sz;     // alias of size_t

                        double_value = std::stod (value_str,&sz);
                        
                    }
                    catch (std::invalid_argument const& ex)
                    {
                        object_error(&m_ob, "single float parsing %s in %s",ex.what(),atoms.c_str());
                        return;
                    }
                    
                    break;
            }
            
            atoms = "const( val=" + std::to_string(double_value) + " )";
            atoms_set = true;
            double_param = true;
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
                    int ret = maxlang::deftree_make(*root, defTreeA_list[i] , &m_ob);
                    
                    ret = maxlang::modtree_make(*root, modtor_vector[i], &m_ob, lastval[i]);
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
                        modtor_vector[i]->traverse_for_ref(named_modtor_ref_vector[i]);
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
    
    void parse_parameter( long ac, t_atom * av, std::vector<maxlang::modtor*> &modtor_ref, std::string arg_name) {
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
                    
                    int ret = maxlang::valtree_make(*root, modtor_ref[i], arg_name ,&m_ob, lastval[i]);
                    systhread_mutex_unlock(mutx);
                    
                    if(!ret)
                    {
                        object_error(&m_ob, "error making valtree : %s",atoms.c_str());
                        return;
                    }
                    else
                    {
                        named_modtor_ref_vector[i].clear();
                        modtor_vector[i]->traverse_for_ref(named_modtor_ref_vector[i]);
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
    
    maxlang::modtor * dictionary_parse(t_dictionary *d)
    {
        maxlang::modtor * returned_modtor = NULL;
        t_symbol * modtor_key = gensym("modtor");
        t_symbol * param_key = gensym("param");
        
        if(dictionary_hasentry(d,modtor_key) )
        {
            const char * modtor_type;
            dictionary_getstring(d,modtor_key, &modtor_type);
            maxlang::modtor_type_enum modtor_type_e = modtor_create_fromstring(modtor_type,returned_modtor,true);
            if(modtor_type_e == maxlang::modtor_type_enum::unknown)
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
                        _modtor_param = new maxlang::modtor_param(v);
                        returned_modtor->setparam(name, _modtor_param);
                        if(name=="seed")
                            returned_modtor->seed(std::to_string(v));
                        if(name=="sync")
                            returned_modtor->sync(v);
                    }
                    else if (atoms[0].a_type == A_LONG)
                    {
                        int v = atoms[0].a_w.w_long;
                        _modtor_param = new maxlang::modtor_param(v);
                        returned_modtor->setparam(name, _modtor_param);
                        if(name=="seed")
                            returned_modtor->seed(std::to_string(v));
                        if(name=="sync")
                            returned_modtor->sync(v);

                    }else if (atoms[0].a_type == A_SYM)
                    {
                        char * s = atoms[0].a_w.w_sym->s_name;
                        if(s)
                        {
                            _modtor_param = new maxlang::modtor_param(s);
                            returned_modtor->setparam(name, _modtor_param);
                            if(name=="seed")
                                returned_modtor->seed(std::string(s));
                        }
                        
                    }else if (atoms[0].a_type == A_OBJ)
                    {
                        t_dictionary * dchild;
                        maxlang::modtor * modtorchild;
                        
                        dictionary_getdictionary(sub_dict, keys[i], (t_object**)&dchild);
                        modtorchild = dictionary_parse(dchild);
                        if(modtorchild)
                        {
                            _modtor_param = new maxlang::modtor_param(modtorchild);
                            returned_modtor->setparam(name, _modtor_param);
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
                    
                    _modtor_param = new maxlang::modtor_param(retlist);
                    returned_modtor->setparam(name, _modtor_param);
                }
            }
            if(keys)
                dictionary_freekeys(d, numkeys, keys);
            
            
        }
        
        return returned_modtor;
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
                maxlang::modtor * modtor = dictionary_parse(d);
                if(modtor)
                {
                    if(modtor_vector[i])
                        delete modtor_vector[i];
                    modtor_vector[i] = modtor;
                    named_modtor_ref_vector[i].clear();
                    modtor_vector[i]->traverse_for_ref(named_modtor_ref_vector[i]);
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
    
    void merge(long inlet, t_symbol * s, long ac, t_atom * av) {
        std::string name;
        std::string atoms;
        bool atoms_set = false;
        bool double_param = false;
        double double_value = 0;
        
        /* if only float or long parameter do a parse const*/
        if (ac==1)
        {
            
            std::string value_str;
            switch(av[0].a_type)
            {
                case A_LONG:
                    double_value = av[0].a_w.w_long;
                    break;
                case A_FLOAT:
                    double_value = av[0].a_w.w_float;
                    break;
                case A_SYM:
                    try
                    {
                        std::string value_str (av[0].a_w.w_sym->s_name);
                        //object_post(&m_ob, "parsing float from %s",av[0].a_w.w_sym->s_name);
                        std::string::size_type sz;     // alias of size_t

                        double_value = std::stod (value_str,&sz);
                        
                    }
                    catch (std::invalid_argument const& ex)
                    {
                        object_error(&m_ob, "single float parsing %s in %s",ex.what(),atoms.c_str());
                        return;
                    }
                    
                    break;
            }
            
            atoms = "const( val=" + std::to_string(double_value) + " )";
            atoms_set = true;
            double_param = true;
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
                    int ret = maxlang::deftree_make(*root, defTreeB_list[i], &m_ob);
                    
                    /* do merge */
                    maxlang::modtordef _modtordef_new = defTreeA_list[i]->merge_modtordef(*(defTreeB_list[i]));
                    
                    /* replace deftree A */
                    *defTreeA_list[i] = _modtordef_new;
                    
                    /* replace source str*/
                    modtor_sources[i] = _modtordef_new.get_modtordef_str();
                    
                    /* reconstruct modtree*/
                    pegtl::string_input inputM( modtor_sources[i], std::string("input"));
                
                    if( const auto rootM = pegtl::parse_tree::parse< maxlang::modtor_start, maxlang::store >(inputM) )
                    {
                        int ret = maxlang::deftree_make(*rootM, defTreeA_list[i], &m_ob);
                        ret = maxlang::modtree_make(*rootM, modtor_vector[i], &m_ob, lastval[i]);
                    }
                    
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
                        modtor_vector[i]->traverse_for_ref(named_modtor_ref_vector[i]);
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
    
    void clear(long inlet)
    {
        systhread_mutex_lock(mutx);
        for(int i=0; i<n_chans; i++)
        {
            if(modtor_vector[i]){
                delete modtor_vector[i];
                modtor_vector[i] = NULL;
                named_modtor_ref_vector[i].clear();
            }
        }
        systhread_mutex_unlock(mutx);
        
    }
    
    void sample(long inlet, t_symbol * s, long ac, t_atom * av) {
        long points=256;
        double duration = 5000;
        
        maxlang::modtor *_modtor = modtor_vector[0];
        
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
	REGISTER_METHOD(maxlang_modulator, bang);
	REGISTER_METHOD_GIMME(maxlang_modulator, test);
    REGISTER_METHOD_GIMME(maxlang_modulator, verbose);
    REGISTER_METHOD_GIMME(maxlang_modulator, parse);
    REGISTER_METHOD_GIMME(maxlang_modulator, merge);
    REGISTER_METHOD_GIMME(maxlang_modulator, parameter);
    REGISTER_METHOD_GIMME(maxlang_modulator, sync);
    REGISTER_METHOD_GIMME(maxlang_modulator, dictionary);
    REGISTER_METHOD_GIMME(maxlang_modulator, sample);
    REGISTER_METHOD(maxlang_modulator, clear);
	

}
