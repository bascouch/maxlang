#include "maxcpp6.h"
#include <string>
#include <iostream>

#include <tao/pegtl.hpp>
#include <tao/pegtl/contrib/parse_tree.hpp>

namespace pegtl = tao::pegtl;


namespace maxlang
{
    // Parsing rule that matches a literal "Hello, ".
    // clang-format off
    // keywords
    
    // One or more spaces (between expressions)
    struct seps : pegtl::star< pegtl::blank > {};
    
    struct key_bool_true : TAO_PEGTL_KEYWORD( "true" ) {};
    struct key_bool_false : TAO_PEGTL_KEYWORD( "false" ) {};
    
    // Values
    struct double_value
    : pegtl::sor<
    pegtl::seq< pegtl::opt< pegtl::one< '+', '-' > >, seps, pegtl::plus< pegtl::digit > , pegtl::one<'.'>, pegtl::plus<pegtl::digit> , seps >,
    pegtl::seq< pegtl::opt< pegtl::one< '+', '-' > >, seps, pegtl::one<'.'>, pegtl::plus<pegtl::digit> ,seps>,
    pegtl::seq< pegtl::opt< pegtl::one< '+', '-' > >, seps, pegtl::plus< pegtl::digit > , pegtl::one<'.'> , seps>,
    pegtl::seq< pegtl::opt< pegtl::one< '+', '-' > >, seps, pegtl::plus< pegtl::digit > , seps >
    >{};
    struct int_value : pegtl::seq< pegtl::opt< pegtl::one< '+', '-' > >, seps, pegtl::plus< pegtl::digit > , seps> {};
    struct bool_value
    : pegtl::sor<
    pegtl::seq<key_bool_true>,
    pegtl::seq<key_bool_false>,
    pegtl::one< '0' >,
    pegtl::one< '1' >
    >{};
    struct positive_int_value : pegtl::seq< pegtl::opt< pegtl::one< '+' > >, seps, pegtl::plus< pegtl::digit >, seps > {};
    
    
    struct modtor_argument_value;
    
    struct list_value : pegtl::list<modtor_argument_value, seps> {};
    
    struct list_expression : pegtl::seq< seps, pegtl::one<'['>, list_value, pegtl::one<']'>, seps > {};
    
    struct litteral : pegtl::plus<pegtl::alpha> {};
    
    // modtor specific
    struct modtor_expression;
    struct modtor_argument_value : pegtl::sor<double_value,  int_value, bool_value, list_expression, modtor_expression > {};
    struct modtor_argument_name : litteral {};
    
    struct modtor_type : litteral {};
    

    struct modtor_argument : pegtl::seq< modtor_argument_name, seps, pegtl::one<'='>, seps, modtor_argument_value, seps > {};

    
    struct modtor_expression : pegtl::must<modtor_type, seps, pegtl::one<'('>, seps, pegtl::star<modtor_argument>, seps, pegtl::one<')'>, seps, pegtl::eolf > {};
    
    
    struct prefix
    : pegtl::string< 'H', 'e', 'l', 'l', 'o', ',', ' ' >
    {};
    
    // Parsing rule that matches a non-empty sequence of
    // alphabetic ascii-characters with greedy-matching.
    
    struct name
    : pegtl::plus< pegtl::alpha >
    {};
    
    // Parsing rule that matches a sequence of the 'prefix'
    // rule, the 'name' rule, a literal "!", and 'eof'
    // (end-of-file/input), and that throws an exception
    // on failure.
    
    struct grammar
    : pegtl::must< prefix, name, pegtl::one< '!' > >
    {};
    
    // Class template for user-defined actions that does
    // nothing by default.
    
    template< typename Rule >
    struct action
    {};
    
    // Specialisation of the user-defined action to do
    // something when the 'name' rule succeeds; is called
    // with the portion of the input that matched the rule.
    
    template<>
    struct action< modtor_argument >
    {
        template< typename Input >
        static void apply( const Input& in, std::string& v )
        {
            v += " " + in.string();
            post("modtor_argument : %s",in.string().c_str());
        }
    };
    
    template<>
    struct action< modtor_type >
    {
        template< typename Input >
        static void apply( const Input& in, std::string& v )
        {
            v += " " + in.string();
            post("modtor_type : %s",in.string().c_str());
        }
    };
    
    template<>
    struct action< modtor_argument_value >
    {
        template< typename Input >
        static void apply( const Input& in, std::string& v )
        {
            v += " " + in.string();
            post("modtor_argument_value : %s",in.string().c_str());
        }
    };
    
    template<>
    struct action< modtor_argument_name >
    {
        template< typename Input >
        static void apply( const Input& in, std::string& v )
        {
            v += " " + in.string();
            post("modtor_argument_name : %s",in.string().c_str());
        }
    };
    
    template<>
    struct action< list_value >
    {
        template< typename Input >
        static void apply( const Input& in, std::string& v )
        {
            v += " " + in.string();
            post("modtor_list_value : %s",in.string().c_str());
        }
    };
    
}  // namespace hello


class maxlang_modulator : public MaxCpp6<maxlang_modulator> {
public:
	maxlang_modulator(t_symbol * sym, long ac, t_atom * av) {
		setupIO(1, 1); // inlets / outlets
	}
	~maxlang_modulator() {}
	
	// methods:
	void bang(long inlet) { 
		outlet_bang(m_outlets[0]);
	}
	
	void testfloat(long inlet, double v) {
		post("inlet %ld float %f", inlet, v);
		outlet_float(m_outlets[0], v);
	}
	
	void testint(long inlet, long v) {
		post("inlet %ld int %ld", inlet, v);
		outlet_int(m_outlets[0], v);
	}
	
	void test(long inlet, t_symbol * s, long ac, t_atom * av) { 
		outlet_anything(m_outlets[0], gensym("test"), ac, av);
	}
    
    void parse(long inlet, t_symbol * s, long ac, t_atom * av) {
        std::string name;
        std::string atoms;
        
        for(int i=0; i<ac; i++)
            switch(av[i].a_type)
            {
                case A_SYM:
                    atoms += av[i].a_w.w_sym->s_name;
                    atoms += " ";
                    break;
                case A_LONG:
                    atoms += av[i].a_w.w_long;
                    atoms += " ";
                    break;
                case A_FLOAT:
                    atoms += av[i].a_w.w_float;
                    atoms += " ";
                    break;
            }
        
        post("maxlang.modulator: parsing %s",atoms.c_str());

        pegtl::string_input input( atoms, std::string("input"));
        try {
            pegtl::parse< maxlang::modtor_expression, maxlang::action >( input, name );
        } catch (const std::exception& e) {
            error(e.what());
            return;
        }
        
        if(name.c_str() != nullptr)
            outlet_anything(m_outlets[0], gensym(name.c_str()), 0, av);
    }
};

C74_EXPORT int main(void) {
	// create a class with the given name:
	maxlang_modulator::makeMaxClass("maxlang.modulator");
	REGISTER_METHOD(maxlang_modulator, bang);
	REGISTER_METHOD_FLOAT(maxlang_modulator, testfloat);
	REGISTER_METHOD_LONG(maxlang_modulator, testint);
	REGISTER_METHOD_GIMME(maxlang_modulator, test);
    REGISTER_METHOD_GIMME(maxlang_modulator, parse);
	
	// these are for handling float/int messages directly (no method name in Max):
	REGISTER_INLET_FLOAT(maxlang_modulator, testfloat);
	REGISTER_INLET_LONG(maxlang_modulator, testint);
}
