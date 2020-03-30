#include "maxcpp6.h"
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
        
        systhread_mutex_new(&mutx, SYSTHREAD_MUTEX_NORMAL );
        
	}
	~maxlang_modulator() {
        if(modtor_head)
            delete modtor_head;
    }
	
	// methods:
	void bang(long inlet) {
        
        double time = getTack();
        // mutex lock
        systhread_mutex_lock(mutx);
        double val = test_lfo.get(time);
        // mutex unlock
        systhread_mutex_unlock(mutx);
        
        pushTack();
        outlet_float(m_outlets[1],time);
        outlet_float(m_outlets[0],val);
	}
    
    void parameter(long inlet, t_symbol * s, long ac, t_atom * av) {
        std::string name;
        double value;
        
        if(ac< 2)
        {
            object_error(&m_ob,"parameter mess need 2 args");
            return;
        }

        name = av[0].a_w.w_sym->s_name;
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
            if(test_lfo.setparam(name, maxlang::modtor_param(value))==0)
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
                if(test_lfo.setparam(name, maxlang::modtor_param(input_list))==0)
                    object_error(&m_ob, "parameter %s not found",name.c_str());
                systhread_mutex_unlock(mutx);
            }else
                object_error(&m_ob, "input list is too short for %s",name.c_str());
            
        }
        
    }
	
	void test(long inlet, t_symbol * s, long ac, t_atom * av) {
        std::string name("max");
        systhread_mutex_lock(mutx);
        if(test_lfo.setparam(name, maxlang::modtor_param(new maxlang::m_randi()))==0)
            object_error(&m_ob, "parameter %s not found",name.c_str());
        systhread_mutex_unlock(mutx);
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
        
        object_post(&m_ob, "parsing %s",atoms.c_str());
    try {
        pegtl::string_input input( atoms, std::string("input"));
        
        
        if( const auto root = pegtl::parse_tree::parse< maxlang::modtor_start, maxlang::store >(input) ) {
            maxlang::print_node( *root );
        }
        else {
            std::cout << "PARSE FAILED" << std::endl;
        }
    }
    catch( const std::exception& e ) {
        //std::cout << "PARSE FAILED WITH EXCEPTION: " << e.what() << std::endl;
        object_error(&m_ob, "parse error : %s",e.what());
        return;
        }
        
        //if(name.c_str() != nullptr)
            //outlet_anything(m_outlets[0], gensym(name.c_str()), 0, av);
    }
    
    
    // TIMING
    double tick()
    {
        tmpTick = prevTick;
        prevTick = std::chrono::high_resolution_clock::now();
        
        auto duration = prevTick - tmpTick;
        return  std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
    }
    
    double getTack()
    {
        return  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - tack).count()/1000.;
    }
    
    void pushTack()
    {
        tack = std::chrono::high_resolution_clock::now();
    }
    
    // members
    std::chrono::high_resolution_clock::time_point prevTick = std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::time_point tack;
    std::chrono::high_resolution_clock::time_point tmpTick;
    
    maxlang::m_seq test_lfo;
    
    maxlang::modtor * modtor_head;
    
    t_systhread_mutex mutx;
    
};

C74_EXPORT int main(void) {
	// create a class with the given name:
	maxlang_modulator::makeMaxClass("maxlang.modulator");
	REGISTER_METHOD(maxlang_modulator, bang);
	REGISTER_METHOD_GIMME(maxlang_modulator, test);
    REGISTER_METHOD_GIMME(maxlang_modulator, parse);
    REGISTER_METHOD_GIMME(maxlang_modulator, parameter);
	

}
