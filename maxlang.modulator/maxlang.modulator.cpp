#include "maxcpp6.h"
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


class maxlang_modulator : public MaxCpp6<maxlang_modulator> {
public:
	maxlang_modulator(t_symbol * sym, long ac, t_atom * av) {
		setupIO(1, 2); // inlets / outlets
        
        systhread_mutex_new(&mutx, SYSTHREAD_MUTEX_NORMAL);
        // create clock
        //m_clock = clock_new(m_ob, (void (maxlang_modulator::*)(t_object*)) clock_tick)); // make a clock

        
	}
	~maxlang_modulator() {
        if(modtor_head)
            delete modtor_head;
        // delete clock
        //freeobject((t_object *)m_clock);
    }
	
	// methods:
	void bang(long inlet) {
        
        double time = getTack();
        // mutex lock
        systhread_mutex_lock(mutx);
        if(modtor_head)
        {
            val = modtor_head->get(time);
        }
        // mutex unlock
        systhread_mutex_unlock(mutx);
        
        pushTack();
        outlet_float(m_outlets[1],time);
        outlet_float(m_outlets[0],val);
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
        maxlang::modtor * modtor_ = modtor_head;
        
        std::string modtor_name;
        
        std::string delimiter = ".";
        int pos = name.find_last_of(delimiter);
        if( pos != std::string::npos) // '.' found
        {
            modtor_name = name.substr(0,pos);
            name.erase(0,pos+1);
            
            if(named_modtor_ref.find(modtor_name)!=named_modtor_ref.end())
            {
                modtor_ = named_modtor_ref[modtor_name];
            }
            else
            {
                object_error(&m_ob, "modtor named %s not found",modtor_name.c_str());
                return;
            }
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
            parse_parameter(ac,av,modtor_,name);
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
                if(modtor_->setparam(name, maxlang::modtor_param(value))==0)
                    object_error(&m_ob, "parameter %s not found",name.c_str());
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
                    if(modtor_->setparam(name, maxlang::modtor_param(input_list))==0)
                        object_error(&m_ob, "parameter %s not found",name.c_str());
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
        if(modtor_head)
            modtor_head->sync(phase);
        systhread_mutex_unlock(mutx);
    }
	
	void test(long inlet, t_symbol * s, long ac, t_atom * av) {
        std::string name("max");
        systhread_mutex_lock(mutx);
        if(modtor_head)
        {}
        systhread_mutex_unlock(mutx);
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
            pegtl::string_input input( atoms, std::string("input"));
        
            if( const auto root = pegtl::parse_tree::parse< maxlang::modtor_start, maxlang::store >(input) ) {
                if(m_verbose)
                    maxlang::print_node( *root );
                systhread_mutex_lock(mutx);
                int ret = maxlang::modtree_make(*root, modtor_head, &m_ob, val);
                systhread_mutex_unlock(mutx);
                
                if(!ret)
                {
                    object_error(&m_ob, "error making modtree : %s",atoms.c_str());
                    return;
                }
                else
                {
                    // ALL GOOD -> get refnames
                    named_modtor_ref.clear();
                    modtor_head->traverse_for_ref(named_modtor_ref);
                }
            }
            else {
                object_error(&m_ob, "error parsing %s",atoms.c_str());
                return;
        }
    }
    catch( const std::exception& e ) {
        object_error(&m_ob, "parse error %s in %s",e.what(),atoms.c_str());
        return;
        }
        
    }
    
    void parse_parameter( long ac, t_atom * av, maxlang::modtor *&modtor, std::string arg_name) {
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
            pegtl::string_input input( atoms, std::string("input"));
            
            if( const auto root = pegtl::parse_tree::parse< maxlang::modtor_argument_value_start, maxlang::store >(input) ) {
                if(m_verbose)
                    maxlang::print_node( *root );
                systhread_mutex_lock(mutx);
                int ret = maxlang::valtree_make(*root, modtor, arg_name ,&m_ob, val);
                systhread_mutex_unlock(mutx);
                
                if(!ret)
                {
                    object_error(&m_ob, "error making valtree : %s",atoms.c_str());
                    return;
                }
                else
                {
                    // ALL GOOD -> get refnames
                    named_modtor_ref.clear();
                    modtor_head->traverse_for_ref(named_modtor_ref);
                }
            }
            else {
                object_error(&m_ob, "error parsing %s",atoms.c_str());
                return;
            }
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
            maxlang::modtor_type_enum modtor_type_e = modtor_create_fromstring(modtor_type,returned_modtor,val);
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
                        returned_modtor->setparam(name, *_modtor_param);
                        if(name=="seed")
                            returned_modtor->seed(std::to_string(v));
                    }
                    else if (atoms[0].a_type == A_LONG)
                    {
                        int v = atoms[0].a_w.w_long;
                        _modtor_param = new maxlang::modtor_param(v);
                        returned_modtor->setparam(name, *_modtor_param);
                        if(name=="seed")
                            returned_modtor->seed(std::to_string(v));

                    }else if (atoms[0].a_type == A_SYM)
                    {
                        char * s = atoms[0].a_w.w_sym->s_name;
                        if(s)
                        {
                            _modtor_param = new maxlang::modtor_param(s);
                            returned_modtor->setparam(name, *_modtor_param);
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
                            returned_modtor->setparam(name, *_modtor_param);
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
                    returned_modtor->setparam(name, *_modtor_param);

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
            
            maxlang::modtor * modtor = dictionary_parse(d);
            if(modtor)
            {
                if(modtor_head)
                    delete modtor_head;
                modtor_head = modtor;
                named_modtor_ref.clear();
                modtor_head->traverse_for_ref(named_modtor_ref);
                return;
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
        if(modtor_head)
        {
            delete modtor_head;
            modtor_head = NULL;
            named_modtor_ref.clear();

        }
        systhread_mutex_unlock(mutx);
        
    }
    
    
    // CLOCKING
    /* void clock_start()
    {
        clock_fdelay(m_clock,0.);
    }
    
    void clock_stop()
    {
        clock_unset(m_clock);
    }
    
    void clock_interval()
    {
        
    }
    
    void clock_tick(t_object * x)
    {
        clock_fdelay(m_clock, m_interval);
        // output modulator val
        bang(0);
    }*/
    
    
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
    
    // members
    std::chrono::high_resolution_clock::time_point std_prevTick = std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::time_point std_tack;
    std::chrono::high_resolution_clock::time_point std_tmpTick;
    
    double prevTick;
    double tack;
    double tmpTick;
    
    int use_system_clock=0;
    
    maxlang::modtor * modtor_head;
    std::map<std::string,maxlang::modtor*> named_modtor_ref;
    t_systhread_mutex mutx;
    double val=0;
    
    // internal clock // disabled
    void *m_clock;
    double m_interval;
    
    int m_verbose=0;
    
};

C74_EXPORT int main(void) {
	// create a class with the given name:
	maxlang_modulator::makeMaxClass("maxlang.modulator");
	REGISTER_METHOD(maxlang_modulator, bang);
	REGISTER_METHOD_GIMME(maxlang_modulator, test);
    REGISTER_METHOD_GIMME(maxlang_modulator, verbose);
    REGISTER_METHOD_GIMME(maxlang_modulator, parse);
    REGISTER_METHOD_GIMME(maxlang_modulator, parameter);
    REGISTER_METHOD_GIMME(maxlang_modulator, sync);
    REGISTER_METHOD_GIMME(maxlang_modulator, dictionary);
    REGISTER_METHOD(maxlang_modulator, clear);
	

}
