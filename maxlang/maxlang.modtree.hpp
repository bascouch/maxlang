//
//  maxlang.modtree.hpp
//  maxlang.modulator
//
//  Created by charles on 23/03/2020.
//

#ifndef maxlang_modtree_h
#define maxlang_modtree_h

#include <vector>
#include <map>
#include <cmath>
#include <random>
#include <algorithm>

namespace maxlang {

    
    
    enum modtor_param_type { e_int, e_double, e_list, e_modtor, e_string };
    

    class modtor;

    class scope
    {
        public :
        std::map<std::string,double> variables ;
        
        double getvariable(std::string varname)
        {
            if ( auto ret = variables.find(varname); ret != variables.end() )
            {
         
                return variables[varname] ;
            } else
                return 0;
        };
        
        int setvariable(std::string varname, double _value)
        {
            if ( auto ret = variables.find(varname); ret != variables.end() )
            {
                variables[varname] =_value;
                return 1 ;
            } else
            {
                variables.insert(std::pair<std::string,double>(varname, _value));
                return 1;
            }
                ;
        };
        
        int touch(std::string varname)
        {
            if ( auto ret = variables.find(varname); ret != variables.end() )
            {
                return 0 ;
            } else
            {
                variables.insert(std::pair<std::string,double>(varname, 0.));
                return 1;
            }
        };
        
        
        void touch(modtor * m)
        {};
    };
    
    
    
    enum modtor_type_enum{
        unknown,
        lfo,
        line,
        rand,
        randi,
        choice,
        choicei,
        seq,
        seqi,
        env,
        quantize,
        input,
        add,
        minus,
        mul,
        div,
        xfade,
        variable,
        interpolate
    };

    modtor_type_enum modtor_create_fromstring(std::string s, modtor *&m, scope * scope);


    class modtor_param
    {
        public :
        modtor_param();
        modtor_param(double v);
        modtor_param(int v);
        modtor_param(std::vector<double> l);
        modtor_param(modtor *m);
        modtor_param(std::string s);
        
        ~modtor_param();
        
        double get(double deltatime);
        void sync(double phase);
        void set(double value);
        std::vector<double> getlist();
        std::string getstring();
        modtor* getmodtor();
        
        modtor_param_type _type;
        
        double _value_d;
        int _value_i;
        std::vector<double> _list;
        modtor * _modtor=0;
        std::string _string;
        
        // buffer
        double * buffer = 0;
        long n_buffer=0;
        
        modtor_param * buffer_proc(int numframes,double deltatime);
        double get_b(int i);
        
        
    };

    
maxlang::modtor_param * merge_modtor_param(modtor_param *&paramA, modtor_param *&paramB, scope * _scope);

    
    class modtor {
        
    public :
        
        std::map<std::string, modtor_param> params;
        
        static std::map<std::string,double> variables ;
        std::string var_ref_internal;
        std::random_device rd_dev;
        std::seed_seq rd_seed;
        std::string modtor_refname;
        std::string modtor_classname;
        
        scope * _scope;
        
        
        modtor()
        {
            //modtor_classname = typeid(this).name();
            //std::cout << typeid(this).name();
        }
        
        virtual ~modtor()
        {}
        
        virtual double get(double deltatime) = 0;
        
        virtual void perform(double * values,int numframes,double deltatime) = 0;

        virtual void sync(double phase) = 0;
        virtual void seed(std::string seed_string) = 0;
                
        int setparam(std::string name, modtor_param *value)
        {
            // check if param is a refname
            if(name=="name")
            {
                modtor_refname = value->getstring();
                params[name] = *value;
                return 1;
            }
            if(name=="id")
            {
                var_ref_internal = value->getstring();
                auto ret = _scope->touch(var_ref_internal) ;
                
                return 1;
            }
            // common seed param
            if(name=="seed")
            {
                params[name] = *value;
                return 1;
            }
            // common seed param
            if(name=="sync")
            {
                params[name] = *value;
                return 1;
            }
            // else specific modtor param
            if ( params.find(name) == params.end() )
            { // not found
                params[name] = *value;
                return 0;
            } else {
                // found
                params.erase(name); // TEST bug
                params[name] = *value;
                //std::cout << value._type << std::endl;
            }
            
            return 1;
        }
        
        
        
        modtor* get_param_modtor(std::string name)
        {
            if ( params.find(name) == params.end() )
            { // not found
                return 0;
            } else {
                // found
                return params[name].getmodtor();
            }
        }
        
        modtor* get_param_double(std::string name)
        {
            if ( params.find(name) == params.end() )
            { // not found
                return 0;
            } else {
                // found
                return params[name].getmodtor();
            }
        }
        
        
        
        // ?? v
        int setvariable_recurse(std::string id, double val);
        
        double getvariable(std::string varname)
        {
            return _scope->getvariable(varname);
        }
        
        modtor * merge_modtor(maxlang::modtor *modtor_B,maxlang::modtor *&returned_modtor, scope * _scope)
        {
            // check modtor A B equality by classname
            if(this->modtor_classname == modtor_B->modtor_classname)
            {
                // recheck for type equality of each params
                // insert xfade or interpolate for each equal paramtype
                // modtor_param_type { e_int, e_double, e_list, e_modtor, e_string };
                // retreive all params from B
                std::vector<std::string> keysB;
                for (std::map<std::string, modtor_param>::iterator it=params.begin(); it!=params.end(); ++it)
                {
                    modtor_param * _paramA = &it->second;
                    modtor_param * _paramB = &modtor_B->params[it->first];
                    if(_paramA->_type == e_double && _paramA->_value_d != _paramB->_value_d)
                    {

                        keysB.push_back(it->first);
                    }
                    
                }
                
                for(std::string k : keysB)
                {
                    maxlang::modtor * tmp_modtor = NULL;
                    modtor_param * paramA = &params[k];
                    modtor_param * paramB = &modtor_B->params[k];
                    
                    //tmp_modtor = merge_modtor_param(paramA,paramB,_scope);
                    
                    // WARNING
                    params[k] = *merge_modtor_param(paramA,paramB,_scope);
                    //params[k].;
                    
                    
                }
                
                //returned_modtor
                
                return this;
 
            }
            else
            {
                // insert interpolate and return
                maxlang::modtor_param * returned_modtor_param = NULL;
                maxlang::modtor * _modtor = NULL;
                maxlang::modtor_param * paramA = new modtor_param(this);
                maxlang::modtor_param * paramB = new modtor_param(modtor_B);
                maxlang::modtor_create_fromstring("interpolate",_modtor,this->_scope);
                _modtor->setparam("a", paramA);
                _modtor->setparam("b", paramB);
                return _modtor;
                
                
                /*
                // create operator modtor
                modtor_type_enum modtor_type_e = modtor_create_fromstring(op_str,_modtor,0.);
                
                // parse the operator arguments and make modtor_param
                maxlang::modtor_param * value_a, *value_b;
                auto ret_a = modtree_parse_modtor_operator_argument(*arg_a_node,_modtor,"a",value_a,m_ob);
                auto ret_b = modtree_parse_modtor_operator_argument(*arg_b_node,_modtor,"b",value_b,m_ob);
                */
            }
            
            
            
            return nullptr; // ;)
        }

        int traverse_for_ref(std::map<std::string,maxlang::modtor*> &name_ref)
        {
            for (std::map<std::string, modtor_param>::iterator it=params.begin(); it!=params.end(); ++it)
            {
                if( it->first == "name" )
                {
                    std::string param_value = it->second.getstring();
                    name_ref.insert(std::pair<std::string,maxlang::modtor*>(param_value, this));
                }
                else // check if modtor has a sub-modtor
                {
                    if(it->second._type == modtor_param_type::e_modtor)
                    {
                        modtor * sub_modtor = it->second.getmodtor();
                        int ret;
                        // RECURSE
                        ret = sub_modtor->traverse_for_ref(name_ref);
                    }
                }
            }
            return 1; // ;)
        }
        


    private:
        
    };
    
    
    /****/
    //IMPL
    /****/
    
    
    modtor_param::modtor_param()
    {
        _type = modtor_param_type::e_double;
        _value_d = 0.;
    }
    modtor_param::modtor_param(double v){
        
        _type = modtor_param_type::e_double;
        _value_d = v;
    }
    
    modtor_param::modtor_param(int v){
        
        _type = modtor_param_type::e_int;
        _value_i = v;
    }
    
    modtor_param::modtor_param(std::vector<double> l){
        
        _type = modtor_param_type::e_list;
        _list = l;
    }
    
    modtor_param::modtor_param(modtor *m){
        
        _type = modtor_param_type::e_modtor;
        _modtor = m;
    }
    
    modtor_param::modtor_param(std::string s){
        
        _type = modtor_param_type::e_string;
        _string = s;
    }
    
    modtor_param::~modtor_param()
    {
        // NASTY
       /* if(modtor_param_type::e_modtor && _modtor)
            delete _modtor;*/
        if(modtor_param_type::e_list)
            _list.clear();
        if( n_buffer || buffer)
        {
            free(buffer);
        }
         
        
    }

    void modtor_param::sync(double phase)
    {
        if(_type == modtor_param_type::e_modtor)
            _modtor->sync(phase);
    }
    
    double modtor_param::get(double deltatime)
    {
        switch (_type)
        {
            case modtor_param_type::e_double:
                return _value_d;
            case modtor_param_type::e_int:
                return _value_i;
            case modtor_param_type::e_list:
                return 0.;
            case modtor_param_type::e_string:
                return 0.;
            case modtor_param_type::e_modtor:
                double v = _modtor->get(deltatime);
                return v;
            
        }
        
    }
    
    std::vector<double> modtor_param::getlist()
    {
        return _list;
    }
    
    std::string modtor_param::getstring()
    {
        return _string;
    }
    
    modtor* modtor_param::getmodtor()
    {
        return _modtor;
    }
    
    void modtor_param::set(double value)
    {
        _value_d=value;
        
    }

    modtor_param * modtor_param::buffer_proc(int numframes,double deltatime)
    {
        double val;
        double k = numframes;
        
        
        if(n_buffer != numframes)
        {
            if(buffer)
                free(buffer);
            buffer = (double *) malloc(numframes*sizeof(double));
            n_buffer = numframes;
        }
        
        double * buf_p = buffer;
        
        switch (_type)
        {
            case modtor_param_type::e_double:
                val =  _value_d;
                while(k--)
                    *(buf_p++) = val;
                break;
            case modtor_param_type::e_int:
                val = _value_i;
                while(k--)
                    *(buf_p++) = val;
                break;
            case modtor_param_type::e_list:
                val = 0.;
                while(k--)
                    *(buf_p++) = val;
                break;
            case modtor_param_type::e_string:
                val = 0.;
                while(k--)
                    *(buf_p++) = val;
                break;
            case modtor_param_type::e_modtor:
                _modtor->perform(buffer, numframes, deltatime);
                break;
            
        }
        return this;
    }

    double modtor_param::get_b(int i)
    {
        return buffer[i];
    }
    

    class m_lfo : public modtor {
        
    public:
        
        m_lfo(scope * modtor_scope)
        {
            modtor_classname = "lfo";
            
            _scope = modtor_scope;
            _scope->touch(this);
            
            params.insert(std::pair<std::string, modtor_param>("freq",modtor_param(0.6)));
            params.insert(std::pair<std::string, modtor_param>("varifreq",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("mode",modtor_param(1.)));
            params.insert(std::pair<std::string, modtor_param>("pw",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("min",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("max",modtor_param(1.)));
            params.insert(std::pair<std::string, modtor_param>("curve",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("mul",modtor_param(1.)));
            params.insert(std::pair<std::string, modtor_param>("add",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("count",modtor_param(-1.)));
            
            
            mt_gen = std::mt19937(rd_dev());
            mt_rand = std::uniform_real_distribution<double>(-1.,1.);
            
            //
            output_scale.setin_minmax(-1., 1.);
            
        };
        
        
        ~m_lfo()
        {
            params.clear();
            
            mt_gen.~mersenne_twister_engine();
            mt_rand.~uniform_real_distribution();
        }
        
        double phase = 1.;
        // variable that are updated only when phase reset
        
        double m_varifreq = 0.;
        double m_pw = 0.;
        double m_curve = 0.;
        scale_curve output_scale;
        std::mt19937 mt_gen;
        std::uniform_real_distribution<double> mt_rand;
        
        
        void seed(std::string seed_str) override
        {
            // for time randomization : reverse string
            std::reverse(seed_str.begin(),seed_str.end());
            
            std::seed_seq rd_seed(seed_str.begin(),seed_str.end());
            mt_gen = std::mt19937(rd_seed);
        }
        
        void sync(double _phase) override
        {
            phase = _phase;
            for (auto &p : params)
                p.second.sync(phase);
        }
        
        double wave(double phase, double mode, double pw )
        {
            // mode : lfo-sine lfo-rect lfo-sawup lfo-sawdown lfo-tri
            // fractionnal mode blend between waveforms
            mode = std::clamp(mode,1.,5.);
            int imode = floor(mode);
            double fmode = fmodf(mode,1.);
            double w1,w2,w;
            double m_pw = (pw+1)*0.5;
            
            switch(imode)
            {
                case 1: // sine + rect
                    w1 = sin(phase*M_PI*2);
                    w2 = (phase < m_pw)? 1 : -1;
                    w = w1*(1-fmode) + w2*fmode;
                    break;
                case 2: // rect + sawup
                    w1 = (phase < m_pw)? 1 : -1;
                    w2 = (phase*2)-1;
                    w = w1*(1-fmode) + w2*fmode;
                    break;
                case 3: // sawup + sawdown
                    w1 = (phase*2)-1;
                    w2 = ((phase*2)-1)*-1;
                    w = w1*(1-fmode) + w2*fmode;
                    break;
                case 4: // sawdown + tri
                    w1 = ((phase*2)-1)*-1;
                    w2 = (phase < m_pw)? ((phase/m_pw)*2)-1 : (((1-phase)/(1-m_pw))*2)-1 ;
                    w = w1*(1-fmode) + w2*fmode;
                    break;
                case 5:
                    w = (phase < m_pw)? ((phase/m_pw)*2)-1 : (((1-phase)/(1-m_pw))*2)-1 ;
                    break;
                    
            }
            
            return w;
        }
        
        
        
        void perform(double * values,int numframes,double deltatime) override
        {
            modtor_param * p_freq = params["freq"].buffer_proc(numframes, deltatime);
            modtor_param * p_varifreq = params["varifreq"].buffer_proc(numframes, deltatime);
            modtor_param * p_mode = params["mode"].buffer_proc(numframes, deltatime);
            modtor_param * p_pw = params["pw"].buffer_proc(numframes, deltatime);
            modtor_param * p_min = params["min"].buffer_proc(numframes, deltatime);
            modtor_param * p_max = params["max"].buffer_proc(numframes, deltatime);
            modtor_param * p_curve = params["curve"].buffer_proc(numframes, deltatime);
            modtor_param * p_mul = params["mul"].buffer_proc(numframes, deltatime);
            modtor_param * p_add = params["add"].buffer_proc(numframes, deltatime);
            modtor_param * p_count = params["count"].buffer_proc(numframes, deltatime);
            
            modtor_param * p_time = 0;
            
            bool time_mode = params.find("time") != params.end();
            
            if(time_mode)
                p_time = params["time"].buffer_proc(numframes, deltatime);

            if(params.find("varitime") != params.end())
                p_varifreq = params["varitime"].buffer_proc(numframes, deltatime);
            
            for(int i=0; i<numframes; i++)
            {
                // get all the parameters (check time or freq format)
                double freq = p_freq->get_b(i);;
                double varifreq = p_varifreq->get_b(i);
                
                if(time_mode)
                    freq = 1000./ std::clamp(p_time->get_b(i),0.001,10000000.);
                double mode = p_mode->get_b(i);
                double pw = p_pw->get_b(i);
                double min = p_min->get_b(i);
                double max = p_max->get_b(i);
                double curve = p_curve->get_b(i);
                double add = p_add->get_b(i);
                double mul = p_mul->get_b(i);
                
                /** count special parameter: if > 0
                    • freq = 1./count
                    • deltatime = 1000.
                */
                double count = p_count->get_b(i);
                if(count >= 0.)
                {
                    freq = (count > 0.01)? 1./count : 100. ;
                    deltatime = 1000.;
                }
                
                double r_freq = freq * exp2( m_varifreq );
                phase += r_freq*deltatime/1000.;
                
                if (phase > 1.)
                {   // reset : new freq jitter varifreq
                    m_varifreq = mt_rand(mt_gen)*varifreq;
                    phase = fmodf(phase,1.);
                    
                    // sample curve and pw param
                    m_curve = curve;
                    output_scale.setcurve(m_curve);
                    m_pw = pw;
                }
                
                output_scale.setout_min(min);
                output_scale.setout_max(max);
                
                
                double w = wave(phase, mode, m_pw);
                values[i] = add+(output_scale.apply(w)*mul);
                
            }
        }
        
        double get(double deltatime) override
        {

            // get all the parameters (check time or freq format)
            double freq = params["freq"].get(deltatime);
            double varifreq = params["varifreq"].get(deltatime);
            
            if(params.find("time") != params.end())
                freq = 1000./ std::clamp(params["time"].get(deltatime),0.001,10000000.);
            if(params.find("varitime") != params.end())
                varifreq = params["varitime"].get(deltatime);
            
            double mode = params["mode"].get(deltatime);
            double pw = params["pw"].get(deltatime);
            double min = params["min"].get(deltatime);
            double max = params["max"].get(deltatime);
            double curve = params["curve"].get(deltatime);
            double add = params["add"].get(deltatime);
            double mul = params["mul"].get(deltatime);
            
            /** count special parameter: if > 0
                • freq = 1./count
                • deltatime = 1000.
            */
            double count = params["count"].get(deltatime);
            if(count >= 0.)
            {
                freq = (count > 0.01)? 1./count : 100. ;
                deltatime = 1000.;
            }
            
            double r_freq = freq * exp2( m_varifreq );
            phase += r_freq*deltatime/1000.;
            
            if (phase > 1.)
            {   // reset : new freq jitter varifreq
                m_varifreq = mt_rand(mt_gen)*varifreq;
                phase = fmodf(phase,1.);
                
                // sample curve and pw param
                m_curve = curve;
                output_scale.setcurve(m_curve);
                m_pw = pw;
            }
            
            output_scale.setout_min(min);
            output_scale.setout_max(max);
            
            double w = wave(phase, mode, m_pw);
            return add+(output_scale.apply(w)*mul);
        }
    };
    
    
    class m_line : public modtor {
        
    public:
        
        m_line(scope * modtor_scope)
        {
            
            modtor_classname = "line";
            
            _scope = modtor_scope;
            _scope->touch(this);
            
            params.insert(std::pair<std::string, modtor_param>("time",modtor_param(5000.)));
            params.insert(std::pair<std::string, modtor_param>("varitime",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("min",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("max",modtor_param(1.)));
            params.insert(std::pair<std::string, modtor_param>("curve",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("mul",modtor_param(1.)));
            params.insert(std::pair<std::string, modtor_param>("add",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("count",modtor_param(-1.)));
            
            mt_gen_time = std::mt19937(rd_dev());
            mt_rand_time = std::uniform_real_distribution<double>(-1.,1.);
            
            //
            segment_scale.setin_minmax(0., 1.);
            segment_scale.setout_min(0.);
            segment_scale.setout_max(1.);
            
            // start the line
            phase = -1.;
            
        };
        
        ~m_line()
        {
            params.clear();
            
            mt_gen_time.~mersenne_twister_engine();
            mt_rand_time.~uniform_real_distribution();
        }
        
        double phase = 0;
        double m_time = 1000.;
        double m_curve = 0.;
        double m_output = 0.;
        scale_curve segment_scale;
        std::mt19937 mt_gen_time;
        std::uniform_real_distribution<double> mt_rand_time;
        
        void seed(std::string seed_str) override
        {
            // for time randomization : reverse string
            std::reverse(seed_str.begin(),seed_str.end());
            std::seed_seq rd_seed(seed_str.begin(),seed_str.end());
            mt_gen_time = std::mt19937(rd_seed);
        }
        
        void sync(double _phase) override
        {
            phase = _phase;
        }
        
        void perform(double * values,int numframes,double deltatime) override
        {
            modtor_param * p_time = params["time"].buffer_proc(numframes, deltatime);
            modtor_param * p_varitime = params["varitime"].buffer_proc(numframes, deltatime);
            modtor_param * p_min = params["min"].buffer_proc(numframes, deltatime);
            modtor_param * p_max = params["max"].buffer_proc(numframes, deltatime);
            modtor_param * p_curve = params["curve"].buffer_proc(numframes, deltatime);
            modtor_param * p_mul = params["mul"].buffer_proc(numframes, deltatime);
            modtor_param * p_add = params["add"].buffer_proc(numframes, deltatime);
            modtor_param * p_count = params["count"].buffer_proc(numframes, deltatime);
            
            modtor_param * p_freq = 0;
            
            bool freq_mode = params.find("freq") != params.end();
            
            if(freq_mode)
                p_freq = params["freq"].buffer_proc(numframes, deltatime);

            if(params.find("varifreq") != params.end())
                p_varitime = params["varifreq"].buffer_proc(numframes, deltatime);
            
            for(int i=0; i<numframes; i++)
            {
                // get all the parameters (check time or freq format)
                double time = p_time->get_b(i);
                double varitime = p_time->get_b(i);

                if(freq_mode)
                    time = 1000./ std::clamp(p_freq->get_b(i),0.001,10000000.);
                
                double min = p_time->get_b(i);
                double max = p_time->get_b(i);
                double curve = p_time->get_b(i);
                double add = p_time->get_b(i);
                double mul = p_time->get_b(i);
                double tmp;
                
                /** count special parameter: if > 0
                    • freq = 1./count
                    • deltatime = 1000.
                */
                double count = p_count->get_b(i);
                if(count >= 0.)
                {
                    time = (count > 0.01)? count * 1000 : 10. ;
                    deltatime = 1000.;
                }
                            
                if (phase < 0.) // start the line
                {
                    // choose time
                    m_time = time * exp2(mt_rand_time(mt_gen_time)*varitime);
                    // sample curve param
                    m_curve=curve;
                    segment_scale.setin_minmax(0., m_time);
                    segment_scale.setout_min(min);
                    segment_scale.setout_max(max);
                    segment_scale.setcurve(m_curve);
                    
                    phase = 0;
                    m_output = min;
                    

                }
                else if(phase <= m_time)
                {
                    segment_scale.setout_min(min);
                    segment_scale.setout_max(max);
                    
                    phase += deltatime;
                    //printf("phase %f\n",phase);
                    if(max < min)
                        m_output = std::clamp(segment_scale.apply(phase),max,min);
                    else
                        m_output = std::clamp(segment_scale.apply(phase),min,max);
                    
                }
                
                values[i] =  add+(m_output*mul);
                
            }
        }
        
        double get(double deltatime) override
        {
            // get all the parameters (check time or freq format)
            double time = params["time"].get(deltatime);
            double varitime = params["varitime"].get(deltatime);

            if(params.find("freq") != params.end())
                time = 1000./ std::clamp(params["freq"].get(deltatime),0.001,10000000.);

            if(params.find("varifreq") != params.end())
                varitime = params["varifreq"].get(deltatime);
            
            double min = params["min"].get(deltatime);
            double max = params["max"].get(deltatime);
            double curve = params["curve"].get(deltatime);
            double add = params["add"].get(deltatime);
            double mul = params["mul"].get(deltatime);
            double tmp;
            
            /** count special parameter: if > 0
                • freq = 1./count
                • deltatime = 1000.
            */
            double count = params["count"].get(deltatime);
            if(count >= 0.)
            {
                time = (count > 0.01)? count * 1000 : 10. ;
                deltatime = 1000.;
            }
                        
            if (phase < 0.) // start the line
            {
                // choose time
                m_time = time * exp2(mt_rand_time(mt_gen_time)*varitime);
                // sample curve param
                m_curve=curve;
                segment_scale.setin_minmax(0., m_time);
                segment_scale.setout_min(min);
                segment_scale.setout_max(max);
                segment_scale.setcurve(m_curve);
                
                phase = 0;
                m_output = min;
                

            }
            else if(phase <= m_time)
            {
                segment_scale.setout_min(min);
                segment_scale.setout_max(max);
                
                phase += deltatime;
                //printf("phase %f\n",phase);
                if(max < min)
                    m_output = std::clamp(segment_scale.apply(phase),max,min);
                else
                    m_output = std::clamp(segment_scale.apply(phase),min,max);
                
            }
            
            return add+(m_output*mul);
            
        }
    };
    

    class m_randi : public modtor {
        
    public:
        
        m_randi(scope * modtor_scope)
        {
            
            modtor_classname = "randi";
            
            _scope = modtor_scope;
            _scope->touch(this);
            
            params.insert(std::pair<std::string, modtor_param>("freq",modtor_param(6.)));
            params.insert(std::pair<std::string, modtor_param>("varifreq",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("walk",modtor_param(1.)));
            params.insert(std::pair<std::string, modtor_param>("min",modtor_param(0)));
            params.insert(std::pair<std::string, modtor_param>("max",modtor_param(1.)));
            params.insert(std::pair<std::string, modtor_param>("curve",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("segcurve",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("mul",modtor_param(1.)));
            params.insert(std::pair<std::string, modtor_param>("add",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("count",modtor_param(-1.)));
            
            //m_rand_prev = from;
            //m_rand_target = from;
            
            
            mt_gen_time = std::mt19937(rd_dev());
            mt_rand_time = std::uniform_real_distribution<double>(-1.,1.);
            
            mt_gen_val = std::mt19937(rd_dev());
            mt_rand_val = std::uniform_real_distribution<double>(-1.,1.);
            
            //
            output_scale.setin_minmax(-1., 1.);
            
            segment_scale.setin_minmax(0., 1.);
            segment_scale.setout_min(0.);
            segment_scale.setout_max(1.);
            
        };
        
        ~m_randi()
        {
            params.clear();
            
            mt_gen_time.~mersenne_twister_engine();
            mt_rand_time.~uniform_real_distribution();
            mt_gen_val.~mersenne_twister_engine();
            mt_rand_val.~uniform_real_distribution();
        }
        
        double phase = 1.;
        double m_varifreq = 0.;
        double m_curve = 0.;
        double m_rand_prev = 0.;
        double m_rand_target = 0.;
        double m_segcurve = 0.;
        scale_curve output_scale, segment_scale;
        std::mt19937 mt_gen_time, mt_gen_val;
        std::uniform_real_distribution<double> mt_rand_time, mt_rand_val;
        
        void seed(std::string seed_str) override
        {
            std::seed_seq rd_seed(seed_str.begin(),seed_str.end());
            mt_gen_time = std::mt19937(rd_seed);
            
            mt_gen_val = std::mt19937(rd_seed);
            
        }
        
        void sync(double _phase) override
        {
            phase = _phase;
        }
        
        void perform(double * values,int numframes,double deltatime) override
        {
            modtor_param * p_freq = params["freq"].buffer_proc(numframes, deltatime);
            modtor_param * p_varifreq = params["varifreq"].buffer_proc(numframes, deltatime);
            modtor_param * p_walk = params["walk"].buffer_proc(numframes, deltatime);
            modtor_param * p_segcurve = params["segcurve"].buffer_proc(numframes, deltatime);
            modtor_param * p_min = params["min"].buffer_proc(numframes, deltatime);
            modtor_param * p_max = params["max"].buffer_proc(numframes, deltatime);
            modtor_param * p_curve = params["curve"].buffer_proc(numframes, deltatime);
            modtor_param * p_mul = params["mul"].buffer_proc(numframes, deltatime);
            modtor_param * p_add = params["add"].buffer_proc(numframes, deltatime);
            modtor_param * p_count = params["count"].buffer_proc(numframes, deltatime);
            
            modtor_param * p_time = 0;
            
            bool time_mode = params.find("time") != params.end();
            
            if(time_mode)
                p_time = params["time"].buffer_proc(numframes, deltatime);

            if(params.find("varitime") != params.end())
                p_varifreq = params["varitime"].buffer_proc(numframes, deltatime);
            
            for(int i=0; i<numframes; i++)
            {
                // get all the parameters (check time or freq format)
                double freq = p_freq->get_b(i);
                double varifreq = p_varifreq->get_b(i);
                
                if(time_mode)
                    freq = 1000./ std::clamp(p_time->get_b(i),0.001,10000000.);
                
                double walk = p_walk->get_b(i);
                double min = p_min->get_b(i);
                double max = p_max->get_b(i);
                double curve = p_curve->get_b(i);
                double segcurve = p_segcurve->get_b(i);
                double add = p_add->get_b(i);
                double mul = p_mul->get_b(i);
                
                /** count special parameter: if > 0
                    • freq = 1./count
                    • deltatime = 1000.
                */
                double count = p_count->get_b(i);
                if(count >= 0.)
                {
                    freq = (count > 0.01)? 1./count : 100. ;
                    deltatime = 1000.;
                }
                
                
                double r_freq = freq * exp2( m_varifreq );
                phase += r_freq*deltatime/1000.;
                
                if (phase > 1.)
                {   // reset : new freq jitter varifreq
                    m_varifreq = mt_rand_time(mt_gen_time)*varifreq;
                    phase = fmodf(phase,1.);
                    
                    // new random target value
                    m_rand_prev = m_rand_target;
                    m_rand_target = maxlang::fold(m_rand_prev + mt_rand_val(mt_gen_val) * walk * 2,-1,1);

                    // sample curve param
                    m_curve=curve;
                    m_segcurve=segcurve;
                    output_scale.setcurve(m_curve);
                    segment_scale.setcurve(m_segcurve);
                }
                
                output_scale.setout_min(min);
                output_scale.setout_max(max);

                double phase_c = segment_scale.apply(phase);
                double w = (1-phase_c)*m_rand_prev + phase_c*m_rand_target;
                values[i] = add+(output_scale.apply(w)*mul);
                
            }
        }
        
        double get(double deltatime) override
        {
            // get all the parameters (check time or freq format)
            double freq = params["freq"].get(deltatime);
            double varifreq = params["varifreq"].get(deltatime);
            
            if(params.find("time") != params.end())
                freq = 1000./ std::clamp(params["time"].get(deltatime),0.001,10000000.);
            if(params.find("varitime") != params.end())
                varifreq = params["varitime"].get(deltatime);
            
            double walk = params["walk"].get(deltatime);
            double min = params["min"].get(deltatime);
            double max = params["max"].get(deltatime);
            double curve = params["curve"].get(deltatime);
            double segcurve = params["segcurve"].get(deltatime);
            double add = params["add"].get(deltatime);
            double mul = params["mul"].get(deltatime);
            
            /** count special parameter: if > 0
                • freq = 1./count
                • deltatime = 1000.
            */
            double count = params["count"].get(deltatime);
            if(count >= 0.)
            {
                freq = (count > 0.01)? 1./count : 100. ;
                deltatime = 1000.;
            }
            
            
            double r_freq = freq * exp2( m_varifreq );
            phase += r_freq*deltatime/1000.;
            
            if (phase > 1.)
            {   // reset : new freq jitter varifreq
                m_varifreq = mt_rand_time(mt_gen_time)*varifreq;
                phase = fmodf(phase,1.);
                
                // new random target value
                m_rand_prev = m_rand_target;
                m_rand_target = maxlang::fold(m_rand_prev + mt_rand_val(mt_gen_val) * walk * 2,-1,1);

                // sample curve param
                m_curve=curve;
                m_segcurve=segcurve;
                output_scale.setcurve(m_curve);
                segment_scale.setcurve(m_segcurve);
            }
            
            output_scale.setout_min(min);
            output_scale.setout_max(max);

            double phase_c = segment_scale.apply(phase);
            double w = (1-phase_c)*m_rand_prev + phase_c*m_rand_target;
            return add+(output_scale.apply(w)*mul);
        }
    };
    
    class m_rand : public modtor {
        
    public:
        
        m_rand(scope * modtor_scope)
        {
            
            modtor_classname = "rand";
            
            _scope = modtor_scope;
            _scope->touch(this);
            params.insert(std::pair<std::string, modtor_param>("freq",modtor_param(6.)));
            params.insert(std::pair<std::string, modtor_param>("varifreq",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("walk",modtor_param(1.)));
            params.insert(std::pair<std::string, modtor_param>("min",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("max",modtor_param(1.)));
            params.insert(std::pair<std::string, modtor_param>("curve",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("mul",modtor_param(1.)));
            params.insert(std::pair<std::string, modtor_param>("add",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("count",modtor_param(-1.)));
            
            //m_rand_prev = from;
            //m_rand_target = from;
            
            mt_gen_time = std::mt19937(rd_dev());
            mt_rand_time = std::uniform_real_distribution<double>(-1.,1.);
            
            mt_gen_val = std::mt19937(rd_dev());
            mt_rand_val = std::uniform_real_distribution<double>(-1.,1.);
            
            //
            output_scale.setin_minmax(-1., 1.);
            
            segment_scale.setin_minmax(0., 1.);
            segment_scale.setout_min(0.);
            segment_scale.setout_max(1.);
            
        };
        
        ~m_rand()
        {
            params.clear();
            
            mt_gen_time.~mersenne_twister_engine();
            mt_rand_time.~uniform_real_distribution();
            mt_gen_val.~mersenne_twister_engine();
            mt_rand_val.~uniform_real_distribution();
        }
        
        double phase = 1.;
        double m_varifreq = 0.;
        double m_curve = 0.;
        double m_rand_prev = 0.;
        double m_rand_target = 0.;
        scale_curve output_scale, segment_scale;
        std::mt19937 mt_gen_time, mt_gen_val;
        std::uniform_real_distribution<double> mt_rand_time, mt_rand_val;
        
        void seed(std::string seed_str) override
        {
            std::seed_seq rd_seed_val(seed_str.begin(),seed_str.end());
            // for time randomization : reverse string
            std::reverse(seed_str.begin(),seed_str.end());
            std::seed_seq rd_seed_time(seed_str.begin(),seed_str.end());
            
            mt_gen_val = std::mt19937(rd_seed_val);
            mt_gen_time = std::mt19937(rd_seed_time);
        }
        
        void sync(double _phase) override
        {
            phase = _phase;
        }
        
        void perform(double * values,int numframes,double deltatime) override
        {
            modtor_param * p_freq = params["freq"].buffer_proc(numframes, deltatime);
            modtor_param * p_varifreq = params["varifreq"].buffer_proc(numframes, deltatime);
            modtor_param * p_walk = params["walk"].buffer_proc(numframes, deltatime);
            modtor_param * p_segcurve = params["segcurve"].buffer_proc(numframes, deltatime);
            modtor_param * p_min = params["min"].buffer_proc(numframes, deltatime);
            modtor_param * p_max = params["max"].buffer_proc(numframes, deltatime);
            modtor_param * p_curve = params["curve"].buffer_proc(numframes, deltatime);
            modtor_param * p_mul = params["mul"].buffer_proc(numframes, deltatime);
            modtor_param * p_add = params["add"].buffer_proc(numframes, deltatime);
            modtor_param * p_count = params["count"].buffer_proc(numframes, deltatime);
            
            modtor_param * p_time = 0;
            
            bool time_mode = params.find("time") != params.end();
            
            if(time_mode)
                p_time = params["time"].buffer_proc(numframes, deltatime);

            if(params.find("varitime") != params.end())
                p_varifreq = params["varitime"].buffer_proc(numframes, deltatime);
            
            for(int i=0; i<numframes; i++)
            {
                // get all the parameters (check time or freq format)
                double freq = p_freq->get_b(i);
                double varifreq = p_varifreq->get_b(i);
                
                if(time_mode)
                    freq = 1000./ std::clamp(p_time->get_b(i),0.001,10000000.);
                
                double walk = p_walk->get_b(i);
                double min = p_min->get_b(i);
                double max = p_max->get_b(i);
                double curve = p_curve->get_b(i);
                double segcurve = p_segcurve->get_b(i);
                double add = p_add->get_b(i);
                double mul = p_mul->get_b(i);
                
                /** count special parameter: if > 0
                    • freq = 1./count
                    • deltatime = 1000.
                */
                double count = p_count->get_b(i);
                if(count >= 0.)
                {
                    freq = (count > 0.01)? 1./count : 100. ;
                    deltatime = 1000.;
                }
                double r_freq = freq * exp2( m_varifreq );
                phase += r_freq*deltatime/1000.;
                
                if (phase > 1.)
                {   // reset : new freq jitter varifreq
                    m_varifreq = mt_rand_time(mt_gen_time)*varifreq;
                    phase = fmodf(phase,1.);
                    
                    // new random target value
                    m_rand_prev = m_rand_target;
                    m_rand_target = maxlang::fold(m_rand_prev + mt_rand_val(mt_gen_val) * walk * 2,-1,1);
                    
                    // sample curve param
                    m_curve=curve;
                    output_scale.setcurve(m_curve);
                }
                
                output_scale.setout_min(min);
                output_scale.setout_max(max);
                
                double w = m_rand_target;
                values[i] = add+(output_scale.apply(w)*mul);
                
            }
        }
        
        double get(double deltatime) override
        {
            // get all the parameters (check time or freq format)
            double freq = params["freq"].get(deltatime);
            double varifreq = params["varifreq"].get(deltatime);
            
            if(params.find("time") != params.end())
                freq = 1000./ std::clamp(params["time"].get(deltatime),0.001,10000000.);
            if(params.find("varitime") != params.end())
                varifreq = params["varitime"].get(deltatime);
            
            double walk = params["walk"].get(deltatime);
            double min = params["min"].get(deltatime);
            double max = params["max"].get(deltatime);
            double curve = params["curve"].get(deltatime);
            double add = params["add"].get(deltatime);
            double mul = params["mul"].get(deltatime);
            
            /** count special parameter: if > 0
                • freq = 1./count
                • deltatime = 1000.
            */
            double count = params["count"].get(deltatime);
            if(count >= 0.)
            {
                freq = (count > 0.01)? 1./count : 100. ;
                deltatime = 1000.;
            }
            double r_freq = freq * exp2( m_varifreq );
            phase += r_freq*deltatime/1000.;
            
            if (phase > 1.)
            {   // reset : new freq jitter varifreq
                m_varifreq = mt_rand_time(mt_gen_time)*varifreq;
                phase = fmodf(phase,1.);
                
                // new random target value
                m_rand_prev = m_rand_target;
                m_rand_target = maxlang::fold(m_rand_prev + mt_rand_val(mt_gen_val) * walk * 2,-1,1);
                
                // sample curve param
                m_curve=curve;
                output_scale.setcurve(m_curve);
            }
            
            output_scale.setout_min(min);
            output_scale.setout_max(max);
            
            double w = m_rand_target;
            return add+(output_scale.apply(w)*mul);
        }
    };

    class m_choice : public modtor {
        
    public:
        
        m_choice(scope * modtor_scope)
        {
            
            modtor_classname = "choice";
            
            _scope = modtor_scope;
            _scope->touch(this);
            
            m_list = std::vector<double>({0.,1.});
            m_list_l = m_list.size();
            params.insert(std::pair<std::string, modtor_param>("freq",modtor_param(6.)));
            params.insert(std::pair<std::string, modtor_param>("varifreq",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("list",modtor_param(m_list)));
            params.insert(std::pair<std::string, modtor_param>("mul",modtor_param(1.)));
            params.insert(std::pair<std::string, modtor_param>("add",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("count",modtor_param(-1.)));
            
            mt_gen_time = std::mt19937(rd_dev());
            mt_rand_time = std::uniform_real_distribution<double>(-1.,1.);
            
            mt_gen_val = std::mt19937(rd_dev());
            mt_rand_val = std::uniform_real_distribution<double>(0.,0.99);
            
        };
        
        ~m_choice()
        {
            params.clear();
            
            mt_gen_time.~mersenne_twister_engine();
            mt_rand_time.~uniform_real_distribution();
            mt_gen_val.~mersenne_twister_engine();
            mt_rand_val.~uniform_real_distribution();
        }
        
        double phase = 1.;
        double m_varifreq = 0.;
        double m_curve = 0.;
        double m_rand_prev = 0.;
        double m_rand_target = 0.;
        int m_last_index=0;
        
        std::vector<double> m_list;
        int m_list_l;
        std::mt19937 mt_gen_time, mt_gen_val;
        std::uniform_real_distribution<double> mt_rand_time, mt_rand_val;
        
        void seed(std::string seed_str) override
        {
            std::seed_seq rd_seed_val(seed_str.begin(),seed_str.end());
            // for time randomization : reverse string
            std::reverse(seed_str.begin(),seed_str.end());
            std::seed_seq rd_seed_time(seed_str.begin(),seed_str.end());
            
            mt_gen_val = std::mt19937(rd_seed_val);
            mt_gen_time = std::mt19937(rd_seed_time);
        }
        
        void sync(double _phase) override
        {
            phase = _phase;
        }
        
        void perform(double * values,int numframes,double deltatime) override
        {
            modtor_param * p_freq = params["freq"].buffer_proc(numframes, deltatime);
            modtor_param * p_varifreq = params["varifreq"].buffer_proc(numframes, deltatime);
            modtor_param * p_mul = params["mul"].buffer_proc(numframes, deltatime);
            modtor_param * p_add = params["add"].buffer_proc(numframes, deltatime);
            modtor_param * p_count = params["count"].buffer_proc(numframes, deltatime);
            modtor_param * p_list = &params["list"];
            
            modtor_param * p_time = 0;
            
            bool time_mode = params.find("time") != params.end();
            
            if(time_mode)
                p_time = params["time"].buffer_proc(numframes, deltatime);

            if(params.find("varitime") != params.end())
                p_varifreq = params["varitime"].buffer_proc(numframes, deltatime);
            
            for(int i=0; i<numframes; i++)
            {
                // get all the parameters (check time or freq format)
                double freq = p_freq->get_b(i);
                double varifreq = p_varifreq->get_b(i);
                
                if(time_mode)
                    freq = 1000./ std::clamp(p_time->get_b(i),0.001,10000000.);
                
                double mul = p_mul->get_b(i);
                double add = p_add->get_b(i);
                /** count special parameter: if > 0
                    • freq = 1./count
                    • deltatime = 1000.
                */
                double count = p_count->get_b(i);
                if(count >= 0.)
                {
                    freq = (count > 0.01)? 1./count : 100. ;
                    deltatime = 1000.;
                }
                
                double r_freq = freq * exp2( m_varifreq );
                phase += r_freq*deltatime/1000.;
                
                if (phase > 1.)
                {   // reset : new freq jitter varifreq
                    
                    m_varifreq = mt_rand_time(mt_gen_time)*varifreq;
                    phase = fmodf(phase,1.);
                    // TODO : segfault when changing list too often ( not thread safe )
                    m_list = p_list->getlist();
                    m_list_l  = m_list.size();
                    //printf("m_list_l %d\n",m_list_l);
                    // new random index value
                    m_rand_prev = m_rand_target;
                    int index = floor(mt_rand_val(mt_gen_val) * (m_list_l-1));
                    // don't choose same index
                    if(index >= m_last_index)
                        index ++;
                    
                    m_last_index = index;
                    // get value from list
                    m_rand_target = m_list[m_last_index];

                }
                
                values[i] = (m_rand_target * mul)+add;
                
            }
        }
        
        double get(double deltatime) override
        {
            // get all the parameters (check time or freq format)
            double freq = params["freq"].get(deltatime);
            double varifreq = params["varifreq"].get(deltatime);
            
            if(params.find("time") != params.end())
                freq = 1000./ std::clamp(params["time"].get(deltatime),0.001,10000000.);
            if(params.find("varitime") != params.end())
                varifreq = params["varitime"].get(deltatime);
            
            double mul = params["mul"].get(deltatime);
            double add = params["add"].get(deltatime);
            /** count special parameter: if > 0
                • freq = 1./count
                • deltatime = 1000.
            */
            double count = params["count"].get(deltatime);
            if(count >= 0.)
            {
                freq = (count > 0.01)? 1./count : 100. ;
                deltatime = 1000.;
            }
            
            double r_freq = freq * exp2( m_varifreq );
            phase += r_freq*deltatime/1000.;
            
            if (phase > 1.)
            {   // reset : new freq jitter varifreq
                
                m_varifreq = mt_rand_time(mt_gen_time)*varifreq;
                phase = fmodf(phase,1.);
                // TODO : segfault when changing list too often ( not thread safe )
                m_list = params["list"].getlist();
                m_list_l  = m_list.size();
                //printf("m_list_l %d\n",m_list_l);
                // new random index value
                m_rand_prev = m_rand_target;
                int index = floor(mt_rand_val(mt_gen_val) * (m_list_l-1));
                // don't choose same index
                if(index >= m_last_index)
                    index ++;
                
                m_last_index = index;
                // get value from list
                m_rand_target = m_list[m_last_index];

            }
            
            return (m_rand_target * mul)+add;
        }
    };
    
    class m_choicei : public modtor {
        
    public:
        
        m_choicei(scope * modtor_scope)
        {
            
            modtor_classname = "choicei";
            
            _scope = modtor_scope;
            _scope->touch(this);
            
            m_list = std::vector<double>({0.,1.});
            m_list_l = m_list.size();
            params.insert(std::pair<std::string, modtor_param>("freq",modtor_param(6.)));
            params.insert(std::pair<std::string, modtor_param>("varifreq",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("list",modtor_param(m_list)));
            params.insert(std::pair<std::string, modtor_param>("mul",modtor_param(1.)));
            params.insert(std::pair<std::string, modtor_param>("add",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("segcurve",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("count",modtor_param(-1.)));
            
            mt_gen_time = std::mt19937(rd_dev());
            mt_rand_time = std::uniform_real_distribution<double>(-1.,1.);
            
            mt_gen_val = std::mt19937(rd_dev());
            mt_rand_val = std::uniform_real_distribution<double>(0.,0.99);
            
            segment_scale.setin_minmax(0., 1.);
            segment_scale.setout_min(0.);
            segment_scale.setout_max(1.);
            
        };
        
        ~m_choicei()
        {
            params.clear();
            
            mt_gen_time.~mersenne_twister_engine();
            mt_rand_time.~uniform_real_distribution();
            mt_gen_val.~mersenne_twister_engine();
            mt_rand_val.~uniform_real_distribution();
        }
        
        double phase = 1.;
        double m_varifreq = 0.;
        double m_curve = 0.;
        double m_rand_prev = 0.;
        double m_rand_target = 0.;
        int m_last_index=0;
        double m_segcurve = 0.;
        scale_curve segment_scale;
        
        std::vector<double> m_list;
        int m_list_l;
        std::mt19937 mt_gen_time, mt_gen_val;
        std::uniform_real_distribution<double> mt_rand_time, mt_rand_val;
        
        void seed(std::string seed_str) override
        {
            std::seed_seq rd_seed_val(seed_str.begin(),seed_str.end());
            // for time randomization : reverse string
            std::reverse(seed_str.begin(),seed_str.end());
            std::seed_seq rd_seed_time(seed_str.begin(),seed_str.end());
            
            mt_gen_val = std::mt19937(rd_seed_val);
            mt_gen_time = std::mt19937(rd_seed_time);
        }
        
        void sync(double _phase) override
        {
            phase = _phase;
        }
        
        void perform(double * values,int numframes,double deltatime) override
        {
            modtor_param * p_freq = params["freq"].buffer_proc(numframes, deltatime);
            modtor_param * p_varifreq = params["varifreq"].buffer_proc(numframes, deltatime);
            modtor_param * p_mul = params["mul"].buffer_proc(numframes, deltatime);
            modtor_param * p_add = params["add"].buffer_proc(numframes, deltatime);
            modtor_param * p_count = params["count"].buffer_proc(numframes, deltatime);
            modtor_param * p_segcurve = params["segcurve"].buffer_proc(numframes, deltatime);
            modtor_param * p_list = &params["list"];
            
            modtor_param * p_time = 0;
            
            bool time_mode = params.find("time") != params.end();
            
            if(time_mode)
                p_time = params["time"].buffer_proc(numframes, deltatime);

            if(params.find("varitime") != params.end())
                p_varifreq = params["varitime"].buffer_proc(numframes, deltatime);
            
            for(int i=0; i<numframes; i++)
            {
                // get all the parameters (check time or freq format)
                double freq = p_freq->get_b(i);
                double varifreq = p_varifreq->get_b(i);
                
                double segcurve = p_segcurve->get_b(i);
                
                if(time_mode)
                    freq = 1000./ std::clamp(p_time->get_b(i),0.001,10000000.);
                
                double mul = p_mul->get_b(i);
                double add = p_add->get_b(i);
                /** count special parameter: if > 0
                    • freq = 1./count
                    • deltatime = 1000.
                */
                double count = p_count->get_b(i);
                if(count >= 0.)
                {
                    freq = (count > 0.01)? 1./count : 100. ;
                    deltatime = 1000.;
                }
                
                double r_freq = freq * exp2( m_varifreq );
                phase += r_freq*deltatime/1000.;
                
                if (phase > 1.)
                {   // reset : new freq jitter varifreq
                    
                    m_varifreq = mt_rand_time(mt_gen_time)*varifreq;
                    phase = fmodf(phase,1.);
                    // TODO : segfault when changing list too often ( not thread safe )
                    m_list = p_list->getlist();
                    m_list_l  = m_list.size();
                    //printf("m_list_l %d\n",m_list_l);
                    // new random index value
                    m_rand_prev = m_rand_target;
                    int index = floor(mt_rand_val(mt_gen_val) * (m_list_l-1));
                    // don't choose same index
                    if(index >= m_last_index)
                        index ++;
                    
                    m_last_index = index;
                    // get value from list
                    m_rand_prev = m_rand_target;
                    m_rand_target = m_list[m_last_index];
                    
                    // sample segcurve value
                    m_segcurve=segcurve;
                    segment_scale.setcurve(m_segcurve);
                    
                }
                double phase_c = segment_scale.apply(phase);
                values[i] = add+(((1-phase_c)*m_rand_prev + phase_c*m_rand_target)*mul);
                
            }
        }
        
        double get(double deltatime) override
        {
            // get all the parameters (check time or freq format)
            double freq = params["freq"].get(deltatime);
            double varifreq = params["varifreq"].get(deltatime);
            
            if(params.find("time") != params.end())
                freq = 1000./ std::clamp(params["time"].get(deltatime),0.001,10000000.);
            if(params.find("varitime") != params.end())
                varifreq = params["varitime"].get(deltatime);
            
            double mul = params["mul"].get(deltatime);
            double add = params["add"].get(deltatime);
            double segcurve = params["segcurve"].get(deltatime);
            /** count special parameter: if > 0
                • freq = 1./count
                • deltatime = 1000.
            */
            double count = params["count"].get(deltatime);
            if(count >= 0.)
            {
                freq = (count > 0.01)? 1./count : 100. ;
                deltatime = 1000.;
            }
            
            double r_freq = freq * exp2( m_varifreq );
            phase += r_freq*deltatime/1000.;
            
            if (phase > 1.)
            {   // reset : new freq jitter varifreq
                
                m_varifreq = mt_rand_time(mt_gen_time)*varifreq;
                phase = fmodf(phase,1.);
                // TODO : segfault when changing list too often ( not thread safe )
                m_list = params["list"].getlist();
                m_list_l  = m_list.size();
                //printf("m_list_l %d\n",m_list_l);
                // new random index value
                m_rand_prev = m_rand_target;
                int index = floor(mt_rand_val(mt_gen_val) * (m_list_l-1));
                // don't choose same index
                if(index >= m_last_index)
                    index ++;
                
                m_last_index = index;
                // get value from list
                m_rand_prev = m_rand_target;
                m_rand_target = m_list[m_last_index];
                
                // sample segcurve value
                m_segcurve=segcurve;
                segment_scale.setcurve(m_segcurve);
                
            }
            double phase_c = segment_scale.apply(phase);
            return add+(((1-phase_c)*m_rand_prev + phase_c*m_rand_target)*mul);
        }
    };
    
    
    class m_seqi : public modtor {
        
    public:
        
        m_seqi(scope * modtor_scope)
        {
            
            modtor_classname = "seqi";
            
            _scope = modtor_scope;
            _scope->touch(this);
            
            m_list = std::vector<double>({0.1,0.3,0.5,0.8});
            m_list_l = m_list.size();
            params.insert(std::pair<std::string, modtor_param>("freq",modtor_param(6.)));
            params.insert(std::pair<std::string, modtor_param>("varifreq",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("list",modtor_param(m_list)));
            params.insert(std::pair<std::string, modtor_param>("mul",modtor_param(1.)));
            params.insert(std::pair<std::string, modtor_param>("add",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("segcurve",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("segcurveshape",modtor_param(0)));
            params.insert(std::pair<std::string, modtor_param>("play",modtor_param(1)));
            params.insert(std::pair<std::string, modtor_param>("loop",modtor_param(1)));
            params.insert(std::pair<std::string, modtor_param>("count",modtor_param(-1.)));
            
            mt_gen_time = std::mt19937(rd_dev());
            mt_rand_time = std::uniform_real_distribution<double>(-1.,1.);

            segment_scale.setin_minmax(0., 1.);
            segment_scale.setout_min(0.);
            segment_scale.setout_max(1.);
            
        };
        
        ~m_seqi()
        {
            params.clear();
            
            mt_gen_time.~mersenne_twister_engine();
            mt_rand_time.~uniform_real_distribution();
            
        }
        
        double phase = 1.;
        double m_varifreq = 0.;
        double m_curve = 0.;
        double m_val_prev = 0.;
        double m_val_target = 0.;
        int m_last_index=-1;
        int m_playing=0;
        int m_looping=0;
        double m_segcurve = 0.;
        scale_curve segment_scale;
        
        std::vector<double> m_list;
        int m_list_l;
        std::mt19937 mt_gen_time;
        std::uniform_real_distribution<double> mt_rand_time;
        
        void seed(std::string seed_str) override
        {
            // for time randomization : reverse string
            std::reverse(seed_str.begin(),seed_str.end());
            std::seed_seq rd_seed_time(seed_str.begin(),seed_str.end());
            
            mt_gen_time = std::mt19937(rd_seed_time);
        }
        
        void sync(double _phase) override
        {
            phase = _phase;
        }
        
        void perform(double * values,int numframes,double deltatime) override
        {
            modtor_param * p_freq = params["freq"].buffer_proc(numframes, deltatime);
            modtor_param * p_varifreq = params["varifreq"].buffer_proc(numframes, deltatime);
            modtor_param * p_mul = params["mul"].buffer_proc(numframes, deltatime);
            modtor_param * p_add = params["add"].buffer_proc(numframes, deltatime);
            modtor_param * p_count = params["count"].buffer_proc(numframes, deltatime);
            modtor_param * p_segcurve = params["segcurve"].buffer_proc(numframes, deltatime);
            modtor_param * p_segcurve_s = params["segcurveshape"].buffer_proc(numframes, deltatime);
            modtor_param * p_play = params["play"].buffer_proc(numframes, deltatime);
            modtor_param * p_loop = params["loop"].buffer_proc(numframes, deltatime);
            modtor_param * p_list = &params["list"];
            
            modtor_param * p_time = 0;
            
            m_list = p_list->getlist();
            m_list_l  = m_list.size();
            if(m_list_l == 0 )
                return;
            
            bool time_mode = params.find("time") != params.end();
            
            if(time_mode)
                p_time = params["time"].buffer_proc(numframes, deltatime);

            if(params.find("varitime") != params.end())
                p_varifreq = params["varitime"].buffer_proc(numframes, deltatime);
            
            for(int i=0; i<numframes; i++)
            {
                // get all the parameters (check time or freq format)
                double freq = p_freq->get_b(i);
                double varifreq = p_varifreq->get_b(i);
                
                double segcurve = p_segcurve->get_b(i);
                bool segcurve_s = p_segcurve_s->get_b(i);
                
                if(time_mode)
                    freq = 1000./ std::clamp(p_time->get_b(i),0.001,10000000.);
                
                double mul = p_mul->get_b(i);
                double add = p_add->get_b(i);
                /** count special parameter: if > 0
                    • freq = 1./count
                    • deltatime = 1000.
                */
                
                int play = p_play->get_b(i)>0;
                int loop = p_loop->get_b(i)>0;
                
                double count = p_count->get_b(i);
                if(count >= 0.)
                {
                    freq = (count > 0.01)? 1./count : 100. ;
                    deltatime = 1000.;
                }
                
                if(!m_playing && play) // restart the sequence
                {
                    m_playing = play;
                    m_looping = 1;
                    m_last_index = 0;
                    m_val_prev = m_list[m_last_index];
                    m_val_target = m_list[(m_last_index+1)%m_list_l];
                    phase = 0;
                    
                    values[i] =  add+m_val_prev*mul;
                    
                }
                else
                {
                    if(!play)
                    {
                        m_playing=play;
                        m_looping=0;
                    }
                    
                    if(m_playing && m_looping)
                    {
                        double r_freq = freq * exp2( m_varifreq );
                        phase += r_freq*deltatime/1000.;
                    
                    
                        if (phase > 1.)
                        {   // reset : new freq jitter varifreq

                            
                            m_varifreq = mt_rand_time(mt_gen_time)*varifreq;
                            phase = fmodf(phase,1.);
                            
                            
                            
                            
                            if(!loop && m_last_index+2 == m_list_l ) // stops at end of sequence if loop is off
                            {
                                m_last_index = m_last_index+1;
                                m_val_prev = m_list[m_last_index];
                                m_val_target = m_val_prev;
                                
                                m_looping = 0;
                                
                            }else
                            {
                                m_last_index = (m_last_index+1)%m_list_l;
                                m_val_prev = m_val_target;
                                m_val_target = m_list[m_last_index];
                            }
                            
                            // sample segcurve value
                            m_segcurve=segcurve;
                            segment_scale.setcurve(m_segcurve);
                            segment_scale.setcurve_s(segcurve_s);
                            
                        }
                    }
                    
                    double phase_c = segment_scale.apply(std::clamp(phase,0.,1.));
                    values[i] =  add+(((1-phase_c)*m_val_prev + phase_c*m_val_target)*mul);
                        
                    }
                
            }
        }
        
        double get(double deltatime) override
        {
            // get all the parameters (check time or freq format)
            double freq = params["freq"].get(deltatime);
            double varifreq = params["varifreq"].get(deltatime);
            
            if(params.find("time") != params.end())
                freq = 1000./ std::clamp(params["time"].get(deltatime),0.001,10000000.);
            if(params.find("varitime") != params.end())
                varifreq = params["varitime"].get(deltatime);
            
            double mul = params["mul"].get(deltatime);
            double add = params["add"].get(deltatime);
            double segcurve = params["segcurve"].get(deltatime);
            bool segcurve_s = params["segcurveshape"].get(deltatime)>0;
            int play = params["play"].get(deltatime)>0;
            int loop = params["loop"].get(deltatime)>0;
            /** count special parameter: if > 0
                • freq = 1./count
                • deltatime = 1000.
            */
            double count = params["count"].get(deltatime);
            if(count >= 0.)
            {
                freq = (count > 0.01)? 1./count : 100. ;
                deltatime = 1000.;
            }
            
            m_list = params["list"].getlist();
            m_list_l  = m_list.size();
            if(m_list_l == 0 )
                return 0.;
            
            if(!m_playing && play) // restart the sequence
            {
                m_playing = play;
                m_looping = 1;
                m_last_index = 0;
                m_val_prev = m_list[m_last_index];
                m_val_target = m_list[(m_last_index+1)%m_list_l];
                phase = 0;
                
                return add+m_val_prev*mul;
                
            }
            
            if(!play)
            {
                m_playing=play;
                m_looping=0;
            }
            
            if(m_playing && m_looping)
            {
                double r_freq = freq * exp2( m_varifreq );
                phase += r_freq*deltatime/1000.;
            
            
                if (phase > 1.)
                {   // reset : new freq jitter varifreq

                    
                    m_varifreq = mt_rand_time(mt_gen_time)*varifreq;
                    phase = fmodf(phase,1.);
                    
                    
                    
                    
                    if(!loop && m_last_index+2 == m_list_l ) // stops at end of sequence if loop is off
                    {
                        m_last_index = m_last_index+1;
                        m_val_prev = m_list[m_last_index];
                        m_val_target = m_val_prev;
                        
                        m_looping = 0;
                        
                    }else
                    {
                        m_last_index = (m_last_index+1)%m_list_l;
                        m_val_prev = m_val_target;
                        m_val_target = m_list[m_last_index];
                    }
                    
                    // sample segcurve value
                    m_segcurve=segcurve;
                    segment_scale.setcurve(m_segcurve);
                    segment_scale.setcurve_s(segcurve_s);
                    
                }
            }
            
            double phase_c = segment_scale.apply(std::clamp(phase,0.,1.));
            return add+(((1-phase_c)*m_val_prev + phase_c*m_val_target)*mul);
        }
    };
    
    class m_seq : public modtor {
        
    public:
        
        m_seq(scope * modtor_scope)
        {
            
            modtor_classname = "seq";
            
            _scope = modtor_scope;
            _scope->touch(this);
            
            m_list = std::vector<double>({0.1,0.3,0.5,0.8});
            m_list_l = m_list.size();
            params.insert(std::pair<std::string, modtor_param>("freq",modtor_param(6.)));
            params.insert(std::pair<std::string, modtor_param>("varifreq",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("list",modtor_param(m_list)));
            params.insert(std::pair<std::string, modtor_param>("mul",modtor_param(1.)));
            params.insert(std::pair<std::string, modtor_param>("add",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("play",modtor_param(1)));
            params.insert(std::pair<std::string, modtor_param>("loop",modtor_param(1)));
            params.insert(std::pair<std::string, modtor_param>("count",modtor_param(-1.)));
            
            mt_gen_time = std::mt19937(rd_dev());
            mt_rand_time = std::uniform_real_distribution<double>(-1.,1.);
            
        };
        
        ~m_seq()
        {
            params.clear();
            
            mt_gen_time.~mersenne_twister_engine();
            mt_rand_time.~uniform_real_distribution();

        }
        
        double phase = 1.;
        double m_varifreq = 0.;
        double m_curve = 0.;
        double m_val = 0.;
        int m_last_index=-1;
        int m_playing=0;
        int m_looping=0;
        
        std::vector<double> m_list;
        int m_list_l;
        std::mt19937 mt_gen_time;
        std::uniform_real_distribution<double> mt_rand_time;
        
        void seed(std::string seed_str) override
        {
            // for time randomization : reverse string
            std::reverse(seed_str.begin(),seed_str.end());
            std::seed_seq rd_seed_time(seed_str.begin(),seed_str.end());
            
            mt_gen_time = std::mt19937(rd_seed_time);
        }
        
        void sync(double _phase) override
        {
            phase = _phase;
        }
        
        void perform(double * values,int numframes,double deltatime) override
        {
            modtor_param * p_freq = params["freq"].buffer_proc(numframes, deltatime);
            modtor_param * p_varifreq = params["varifreq"].buffer_proc(numframes, deltatime);
            modtor_param * p_mul = params["mul"].buffer_proc(numframes, deltatime);
            modtor_param * p_add = params["add"].buffer_proc(numframes, deltatime);
            modtor_param * p_count = params["count"].buffer_proc(numframes, deltatime);
            modtor_param * p_play = params["play"].buffer_proc(numframes, deltatime);
            modtor_param * p_loop = params["loop"].buffer_proc(numframes, deltatime);
            modtor_param * p_list = &params["list"];
            
            modtor_param * p_time = 0;
            
            bool time_mode = params.find("time") != params.end();
            
            if(time_mode)
                p_time = params["time"].buffer_proc(numframes, deltatime);

            if(params.find("varitime") != params.end())
                p_varifreq = params["varitime"].buffer_proc(numframes, deltatime);
            
            for(int i=0; i<numframes; i++)
            {
                // get all the parameters (check time or freq format)
                double freq = p_freq->get_b(i);
                double varifreq = p_varifreq->get_b(i);
                
                
                if(time_mode)
                    freq = 1000./ std::clamp(p_time->get_b(i),0.001,10000000.);
                
                double mul = p_mul->get_b(i);
                double add = p_add->get_b(i);
                /** count special parameter: if > 0
                    • freq = 1./count
                    • deltatime = 1000.
                */
                
                int play = p_play->get_b(i)>0;
                int loop = p_loop->get_b(i)>0;
                
                double count = p_count->get_b(i);
                if(count >= 0.)
                {
                    freq = (count > 0.01)? 1./count : 100. ;
                    deltatime = 1000.;
                }
                
                m_list = p_list->getlist();
                m_list_l  = m_list.size();
                if(m_list_l == 0 )
                    return 0.;
                
                if(!m_playing && play) // restart the sequence
                {
                    m_playing = play;
                    m_looping = 1;
                    m_last_index = 0;
                    m_val = m_list[m_last_index];
                    phase = 0;
                    
                    values[i] = add+(m_val*mul);
                    
                }
                else
                    
                {
                    if(!play)
                    {
                        m_playing=play;
                        m_looping=0;
                    }
                    
                    if(m_playing && m_looping)
                    {
                        double r_freq = freq * exp2( m_varifreq );
                        phase += r_freq*deltatime/1000.;
                        
                        
                        if (phase >= 1.)
                        {   // reset : new freq jitter varifreq
                            
                            
                            m_varifreq = mt_rand_time(mt_gen_time)*varifreq;
                            phase = fmodf(phase,1.);
                            
                            
                            
                            
                            if(!loop && m_last_index+2 == m_list_l ) // stops at end of sequence if loop is off
                            {
                                m_last_index = m_last_index+1;
                                m_val = m_list[m_last_index];
                                
                                m_looping = 0;
                                
                            }else
                            {
                                m_last_index = (m_last_index+1)%m_list_l;
                                m_val = m_list[m_last_index];
                            }
                            
                            
                        }
                    }
                    
                    values[i] = add+(m_val*mul);
                        
                }
                
            }
        }
        
        double get(double deltatime) override
        {
            // get all the parameters (check time or freq format)
            double freq = params["freq"].get(deltatime);
            double varifreq = params["varifreq"].get(deltatime);
            
            if(params.find("time") != params.end())
                freq = 1000./ std::clamp(params["time"].get(deltatime),0.001,10000000.);
            if(params.find("varitime") != params.end())
                varifreq = params["varitime"].get(deltatime);
            
            double mul = params["mul"].get(deltatime);
            double add = params["add"].get(deltatime);
            int play = params["play"].get(deltatime)>0;
            int loop = params["loop"].get(deltatime)>0;
            /** count special parameter: if > 0
                • freq = 1./count
                • deltatime = 1000.
            */
            double count = params["count"].get(deltatime);
            if(count >= 0.)
            {
                freq = (count > 0.01)? 1./count : 100. ;
                deltatime = 1000.;
            }
            
            m_list = params["list"].getlist();
            m_list_l  = m_list.size();
            if(m_list_l == 0 )
                return 0.;
            
            if(!m_playing && play) // restart the sequence
            {
                m_playing = play;
                m_looping = 1;
                m_last_index = 0;
                m_val = m_list[m_last_index];
                phase = 0;
                
                return add+(m_val*mul);
                
            }
            
            if(!play)
            {
                m_playing=play;
                m_looping=0;
            }
            
            if(m_playing && m_looping)
            {
                double r_freq = freq * exp2( m_varifreq );
                phase += r_freq*deltatime/1000.;
                
                
                if (phase >= 1.)
                {   // reset : new freq jitter varifreq
                    
                    
                    m_varifreq = mt_rand_time(mt_gen_time)*varifreq;
                    phase = fmodf(phase,1.);
                    
                    
                    
                    
                    if(!loop && m_last_index+2 == m_list_l ) // stops at end of sequence if loop is off
                    {
                        m_last_index = m_last_index+1;
                        m_val = m_list[m_last_index];
                        
                        m_looping = 0;
                        
                    }else
                    {
                        m_last_index = (m_last_index+1)%m_list_l;
                        m_val = m_list[m_last_index];
                    }
                    
                    
                }
            }
            
            return add+(m_val*mul);
        }
    };
    
    class m_env : public modtor {
        
    public:
        
        m_env(scope * modtor_scope)
        {
            
            modtor_classname = "env";
            
            _scope = modtor_scope;
            _scope->touch(this);
            
            // y1 dt1 y2 dt2 y3

            params.insert(std::pair<std::string, modtor_param>("time",modtor_param(1000.)));
            params.insert(std::pair<std::string, modtor_param>("varitime",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("list",modtor_param(std::vector<double>({0.,0.3,1,0.3,1.,0.3,0.}))));
            params.insert(std::pair<std::string, modtor_param>("mul",modtor_param(1.)));
            params.insert(std::pair<std::string, modtor_param>("add",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("segcurve",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("play",modtor_param(1)));
            params.insert(std::pair<std::string, modtor_param>("loop",modtor_param(0)));
            params.insert(std::pair<std::string, modtor_param>("count",modtor_param(-1.)));
            
            mt_gen_time = std::mt19937(rd_dev());
            mt_rand_time = std::uniform_real_distribution<double>(-1.,1.);
            
            segment_scale.setin_minmax(0., 1.);
            segment_scale.setout_min(0.);
            segment_scale.setout_max(1.);
            
        };
        
        ~m_env()
        {
            params.clear();
            
            mt_gen_time.~mersenne_twister_engine();
            mt_rand_time.~uniform_real_distribution();

        }
        
        struct seg {
            double min;
            double max;
            double onset_f; // between 0. and 1.
            double offset_f;
        };
        
        double phase = 1.;
        double m_time=1000;
        double m_varitime = 0.;
        double m_curve = 0.;
        double m_val = 0., m_end_val=0.;
        int m_seg_index=-1;
        int m_playing=0;
        int m_looping=0;
        double m_segcurve = -666.;
        scale_curve segment_scale;
        
        std::vector<double> m_list;
        std::vector<double> p_list;
        std::vector<struct seg> m_segments;
        int m_seg_l;
        std::mt19937 mt_gen_time;
        std::uniform_real_distribution<double> mt_rand_time;
        
        void seed(std::string seed_str) override
        {
            // for time randomization : reverse string
            std::reverse(seed_str.begin(),seed_str.end());
            std::seed_seq rd_seed_time(seed_str.begin(),seed_str.end());
            
            mt_gen_time = std::mt19937(rd_seed_time);
        }
        
        void sync(double _phase) override
        {
            phase = _phase;
            for (auto &p : params)
                p.second.sync(phase);
        }
        
        int parse_segments(std::vector<double> list)
        {
            // assertion:
            // len must be odd
            // len >=3
            // dtime mustbe positive

            int l = list.size();
            if(l%2==1 && l >=3)
            {
                int np = (l-1)/2;
                double tot_length=0., accum_length=0., seg_length;
                m_segments.clear();
                
                for(int k=0; k<np;k++)
                {
                    double dt = list[(k*2)+1];
                    if(dt >=0.)
                        tot_length += dt;
                    else
                        return 0; // ERROR one dt is negative
                }
                for(int i=0; i<np; i++)
                {
                    struct seg * c_seg = new seg();
                    c_seg->min = list[i*2];
                    c_seg->max = list[(i+1)*2];
                    c_seg->onset_f = accum_length;
                    seg_length = list[(i*2)+1] / tot_length;
                    c_seg->offset_f =  c_seg->onset_f + seg_length;
                    accum_length += seg_length;
                    m_segments.push_back(*c_seg);
                }
                // endval
                m_end_val = list[l-1];
                return np;
            }
            return 0; // ERROR wrong size of arguments
        }
        
        void perform(double * values,int numframes,double deltatime) override
        {
            modtor_param * p_time = params["time"].buffer_proc(numframes, deltatime);
            modtor_param * p_varitime = params["varitime"].buffer_proc(numframes, deltatime);
            modtor_param * p_mul = params["mul"].buffer_proc(numframes, deltatime);
            modtor_param * p_add = params["add"].buffer_proc(numframes, deltatime);
            modtor_param * p_count = params["count"].buffer_proc(numframes, deltatime);
            modtor_param * p_segcurve = params["segcurve"].buffer_proc(numframes, deltatime);
            modtor_param * p_play = params["play"].buffer_proc(numframes, deltatime);
            modtor_param * p_loop = params["loop"].buffer_proc(numframes, deltatime);
            modtor_param * p_list_ = &params["list"];
            
            modtor_param * p_freq = 0;
            
            p_list = p_list_->getlist();
            
            bool freq_mode = params.find("freq") != params.end();
            
            if(freq_mode)
                p_freq = params["freq"].buffer_proc(numframes, deltatime);

            if(params.find("varifreq") != params.end())
                p_varitime = params["varifreq"].buffer_proc(numframes, deltatime);
            
            for(int i=0; i<numframes; i++)
            {
                // get all the parameters (check time or freq format)
                double time = p_time->get_b(i);
                double varitime = p_varitime->get_b(i);
                
                double segcurve = p_segcurve->get_b(i);
                
                if(freq_mode)
                    time = 1000./ std::clamp(p_freq->get_b(i),0.001,10000000.);
                
                
                double mul = p_mul->get_b(i);
                double add = p_add->get_b(i);
                /** count special parameter: if > 0
                    • freq = 1./count
                    • deltatime = 1000.
                */
                
                int play = p_play->get_b(i)>0;
                int loop = p_loop->get_b(i)>0;
                
                double count = p_count->get_b(i);
                if(count >= 0.)
                {
                    time = (count > 0.01)? count * 1000 : 10. ;
                    deltatime = 1000.;
                }
                
                
                if(m_list != p_list)
                {
                    m_list = p_list;
                    m_seg_l = parse_segments(m_list);
                }
                
                if(!m_seg_l)
                    return 0.;
                
                if(!m_playing && play) // restart the sequence
                {
                    m_playing = play;
                    m_looping = 1;
                    m_seg_index = 0;

                    phase = 0;
                    
                    // choose new length for env
                    m_varitime = mt_rand_time(mt_gen_time)*varitime;
                    m_time = time * exp2( m_varitime );
                    
                }
                
                if(!play)
                {
                    m_playing=play;
                    m_looping=0;
                }
                
                if(m_playing && m_looping)
                {
                    
                    int seg_index = 0;
                    while(phase > m_segments[seg_index].offset_f)
                    {
                        seg_index++;
                    }
                    
                    if(seg_index!= m_seg_index)
                    {
                        m_seg_index = seg_index;
                        segment_scale.setcurve(m_segcurve = segcurve);
                    }
                    
                    segment_scale.setin_minmax(m_segments[m_seg_index].onset_f, m_segments[m_seg_index].offset_f);
                    
                    segment_scale.setout_min(m_segments[m_seg_index].min);
                    segment_scale.setout_max(m_segments[m_seg_index].max);
                    
                    m_val = segment_scale.apply(std::clamp(phase,0.,1.));
                    
                    phase += deltatime / m_time;
                    
                    if (phase > 1.)
                    {   // reset env : new freq jitter varifreq
                        if(loop)
                        {
                            m_varitime = mt_rand_time(mt_gen_time)*varitime;
                        
                            phase = fmodf(phase,1.);
                            m_time = time * exp2( m_varitime );
                            m_looping=1;
                        
                        }else
                        {
                            m_looping = 0;
                            m_val = m_end_val;
                        }
                        
                    }
                }
                
                values[i] = add+(m_val*mul);
                
            }
        }
        
        double get(double deltatime) override
        {
            // get all the parameters (check time or freq format)
            double time = params["time"].get(deltatime);
            double varitime = params["varitime"].get(deltatime);

            if(params.find("freq") != params.end())
                time = 1000./ std::clamp(params["freq"].get(deltatime),0.001,10000000.);

            if(params.find("varifreq") != params.end())
                varitime = params["varifreq"].get(deltatime);
            
            double mul = params["mul"].get(deltatime);
            double add = params["add"].get(deltatime);
            double segcurve = params["segcurve"].get(deltatime);
            int play = params["play"].get(deltatime)>0;
            int loop = params["loop"].get(deltatime)>0;
            /** count special parameter: if > 0
                • freq = 1./count
                • deltatime = 1000.
            */
            double count = params["count"].get(deltatime);
            if(count >= 0.)
            {
                time = (count > 0.01)? count * 1000 : 10. ;
                deltatime = 1000.;
            }
            
            p_list = params["list"].getlist();
            if(m_list != p_list)
            {
                m_list = p_list;
                m_seg_l = parse_segments(m_list);
            }
            
            if(!m_seg_l)
                return 0.;
            
            if(!m_playing && play) // restart the sequence
            {
                m_playing = play;
                m_looping = 1;
                m_seg_index = 0;

                phase = 0;
                
                // choose new length for env
                m_varitime = mt_rand_time(mt_gen_time)*varitime;
                m_time = time * exp2( m_varitime );
                
            }
            
            if(!play)
            {
                m_playing=play;
                m_looping=0;
            }
            
            if(m_playing && m_looping)
            {
                
                int seg_index = 0;
                while(phase > m_segments[seg_index].offset_f)
                {
                    seg_index++;
                }
                
                if(seg_index!= m_seg_index)
                {
                    m_seg_index = seg_index;
                    segment_scale.setin_minmax(m_segments[m_seg_index].onset_f, m_segments[m_seg_index].offset_f);
                    segment_scale.setout_min(m_segments[m_seg_index].min);
                    segment_scale.setout_max(m_segments[m_seg_index].max);
                    segment_scale.setcurve(m_segcurve = segcurve);
                }
                
                m_val = segment_scale.apply(std::clamp(phase,0.,1.));
                
                phase += deltatime / m_time;
                
                if (phase > 1.)
                {   // reset env : new freq jitter varifreq
                    if(loop)
                    {
                        m_varitime = mt_rand_time(mt_gen_time)*varitime;
                    
                        phase = fmodf(phase,1.);
                        m_time = time * exp2( m_varitime );
                        m_looping=1;
                    
                    }else
                    {
                        m_looping = 0;
                        m_val = m_end_val;
                    }
                    
                }
            }
            
            return add+(m_val*mul);
        }
    };

    class m_quantize : public modtor {
        
    public:
        
        m_quantize(scope * modtor_scope)
        {
            
            modtor_classname = "quantize";
            
            _scope = modtor_scope;
            _scope->touch(this);
            
            // y1 dt1 y2 dt2 y3

            params.insert(std::pair<std::string, modtor_param>("in",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("depth",modtor_param(1.)));
            params.insert(std::pair<std::string, modtor_param>("mod",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("list",modtor_param(std::vector<double>({0.}))));
            params.insert(std::pair<std::string, modtor_param>("mul",modtor_param(1.)));
            params.insert(std::pair<std::string, modtor_param>("add",modtor_param(0.)));

            
        };
        
        ~m_quantize()
        {
            params.clear();
        }
        
        
        double m_depth = 1.;
        double m_mod = 0.;
        double m_val = 0., m_end_val=0.;

        
        std::vector<double> m_list;
        std::vector<double> p_list;
        std::vector<double> m_list_mod;
       
        
        void seed(std::string seed_str) override
        {

        }
        
        void sync(double _phase) override
        {
            for (auto &p : params)
                p.second.sync(_phase);
        }
        
        /**
        get nearest without mod
         */
        double getnearest(double in, double depth)
        {
            double dist = 1.e20;
            double d,imin=0,imax=0;
            double a,b;
            
            for(int i = 0; i<m_list.size(); i++)
                if(in > m_list[i])
                {
                    imin = i;
                    imax = i+1;
                }
            
            if ((imin == 0 && imax==0) || imax == (m_list.size()))
                return m_list[imin];
            
            dist = m_list[imax] - m_list[imin];
            d = in - m_list[imin];
            double fact = d / dist;
            double fade;
            
            if(depth >= 1.)
                return (fact<0.5)?  m_list[imin] : m_list[imax];
    
            if(fact < 0.5)
            {
                fade = powf(fact *2, exp(depth*5.))*0.5;
                b = fade;
                a = 1. - fade;
            }
            else
            {
                fade = powf((1.-fact)*2, exp(depth*5.))*0.5 ;
                a = fade;
                b = 1. - fade;
            }

            return a * m_list[imin] + b * m_list[imax];
           
        }
        
        
        /**
        get nearest without mod
         */
        double getnearestmod(double in, double depth, double mod)
        {
            double dist = 1.e20;
            double d,imin=0,imax=0;
            double a,b;
            double in_mod = modulo(in,mod);
            double in_remainder = in - in_mod;
            double v_min,v_max;
            
            if (in_mod < m_list_mod[0])
            {
                v_min = m_list_mod[m_list_mod.size()-1] - mod;
                v_max = m_list_mod[0];
            }
            else if (in_mod >= m_list_mod[m_list_mod.size()-1])
            {
                v_min = m_list_mod[m_list_mod.size()-1];
                v_max = m_list_mod[0]+mod;
            }
            else
            {
                for(int i = 0; i<m_list_mod.size(); i++)
                    if(in_mod > m_list_mod[i])
                    {
                        imin = i;
                        imax = i+1;
                    }
                
                v_min = m_list_mod[imin];
                v_max = m_list_mod[imax];
            }
            
            dist = v_max - v_min;
            d = in_mod - v_min;
            double fact = d / dist;
            double fade;
            
            if(depth >= 1.)
                return in_remainder+((fact<0.5)?  v_min : v_max);
    
            if(fact < 0.5)
            {
                fade = powf(fact *2, exp(depth*5.))*0.5;
                b = fade;
                a = 1. - fade;
            }
            else
            {
                fade = powf((1.-fact)*2, exp(depth*5.))*0.5 ;
                a = fade;
                b = 1. - fade;
            }

            return in_remainder+(a * v_min + b * v_max);
           
        }
        
        
        void perform(double * values,int numframes,double deltatime) override
        {
            modtor_param * p_in = params["in"].buffer_proc(numframes, deltatime);
            modtor_param * p_depth = params["depth"].buffer_proc(numframes, deltatime);
            modtor_param * p_mod = params["mod"].buffer_proc(numframes, deltatime);
            modtor_param * p_mul = params["mul"].buffer_proc(numframes, deltatime);
            modtor_param * p_add = params["add"].buffer_proc(numframes, deltatime);
            modtor_param * p_list_ = &params["list"];

            modtor_param * p_time = 0;
            
            // mod is sampled every buffer
            double mod = std::max(p_mod->get_b(0),0.);
            
            p_list = p_list_->getlist();
            
            if(m_list != p_list)
            {
                m_list = p_list;
                sort(m_list.begin(),m_list.end());
                m_list_mod = m_list;
                
                std::for_each(m_list_mod.begin(), m_list_mod.end(),[&mod](double &d){ d = modulo(d,mod);});
                sort(m_list_mod.begin(),m_list_mod.end());

            }
            
            for(int i=0; i<numframes; i++)
            {
                // get all the parameters
                double in = p_in->get_b(i);
                
                double depth = std::clamp(p_depth->get_b(i),0.,1.);

                double mul = p_mul->get_b(i);
                double add = p_add->get_b(i);
                
                
                if(!m_list.size())
                    values[i] = m_val = add+(in*mul);
                else
                {
                    if(mod == 0.)
                        m_val = getnearest(in,depth);
                    else
                        m_val = getnearestmod(in,depth,mod);

                    values[i] = add+(m_val*mul);
                }
                
            }
        }
        
        double get(double deltatime) override
        {
            // get all the parameters
            double in = params["in"].get(deltatime);
            
            double depth = std::clamp(params["depth"].get(deltatime),0.,1.);
            double mod = std::max(params["mod"].get(deltatime),0.);

            double mul = params["mul"].get(deltatime);
            double add = params["add"].get(deltatime);

            p_list = params["list"].getlist();
            
            
            if(m_list != p_list)
            {
                m_list = p_list;
                sort(m_list.begin(),m_list.end());
                m_list_mod = m_list;
                if(mod > 0.)
                {
                    std::for_each(m_list_mod.begin(), m_list_mod.end(),[&mod](double &d){ d = modulo(d,mod);});
                    sort(m_list_mod.begin(),m_list_mod.end());
                
                }
                
            }
            
            if(!m_list.size())
                return m_val = add+(in*mul);
            
            if(mod == 0.)
                m_val = getnearest(in,depth);
            else
                m_val = getnearestmod(in,depth,mod);

            return add+(m_val*mul);
        }
    };
        
    
    class m_input : public modtor {
        
    public:
        
        m_input(scope * modtor_scope)
        {
            
            modtor_classname = "input";
            
            _scope = modtor_scope;
            _scope->touch(this);
            
            params.insert(std::pair<std::string, modtor_param>("name",modtor_param("name")));
            params.insert(std::pair<std::string, modtor_param>("in",modtor_param("input")));
            params.insert(std::pair<std::string, modtor_param>("min",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("max",modtor_param(1.)));
            params.insert(std::pair<std::string, modtor_param>("mul",modtor_param(1.)));
            params.insert(std::pair<std::string, modtor_param>("add",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("curve",modtor_param(0.)));
            
            output_scale.setin_minmax(0., 1.);
            m_dict = dictobj_findregistered_retain (gensym("maxlang.input-internal.dict"));
            
            m_val_sym = gensym("value");
            m_min_sym = gensym("min");
            m_max_sym = gensym("max");
            
        };
        
        ~m_input()
        {
            params.clear();
            dictobj_release(m_dict);
        }
        
        double m_input_val = 0;
        scale_curve output_scale;
        t_dictionary * m_dict;
        t_symbol * m_val_sym, * m_min_sym, * m_max_sym;
        
        void seed(std::string seed_str) override
        {

        }
        
        void sync(double _phase) override
        {
        	for (auto &p : params)
                p.second.sync(_phase);
        }
        
        void perform(double * values,int numframes,double deltatime) override
        {
            
            // TODO
        }
        
        double get(double deltatime) override
        {
            // get all the parameters
            std::string name = params["in"].getstring();
            t_symbol * m_sym = gensym(name.c_str());
            double min = params["min"].get(deltatime);
            double max = params["max"].get(deltatime);
            double curve = params["curve"].get(deltatime);
            double add = params["add"].get(deltatime);
            double mul = params["mul"].get(deltatime);
            
            // get val, min and max from global dictionary maxlang.input-internal.dict
            double v, inmin, inmax;
            
            if(dictionary_hasentry (m_dict,m_sym))
            {
                v=1;
                t_dictionary * dchild;
                dictionary_getdictionary(m_dict, m_sym, (t_object**)&dchild);
                dictionary_getfloat(dchild, m_val_sym, &v);
                dictionary_getfloat(dchild, m_min_sym, &inmin);
                dictionary_getfloat(dchild, m_max_sym, &inmax);
            }
            else
            {
                inmin=0.;
                inmax=1.;
                v=0.;
            }
            
            output_scale.setin_minmax(inmin, inmax);
            output_scale.setout_min(min);
            output_scale.setout_max(max);
            output_scale.setcurve(curve);
            
            return add+(output_scale.apply(v)*mul);
        }
    };
    
    class m_variable : public modtor {
        
    public:
    
        m_variable(scope * modtor_scope)
        {
            
            modtor_classname = "variable";
            
            _scope = modtor_scope;
            _scope->touch(this);
            
            params.insert(std::pair<std::string, modtor_param>("name",modtor_param("name")));
            params.insert(std::pair<std::string, modtor_param>("id",modtor_param("var0")));
            params.insert(std::pair<std::string, modtor_param>("init",modtor_param(0)));
            params.insert(std::pair<std::string, modtor_param>("max",modtor_param(1.)));
            params.insert(std::pair<std::string, modtor_param>("mul",modtor_param(1.)));
            params.insert(std::pair<std::string, modtor_param>("add",modtor_param(0.)));
            
            output_scale.setin_minmax(0., 1.);
            
            std::string varname = params["id"].getstring();
            m_variable_val =_scope->touch(varname);
            
            /*if ( (variables.find("id")) == params.end() )
            { // not found
             variables.insert(std::pair<std::string,double>(param_value, this));
                variables[
                return 0;
            } else {
                // found
                ret_param->
                if(
                params.erase(name); // TEST bug
                params[name] = *value;
                //std::cout << value._type << std::endl;
            }
            //m_dict = dictobj_findregistered_retain (gensym("maxlang.variable-internal.dict"));
            */
            m_val_sym = gensym("value");
            m_min_sym = gensym("min");
            m_max_sym = gensym("max");
            
        };
        
        ~m_variable()
        {
            params.clear();
        }
        
        double m_variable_val = 0;
        scale_curve output_scale;
        t_symbol * m_val_sym, * m_min_sym, * m_max_sym;
        
        void seed(std::string seed_str) override
        {

        }
        
        void sync(double _phase) override
        {
            for (auto &p : params)
                p.second.sync(_phase);
        }
        
        void perform(double * values,int numframes,double deltatime) override
        {
            
            // TODO
        }
        
        double get(double deltatime) override
        {
            // get all the parameters
            std::string varname = params["id"].getstring();
            double val = this->getvariable(varname);
            
            
            m_variable_val = val ;
            

            double add = params["add"].get(deltatime);
            double mul = params["mul"].get(deltatime);
            
            return add+(m_variable_val*mul);
        }
    };



    class m_add : public modtor {
        
    public:
        
        m_add(scope * modtor_scope)
        {
            
            modtor_classname = "add";
            
            _scope = modtor_scope;
            _scope->touch(this);
            
            params.insert(std::pair<std::string, modtor_param>("a",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("b",modtor_param(0.)));
        };
        
        ~m_add()
        {
            params.clear();
        }
        
        void seed(std::string seed_str) override
        {
        }
        
        void sync(double _phase) override
        {
        }
        
        void perform(double * values,int numframes,double deltatime) override
        {
            modtor_param * p_a = params["a"].buffer_proc(numframes, deltatime);
            modtor_param * p_b = params["b"].buffer_proc(numframes, deltatime);

            
            for(int i=0; i<numframes; i++)
                values[i] = p_a->get_b(i) + p_b->get_b(i);
    
        }
        
        double get(double deltatime) override
        {
            // get all the parameters
            double in1 = params["a"].get(deltatime);
            double in2 = params["b"].get(deltatime);
            
            return in1 + in2;
        }
    };

    class m_minus : public modtor {
        
    public:
        
        m_minus(scope * modtor_scope)
        {
            
            modtor_classname = "minus";
            
            _scope = modtor_scope;
            _scope->touch(this);
            
            params.insert(std::pair<std::string, modtor_param>("a",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("b",modtor_param(0)));
        };
        
        ~m_minus()
        {
            params.clear();
        }
        
        void seed(std::string seed_str) override
        {
        }
        
        void sync(double _phase) override
        {
        }
        
        void perform(double * values,int numframes,double deltatime) override
        {
            modtor_param * p_a = params["a"].buffer_proc(numframes, deltatime);
            modtor_param * p_b = params["b"].buffer_proc(numframes, deltatime);

            
            for(int i=0; i<numframes; i++)
                values[i] = p_a->get_b(i) - p_b->get_b(i);
    
        }
        
        double get(double deltatime) override
        {
            // get all the parameters
            double in1 = params["a"].get(deltatime);
            double in2 = params["b"].get(deltatime);
            
            return in1 - in2;
        }
    };

    class m_mul : public modtor {
        
    public:
        
        m_mul(scope * modtor_scope)
        {
            
            modtor_classname = "mul";
            
            _scope = modtor_scope;
            _scope->touch(this);
            
            params.insert(std::pair<std::string, modtor_param>("a",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("b",modtor_param(1)));
        };
        
        ~m_mul()
        {
            params.clear();
        }
        
        void seed(std::string seed_str) override
        {
        }
        
        void sync(double _phase) override
        {
        }
        
        void perform(double * values,int numframes,double deltatime) override
        {
            modtor_param * p_a = params["a"].buffer_proc(numframes, deltatime);
            modtor_param * p_b = params["b"].buffer_proc(numframes, deltatime);

            
            for(int i=0; i<numframes; i++)
                values[i] = p_a->get_b(i) * p_b->get_b(i);
    
        }
        
        double get(double deltatime) override
        {
            // get all the parameters
            double in1 = params["a"].get(deltatime);
            double in2 = params["b"].get(deltatime);
            
            return in1 * in2;
        }
    };

    class m_div : public modtor {
        
    public:
        
        m_div(scope * modtor_scope)
        {
            
            modtor_classname = "div";
            
            _scope = modtor_scope;
            _scope->touch(this);
            
            params.insert(std::pair<std::string, modtor_param>("a",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("b",modtor_param(1)));
        };
        
        ~m_div()
        {
            params.clear();
        }
        
        void seed(std::string seed_str) override
        {
        }
        
        void sync(double _phase) override
        {
        }
        
        void perform(double * values,int numframes,double deltatime) override
        {
            modtor_param * p_a = params["a"].buffer_proc(numframes, deltatime);
            modtor_param * p_b = params["b"].buffer_proc(numframes, deltatime);

            
            for(int i=0; i<numframes; i++)
                values[i] = p_a->get_b(i) / p_b->get_b(i);
    
        }
        
        double get(double deltatime) override
        {
            // get all the parameters
            double in1 = params["a"].get(deltatime);
            double in2 = params["b"].get(deltatime);
            
            return in1 / in2;
        }
    };
    
    class m_xfade : public modtor {
        
    public:
        
        m_xfade(scope * modtor_scope)
        {
            
            modtor_classname = "xfade";
            
            _scope = modtor_scope;
            _scope->touch(this);

            params.insert(std::pair<std::string, modtor_param>("a",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("b",modtor_param(1)));
            params.insert(std::pair<std::string, modtor_param>("fade",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("fadecurve",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("mul",modtor_param(1.)));
            params.insert(std::pair<std::string, modtor_param>("add",modtor_param(0.)));
            
            segment_scale.setin_minmax(0., 1.);
            segment_scale.setout_min(0.);
            segment_scale.setout_max(1.);
        };
        
        ~m_xfade()
        {
            params.clear();
        }
        
        double m_fade1 = 1.;
        double m_fade2 = 0.;
        double m_curve = 0.;
        double m_val = 0.;
        scale_curve segment_scale;

        void seed(std::string seed_str) override
        {
        }
        
        void sync(double _phase) override
        {
        }
        
        void perform(double * values,int numframes,double deltatime) override
        {
            modtor_param * p_a = params["a"].buffer_proc(numframes, deltatime);
            modtor_param * p_b = params["b"].buffer_proc(numframes, deltatime);
            modtor_param * p_mul = params["mul"].buffer_proc(numframes, deltatime);
            modtor_param * p_add = params["add"].buffer_proc(numframes, deltatime);
            modtor_param * p_fade = params["fade"].buffer_proc(numframes, deltatime);
            modtor_param * p_fadecurve = params["fadecurve"].buffer_proc(numframes, deltatime);
            
            // sample curve
            double fadecurve = std::clamp(p_fadecurve->get_b(0),-1.04,1.04);
            // fadecurve = 0 : linear
            // fadecurve = 1 : tight (square)
            segment_scale.setcurve(fadecurve);
            
            for(int i=0; i<numframes; i++)
            {
                // get all the parameters
                double in1 = p_a->get_b(i);
                double in2 = p_b->get_b(i);
                double mul = p_mul->get_b(i);
                double add = p_add->get_b(i);
                double fade = std::clamp(p_fade->get_b(i),0.,1.);
                
                //m_fade1 = std::clamp((((1-fade)-0.5)*(1./(1.-fadecurve)))+0.5,0.,1.);
                m_fade2 = segment_scale.apply(fade);
                m_fade1 = 1.- m_fade2;
                    
                m_val = (m_fade1 * in1) + (m_fade2 * in2);
                
                return add+(m_val*mul);
            }
    
        }
        
        double get(double deltatime) override
        {
            // get all the parameters
            double in1 = params["a"].get(deltatime);
            double in2 = params["b"].get(deltatime);
            double mul = params["mul"].get(deltatime);
            double add = params["add"].get(deltatime);
            double fade = std::clamp(params["fade"].get(deltatime),0.,1.);
            double fadecurve = std::clamp(params["fadecurve"].get(deltatime),-1.04,1.04);
            
            // fadecurve = 0 : linear
            // fadecurve = 1 : tight (square)

            
            segment_scale.setcurve(fadecurve);
            //m_fade1 = std::clamp((((1-fade)-0.5)*(1./(1.-fadecurve)))+0.5,0.,1.);
            m_fade2 = segment_scale.apply(fade);
            m_fade1 = 1.- m_fade2;
                
            m_val = (m_fade1 * in1) + (m_fade2 * in2);
            
            return add+(m_val*mul);
        }
    };

    class m_interpolate : public modtor {
        
    public:
        
        m_interpolate(scope * modtor_scope)
        {
            
            modtor_classname = "interpolate";
            
            _scope = modtor_scope;
            _scope->touch(this);

            params.insert(std::pair<std::string, modtor_param>("a",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("b",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("id",modtor_param("interpolate")));
            params.insert(std::pair<std::string, modtor_param>("fadecurve",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("mul",modtor_param(1.)));
            params.insert(std::pair<std::string, modtor_param>("add",modtor_param(0.)));
            
            // create new modtor variable
            modtor*  m = new m_variable(modtor_scope);
            m->setparam("id",new modtor_param("interpolate"));
            params.insert(std::pair<std::string, modtor_param>("fade",modtor_param(m)));
            
            segment_scale.setin_minmax(0., 1.);
            segment_scale.setout_min(0.);
            segment_scale.setout_max(1.);
        };
        
        ~m_interpolate()
        {
            params.clear();
        }
        
        double m_fade1 = 1.;
        double m_fade2 = 0.;
        double m_curve = 0.;
        double m_val = 0.;
        scale_curve segment_scale;

        void seed(std::string seed_str) override
        {
        }
        
        void sync(double _phase) override
        {
        }
        
        void perform(double * values,int numframes,double deltatime) override
        {
            modtor_param * p_a = params["a"].buffer_proc(numframes, deltatime);
            modtor_param * p_b = params["b"].buffer_proc(numframes, deltatime);
            modtor_param * p_mul = params["mul"].buffer_proc(numframes, deltatime);
            modtor_param * p_add = params["add"].buffer_proc(numframes, deltatime);
            modtor_param * p_fade = params["fade"].buffer_proc(numframes, deltatime);
            
            modtor_param * p_fadecurve = params["fadecurve"].buffer_proc(numframes, deltatime);
            
            // sample curve
            double fadecurve = std::clamp(p_fadecurve->get_b(0),-1.04,1.04);
            // fadecurve = 0 : linear
            // fadecurve = 1 : tight (square)
            segment_scale.setcurve(fadecurve);
            
            for(int i=0; i<numframes; i++)
            {
                // get all the parameters
                double in1 = p_a->get_b(i);
                double in2 = p_b->get_b(i);
                double mul = p_mul->get_b(i);
                double add = p_add->get_b(i);
                double fade = std::clamp(p_fade->get_b(i),0.,1.);
                
                //m_fade1 = std::clamp((((1-fade)-0.5)*(1./(1.-fadecurve)))+0.5,0.,1.);
                m_fade2 = segment_scale.apply(fade);
                m_fade1 = 1.- m_fade2;
                    
                m_val = (m_fade1 * in1) + (m_fade2 * in2);
                
                return add+(m_val*mul);
            }

        }
        
        double get(double deltatime) override
        {
            // get all the parameters
            double in1 = params["a"].get(deltatime);
            double in2 = params["b"].get(deltatime);
            double mul = params["mul"].get(deltatime);
            double add = params["add"].get(deltatime);
            double fade = std::clamp(params["fade"].get(deltatime),0.,1.);
            
            double fadecurve = std::clamp(params["fadecurve"].get(deltatime),-1.04,1.04);
            
            // fadecurve = 0 : linear
            // fadecurve = 1 : tight (square)

            
            segment_scale.setcurve(fadecurve);
            //m_fade1 = std::clamp((((1-fade)-0.5)*(1./(1.-fadecurve)))+0.5,0.,1.);
            m_fade2 = segment_scale.apply(fade);
            m_fade1 = 1.- m_fade2;
                
            m_val = (m_fade1 * in1) + (m_fade2 * in2);
            
            return add+(m_val*mul);
        }
    };
    
    
    
    
    modtor_type_enum modtor_create_fromstring(std::string s, modtor *&m, scope * scope)
    {
        /* operators */
        if(s == "add" || s == "+")
        {
            if(m) delete m;
            m = new m_add(scope);
            return modtor_type_enum::add;
        }
        
        if(s == "minus" || s == "-")
        {
            if(m) delete m;
            m = new m_minus(scope);
            return modtor_type_enum::minus;
        }
        
        if(s == "mul" || s == "*")
        {
            if(m) delete m;
            m = new m_mul(scope);
            return modtor_type_enum::mul;
        }
        
        if(s == "div" || s == "/")
        {
            if(m) delete m;
            m = new m_div(scope);
            return modtor_type_enum::div;
        }
        
        /* modulators */
        if(s == "lfo")
        {
            if(m) delete m;
            m = new m_lfo(scope);
            return modtor_type_enum::lfo;
        }
        if(s == "line")
        {
            if(m) delete m;
            m = new m_line(scope);
            return modtor_type_enum::line;
        }

        if(s == "rand")
        {
            if(m) delete m;
            m = new m_rand(scope);
            return modtor_type_enum::rand;
        }
        if(s == "randi")
        {
            if(m) delete m;
            m = new m_randi(scope);
            return modtor_type_enum::randi;
        }
        if(s == "choice")
        {
            if(m) delete m;
            m = new m_choice(scope);
            return modtor_type_enum::choice;
        }
        if(s == "choicei")
        {
            if(m) delete m;
            m = new m_choicei(scope);
            return modtor_type_enum::choicei;
        }
        if(s == "seq")
        {
            if(m) delete m;
            m = new m_seq(scope);
            return modtor_type_enum::seq;
        }
        if(s == "seqi")
        {
            if(m) delete m;
            m = new m_seqi(scope);
            return modtor_type_enum::seqi;
        }
        if(s == "env")
        {
            if(m) delete m;
            m = new m_env(scope);
            return modtor_type_enum::env;
        }
        
        if(s == "quantize")
        {
            if(m) delete m;
            m = new m_quantize(scope);
            return modtor_type_enum::quantize;
        }
        
        if(s == "xfade")
        {
            if(m) delete m;
            m = new m_xfade(scope);
            return modtor_type_enum::xfade;
        }
        
        
        if(s == "input")
        {
            if(m) delete m;
            m = new m_input(scope);
            return modtor_type_enum::input;
        }
        
        if(s == "variable")
        {
            if(m) delete m;
            m = new m_variable(scope);
            return modtor_type_enum::variable;
        }
        
        
        if(s == "interpolate")
        {
            if(m) delete m;
            m = new m_interpolate(scope);
            return modtor_type_enum::interpolate;
        }
        
        return modtor_type_enum::unknown;

    }


}


#endif /* maxlang_modtree_h */
