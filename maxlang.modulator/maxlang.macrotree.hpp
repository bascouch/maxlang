//
//  maxlang.macrotree.hpp
//  maxlang.modulator
//
//  Created by charles on 02/05/2022.
//

#ifndef maxlang_macrotree_h
#define maxlang_macrotree_h


#include <vector>
#include <map>
#include <cmath>
#include <random>
#include <algorithm>

namespace maxlang {

/** MACROS
 @rand(min max curve)
 @randint(min max curve)
 @nrand(n min max curve)
 @nrandint(n min max curve)
 @choice(val1 val2 ...)
 @nchoice(n val1 val2 ...)
 @range(min max curve)
 @nrange(n min max curve)
 @list(val1 val2 ...)
 @inc(start inc)
 @ninc(n start inc max)
 */

int macro_get_int(const pegtl::parse_tree::node& n)
{
    if(n.type == "maxlang::double_value")
    {
        double v = stod(n.string());
        return (int)v;
    }
    else if (n.type == "maxlang::int_value")
    {
        int v = stoi(n.string());
        return v;
    }
    return 0;
}

double macro_get_double(const pegtl::parse_tree::node& n)
{
    if(n.type == "maxlang::double_value")
    {
        double v = stod(n.string());
        return v;
    }
    else if (n.type == "maxlang::int_value")
    {
        int v = stoi(n.string());
        return (double)v;
    }
    return 0;
}

std::string macro_dshrnk(std::string& in)
{
    return in.erase (in.find_last_not_of('0') + 1, std::string::npos );
}

int macro_apply(const pegtl::parse_tree::node& n, std::vector<std::string>& out_str, int num, t_object * m_ob)
{
    std::string s;
    std::random_device rd_dev;
    std::mt19937 mt_gen = std::mt19937(rd_dev());
    
    
    // detect the root node:
    if( !n.is_root() ) {
        return 0;
    }
    
    // get into child which should be maxlang::modtor_expression
    if( n.children.empty() || n.children.size()<1 ) {
        return 0;
    }
    
    for( auto& node : n.children ) {
        
        maxlang::print_node( *node );
        if( node->type == "maxlang::macro_unmatched" )
            for( int i=0; i<num; i++ )
                out_str[i] += node->string();
        
        else if( node->type == "maxlang::macro_def" )
        {
            pegtl::parse_tree::node *type_node = node->children[0].get();
            // type_node->type == "maxlang::macro_type_name"
            std::string name = type_node->string();
            int num_c = (int) node->children.size();
            num_c--;
            
            if(name == "rand")
            {
                double min = (num_c-- > 0)? macro_get_double(*node->children[1].get()) : 0;
                double max = (num_c-- > 0)? macro_get_double(*node->children[2].get()) : 1;
                double curve = (num_c-- > 0)? macro_get_double(*node->children[3].get()) : 0;
                
                std::uniform_real_distribution<double> mt_rand = std::uniform_real_distribution<double>(-1.,1.);
                scale_curve output_scale;
                output_scale.setinout_minmax(-1., 1.,min,max);
                output_scale.setcurve(curve);
                double val;
                
                for( int i=0; i<num; i++ )
                {
                    val = output_scale.apply(mt_rand(mt_gen));
                    s = std::to_string(val);
                    out_str[i] += macro_dshrnk(s);
                }
                
                
            }
            else if (name == "nrand")
            {
                int vnum = (num_c-- > 0)? macro_get_int(*node->children[1].get()) : 1;
                double min = (num_c-- > 0)? macro_get_double(*node->children[2].get()) : 0;
                double max = (num_c-- > 0)? macro_get_double(*node->children[3].get()) : 1;
                double curve = (num_c-- > 0)? macro_get_double(*node->children[4].get()) : 0;
                
                std::uniform_real_distribution<double> mt_rand = std::uniform_real_distribution<double>(-1.,1.);
                scale_curve output_scale;
                output_scale.setinout_minmax(-1., 1.,min,max);
                output_scale.setcurve(curve);
                double val;
                
                for( int i=0; i<num; i++ )
                    for(int j=0; j< vnum; j++)
                    {
                        val = output_scale.apply(mt_rand(mt_gen));
                        s = std::to_string(val);
                        out_str[i] += macro_dshrnk(s);
                        out_str[i] += " ";
                    }
                
            } else if(name == "randint")
            {
                int min = (num_c-- > 0)? macro_get_int(*node->children[1].get()) : 0;
                int max = (num_c-- > 0)? macro_get_int(*node->children[2].get()) : 1;
                double curve = (num_c-- > 0)? macro_get_double(*node->children[3].get()) : 0;
                
                std::uniform_real_distribution<double> mt_rand = std::uniform_real_distribution<double>(-1.,1.);
                scale_curve output_scale;
                output_scale.setinout_minmax(-1., 1.,min,max);
                output_scale.setcurve(curve);
                long val;
                
                for( int i=0; i<num; i++ )
                {
                    val = lrint(output_scale.apply(mt_rand(mt_gen)));
                    out_str[i] += std::to_string(val);
                }
                
                
            }
            else if (name == "nrandint")
            {
                int vnum = (num_c-- > 0)? macro_get_int(*node->children[1].get()) : 1;
                int min = (num_c-- > 0)? macro_get_int(*node->children[2].get()) : 0;
                int max = (num_c-- > 0)? macro_get_int(*node->children[3].get()) : 1;
                double curve = (num_c-- > 0)? macro_get_double(*node->children[4].get()) : 0;
                
                std::uniform_real_distribution<double> mt_rand = std::uniform_real_distribution<double>(-1.,1.);
                scale_curve output_scale;
                output_scale.setinout_minmax(-1., 1.,min,max);
                output_scale.setcurve(curve);
                long val;
                
                for( int i=0; i<num; i++ )
                    for(int j=0; j< vnum; j++)
                    {
                        val = lrint(output_scale.apply(mt_rand(mt_gen)));
                        out_str[i] += std::to_string(val);
                        out_str[i] += " ";
                    }
                
            }else if(name == "range")
            {
                double min = (num_c-- > 0)? macro_get_double(*node->children[1].get()) : 0;
                double max = (num_c-- > 0)? macro_get_double(*node->children[2].get()) : 1;
                double curve = (num_c-- > 0)? macro_get_double(*node->children[3].get()) : 0;
                
                scale_curve output_scale;
                output_scale.setinout_minmax(0, 1.,min,max);
                output_scale.setcurve(curve);
                double val;
                
                if(num==1)
                {
                    s = std::to_string((double)(max-min)/2);
                    out_str[0] += macro_dshrnk(s);
                }
                else
                    for( int i=0; i<num; i++ )
                    {
                        val = output_scale.apply((double)i/(num-1));
                        s = std::to_string(val);
                        out_str[i] += macro_dshrnk(s);
                    }
                
                
            }
            else if (name == "nrange")
            {
                int vnum = (num_c-- > 0)? macro_get_int(*node->children[1].get()) : 1;
                double min = (num_c-- > 0)? macro_get_double(*node->children[2].get()) : 0;
                double max = (num_c-- > 0)? macro_get_double(*node->children[3].get()) : 1;
                double curve = (num_c-- > 0)? macro_get_double(*node->children[4].get()) : 0;
                
                scale_curve output_scale;
                output_scale.setinout_minmax(0., 1.,min,max);
                output_scale.setcurve(curve);
                double val;
                
                
                
                
                for( int i=0; i<num; i++ )
                    for(int j=0; j< vnum; j++)
                    {
                        if(vnum==1)
                        {
                            s = std::to_string((double)(max-min)/2);
                            out_str[0] += macro_dshrnk(s);
                            
                        }
                        else
                        {
                            val = output_scale.apply((double)j/(vnum-1));
                            s = std::to_string(val);
                            out_str[i] += macro_dshrnk(s);
                            out_str[i] += " ";
                        }
                    }
                
            }else if(name == "inc")
            {
                double start = (num_c-- > 0)? macro_get_double(*node->children[1].get()) : 0;
                double inc = (num_c-- > 0)? macro_get_double(*node->children[2].get()) : 1;
                
                double val = start;
                
                for( int i=0; i<num; i++ )
                {
                    s = std::to_string(val);
                    out_str[i] += macro_dshrnk(s);
                    val += inc;
                }
                
                
            }
            else if (name == "ninc")
            {
                int vnum = (num_c-- > 0)? macro_get_int(*node->children[1].get()) : 1;
                double start = (num_c-- > 0)? macro_get_double(*node->children[2].get()) : 0;
                double inc = (num_c-- > 0)? macro_get_double(*node->children[3].get()) : 1;
                
                double val;
                
                for( int i=0; i<num; i++ )
                {
                    val = start;
                    for(int j=0; j< vnum; j++)
                    {
                        s = std::to_string(val);
                        out_str[i] += macro_dshrnk(s);
                        val += inc;
                    }
                    
                }
                
            }else if(name == "choice")
            {
                std::vector<std::string> m_list;
                for (int k = 0; k < num_c; k++)
                    m_list.push_back(node->children[k+1].get()->string());
                int m_list_l = (int)m_list.size();
                std::uniform_real_distribution<double> mt_rand = std::uniform_real_distribution<double>(0.,1.);
                int index;
                
                for( int i=0; i<num; i++ )
                {
                    index = floor(mt_rand(mt_gen) * (m_list_l));
                    out_str[i] += m_list[index];
                }
                
                
            }
            else if (name == "nchoice")
            {
                int vnum = (num_c-- > 0)? macro_get_int(*node->children[1].get()) : 1;
                
                std::vector<std::string> m_list;
                for (int k = 0; k < num_c; k++)
                    m_list.push_back(node->children[k+2].get()->string());
                int m_list_l = (int)m_list.size();
                std::uniform_real_distribution<double> mt_rand = std::uniform_real_distribution<double>(0.,1.);
                int index;
                
                for( int i=0; i<num; i++ )
                    for(int j=0; j< vnum; j++)
                    {
                        index = floor(mt_rand(mt_gen) * (m_list_l));
                        out_str[i] += m_list[index];
                        out_str[i] += " ";
                    }
                
            }else if(name == "urn")
            {
                std::vector<std::string> m_list;
                std::vector<std::string> m_list_shuffle;
                for (int k = 0; k < num_c; k++)
                    m_list.push_back(node->children[k+1].get()->string());
                auto rng = std::default_random_engine {};
                
                m_list_shuffle = m_list;
                std::shuffle(std::begin(m_list_shuffle), std::end(m_list_shuffle), rng);
                
                
                for( int i=0; i<num; i++ )
                {
                    if(m_list_shuffle.empty())
                    {
                        m_list_shuffle = m_list;
                        std::shuffle(std::begin(m_list_shuffle), std::end(m_list_shuffle), rng);
                    }
                    out_str[i] += m_list_shuffle.back();
                    m_list_shuffle.pop_back();

                }
                
                
            }
            else if (name == "nurn")
            {
                int vnum = (num_c-- > 0)? macro_get_int(*node->children[1].get()) : 1;
                
                std::vector<std::string> m_list;
                std::vector<std::string> m_list_shuffle;
                for (int k = 0; k < num_c; k++)
                    m_list.push_back(node->children[k+2].get()->string());
                auto rng = std::default_random_engine {};
                
                for( int i=0; i<num; i++ )
                {
                    m_list_shuffle = m_list;
                    std::shuffle(std::begin(m_list_shuffle), std::end(m_list_shuffle), rng);
                    for(int j=0; j< vnum; j++)
                    {
                        if(m_list_shuffle.empty())
                        {
                            m_list_shuffle = m_list;
                            std::shuffle(std::begin(m_list_shuffle), std::end(m_list_shuffle), rng);
                        }
                        out_str[i] += m_list_shuffle.back();
                        out_str[i] += " ";
                        m_list_shuffle.pop_back();
                            
                    }
                     
                }
                
            }else if(name == "list")
            {
                std::vector<std::string> m_list;
                std::vector<std::string> m_list_shuffle;
                for (int k = 0; k < num_c; k++)
                    m_list.push_back(node->children[k+1].get()->string());
                int m_list_l = (int) m_list.size();
                int index = 0;
                
                for( int i=0; i<num; i++ )
                {
                    out_str[i] += m_list[index];
                    index = (index + 1) % m_list_l;

                }
                
                
            }
            
            
        }
        

    }
    
    return 1;
}

int macro_parse_and_apply(const std::string& input, const int n,std::vector<std::string>& out_str, t_object * m_ob )
{
    
    pegtl::string_input<> in(input, "source" );
    const auto root = pegtl::parse_tree::parse< maxlang::macro_start, maxlang::store >(in);
    if(!root)
    {
        std::cout << "PARSE FAILED" << std::endl;
        return 0;
    }
    
    maxlang::print_node( *root );

    out_str.clear();
    out_str.assign(10,"");
    
    return macro_apply(*root, out_str, n, m_ob );

}


}



#endif /* maxlang_macrotree_h */
