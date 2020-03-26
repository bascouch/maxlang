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

namespace maxlang {
    
    enum modtor_param_type { e_int, e_double, e_list, e_modtor };
    
    class modtor;
    
    class modtor_param
    {
        public :
        modtor_param();
        modtor_param(double v);
        modtor_param(int v);
        modtor_param(std::vector<modtor_param> l);
        modtor_param(modtor *m);
        ~modtor_param();
        
        double get(double deltatime);
        void set(double value);
        
        modtor_param_type _type;
        
        double _value_d;
        int _value_i;
        std::vector<modtor_param> _list;
        modtor * _modtor;
        
    };
    
    
    class modtor {
        
    public :
        modtor()
        {}
        ~modtor()
        {}
        double get(double deltatime){
            // WILL be overidden
            return 0.;
        }
        
        int setparam(std::string name, modtor_param value)
        {
            if ( params.find(name) == params.end() )
            { // not found
                error("maxlang.modulator : param %s not found",name.c_str());
                return 0;
            } else {
                // found
                params[name] = value;
            }
            return 1;
        }
        
        std::map<std::string, modtor_param> params;

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
    
    modtor_param::modtor_param(std::vector<modtor_param> l){
        
        _type = modtor_param_type::e_list;
        _list = l;
    }
    
    modtor_param::modtor_param(modtor *m){
        
        _type = modtor_param_type::e_modtor;
        _modtor = m;
    }
    
    modtor_param::~modtor_param()
    {
        
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
            case modtor_param_type::e_modtor:
                double v = _modtor->get(deltatime);
                printf("line : %f\n",v);
                return v;
        }
        
    }
    void modtor_param::set(double value)
    {
        _value_d=value;
        
    }
    
    
    class m_line : public modtor {
        
    public:
        
        m_line()
        {
            params.insert(std::pair<std::string, modtor_param>("time",modtor_param(5000.)));
            params.insert(std::pair<std::string, modtor_param>("varitime",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("min",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("max",modtor_param(1.)));
            params.insert(std::pair<std::string, modtor_param>("curve",modtor_param(0.)));
            
            
            mt_gen_time = std::mt19937(std::time(0));
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
            
        }
        
        double phase = 0.;
        double m_time = 1000.;
        double m_curve = 0.;
        double m_output = 0.;
        scale_curve segment_scale;
        std::mt19937 mt_gen_time;
        std::uniform_real_distribution<double> mt_rand_time;
        
        
        double get(double deltatime)
        {
            // get all the parameters
            double time = params["time"].get(deltatime);
            double varitime = params["varitime"].get(deltatime);
            double min = params["min"].get(deltatime);
            double max = params["max"].get(deltatime);
            double curve = params["curve"].get(deltatime);
            
            printf("get line\n");
            
            if (phase < 0.) // start the line
            {
                // choose time
                m_time = time + exp2(mt_rand_time(mt_gen_time)*varitime);
                // sample curve param
                m_curve=curve;
                segment_scale.setin_minmax(0., m_time);
                segment_scale.setcurve(m_curve);
                segment_scale.setout_min(min);
                segment_scale.setout_max(max);
                
                phase = 0;
                m_output = min;
                

            }
            else if(phase <= m_time)
            {
                segment_scale.setout_min(min);
                segment_scale.setout_max(max);
                
                phase += deltatime;
                printf("phase %f\n",phase);
                m_output = std::clamp(segment_scale.apply(phase),min,max);
                
            }
            
            return m_output;
            
        }
    };
    
    class m_lfo : public modtor {
        
    public:
        
        m_lfo()
        {
            params.insert(std::pair<std::string, modtor_param>("freq",modtor_param(6.)));
            params.insert(std::pair<std::string, modtor_param>("varifreq",modtor_param(0.5)));
            params.insert(std::pair<std::string, modtor_param>("mode",modtor_param(1.)));
            params.insert(std::pair<std::string, modtor_param>("pw",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("min",modtor_param(0.)));
            //params.insert(std::pair<std::string, modtor_param>("max",modtor_param(1.)));
            m_line* line = new m_line();
            params.insert(std::pair<std::string, modtor_param>("max",modtor_param(line)));
            params.insert(std::pair<std::string, modtor_param>("curve",modtor_param(0.)));
            
            
            mt_gen = std::mt19937(std::time(0));
            mt_rand = std::uniform_real_distribution<double>(-1.,1.);

            //
            output_scale.setin_minmax(-1., 1.);
            
        };
        
        ~m_lfo()
        {
            
        }
        
        double phase = 1.;
        // variable that are updated only when phase reset
        
        double m_varifreq = 0.;
        double m_pw = 0.;
        double m_curve = 0.;
        scale_curve output_scale;
        std::mt19937 mt_gen;
        std::uniform_real_distribution<double> mt_rand;

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
        
        double get(double deltatime)
        {
            // get all the parameters
            double freq = params["freq"].get(deltatime);
            double varifreq = params["varifreq"].get(deltatime);
            double mode = params["mode"].get(deltatime);
            double pw = params["pw"].get(deltatime);
            double min = params["min"].get(deltatime);
            double max = params["max"].get(deltatime);
            double curve = params["curve"].get(deltatime);
            
            
            double r_freq = freq * exp2( m_varifreq );
            phase += r_freq*deltatime/1000.;
            
            if (phase > 1.)
            {   // reset : new freq jitter varifreq
                m_varifreq = mt_rand(mt_gen)*varifreq;
                phase = fmodf(phase,1.);
                
                // sample curve and pw param
                m_curve = curve;
                m_pw = pw;
            }
            
            output_scale.setout_min(min);
            output_scale.setout_max(max);
            output_scale.setcurve(m_curve);
            
            double w = wave(phase, mode, m_pw);
            return output_scale.apply(w);
        }
    };
    
    
    
    class m_randi : public modtor {
        
    public:
        
        m_randi()
        {
            params.insert(std::pair<std::string, modtor_param>("freq",modtor_param(6.)));
            params.insert(std::pair<std::string, modtor_param>("varifreq",modtor_param(0.5)));
            params.insert(std::pair<std::string, modtor_param>("walk",modtor_param(1.)));
            params.insert(std::pair<std::string, modtor_param>("min",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("max",modtor_param(1.)));
            params.insert(std::pair<std::string, modtor_param>("curve",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("segcurve",modtor_param(0.)));
            
            
            mt_gen_time = std::mt19937(std::time(0));
            mt_rand_time = std::uniform_real_distribution<double>(-1.,1.);
            
            mt_gen_val = std::mt19937(std::time(0));
            mt_rand_val = std::uniform_real_distribution<double>(-1.,1.);
            
            //
            output_scale.setin_minmax(-1., 1.);
            
            segment_scale.setin_minmax(0., 1.);
            segment_scale.setout_min(0.);
            segment_scale.setout_max(1.);
            
        };
        
        ~m_randi()
        {
            
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
        
        
        double get(double deltatime)
        {
            // get all the parameters
            double freq = params["freq"].get(deltatime);
            double varifreq = params["varifreq"].get(deltatime);
            double walk = params["walk"].get(deltatime);
            double min = params["min"].get(deltatime);
            double max = params["max"].get(deltatime);
            double curve = params["curve"].get(deltatime);
            double segcurve = params["segcurve"].get(deltatime);
            
            
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
            return output_scale.apply(w);
        }
    };
    
    class m_rand : public modtor {
        
    public:
        
        m_rand()
        {
            params.insert(std::pair<std::string, modtor_param>("freq",modtor_param(6.)));
            params.insert(std::pair<std::string, modtor_param>("varifreq",modtor_param(0.5)));
            params.insert(std::pair<std::string, modtor_param>("walk",modtor_param(1.)));
            params.insert(std::pair<std::string, modtor_param>("min",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("max",modtor_param(1.)));
            params.insert(std::pair<std::string, modtor_param>("curve",modtor_param(0.)));
            
            
            mt_gen_time = std::mt19937(std::time(0));
            mt_rand_time = std::uniform_real_distribution<double>(-1.,1.);
            
            mt_gen_val = std::mt19937(std::time(0));
            mt_rand_val = std::uniform_real_distribution<double>(-1.,1.);
            
            //
            output_scale.setin_minmax(-1., 1.);
            
            segment_scale.setin_minmax(0., 1.);
            segment_scale.setout_min(0.);
            segment_scale.setout_max(1.);
            
        };
        
        ~m_rand()
        {
            
        }
        
        double phase = 1.;
        double m_varifreq = 0.;
        double m_curve = 0.;
        double m_rand_prev = 0.;
        double m_rand_target = 0.;
        scale_curve output_scale, segment_scale;
        std::mt19937 mt_gen_time, mt_gen_val;
        std::uniform_real_distribution<double> mt_rand_time, mt_rand_val;
        
        
        double get(double deltatime)
        {
            // get all the parameters
            double freq = params["freq"].get(deltatime);
            double varifreq = params["varifreq"].get(deltatime);
            double walk = params["walk"].get(deltatime);
            double min = params["min"].get(deltatime);
            double max = params["max"].get(deltatime);
            double curve = params["curve"].get(deltatime);
            
            
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
            return output_scale.apply(w);
        }
    };
}


#endif /* maxlang_modtree_h */
