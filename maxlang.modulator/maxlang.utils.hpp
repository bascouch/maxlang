//
//  maxlang.utils.h
//  maxlang.modulator
//
//  Created by charles on 24/03/2020.
//

#ifndef maxlang_utils_h
#define maxlang_utils_h

#include <cstdint>
#include <cmath>

namespace maxlang
{
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
        double in_min=0, in_max=1, out_min=0., out_max=1.;
        // curve magical coeff
        double c1 = 1e-20, c2 = 1.2, c3 = .41, c4 = .91;
        double bb,mm,dy=1.,dx = 1.;
        
        
        double apply(double f)
        {
            // scaled between 0 & 1
            double scaled_in = (f-in_min)/dx;
            return (bb * (pow(mm,scaled_in)-1))*dy + out_min;
        }
        
        void setcurve(double c)
        {
            curve = std::clamp(c,-1.4,1.04);
            double hh, ff, eff, gh;
            
            dy = out_max - out_min;
            
            if(curve<0)
            {
                hh = pow(((c1 - curve ) * c2), c3) * c4;
                dy*=-1;
            }else
                hh = pow(((curve + c1) * c2), c3) * c4;
            
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
            dy = out_max - out_min;
        }
        
        void setout_max(double _out_max)
        {
            out_max  = _out_max;
            dy = out_max - out_min;
        }
        
        
    };
}

#endif /* maxlang_utils_h */
