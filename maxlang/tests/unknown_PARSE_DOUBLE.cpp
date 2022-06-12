// Copyright (c) 2017-2018 Dr. Colin Hirsch and Daniel Frey
// Please see LICENSE for license or visit https://github.com/taocpp/PEGTL/

#include <iostream>
#include <string>
#include <type_traits>
#include <iostream>
#include <tuple>

#include <tao/pegtl.hpp>
#include <tao/pegtl/contrib/parse_tree.hpp>

using namespace tao::TAO_PEGTL_NAMESPACE;  // NOLINT
using namespace tao;

namespace pop {

    // clang-format off
    // keywords
    struct key_minimize : keyword< "minimize" ) {};
    struct key_maximize : TAOCPP_PEGTL_STRING( "maximize" ) {};
    struct key_constraint : TAOCPP_PEGTL_STRING( "constraint" ) {};
    struct key_double : TAOCPP_PEGTL_STRING( "double" ) {};
    struct key_int : TAOCPP_PEGTL_STRING( "int" ) {};
    struct key_bool : TAOCPP_PEGTL_STRING( "bool" ) {};
    struct key_bool_true : TAOCPP_PEGTL_STRING( "true" ) {};
    struct key_bool_false : TAOCPP_PEGTL_STRING( "false" ) {};

    // One or more spaces (between expressions)
    struct seps : star< pegtl::one<' '> > {};

    // C++-style identifiers
    struct var_name : identifier {};

    // Values
    struct double_value
            : sor<
                    seq< opt< pegtl::one< '+', '-' > >, seps, pegtl::plus< digit > , pegtl::one<'.'>, pegtl::plus<digit> , seps >,
                    seq< opt< pegtl::one< '+', '-' > >, seps, pegtl::one<'.'>, pegtl::plus<digit> ,seps>,
                    seq< opt< pegtl::one< '+', '-' > >, seps, pegtl::plus< digit > , pegtl::one<'.'> , seps>,
                    seq< opt< pegtl::one< '+', '-' > >, seps, pegtl::plus< digit > , seps >
            >{};
    struct int_value : seq< opt< pegtl::one< '+', '-' > >, seps, pegtl::plus< digit > , seps> {};
    struct bool_value
            : sor<
                    seq<key_bool_true>,
                    seq<key_bool_false>,
                    pegtl::one< '0' >,
                    pegtl::one< '1' >
            >{};
    struct positive_int_value : seq< opt< pegtl::one< '+' > >, seps, pegtl::plus< digit >, seps > {};
    struct expression;
    struct bracket : seq< pegtl::one< '(' >, expression, pegtl::one< ')' > > {};
    struct atomic : sor< int_value, double_value, bracket, var_name > {};

    // infixes for mathematical expressions
    struct arithmetic_infix : pegtl::one< '+', '-', '*', '/', '^','%'>{};
    struct relational_infix : sor<pegtl::one< '=', '>', '<'>,pegtl::string<'<','='>,pegtl::string<'=','='>,pegtl::string<'>','='>,pegtl::string<'!','='>,pegtl::string<'~','='>>{};
    struct logical_infix : sor<pegtl::one<'!'>,pegtl::string<'&','&'>,pegtl::string<'n','o','t'>,pegtl::string<'|','|'>,pegtl::string<'a','n','d'>,pegtl::string<'o','r'>>{};
    struct infix : sor<arithmetic_infix,logical_infix,relational_infix>{};
    struct expression : pegtl::list< atomic, arithmetic_infix, pegtl::one<' '> > {};
    struct expression_complete : expression {};

    // rules to identify beggining and endding of C++ code
    struct cpp_code : star<not_one<'{','}'>>{};
    struct cpp_sub_brackets
            : sor<
                    seq< pegtl::one< '{' >, cpp_code, cpp_sub_brackets, cpp_code, pegtl::one< '}' >, seps >,
                    seq< pegtl::one< '{' >, cpp_code, pegtl::one< '}' >, seps >
            >{};
    struct cpp_brackets
            : sor<
                    seq< pegtl::one< '{' >, cpp_code, cpp_sub_brackets, cpp_code , pegtl::one< '}' >, seps >,
                    seq< pegtl::one< '{' >, cpp_code, star<cpp_sub_brackets, cpp_code> , pegtl::one< '}' >, seps >,
                    seq< pegtl::one< '{' >, cpp_code, pegtl::one< '}' >, seps >
            >{};

    // Rules to define objective functions
    struct function_keyword: sor<key_minimize,key_maximize>{};
    struct cpp_function: seq<function_keyword,seps,var_name,seps,pegtl::one<'='>,seps,cpp_brackets,seps>{};
    struct inline_function: seq<function_keyword,seps,var_name,seps,pegtl::one<'='>,seps,expression_complete,seps>{};
    struct function : sor <cpp_function,inline_function>{};

    // Rules to define constraints
    struct constraint_infix:relational_infix{};
    struct cpp_constraint: seq<key_constraint,seps,var_name,seps,pegtl::one<'='>,seps,cpp_brackets>{};
    struct inline_constraint: seq<key_constraint,seps,var_name,seps,pegtl::one<'='>,seps,expression_complete,seps,constraint_infix,seps,expression_complete>{};
    struct constraint : sor <cpp_constraint,inline_constraint>{};

    // Rules to define decision variables
    struct decision_variable_dec_double_array: seq<key_double,seps,pegtl::one<'['>,seps,double_value,seps,pegtl::one<','>,seps,double_value,seps,pegtl::one<']'>,seps,var_name,seps,pegtl::one<'['>,seps,positive_int_value,seps,pegtl::one<']'>,seps>{};
    struct decision_variable_dec_int_array: seq<key_int,seps,pegtl::one<'['>,seps,int_value,seps,pegtl::one<','>,seps,int_value,seps,pegtl::one<']'>,seps,var_name,seps,pegtl::one<'['>,seps,positive_int_value,seps,pegtl::one<']'>,seps>{};
    struct decision_variable_dec_bool_array: seq<key_bool,seps,var_name,seps,pegtl::one<'['>,seps,positive_int_value,seps,pegtl::one<']'>,seps>{};
    struct decision_variable_dec_double: seq<key_double,seps,pegtl::one<'['>,seps,double_value,seps,pegtl::one<','>,seps,double_value,seps,pegtl::one<']'>,seps,var_name,seps>{};
    struct decision_variable_dec_int: seq<key_int,seps,pegtl::one<'['>,seps,int_value,seps,pegtl::one<','>,seps,int_value,seps,pegtl::one<']'>,seps,var_name,seps>{};
    struct decision_variable_dec_bool: seq<key_bool,seps,var_name,seps>{};
    struct decision_variable_dec_int_unbounded: seq<key_int,seps,var_name,seps>{};
    struct decision_variable_dec_double_unbounded: seq<key_double,seps,var_name,seps>{};
    struct decision_variable_dec
            : sor<
                    decision_variable_dec_double_array,
                    decision_variable_dec_int_array,
                    decision_variable_dec_bool_array,
                    decision_variable_dec_double,
                    decision_variable_dec_int,
                    decision_variable_dec_bool,
                    decision_variable_dec_double_unbounded,
                    decision_variable_dec_int_unbounded
            >{};

    // Rules to define parameters
    struct double_list :pegtl::list<double_value,pegtl::one<','>,space>{};
    struct int_list :pegtl::list<int_value,pegtl::one<','>,space>{};
    struct bool_list :pegtl::list<bool_value,pegtl::one<','>,space>{};
    struct parameter_dec_double: seq<key_double,seps, var_name,seps, pegtl::one<'='>,seps, double_value, seps>{};
    struct parameter_dec_int: seq<key_int,seps, var_name,seps, pegtl::one<'='>,seps, int_value, seps>{};
    struct parameter_dec_bool: seq<key_bool,seps, var_name,seps, pegtl::one<'='>,seps, bool_value, seps>{};
    struct parameter_dec_double_array: seq<key_double,seps, var_name,seps, pegtl::one<'='>,seps, pegtl::one<'{'>,seps, double_list,seps,pegtl::one<'}'>,seps>{};
    struct parameter_dec_int_array: seq<key_int,seps, var_name,seps, pegtl::one<'='>,seps, pegtl::one<'{'>,seps,int_list,pegtl::one<'}'>,seps>{};
    struct parameter_dec_bool_array: seq<key_bool,seps, var_name,seps, pegtl::one<'='>,seps, pegtl::one<'{'>,seps,bool_list,pegtl::one<'}'>,seps>{};
    struct parameter_dec
            : sor<
                    parameter_dec_double,
                    parameter_dec_int,
                    parameter_dec_bool,
                    parameter_dec_double_array,
                    parameter_dec_int_array,
                    parameter_dec_bool_array
            >{};

    // All possible kinds of declaration
    struct declaration
            : sor<
                    parameter_dec,
                    decision_variable_dec,
                    function,
                    constraint
            >{};


    // All possible kinds of comments
    struct special_character : pegtl::one< ' ','(',')','+', '-', '*', '/', '^', '%', ':', '.', ',', '~', '='>{};
    struct comment
            : sor<
                    seq< pegtl::string<'/','/'>, star<sor<alnum, special_character>> >,
                    seq< pegtl::string<'-','-'>, star<sor<alnum, special_character>> >,
                    seq< pegtl::one<'#'>, star<sor<alnum, special_character>> >,
                    seq< pegtl::string<'/','*'>, star<sor<alnum, special_character>>, pegtl::string<'*','/'>>
            >{};

    // The whole code is just a list of statements (declaration or comment)
    struct statement : seq<sor<comment,declaration,success>>{};
    struct statement_with_eol: seq<seps,statement,eol>{};
    struct statement_list: seq<star<statement_with_eol>,opt<seq<seps,statement>>>{};
    struct grammar : must<statement_list,eof> {};

    // by default, nodes are not generated/stored
    template< typename > struct store : std::false_type {};
    // select which rules in the grammar will produce parse tree nodes:
    template<> struct store<double_value> : std::true_type {};
    template<> struct store<bool_value> : std::true_type {};
    template<> struct store<int_value> : std::true_type {};
    template<> struct store<positive_int_value> : std::true_type {};
    template<> struct store<cpp_brackets> : std::true_type {};
    template<> struct store<expression> : std::true_type {};
    template<> struct store<double_list> : std::true_type {};
    template<> struct store<constraint_infix> : std::true_type {};
    template<> struct store<int_list> : std::true_type {};
    template<> struct store<bool_list> : std::true_type {};
    template<> struct store<var_name> : std::true_type {};
    template<> struct store<function_keyword> : std::true_type {};
    template<> struct store<cpp_function> : parse_tree::remove_content {};
    template<> struct store<inline_function> : std::true_type {};
    template<> struct store<cpp_constraint> : parse_tree::remove_content {};
    template<> struct store<inline_constraint> : std::true_type {};
    template<> struct store<expression_complete> : std::true_type {};
    template<> struct store<key_constraint> : std::true_type {};
    template<> struct store<parameter_dec_double> : parse_tree::remove_content {};
    template<> struct store<parameter_dec_int> : parse_tree::remove_content {};
    template<> struct store<parameter_dec_bool> : parse_tree::remove_content {};
    template<> struct store<parameter_dec_double_array> : parse_tree::remove_content {};
    template<> struct store<parameter_dec_int_array> : parse_tree::remove_content {};
    template<> struct store<parameter_dec_bool_array> : parse_tree::remove_content {};
    template<> struct store<decision_variable_dec_double> : parse_tree::remove_content {};
    template<> struct store<decision_variable_dec_double_array> : parse_tree::remove_content {};
    template<> struct store<decision_variable_dec_double_unbounded> : parse_tree::remove_content {};
    template<> struct store<decision_variable_dec_int_unbounded> : parse_tree::remove_content {};
    template<> struct store<decision_variable_dec_bool> : parse_tree::remove_content {};
    template<> struct store<decision_variable_dec_bool_array> : parse_tree::remove_content {};
    template<> struct store<decision_variable_dec_int> : parse_tree::remove_content {};
    template<> struct store<decision_variable_dec_int_array> : parse_tree::remove_content {};
    // clang-format on


    // debugging/show result:

    void print_node( const parse_tree::node& n, const std::string& s = "" )
    {
        // detect the root node:
        if( n.is_root() ) {
            std::cout << "ROOT" << std::endl;
        }
        else {
            if( n.has_content() ) {
                std::cout << s << n.name() << " \"" << n.content() << "\" at " << n.begin() << " to " << n.end() << std::endl;
            }
            else {
                std::cout << s << n.name() << " at " << n.begin() << std::endl;
            }
        }
        // print all child nodes
        if( !n.children.empty() ) {
            const auto s2 = s + "  ";
            for( auto& up : n.children ) {
                print_node( *up, s2 );
            }
        }
    }
}

int main()
{
  try {
     string_input<> in( std::string("\n"
                                            "// Reference:  https://www.sfu.ca/~ssurjano/levy.html\n"
                                            "\n"
                                            "\n"
                                            "// decision variables\n"
                                            "double[-10.0, 10.0] x[10]\n"
                                            "\n"
                                            "\n"
                                            "// parameter variables\n"
                                            "int dim = 10\n"
                                            "double pi = 3.14159265359\n"
                                            "\n"
                                            "\n"
                                            "// objective functions (c++ like and simplified)\n"
                                            "minimize levyFunction = {\n"
                                            "    //double w[10];\n"
                                            "    double term1;\n"
                                            "    double term3;\n"
                                            "    double sum = 0;\n"
                                            "    double newTerm;\n"
                                            "\n"
                                            "    //for(int i = 0; i < dim; i++){\n"
                                            "    //    w[i] = 1 + (x[i] - 1) / 4;\n"
                                            "    //}\n"
                                            "\n"
                                            "    //term1 = pow(sin(pi*w[1]), 2);\n"
                                            "    term1 = pow(sin(pi*(1 + (x[1] - 1) / 4)), 2);\n"
                                            "\n"
                                            "    //term3 = pow(w[dim]-1, 2) * (1 + pow(sin(2*pi*w[dim]),2));\n"
                                            "    term3 = pow((1 + (x[dim-1] - 1) / 4)-1, 2) * (1 + pow(sin(2*pi*(1 + (x[dim-1] - 1) / 4)),2));\n"
                                            "\n"
                                            "    for(int i = 0; i < dim-1; i++){\n"
                                            "        //newTerm = pow(w[i] - 1, 2) * (1 + 10*pow(sin(pi*w[i]+1), 2));\n"
                                            "        newTerm = pow((1 + (x[i] - 1) / 4) - 1, 2) * (1 + 10*pow(sin(pi*(1 + (x[i] - 1) / 4)+1), 2));\n"
                                            "        sum += newTerm;\n"
                                            "    }\n"
                                            "    return term1 + sum + term3;\n"
                                            "}"), "source" );
     if( const auto root = parse_tree::parse< pop::grammar, pop::store >( in ) ) {
        pop::print_node( *root );
     }
     else {
        std::cout << "PARSE FAILED" << std::endl;
     }
  }
  catch( const std::exception& e ) {
     std::cout << "PARSE FAILED WITH EXCEPTION: " << e.what() << std::endl;
  }
   return 0;
}
