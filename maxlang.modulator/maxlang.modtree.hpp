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
    
    enum modtor_param_type { e_int, e_double, e_list, e_modtor, e_string };
    
    class modtor;
    
    class modtor_param
    {
        public :
        modtor_param();
        modtor_param(double v);
        modtor_param(int v);
        modtor_param(std::vector<double> l);
        modtor_param(modtor * m);
        modtor_param(std::string s);
        ~modtor_param();
        
        double get(double deltatime);
        void set(double value);
        std::vector<double> getlist();
        std::string getstring();
        
        modtor_param_type _type;
        
        double _value_d;
        int _value_i;
        std::vector<double> _list;
        modtor * _modtor=0;
        std::string _string;
        
    };
    
    
    class modtor {
        
    public :
        modtor()
        {}
        virtual ~modtor()
        {}
        
        virtual double get(double deltatime) = 0;
        virtual void sync(double phase) = 0;
        
        
        int setparam(std::string name, modtor_param value)
        {
            if ( params.find(name) == params.end() )
            { // not found
                return 0;
            } else {
                // found
                params.erase(name);
                params[name] = value;
                //std::cout << value._type << std::endl;
            }
            return 1;
        }
        
        std::map<std::string, modtor_param> params;
        std::random_device rd_seed;

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
        /*if(_modtor)
            delete _modtor;
         */
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
    
    void modtor_param::set(double value)
    {
        _value_d=value;
        
    }
    

    class m_lfo : public modtor {
        
    public:
        
        m_lfo(double from)
        {
            params.insert(std::pair<std::string, modtor_param>("freq",modtor_param(0.6)));
            params.insert(std::pair<std::string, modtor_param>("varifreq",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("mode",modtor_param(1.)));
            params.insert(std::pair<std::string, modtor_param>("pw",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("min",modtor_param(from)));
            params.insert(std::pair<std::string, modtor_param>("max",modtor_param(1.)));
            params.insert(std::pair<std::string, modtor_param>("curve",modtor_param(0.)));
            
            
            mt_gen = std::mt19937(rd_seed());
            mt_rand = std::uniform_real_distribution<double>(-1.,1.);
            
            //
            output_scale.setin_minmax(-1., 1.);
            
        };
        
        ~m_lfo()
        {
            params.clear();
        }
        
        double phase = 1.;
        // variable that are updated only when phase reset
        
        double m_varifreq = 0.;
        double m_pw = 0.;
        double m_curve = 0.;
        scale_curve output_scale;
        std::mt19937 mt_gen;
        std::uniform_real_distribution<double> mt_rand;
        
        void sync(double _phase) override
        {
            phase = _phase;
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
        
        double get(double deltatime) override
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
    
    
    class m_line : public modtor {
        
    public:
        
        m_line(double from)
        {
            params.insert(std::pair<std::string, modtor_param>("time",modtor_param(5000.)));
            params.insert(std::pair<std::string, modtor_param>("varitime",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("min",modtor_param(from)));
            params.insert(std::pair<std::string, modtor_param>("max",modtor_param(1.)));
            params.insert(std::pair<std::string, modtor_param>("curve",modtor_param(0.)));
            
            
            mt_gen_time = std::mt19937(rd_seed());
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
        }
        
        double phase = 0;
        double m_time = 1000.;
        double m_curve = 0.;
        double m_output = 0.;
        scale_curve segment_scale;
        std::mt19937 mt_gen_time;
        std::uniform_real_distribution<double> mt_rand_time;
        
        void sync(double _phase) override
        {
            phase = _phase;
        }
        
        double get(double deltatime) override
        {
            // get all the parameters
            double time = params["time"].get(deltatime);
            double varitime = params["varitime"].get(deltatime);
            double min = params["min"].get(deltatime);
            double max = params["max"].get(deltatime);
            double curve = params["curve"].get(deltatime);
            double tmp;
                        
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
            
            return m_output;
            
        }
    };
    

    class m_randi : public modtor {
        
    public:
        
        m_randi(double from)
        {
            params.insert(std::pair<std::string, modtor_param>("freq",modtor_param(6.)));
            params.insert(std::pair<std::string, modtor_param>("varifreq",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("walk",modtor_param(1.)));
            params.insert(std::pair<std::string, modtor_param>("min",modtor_param(from)));
            params.insert(std::pair<std::string, modtor_param>("max",modtor_param(1.)));
            params.insert(std::pair<std::string, modtor_param>("curve",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("segcurve",modtor_param(0.)));
            
            //m_rand_prev = from;
            //m_rand_target = from;
            
            
            mt_gen_time = std::mt19937(rd_seed());
            mt_rand_time = std::uniform_real_distribution<double>(-1.,1.);
            
            mt_gen_val = std::mt19937(rd_seed());
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
        
        void sync(double _phase) override
        {
            phase = _phase;
        }
        
        double get(double deltatime) override
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
        
        m_rand(double from)
        {
            params.insert(std::pair<std::string, modtor_param>("freq",modtor_param(6.)));
            params.insert(std::pair<std::string, modtor_param>("varifreq",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("walk",modtor_param(1.)));
            params.insert(std::pair<std::string, modtor_param>("min",modtor_param(from)));
            params.insert(std::pair<std::string, modtor_param>("max",modtor_param(1.)));
            params.insert(std::pair<std::string, modtor_param>("curve",modtor_param(0.)));
            
            //m_rand_prev = from;
            //m_rand_target = from;
            
            mt_gen_time = std::mt19937(rd_seed());
            mt_rand_time = std::uniform_real_distribution<double>(-1.,1.);
            
            mt_gen_val = std::mt19937(rd_seed());
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
        }
        
        double phase = 1.;
        double m_varifreq = 0.;
        double m_curve = 0.;
        double m_rand_prev = 0.;
        double m_rand_target = 0.;
        scale_curve output_scale, segment_scale;
        std::mt19937 mt_gen_time, mt_gen_val;
        std::uniform_real_distribution<double> mt_rand_time, mt_rand_val;
        
        void sync(double _phase) override
        {
            phase = _phase;
        }
        
        double get(double deltatime) override
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

    class m_choice : public modtor {
        
    public:
        
        m_choice(double from)
        {
            m_list = std::vector<double>({0.,1.});
            m_list_l = m_list.size();
            params.insert(std::pair<std::string, modtor_param>("freq",modtor_param(6.)));
            params.insert(std::pair<std::string, modtor_param>("varifreq",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("list",modtor_param(m_list)));
            params.insert(std::pair<std::string, modtor_param>("mul",modtor_param(1.)));
            params.insert(std::pair<std::string, modtor_param>("add",modtor_param(0.)));
            
            mt_gen_time = std::mt19937(rd_seed());
            mt_rand_time = std::uniform_real_distribution<double>(-1.,1.);
            
            mt_gen_val = std::mt19937(rd_seed());
            mt_rand_val = std::uniform_real_distribution<double>(0.,0.99);
            
        };
        
        ~m_choice()
        {
            params.clear();
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
        
        void sync(double _phase) override
        {
            phase = _phase;
        }
        
        double get(double deltatime) override
        {
            // get all the parameters
            double freq = params["freq"].get(deltatime);
            double varifreq = params["varifreq"].get(deltatime);
            double mul = params["mul"].get(deltatime);
            double add = params["add"].get(deltatime);

            
            
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
        
        m_choicei(double from)
        {
            m_list = std::vector<double>({0.,1.});
            m_list_l = m_list.size();
            params.insert(std::pair<std::string, modtor_param>("freq",modtor_param(6.)));
            params.insert(std::pair<std::string, modtor_param>("varifreq",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("list",modtor_param(m_list)));
            params.insert(std::pair<std::string, modtor_param>("mul",modtor_param(1.)));
            params.insert(std::pair<std::string, modtor_param>("add",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("segcurve",modtor_param(0.)));

            
            mt_gen_time = std::mt19937(rd_seed());
            mt_rand_time = std::uniform_real_distribution<double>(-1.,1.);
            
            mt_gen_val = std::mt19937(rd_seed());
            mt_rand_val = std::uniform_real_distribution<double>(0.,0.99);
            
            segment_scale.setin_minmax(0., 1.);
            segment_scale.setout_min(0.);
            segment_scale.setout_max(1.);
            
        };
        
        ~m_choicei()
        {
            params.clear();
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
        
        void sync(double _phase) override
        {
            phase = _phase;
        }
        
        double get(double deltatime) override
        {
            // get all the parameters
            double freq = params["freq"].get(deltatime);
            double varifreq = params["varifreq"].get(deltatime);
            double mul = params["mul"].get(deltatime);
            double add = params["add"].get(deltatime);
            double segcurve = params["segcurve"].get(deltatime);
            
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
            return add+((1-phase_c)*m_rand_prev + phase_c*m_rand_target)*mul;
        }
    };
    
    
    class m_seqi : public modtor {
        
    public:
        
        m_seqi(double from)
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
            
            
            mt_gen_time = std::mt19937(rd_seed());
            mt_rand_time = std::uniform_real_distribution<double>(-1.,1.);

            segment_scale.setin_minmax(0., 1.);
            segment_scale.setout_min(0.);
            segment_scale.setout_max(1.);
            
        };
        
        ~m_seqi()
        {
            params.clear();
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
        
        void sync(double _phase) override
        {
            phase = _phase;
        }
        
        double get(double deltatime) override
        {
            // get all the parameters
            double freq = params["freq"].get(deltatime);
            double varifreq = params["varifreq"].get(deltatime);
            double mul = params["mul"].get(deltatime);
            double add = params["add"].get(deltatime);
            double segcurve = params["segcurve"].get(deltatime);
            int play = params["play"].get(deltatime)>0;
            int loop = params["loop"].get(deltatime)>0;
            
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
        
        m_seq(double from)
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
            
            
            mt_gen_time = std::mt19937(rd_seed());
            mt_rand_time = std::uniform_real_distribution<double>(-1.,1.);
            
        };
        
        ~m_seq()
        {
            params.clear();
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
        
        void sync(double _phase) override
        {
            phase = _phase;
        }
        
        double get(double deltatime) override
        {
            // get all the parameters
            double freq = params["freq"].get(deltatime);
            double varifreq = params["varifreq"].get(deltatime);
            double mul = params["mul"].get(deltatime);
            double add = params["add"].get(deltatime);
            int play = params["play"].get(deltatime)>0;
            int loop = params["loop"].get(deltatime)>0;
            
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
        
        m_env(double from)
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
            
            
            mt_gen_time = std::mt19937(rd_seed());
            mt_rand_time = std::uniform_real_distribution<double>(-1.,1.);
            
            segment_scale.setin_minmax(0., 1.);
            segment_scale.setout_min(0.);
            segment_scale.setout_max(1.);
            
        };
        
        ~m_env()
        {
            params.clear();
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
        
        void sync(double _phase) override
        {
            phase = _phase;
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
        
        double get(double deltatime) override
        {
            // get all the parameters
            double time = params["time"].get(deltatime);
            double varitime = params["varitime"].get(deltatime);
            double mul = params["mul"].get(deltatime);
            double add = params["add"].get(deltatime);
            double segcurve = params["segcurve"].get(deltatime);
            int play = params["play"].get(deltatime)>0;
            int loop = params["loop"].get(deltatime)>0;
            
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
                
                m_seg_index = 0;
                while(phase > m_segments[m_seg_index].offset_f)
                    m_seg_index++;
                
                segment_scale.setin_minmax(m_segments[m_seg_index].onset_f, m_segments[m_seg_index].offset_f);
                
                segment_scale.setout_min(m_segments[m_seg_index].min);
                segment_scale.setout_max(m_segments[m_seg_index].max);
                
               if(m_segcurve != segcurve)
                    segment_scale.setcurve(m_segcurve = segcurve);
                
                
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
    
    
    class m_input : public modtor {
        
    public:
        
        m_input(double from)
        {
            params.insert(std::pair<std::string, modtor_param>("name",modtor_param("random")));
            params.insert(std::pair<std::string, modtor_param>("in",modtor_param("input")));
            params.insert(std::pair<std::string, modtor_param>("min",modtor_param(0.)));
            params.insert(std::pair<std::string, modtor_param>("max",modtor_param(1.)));
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
        
        void sync(double _phase) override
        {
            
        }
        
        double get(double deltatime) override
        {
            // get all the parameters
            std::string name = params["in"].getstring();
            t_symbol * m_sym = gensym(name.c_str());
            double min = params["min"].get(deltatime);
            double max = params["max"].get(deltatime);
            double curve = params["curve"].get(deltatime);
            
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
            
            return output_scale.apply(v);
        }
    };
    
    
    class m_xfade : public modtor {
        
    public:
        
        m_xfade(double from)
        {

            params.insert(std::pair<std::string, modtor_param>("a",modtor_param(from)));
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

        
        void sync(double _phase) override
        {
    
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
            m_fade2 = fade;
            m_fade1 = 1.- m_fade2;
                
            m_val = (segment_scale.apply(m_fade1) * in1) + (segment_scale.apply(m_fade2) * in2);
            
            return add+(m_val*mul);
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
        input,
        xfade
    };
    
    modtor_type_enum modtor_create_fromstring(std::string s, modtor *&m, double from)
    {
        if(s == "lfo")
        {
            if(m) delete m;
            m = new m_lfo(from);
            return modtor_type_enum::lfo;
        }
        if(s == "line")
        {
            if(m) delete m;
            m = new m_line(from);
            return modtor_type_enum::line;
        }

        if(s == "rand")
        {
            if(m) delete m;
            m = new m_rand(from);
            return modtor_type_enum::rand;
        }
        if(s == "randi")
        {
            if(m) delete m;
            m = new m_randi(from);
            return modtor_type_enum::randi;
        }
        if(s == "choice")
        {
            if(m) delete m;
            m = new m_choice(from);
            return modtor_type_enum::choice;
        }
        if(s == "choicei")
        {
            if(m) delete m;
            m = new m_choicei(from);
            return modtor_type_enum::choicei;
        }
        if(s == "seq")
        {
            if(m) delete m;
            m = new m_seq(from);
            return modtor_type_enum::seq;
        }
        if(s == "seqi")
        {
            if(m) delete m;
            m = new m_seqi(from);
            return modtor_type_enum::seqi;
        }
        if(s == "env")
        {
            if(m) delete m;
            m = new m_env(from);
            return modtor_type_enum::env;
        }
        
        if(s == "xfade")
        {
            if(m) delete m;
            m = new m_xfade(from);
            return modtor_type_enum::xfade;
        }
        
        if(s == "input")
        {
            if(m) delete m;
            m = new m_input(from);
            return modtor_type_enum::input;
        }
        
        return modtor_type_enum::unknown;

    }


}


#endif /* maxlang_modtree_h */
