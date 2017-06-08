#include <Rcpp.h>
#include "tao/pegtl/contrib/tracer.hpp"
#include "tao/pegtl/analyze.hpp"
#include "xltoken.hpp"

// [[Rcpp::export]]
void xl_check_grammar_()
{
  const size_t issues_found = tao::pegtl::analyze< xltoken::root >();
}

// [[Rcpp::export]]
Rcpp::CharacterVector xl_formula_(Rcpp::CharacterVector x)
{
  int n = x.size();
  Rcpp::CharacterVector out(n);

  std::string in_string;
  std::string out_string;

  Rcpp::CharacterVector::iterator in_it, out_it;

  for(in_it = x.begin(), out_it = out.begin();
      in_it != x.end();
      ++in_it, ++out_it) {
    in_string = Rcpp::as< std::string >(*in_it);
    out_string = ""; // The parse step doesn't return an empty string on failure

    /* Rcpp::Rcout << in_string + "\n"; */

    memory_input<> in_mem( in_string, "original-formula" );
    /* parse< xltoken::root, xltoken::tokenize, tracer >( in_mem, out_string); */
    parse< xltoken::root, xltoken::tokenize >( in_mem, out_string);

    *out_it = out_string;
  }

  return out;
  /* Rcpp::Rcout << "Formula is: " << formula_tokens << "\n"; */
}
