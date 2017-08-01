#include "tao/pegtl.hpp"
#include <string>
#include <Rcpp.h>

using namespace tao::pegtl;

namespace xlref
{
  // Three tokens: Ref (A1-style), Text, Other

  // Text matches two QuoteD (") and anything between, i.e. character and
  // the surrounding pair of double-quotes.
  struct QuoteD : one< '"' > {};
  struct NotQuoteD : not_one< '"' > {};
  struct DoubleQuotedString : star< sor< seq< QuoteD, QuoteD >,
                                         NotQuoteD > >
  {};
  struct Text : if_must< QuoteD, DoubleQuotedString, QuoteD > {};

  // After attempting a Ref, consume sequences of letters, or sequences of
  // punctuation, then try again.
  /* struct Alpha : ranges< 'a', 'z', 'A', 'Z' > {}; */
  struct Alnum : ranges< 'a', 'z', 'A', 'Z', '0', '9' > {};
  /* struct AlphaQuoteD : ranges< 'a', 'z', 'A', 'Z', '"' > {}; */
  struct AlnumQuoteD : ranges< 'a', 'z', 'A', 'Z', '0', '9', '"' > {};
  /* struct NotAlphaQuoteD : if_then_else< at< AlphaQuoteD >, failure, any > {}; */
  struct NotAlnumQuoteD : if_then_else< at< AlnumQuoteD >, failure, any > {};
  /* struct Other: sor< plus< Alpha >, plus< NotAlphaQuoteD > > {}; */
  struct Other: sor< plus< Alnum >, plus< NotAlnumQuoteD > > {};
  struct NotRef : sor< Text, Other > {};

  struct dollar : one< '$' > {};
  struct OptDollar : opt< dollar > {};

  struct RowToken : rep_min_max< 1, 7, digit > {};

  // Anything above XFD is not a valid column
  struct MaybeColToken : rep_min_max< 1, 3, upper > {};
  struct BadColToken : seq< range< 'X', 'Z' >,
                            range< 'F', 'Z' >,
                            range< 'E', 'Z' > > {};
  struct ColToken : seq< not_at< BadColToken >, MaybeColToken > {};

  struct OptRowToken : seq< OptDollar, RowToken > {};
  struct OptColToken : seq< OptDollar, ColToken > {};

  // Attempt to match addresses in this order
  // A:A
  // A1
  // A1:A2
  // 1:1
  struct colon : one< ':' > {};
  struct Ref :
    seq< OptDollar,
         sor< seq< ColToken,
                   if_then_else< colon,
                                 OptColToken,                    // A:A
                                 seq< OptRowToken,               // A1
                                      opt< colon,
                                           OptColToken,
                                           OptRowToken  > > > >, // A1:A2
              seq< RowToken,
                   colon,
                   OptRowToken > >,
         /* sor< not_at< Alnum >, at< eof > >, */
         not_at< Alnum >,                                        // not e.g. A1A
         not_at< one< '(' > > > {};                              // not e.g. LOG10()

  // Overall parsing rule
  struct root : seq< opt< Ref >,
                     star< seq< NotRef,
                           opt< Ref > > > > {};

  // Class template for user-defined actions that does
  // nothing by default.

  template<typename Rule>
    struct tokenize : nothing<Rule> {};

  // Specialisation of the user-defined action to do something when a rule
  // succeeds; is called with the portion of the input that matched the rule.

/*   template<> struct tokenize< root > */
/*   { */
/*     template< typename Input > */
/*       static void apply( const Input & in, std::string & token ) */
/*       { */
/*         token = in.string(); */
/*       } */
/*   }; */

  template<> struct tokenize< Ref >
  {
    template< typename Input >
      static void apply( const Input & in,
                         std::vector<std::string> & token_type,
                         std::vector<std::string> & token )
      {
        /* Rcpp::Rcout << "Ref: " << in.string() << "\n"; */
        token_type.push_back("ref");
        token.push_back(in.string());
      }
  };

  template<> struct tokenize< Text >
  {
    template< typename Input >
      static void apply( const Input & in,
                         std::vector<std::string> & token_type,
                         std::vector<std::string> & token )
      {
        /* Rcpp::Rcout << "Text: " << in.string() << "\n"; */
        token_type.push_back("text");
        token.push_back(in.string());
      }
  };

  template<> struct tokenize< Other >
  {
    template< typename Input >
      static void apply( const Input & in,
                         std::vector<std::string> & token_type,
                         std::vector<std::string> & token )
      {
        /* Rcpp::Rcout << "Other: " << in.string() << "\n"; */
        token_type.push_back("other");
        token.push_back(in.string());
      }
  };

} // xlref
