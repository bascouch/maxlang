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
    long n_buffer = 0;
    
    double * buffer_proc(int numframes,double deltatime);
    
    double get_b(int i);
    
    // buffer_proc needs update
    bool has_update = true;
    
    
};


class modtor {
    
public :
    modtor()
    {}
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
    
    int traverse_for_ref(std::map<std::string,maxlang::modtor*> &name_ref)
    {
        for (std::unordered_map<std::string, modtor_param>::iterator it=params.begin(); it!=params.end(); ++it)
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
    
    std::unordered_map<std::string, modtor_param> params;
    std::random_device rd_dev;
    std::seed_seq rd_seed;
    std::string modtor_refname;

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
    if(modtor_param_type::e_modtor && _modtor)
        delete _modtor;
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
    has_update = true;
    
}

double * modtor_param::buffer_proc(int numframes,double deltatime)
{
    double val;
    double k = numframes;
    
    
    if(n_buffer != numframes)
    {
        if(buffer)
            free(buffer);
        buffer = (double *) malloc(numframes*sizeof(double));
        n_buffer = numframes;
        has_update = true;
    }
    
    if(has_update || _type == modtor_param_type::e_modtor)
    {
        double * buf_p = buffer;
        has_update = false;
        
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
    }
    
    
    return buffer;
}


double modtor_param::get_b(int i)
{
    return buffer[i];
}
    


class m_clockout : public modtor {
public:
    
    m_clockout()
    {
        params.insert(std::pair<std::string, modtor_param>("id",modtor_param("clock00")));
        params.insert(std::pair<std::string, modtor_param>("freq",modtor_param(0.6)));
        params.insert(std::pair<std::string, modtor_param>("varifreq",modtor_param(0.)));
        params.insert(std::pair<std::string, modtor_param>("count",modtor_param(-1.)));
        params.insert(std::pair<std::string, modtor_param>("speed",modtor_param(1.)));
        params.insert(std::pair<std::string, modtor_param>("curve",modtor_param(0.)));
        params.insert(std::pair<std::string, modtor_param>("mul",modtor_param(1.)));
        params.insert(std::pair<std::string, modtor_param>("add",modtor_param(0.)));

        
        output_scale.setin_minmax(0., 1.);
        m_dict = dictobj_findregistered_retain (gensym("maxlang.input-internal.dict"));
        
        m_val_sym = gensym("value");
        m_min_sym = gensym("min");
        m_max_sym = gensym("max");
        
        
        mt_gen = std::mt19937(rd_dev());
        mt_rand = std::uniform_real_distribution<double>(-1.,1.);
                
    };
    
    ~m_clockout()
    {
        params.clear();
        
        mt_gen.~mersenne_twister_engine();
        mt_rand.~uniform_real_distribution();
    }
    
    double phase = 1.;
    scale_curve output_scale; // used only for curve param
    t_dictionary * m_dict;
    t_symbol * m_val_sym, * m_min_sym, * m_max_sym;
    
    double m_varifreq = 0.;
    double m_curve = 0.;
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
    
    
    
    
    void perform(double * values,int numframes,double deltatime) override
    {
        
    }
    
    double get(double deltatime) override
    {

        // get all the parameters (check time or freq format)
        double freq = params["freq"].get(deltatime);
        double varifreq = params["varifreq"].get(deltatime);
        
        
        if(params.find("time") != params.end())
            freq = 1000./ std::clamp(params["time"].get(deltatime),0.001,DBL_MAX);
        if(params.find("varitime") != params.end())
            varifreq = params["varitime"].get(deltatime);
        
        double speed = std::clamp(params["speed"].get(deltatime),0.,DBL_MAX);
        double curve = params["curve"].get(deltatime);
        double add = params["add"].get(deltatime);
        double mul = params["mul"].get(deltatime);
        
        /** count special parameter: if > 0
            • freq = 1./count
            • deltatime = 1000.
        **/
        
        double count = params["count"].get(deltatime);
        
        // get all the parameters
        std::string name = params["id"].getstring();
        t_symbol * m_sym = gensym(name.c_str());
        
        if(count >= 0.)
        {
            freq = (count > 0.01)? 1./count : 100. ;
            deltatime = 1000.;
        }
        
        double r_freq = speed * (freq * exp2(m_varifreq));
        
        double phase_out = 0.;
        
        phase += r_freq*deltatime/1000.;
        
        if (phase > 1.)
        {   // reset : new freq jitter varifreq
            m_varifreq = mt_rand(mt_gen)*varifreq;
            phase = fmodf(phase,1.);
            
            // sample curve and pw param
            m_curve = curve;
            output_scale.setcurve(m_curve);
        }
        
        phase_out = add+(output_scale.apply(phase)*mul);
        
        // set float in global dict
        t_dictionary * dchild = dictionary_new();
        dictionary_appendfloat(dchild,m_val_sym,phase_out);
        dictionary_appendfloat(dchild,m_min_sym,0.);
        dictionary_appendfloat(dchild,m_max_sym,1.);
        
        dictionary_appenddictionary(m_dict, m_sym, (t_object*)dchild);
        
        return phase_out;
    }
    
    
};

class m_clockin : public modtor {
public:
    
    m_clockin()
    {
        params.insert(std::pair<std::string, modtor_param>("id",modtor_param("clock00")));
        params.insert(std::pair<std::string, modtor_param>("freq",modtor_param(0.6)));
        params.insert(std::pair<std::string, modtor_param>("varifreq",modtor_param(0.)));
        params.insert(std::pair<std::string, modtor_param>("count",modtor_param(-1.)));
        params.insert(std::pair<std::string, modtor_param>("speed",modtor_param(1.)));
        params.insert(std::pair<std::string, modtor_param>("curve",modtor_param(0.)));
        params.insert(std::pair<std::string, modtor_param>("mul",modtor_param(1.)));
        params.insert(std::pair<std::string, modtor_param>("add",modtor_param(0.)));

        
        output_scale.setin_minmax(0., 1.);
        m_dict = dictobj_findregistered_retain (gensym("maxlang.input-internal.dict"));
        
        m_val_sym = gensym("value");
        m_min_sym = gensym("min");
        m_max_sym = gensym("max");
        
                
    };
    
    ~m_clockin()
    {
        params.clear();
        
        mt_gen.~mersenne_twister_engine();
        mt_rand.~uniform_real_distribution();
    }
    
    double phase = 1.;
    scale_curve output_scale; // used only for curve param
    t_dictionary * m_dict;
    t_symbol * m_val_sym, * m_min_sym, * m_max_sym;
    
    double m_varifreq = 0.;
    double m_curve = 0.;
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
    
    void perform(double * values,int numframes,double deltatime) override
    {
        
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
        
        double speed = params["speed"].get(deltatime);
        double curve = params["curve"].get(deltatime);
        double add = params["add"].get(deltatime);
        double mul = params["mul"].get(deltatime);
        
        /** count special parameter: if > 0
            • freq = 1./count
            • deltatime = 1000.
        **/
        double count = params["count"].get(deltatime);
        
        // get all the parameters
        std::string name = params["id"].getstring();
        t_symbol * m_sym = gensym(name.c_str());
        
        if(count >= 0.)
        {
            freq = (count > 0.01)? 1./count : 100. ;
            deltatime = 1000.;
        }
        
        double r_freq = freq * exp2( m_varifreq );
        
        double phase_out = 0.;
        
        phase += r_freq*deltatime/1000.;
        
        if (phase > 1.)
        {   // reset : new freq jitter varifreq
            m_varifreq = mt_rand(mt_gen)*varifreq;
            phase = fmodf(phase,1.);
            
            // sample curve and pw param
            m_curve = curve;
            output_scale.setcurve(m_curve);
        }
        
        phase_out = add+(output_scale.apply(phase)*mul);
        
        // set float in global dict
        t_dictionary * dchild = dictionary_new();
        dictionary_appendfloat(dchild,m_val_sym,phase_out);
        dictionary_appendfloat(dchild,m_min_sym,0.);
        dictionary_appendfloat(dchild,m_max_sym,1.);
        
        dictionary_appenddictionary(m_dict, m_sym, (t_object*)dchild);
        
        return phase_out;
    }
    
    
};

class m_lfo : public modtor {
    
public:
    
    m_lfo()
    {
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
        //mode = std::clamp(mode,1.,5.);
        mode = (mode < 1)? 1 : ((mode > 5)? 5 : mode);
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
        double * p_freq = params["freq"].buffer_proc(numframes, deltatime);
        double * p_varifreq = params["varifreq"].buffer_proc(numframes, deltatime);
        double * p_mode = params["mode"].buffer_proc(numframes, deltatime);
        double * p_pw = params["pw"].buffer_proc(numframes, deltatime);
        double * p_min = params["min"].buffer_proc(numframes, deltatime);
        double * p_max = params["max"].buffer_proc(numframes, deltatime);
        double * p_curve = params["curve"].buffer_proc(numframes, deltatime);
        double * p_mul = params["mul"].buffer_proc(numframes, deltatime);
        double * p_add = params["add"].buffer_proc(numframes, deltatime);
        double * p_count = params["count"].buffer_proc(numframes, deltatime);
        
        double * p_time = 0;
        
        double freq, varifreq, mode, pw, min, max, curve, add, mul, count;
        double r_freq, w;
        
        bool time_mode = params.find("time") != params.end();
        
        if(time_mode)
            p_time = params["time"].buffer_proc(numframes, deltatime);

        if(params.find("varitime") != params.end())
            p_varifreq = params["varitime"].buffer_proc(numframes, deltatime);
        
        for(int i=0; i<numframes; i++)
        {
            // get all the parameters (check time or freq format)
            freq = *(p_freq++);
            varifreq = *(p_varifreq++);
            
            if(time_mode)
                freq = 1000./ std::clamp(*(p_time++),0.001,10000000.);
            mode = *(p_mode++);
            pw = *(p_pw++);
            min = *(p_min++);
            max = *(p_max++);
            curve = *(p_curve++);
            add = *(p_add++);
            mul = *(p_mul++);
            
            /** count special parameter: if > 0
                • freq = 1./count
                • deltatime = 1000.
            */
            count = *(p_count++);
            if(count >= 0.)
            {
                freq = (count > 0.01)? 1./count : 100. ;
                deltatime = 1000.;
            }
            
            r_freq = freq * exp2( m_varifreq );
            phase += r_freq*deltatime/1000.;
            
            if (phase > 1.)
            {   // reset : new freq jitter varifreq
                m_varifreq = mt_rand(mt_gen)*varifreq;
                phase = fmodf(phase,1.);
                
                // sample curve and pw param
                if(curve != m_curve)
                {
                    m_curve = curve;
                    output_scale.setcurve(m_curve);
                }
                m_pw = pw;
            }
            
            output_scale.setout_min(min);
            output_scale.setout_max(max);
            
            
            w = wave(phase, mode, m_pw);
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
        
        if(params.find("clock") != params.end())
        {
            if( params["clock"]._type == modtor_param_type::e_modtor)
                phase = params["clock"].get(deltatime);
            if( params["clock"]._type == modtor_param_type::e_string)
                phase = params["clock"].get(deltatime);
            
                
            float a = params["clock"].get(deltatime);
        }
        
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
        **/
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
    
    m_line()
    {
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
        double * p_time = params["time"].buffer_proc(numframes, deltatime);
        double * p_varitime = params["varitime"].buffer_proc(numframes, deltatime);
        double * p_min = params["min"].buffer_proc(numframes, deltatime);
        double * p_max = params["max"].buffer_proc(numframes, deltatime);
        double * p_curve = params["curve"].buffer_proc(numframes, deltatime);
        double * p_mul = params["mul"].buffer_proc(numframes, deltatime);
        double * p_add = params["add"].buffer_proc(numframes, deltatime);
        double * p_count = params["count"].buffer_proc(numframes, deltatime);
        
        double * p_freq = 0;
        
        double time, varitime, min, max, curve, add, mul, count, tmp;
        
        bool freq_mode = params.find("freq") != params.end();
        
        if(freq_mode)
            p_freq = params["freq"].buffer_proc(numframes, deltatime);

        if(params.find("varifreq") != params.end())
            p_varitime = params["varifreq"].buffer_proc(numframes, deltatime);
        
        for(int i=0; i<numframes; i++)
        {
            // get all the parameters (check time or freq format)
            time = *(p_time++);
            varitime = *(p_varitime++);

            if(freq_mode)
                time = 1000./ std::clamp(*(p_freq++),0.001,10000000.);
            
            min = *(p_min++);
            max = *(p_max++);
            curve = *(p_curve++);
            add = *(p_add++);
            mul = *(p_mul++);
            
            
            /** count special parameter: if > 0
                • freq = 1./count
                • deltatime = 1000.
            */
            count = *(p_count++);
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
    
    m_randi()
    {
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
        double * p_freq = params["freq"].buffer_proc(numframes, deltatime);
        double * p_varifreq = params["varifreq"].buffer_proc(numframes, deltatime);
        double * p_walk = params["walk"].buffer_proc(numframes, deltatime);
        double * p_segcurve = params["segcurve"].buffer_proc(numframes, deltatime);
        double * p_min = params["min"].buffer_proc(numframes, deltatime);
        double * p_max = params["max"].buffer_proc(numframes, deltatime);
        double * p_curve = params["curve"].buffer_proc(numframes, deltatime);
        double * p_mul = params["mul"].buffer_proc(numframes, deltatime);
        double * p_add = params["add"].buffer_proc(numframes, deltatime);
        double * p_count = params["count"].buffer_proc(numframes, deltatime);
        
        double * p_time = 0;
        
        double freq, varifreq, walk, min, max, curve, segcurve, add, mul, count;
        
        bool time_mode = params.find("time") != params.end();
        
        if(time_mode)
            p_time = params["time"].buffer_proc(numframes, deltatime);

        if(params.find("varitime") != params.end())
            p_varifreq = params["varitime"].buffer_proc(numframes, deltatime);
        
        for(int i=0; i<numframes; i++)
        {
            // get all the parameters (check time or freq format)
            freq = *(p_freq++);
            varifreq = *(p_varifreq++);
            
            if(time_mode)
                freq = 1000./ std::clamp(*(p_time++),0.001,10000000.);
            
            walk = *(p_walk++);
            min = *(p_min++);
            max = *(p_max++);
            curve = *(p_curve++);
            segcurve = *(p_segcurve++);
            add = *(p_add++);
            mul = *(p_mul++);
            
            /** count special parameter: if > 0
                • freq = 1./count
                • deltatime = 1000.
            */
            count = *(p_count++);
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
    
    m_rand()
    {
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
        double * p_freq = params["freq"].buffer_proc(numframes, deltatime);
        double * p_varifreq = params["varifreq"].buffer_proc(numframes, deltatime);
        double * p_walk = params["walk"].buffer_proc(numframes, deltatime);
        double * p_segcurve = params["segcurve"].buffer_proc(numframes, deltatime);
        double * p_min = params["min"].buffer_proc(numframes, deltatime);
        double * p_max = params["max"].buffer_proc(numframes, deltatime);
        double * p_curve = params["curve"].buffer_proc(numframes, deltatime);
        double * p_mul = params["mul"].buffer_proc(numframes, deltatime);
        double * p_add = params["add"].buffer_proc(numframes, deltatime);
        double * p_count = params["count"].buffer_proc(numframes, deltatime);
        
        double * p_time = 0;
        
        double freq, varifreq, walk, min, max, curve, segcurve, add, mul, count;
        
        bool time_mode = params.find("time") != params.end();
        
        if(time_mode)
            p_time = params["time"].buffer_proc(numframes, deltatime);

        if(params.find("varitime") != params.end())
            p_varifreq = params["varitime"].buffer_proc(numframes, deltatime);
        
        for(int i=0; i<numframes; i++)
        {
            // get all the parameters (check time or freq format)
            freq = *(p_freq++);
            varifreq = *(p_varifreq++);
            
            if(time_mode)
                freq = 1000./ std::clamp(*(p_time++),0.001,10000000.);
            
            walk = *(p_walk++);
            min = *(p_min++);
            max = *(p_max++);
            curve = *(p_curve++);
            segcurve = *(p_segcurve++);
            add = *(p_add++);
            mul = *(p_mul++);
            
            /** count special parameter: if > 0
                • freq = 1./count
                • deltatime = 1000.
            */
            double count = *(p_count++);
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
    
    m_choice()
    {
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
        double * p_freq = params["freq"].buffer_proc(numframes, deltatime);
        double * p_varifreq = params["varifreq"].buffer_proc(numframes, deltatime);
        double * p_mul = params["mul"].buffer_proc(numframes, deltatime);
        double * p_add = params["add"].buffer_proc(numframes, deltatime);
        double * p_count = params["count"].buffer_proc(numframes, deltatime);
        modtor_param * p_list = &params["list"];
        
        double * p_time = 0;
        
        double freq, varifreq, add, mul, count;
        
        bool time_mode = params.find("time") != params.end();
        
        if(time_mode)
            p_time = params["time"].buffer_proc(numframes, deltatime);

        if(params.find("varitime") != params.end())
            p_varifreq = params["varitime"].buffer_proc(numframes, deltatime);
        
        for(int i=0; i<numframes; i++)
        {
            // get all the parameters (check time or freq format)
            double freq = *(p_freq++);
            double varifreq = *(p_varifreq++);
            
            if(time_mode)
                freq = 1000./ std::clamp(*(p_time++),0.001,10000000.);
            
            double mul = *(p_mul++);
            double add = *(p_add++);
            /** count special parameter: if > 0
                • freq = 1./count
                • deltatime = 1000.
            */
            double count = *(p_count++);
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
    
    m_choicei()
    {
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
        double * p_freq = params["freq"].buffer_proc(numframes, deltatime);
        double * p_varifreq = params["varifreq"].buffer_proc(numframes, deltatime);
        double * p_mul = params["mul"].buffer_proc(numframes, deltatime);
        double * p_add = params["add"].buffer_proc(numframes, deltatime);
        double * p_count = params["count"].buffer_proc(numframes, deltatime);
        double * p_segcurve = params["segcurve"].buffer_proc(numframes, deltatime);
        modtor_param * p_list = &params["list"];
        
        double * p_time = 0;
        
        double freq, varifreq, segcurve, add, mul, count;
        
        bool time_mode = params.find("time") != params.end();
        
        if(time_mode)
            p_time = params["time"].buffer_proc(numframes, deltatime);

        if(params.find("varitime") != params.end())
            p_varifreq = params["varitime"].buffer_proc(numframes, deltatime);
        
        for(int i=0; i<numframes; i++)
        {
            // get all the parameters (check time or freq format)
            double freq = *(p_freq++);
            double varifreq = *(p_varifreq++);
            
            double segcurve = *(p_segcurve++);
            
            if(time_mode)
                freq = 1000./ std::clamp(*(p_time++),0.001,10000000.);
            
            double mul = *(p_mul++);
            double add = *(p_add++);
            /** count special parameter: if > 0
                • freq = 1./count
                • deltatime = 1000.
            */
            double count = *(p_count++);
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
    
    m_seqi()
    {
        m_list = std::vector<double>({0.1,0.3,0.5,0.8});
        m_list_l = m_list.size();
        params.insert(std::pair<std::string, modtor_param>("freq",modtor_param(6.)));
        params.insert(std::pair<std::string, modtor_param>("varifreq",modtor_param(0.)));
        params.insert(std::pair<std::string, modtor_param>("list",modtor_param(m_list)));
        params.insert(std::pair<std::string, modtor_param>("mul",modtor_param(1.)));
        params.insert(std::pair<std::string, modtor_param>("add",modtor_param(0.)));
        params.insert(std::pair<std::string, modtor_param>("segcurve",modtor_param(0.)));
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
        double * p_freq = params["freq"].buffer_proc(numframes, deltatime);
        double * p_varifreq = params["varifreq"].buffer_proc(numframes, deltatime);
        double * p_mul = params["mul"].buffer_proc(numframes, deltatime);
        double * p_add = params["add"].buffer_proc(numframes, deltatime);
        double * p_count = params["count"].buffer_proc(numframes, deltatime);
        double * p_segcurve = params["segcurve"].buffer_proc(numframes, deltatime);
        double * p_play = params["play"].buffer_proc(numframes, deltatime);
        double * p_loop = params["loop"].buffer_proc(numframes, deltatime);
        modtor_param * p_list = &params["list"];
        
        double * p_time = 0;
        
        double freq, varifreq, segcurve, add, mul, count;
        int play, loop;
        
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
            freq = *(p_freq++);
            varifreq = *(p_varifreq++);
            
            segcurve = *(p_segcurve++);
            
            if(time_mode)
                freq = 1000./ std::clamp(*(p_time++),0.001,10000000.);
            
            mul = *(p_mul++);
            add = *(p_add++);
            /** count special parameter: if > 0
                • freq = 1./count
                • deltatime = 1000.
            */
            
            play = *(p_play++)>0;
            loop = *(p_loop++)>0;
            
            count = *(p_count++);
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
                
            }
        }
        
        double phase_c = segment_scale.apply(std::clamp(phase,0.,1.));
        return add+(((1-phase_c)*m_val_prev + phase_c*m_val_target)*mul);
    }
};

class m_seq : public modtor {
    
public:
    
    m_seq()
    {
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
        double * p_freq = params["freq"].buffer_proc(numframes, deltatime);
        double * p_varifreq = params["varifreq"].buffer_proc(numframes, deltatime);
        double * p_mul = params["mul"].buffer_proc(numframes, deltatime);
        double * p_add = params["add"].buffer_proc(numframes, deltatime);
        double * p_count = params["count"].buffer_proc(numframes, deltatime);
        double * p_play = params["play"].buffer_proc(numframes, deltatime);
        double * p_loop = params["loop"].buffer_proc(numframes, deltatime);
        modtor_param * p_list = &params["list"];
        
        double * p_time = 0;
        
        double freq, varifreq, add, mul, count;
        
        int play, loop;
        
        bool time_mode = params.find("time") != params.end();
        
        if(time_mode)
            p_time = params["time"].buffer_proc(numframes, deltatime);

        if(params.find("varitime") != params.end())
            p_varifreq = params["varitime"].buffer_proc(numframes, deltatime);
        
        for(int i=0; i<numframes; i++)
        {
            // get all the parameters (check time or freq format)
            freq = *(p_freq++);
            varifreq = *(p_varifreq++);
            
            
            if(time_mode)
                freq = 1000./ std::clamp(*(p_time++),0.001,10000000.);
            
            mul = *(p_mul++);
            add = *(p_add++);
            /** count special parameter: if > 0
                • freq = 1./count
                • deltatime = 1000.
            */
            
            play = *(p_play++)>0;
            loop = *(p_loop++)>0;
            
            count = *(p_count++);
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
    
    m_env()
    {
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
        double * p_time = params["time"].buffer_proc(numframes, deltatime);
        double * p_varitime = params["varitime"].buffer_proc(numframes, deltatime);
        double * p_mul = params["mul"].buffer_proc(numframes, deltatime);
        double * p_add = params["add"].buffer_proc(numframes, deltatime);
        double * p_count = params["count"].buffer_proc(numframes, deltatime);
        double * p_segcurve = params["segcurve"].buffer_proc(numframes, deltatime);
        double * p_play = params["play"].buffer_proc(numframes, deltatime);
        double * p_loop = params["loop"].buffer_proc(numframes, deltatime);
        modtor_param * p_list_ = &params["list"];
        
        double * p_freq = 0;
        
        double time, varitime, freq, varifreq, segcurve, add, mul, count;
        
        int play, loop;
        
        p_list = p_list_->getlist();
        
        bool freq_mode = params.find("freq") != params.end();
        
        if(freq_mode)
            p_freq = params["freq"].buffer_proc(numframes, deltatime);

        if(params.find("varifreq") != params.end())
            p_varitime = params["varifreq"].buffer_proc(numframes, deltatime);
        
        for(int i=0; i<numframes; i++)
        {
            // get all the parameters (check time or freq format)
            time = *(p_time++);
            varitime = *(p_varitime++);
            
            segcurve = *(p_segcurve++);
            
            if(freq_mode)
                time = 1000./ std::clamp(*(p_freq++),0.001,10000000.);
            
            
            mul = *(p_mul++);
            add = *(p_add++);
            /** count special parameter: if > 0
                • freq = 1./count
                • deltatime = 1000.
            */
            
            play = *(p_play++)>0;
            loop = *(p_loop++)>0;
            
            count = *(p_count++);
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
    
    m_quantize()
    {
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
        double * p_in = params["in"].buffer_proc(numframes, deltatime);
        double * p_depth = params["depth"].buffer_proc(numframes, deltatime);
        double * p_mod = params["mod"].buffer_proc(numframes, deltatime);
        double * p_mul = params["mul"].buffer_proc(numframes, deltatime);
        double * p_add = params["add"].buffer_proc(numframes, deltatime);
        modtor_param * p_list_ = &params["list"];

        double * p_time = 0;
        
        double in, depth, add, mul;
        
        // mod is sampled every buffer
        double mod = std::max(*(p_mod),0.);
        
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
            in = *(p_in++);
            
            depth = std::clamp(*(p_depth++),0.,1.);

            mul = *(p_mul++);
            add = *(p_add++);
            
            
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
    
    m_input()
    {
        params.insert(std::pair<std::string, modtor_param>("id",modtor_param("input00")));
        params.insert(std::pair<std::string, modtor_param>("min",modtor_param(0.)));
        params.insert(std::pair<std::string, modtor_param>("max",modtor_param(1.)));
        params.insert(std::pair<std::string, modtor_param>("curve",modtor_param(0.)));
        params.insert(std::pair<std::string, modtor_param>("mul",modtor_param(1.)));
        params.insert(std::pair<std::string, modtor_param>("add",modtor_param(0.)));
        
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
        
    }
    
    double get(double deltatime) override
    {
        // get all the parameters
        std::string name = params["id"].getstring();
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

class m_output : public modtor {
    
public:
    
    m_output()
    {
        params.insert(std::pair<std::string, modtor_param>("in",modtor_param(0.)));
        params.insert(std::pair<std::string, modtor_param>("id",modtor_param("output00")));
        params.insert(std::pair<std::string, modtor_param>("min",modtor_param(0.)));
        params.insert(std::pair<std::string, modtor_param>("max",modtor_param(1.)));
        params.insert(std::pair<std::string, modtor_param>("curve",modtor_param(0.)));
        params.insert(std::pair<std::string, modtor_param>("mul",modtor_param(1.)));
        params.insert(std::pair<std::string, modtor_param>("add",modtor_param(0.)));
        
        output_scale.setin_minmax(0., 1.);
        m_dict = dictobj_findregistered_retain (gensym("maxlang.input-internal.dict"));
        
        
        m_val_sym = gensym("value");
        m_min_sym = gensym("min");
        m_max_sym = gensym("max");
        
    };
    
    ~m_output()
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
        std::string name = params["id"].getstring();
        t_symbol * m_sym = gensym(name.c_str());
        double in = params["in"].get(deltatime);
        double min = params["min"].get(deltatime);
        double max = params["max"].get(deltatime);
        double curve = params["curve"].get(deltatime);
        double add = params["add"].get(deltatime);
        double mul = params["mul"].get(deltatime);
        
        // get val, min and max from global dictionary maxlang.input-internal.dict
        double v, inmin, inmax;
        
        v = in;
        
        
        output_scale.setout_min(min);
        output_scale.setout_max(max);
        output_scale.setcurve(curve);
        
        v = add+(output_scale.apply(v)*mul);
        
        
        // set float in global dict
        t_dictionary * dchild = dictionary_new();
        dictionary_appendfloat(dchild,m_val_sym,v);
        dictionary_appendfloat(dchild,m_min_sym,0.);
        dictionary_appendfloat(dchild,m_max_sym,1.);
        
        dictionary_appenddictionary(m_dict, m_sym, (t_object*)dchild);
        
        return v;
    }
};


class m_xfade : public modtor {
    
public:
    
    m_xfade()
    {

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
        double * p_a = params["a"].buffer_proc(numframes, deltatime);
        double * p_b = params["b"].buffer_proc(numframes, deltatime);
        double * p_mul = params["mul"].buffer_proc(numframes, deltatime);
        double * p_add = params["add"].buffer_proc(numframes, deltatime);
        double * p_fade = params["fade"].buffer_proc(numframes, deltatime);
        double * p_fadecurve = params["fadecurve"].buffer_proc(numframes, deltatime);
        
        double in1, in2, mul, add, fade;
        
        // sample curve
        double fadecurve = std::clamp(*(p_fadecurve),-1.04,1.04);
        // fadecurve = 0 : linear
        // fadecurve = 1 : tight (square)
        segment_scale.setcurve(fadecurve);
        
        for(int i=0; i<numframes; i++)
        {
            // get all the parameters
            in1 = *(p_a++);
            in2 = *(p_b++);
            mul = *(p_mul++);
            add = *(p_add++);
            fade = std::clamp(*(p_fade++),0.,1.);
            
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

class m_interpol : public modtor {
    
public:
    
    m_interpol()
    {

        params.insert(std::pair<std::string, modtor_param>("a",modtor_param(0.)));
        params.insert(std::pair<std::string, modtor_param>("b",modtor_param(1)));
        params.insert(std::pair<std::string, modtor_param>("id",modtor_param("interp00")));
        //params.insert(std::pair<std::string, modtor_param>("fadecurve",modtor_param(0.)));
        params.insert(std::pair<std::string, modtor_param>("mul",modtor_param(1.)));
        params.insert(std::pair<std::string, modtor_param>("add",modtor_param(0.)));
        
        
        m_dict = dictobj_findregistered_retain (gensym("maxlang.input-internal.dict"));
        
        m_val_sym = gensym("value");
        m_min_sym = gensym("min");
        m_max_sym = gensym("max");
        

    };
    
    ~m_interpol()
    {
        params.clear();
    }
    
    double m_fade1 = 1.;
    double m_fade2 = 0.;
    double m_curve = 0.;
    double m_val = 0.;
    t_dictionary * m_dict;
    t_symbol * m_val_sym, * m_min_sym, * m_max_sym;

    void seed(std::string seed_str) override
    {
    }
    
    void sync(double _phase) override
    {
    }
    
    void perform(double * values,int numframes,double deltatime) override
    {
        /* TODO */

    }
    
    double get(double deltatime) override
    {
        // get all the parameters
        std::string name = params["id"].getstring();
        t_symbol * m_sym = gensym(name.c_str());
        
        // get all the parameters
        double in1 = params["a"].get(deltatime);
        double in2 = params["b"].get(deltatime);
        double mul = params["mul"].get(deltatime);
        double add = params["add"].get(deltatime);
        
        // get val, min and max from global dictionary maxlang.input-internal.dict
        double fade, inmin, inmax;
        
        if(dictionary_hasentry (m_dict,m_sym))
        {
            fade=1;
            t_dictionary * dchild;
            dictionary_getdictionary(m_dict, m_sym, (t_object**)&dchild);
            dictionary_getfloat(dchild, m_val_sym, &fade);
            dictionary_getfloat(dchild, m_min_sym, &inmin);
            dictionary_getfloat(dchild, m_max_sym, &inmax);
        }
        else
        {
            inmin=0.;
            inmax=1.;
            fade=0.;
        }
        
        m_fade2 = fade;
        m_fade1 = 1.- m_fade2;
        m_val = (m_fade1 * in1) + (m_fade2 * in2);
        
        return add+(m_val*mul);
    }
};



class m_add : public modtor {
    
public:
    
    m_add()
    {
        params.insert(std::pair<std::string, modtor_param>("a",modtor_param(0.)));
        params.insert(std::pair<std::string, modtor_param>("b",modtor_param(0)));
        params.insert(std::pair<std::string, modtor_param>("mul",modtor_param(1.)));
        params.insert(std::pair<std::string, modtor_param>("add",modtor_param(0.)));
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
        double * p_a = params["a"].buffer_proc(numframes, deltatime);
        double * p_b = params["b"].buffer_proc(numframes, deltatime);
        double * p_mul = params["mul"].buffer_proc(numframes, deltatime);
        double * p_add = params["add"].buffer_proc(numframes, deltatime);

        double mul, add;
        for(int i=0; i<numframes; i++)
        {
            mul = *(p_mul++);
            add = *(p_add++);
            values[i] = add+(mul * (*(p_a++) + *(p_b++)));
            
        }

    }
    
    double get(double deltatime) override
    {
        // get all the parameters
        double in1 = params["a"].get(deltatime);
        double in2 = params["b"].get(deltatime);
        
        double mul = params["mul"].get(deltatime);
        double add = params["add"].get(deltatime);
        
        return add + ( mul * (in1 + in2));
    }
};

class m_minus : public modtor {
    
public:
    
    m_minus()
    {
        params.insert(std::pair<std::string, modtor_param>("a",modtor_param(0.)));
        params.insert(std::pair<std::string, modtor_param>("b",modtor_param(0.)));
        params.insert(std::pair<std::string, modtor_param>("mul",modtor_param(1.)));
        params.insert(std::pair<std::string, modtor_param>("add",modtor_param(0.)));
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
        double * p_a = params["a"].buffer_proc(numframes, deltatime);
        double * p_b = params["b"].buffer_proc(numframes, deltatime);
        double * p_mul = params["mul"].buffer_proc(numframes, deltatime);
        double * p_add = params["add"].buffer_proc(numframes, deltatime);
        
        double mul, add;
        
        for(int i=0; i<numframes; i++)
        {
            mul = *(p_mul++);
            add = *(p_add++);
            values[i] = add+(mul * (*(p_a++) - *(p_b++)));
            
        }

    }
    
    double get(double deltatime) override
    {
        // get all the parameters
        double in1 = params["a"].get(deltatime);
        double in2 = params["b"].get(deltatime);
        double mul = params["mul"].get(deltatime);
        double add = params["add"].get(deltatime);
        
        return add + ( mul * (in1 - in2));
    }
};

class m_mul : public modtor {
    
public:
    
    m_mul()
    {
        params.insert(std::pair<std::string, modtor_param>("a",modtor_param(0.)));
        params.insert(std::pair<std::string, modtor_param>("b",modtor_param(1)));
        params.insert(std::pair<std::string, modtor_param>("mul",modtor_param(1.)));
        params.insert(std::pair<std::string, modtor_param>("add",modtor_param(0.)));
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
        double * p_a = params["a"].buffer_proc(numframes, deltatime);
        double * p_b = params["b"].buffer_proc(numframes, deltatime);
        double * p_mul = params["mul"].buffer_proc(numframes, deltatime);
        double * p_add = params["add"].buffer_proc(numframes, deltatime);

        double mul, add;
        
        for(int i=0; i<numframes; i++)
        {
            mul = *(p_mul++);
            add = *(p_add++);
            values[i] = add+(mul * (*(p_a++) * *(p_b++)));
            
        }

    }
    
    double get(double deltatime) override
    {
        // get all the parameters
        double in1 = params["a"].get(deltatime);
        double in2 = params["b"].get(deltatime);
        double mul = params["mul"].get(deltatime);
        double add = params["add"].get(deltatime);
        
        return add + ( mul * (in1 * in2));
    }
};

class m_div : public modtor {
    
public:
    
    m_div()
    {
        params.insert(std::pair<std::string, modtor_param>("a",modtor_param(0.)));
        params.insert(std::pair<std::string, modtor_param>("b",modtor_param(1)));
        params.insert(std::pair<std::string, modtor_param>("mul",modtor_param(1.)));
        params.insert(std::pair<std::string, modtor_param>("add",modtor_param(0.)));
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
        double * p_a = params["a"].buffer_proc(numframes, deltatime);
        double * p_b = params["b"].buffer_proc(numframes, deltatime);
        double * p_mul = params["mul"].buffer_proc(numframes, deltatime);
        double * p_add = params["add"].buffer_proc(numframes, deltatime);

        double mul, add;
        
        for(int i=0; i<numframes; i++)
        {
            mul = *(p_mul++);
            add = *(p_add++);
            values[i] = add+(mul * (*(p_a++) / *(p_b++)));
            
        }

    }
    
    double get(double deltatime) override
    {
        // get all the parameters
        double in1 = params["a"].get(deltatime);
        double in2 = params["b"].get(deltatime);
        double mul = params["mul"].get(deltatime);
        double add = params["add"].get(deltatime);
        
        return add + ( mul * (in1 / in2));
    }
};


class m_const : public modtor {
    
public:
    
    m_const()
    {
        params.insert(std::pair<std::string, modtor_param>("val",modtor_param(0.)));
        params.insert(std::pair<std::string, modtor_param>("mul",modtor_param(1.)));
        params.insert(std::pair<std::string, modtor_param>("add",modtor_param(0.)));
    };
    
    ~m_const()
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
        double * p_val = params["val"].buffer_proc(numframes, deltatime);
        double * p_mul = params["mul"].buffer_proc(numframes, deltatime);
        double * p_add = params["add"].buffer_proc(numframes, deltatime);

        double mul, add;
        
        for(int i=0; i<numframes; i++)
        {
            mul = *(p_mul++);
            add = *(p_add++);
            values[i] = add+(mul * *(p_val++));
            
        }
        

    }
    
    double get(double deltatime) override
    {
        // get all the parameters
        double val = params["val"].get(deltatime);
        
        double mul = params["mul"].get(deltatime);
        double add = params["add"].get(deltatime);
        
        return add + ( mul * val);
    }
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
    output,
    xfade,
    interpol,
    add,
    minus,
    mul,
    div,
    constant
};


class modtordef;




modtor_type_enum modtor_create_fromstring(std::string s, modtor *&m, bool create)
{
    /* operators */
    if(s == "const")
    {
        if(create)
        {
            m = new m_const();
        }
        return modtor_type_enum::constant;
    }
    
    if(s == "add" || s == "+")
    {
        if(create)
        {
            m = new m_add();
        }
        return modtor_type_enum::add;
    }
    
    if(s == "minus" || s == "-")
    {
        if(create)
        {
            m = new m_minus();
        }
        return modtor_type_enum::minus;
    }
    
    if(s == "mul" || s == "*")
    {
        if(create)
        {
            m = new m_mul();
        }
        return modtor_type_enum::mul;
    }
    
    if(s == "div" || s == "/")
    {
        if(create)
        {
            m = new m_div();
        }
        return modtor_type_enum::div;
    }
    
    /* modulators */
    if(s == "lfo")
    {
        if(create)
        {
            m = new m_lfo();
        }
        return modtor_type_enum::lfo;
    }
    if(s == "line")
    {
        if(create)
        {
            m = new m_line();
        }
        return modtor_type_enum::line;
    }

    if(s == "rand")
    {
        if(create)
        {
            m = new m_rand();
        }
        return modtor_type_enum::rand;
    }
    if(s == "randi")
    {
        if(create)
        {
            m = new m_randi();
        }
        return modtor_type_enum::randi;
    }
    if(s == "choice")
    {
        if(create)
        {
            m = new m_choice();
        }
        return modtor_type_enum::choice;
    }
    if(s == "choicei")
    {
        if(create)
        {
            m = new m_choicei();
        }
        return modtor_type_enum::choicei;
    }
    if(s == "seq")
    {
        if(create)
        {
            m = new m_seq();
        }
        return modtor_type_enum::seq;
    }
    if(s == "seqi")
    {
        if(create)
        {
            m = new m_seqi();
        }
        return modtor_type_enum::seqi;
    }
    if(s == "env")
    {
        if(create)
        {
            m = new m_env();
        }
        return modtor_type_enum::env;
    }
    
    if(s == "quantize")
    {
        if(create)
        {
            m = new m_quantize();
        }
        return modtor_type_enum::quantize;
    }
    
    if(s == "xfade")
    {
        if(create)
        {
            m = new m_xfade();
        }
        return modtor_type_enum::xfade;
    }
    
    if(s == "interpol")
    {
        if(create)
        {
            m = new m_interpol();
        }
        return modtor_type_enum::interpol;
    }
    
    
    if(s == "input")
    {
        if(create)
        {
            m = new m_input();
        }
        return modtor_type_enum::input;
    }
    
    if(s == "output")
    {
        if(create)
        {
            m = new m_output();
        }
        return modtor_type_enum::output;
    }
    
    return modtor_type_enum::unknown;

}


}


#endif /* maxlang_modtree_h */
