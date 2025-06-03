//
//  maxlang.utils.h
//  maxlang.modulator
//
//  Created by charles on 24/03/2020.
//

#ifndef maxlang_utils_h
#define maxlang_utils_h

#include <cstdint>
#include <string>
#include <cstdlib>
#include <cmath>

#include <uuid/uuid.h>

#include <sstream>

namespace maxlang
{

    namespace fmath
    {
        // 4.2 times as fast as normal pow
        inline double fastpow(double a, double b) {
          union {
            double d;
            int x[2];
          } u = { a };
          u.x[1] = (int)(b * (u.x[1] - 1072632447) + 1072632447);
          u.x[0] = 0;
          return u.d;
        }
    
        inline
        double fastexp(double x) {
          x = 1.0 + x / 1024;
          x *= x; x *= x; x *= x; x *= x;
          x *= x; x *= x; x *= x; x *= x;
          x *= x; x *= x;
          return x;
        }
    }

    const std::string CHARS = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
        
    std::string generateUUID(){
        std::string uuid = std::string(36,' ');
        int rnd = 0;
        
        uuid[8] = '-';
        uuid[13] = '-';
        uuid[18] = '-';
        uuid[23] = '-';
        
        uuid[14] = '4';
        
        for(int i=0;i<36;i++){
            if (i != 8 && i != 13 && i != 18 && i != 14 && i != 23) {
                if (rnd <= 0x02) {
                    rnd = 0x2000000 + (std::rand() * 0x1000000) | 0;
                }
                rnd >>= 4;
                uuid[i] = CHARS[(i == 19) ? ((rnd & 0xf) & 0x3) | 0x8 : rnd & 0xf];
            }
        }
        return uuid;
    }

    template <typename T>
    std::string to_string_with_precision(const T a_value, const int n = 6)
    {
        std::ostringstream out;
        out.precision(n);
        out << std::fixed << a_value;
        return out.str();
    }

    
    double modulo(double v, double mod)
    {
        double out;
        
        out = fmod(v,mod);
        out += mod;
        return fmod(out,mod);
        
    }

    double fold(double v, double lo1, double hi1)
    {
        double lo;
        double hi;
        if(lo1 == hi1){ return lo1; }
        if (lo1 > hi1) {
            hi = lo1; lo = hi1;
        } else {
            lo = lo1; hi = hi1;
        }
        const double range = hi - lo;
        long numWraps = 0;
        if(v >= hi){
            v -= range;
            if(v >= hi){
                numWraps = (long)((v - lo)/range);
                v -= range * (double)numWraps;
            }
            numWraps++;
        } else if(v < lo){
            v += range;
            if(v < lo){
                numWraps = (long)((v - lo)/range) - 1;
                v -= range * (double)numWraps;
            }
            numWraps--;
        }
        if(numWraps & 1) v = hi + lo - v;    // flip sign for odd folds
        return v;
    }
    
    class scale_curve
    {
        public :
        scale_curve()
        {
            setcurve(curve);
        }
        ~scale_curve()
        {
            
        }
        
        double curve=0;
        int curve_sign = 1;
        double in_min=0, in_max=1, out_min=0., out_max=1.;
        // curve magical coeff
        double c1 = 1e-20, c2 = 1.2, c3 = .41, c4 = .91;
        double bb,mm,dy=1.,dx = 1.;
        
        
        double apply(double f)
        {
            // scaled between 0 & 1
            double scaled_in = (f-in_min)/dx;
            dy = (out_max - out_min) * curve_sign;
            return (bb * (pow(mm,scaled_in)-1))*dy + out_min;
        }
        
        void setcurve(double c)
        {
            //curve = std::clamp(c,-1.04,1.04);
            curve = (c <= -1.04)? -1.04 : ((c > 1.04)? 1.04 : c);
            double hh, ff, eff, gh;
            
            if(curve<0)
            {
                hh = pow(((c1 - curve ) * c2), c3) * c4;
                curve_sign=-1;
            }else
            {
                hh = pow(((curve + c1) * c2), c3) * c4;
                curve_sign=1;
            }
            
            
            ff = hh / (1 - hh);
            eff = exp(ff) - 1;
            gh = (exp(ff * 0.5) - 1) / eff;
            bb = gh * (gh / (1 - (gh + gh)));
            
            if(curve<0)
            {
                mm = 1 / (((exp(ff) - 1) / (eff * bb)) + 1);
                bb += 1;
            }else
                mm = ((exp(ff) - 1) / (eff * bb)) + 1;
            
        }
        
        void setin_minmax(double _in_min, double _in_max)
        {
            in_min  = _in_min;
            in_max = _in_max;
            dx = in_max - in_min;
        }
        
        void setout_min(double _out_min)
        {
            out_min  = _out_min;
        }
        
        void setout_max(double _out_max)
        {
            out_max  = _out_max;

        }
        
        void setinout_minmax(double _in_min, double _in_max,double _out_min,double _out_max)
        {
            in_min  = _in_min;
            in_max = _in_max;
            dx = in_max - in_min;
            out_min  = _out_min;
            out_max  = _out_max;

        }
        
        
    };
}

#endif /* maxlang_utils_h */
