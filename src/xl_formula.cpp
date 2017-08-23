#include <Rcpp.h>
#include "tao/pegtl/contrib/tracer.hpp"
#include "tao/pegtl/analyze.hpp"
#include "xltoken.hpp"

// [[Rcpp::export]]
void xl_check_grammar_()
{
  const size_t issues_found = tao::pegtl::analyze< xltoken::root >();
}

/* // [[Rcpp::export]] */
/* Rcpp::CharacterVector xl_formula_(Rcpp::CharacterVector x) */
/* { */
/*   int n = x.size(); */
/*   Rcpp::CharacterVector out(n); */

/*   std::string in_string; */
/*   std::string out_string; */

/*   Rcpp::CharacterVector::iterator in_it, out_it; */

/*   for(in_it = x.begin(), out_it = out.begin(); */
/*       in_it != x.end(); */
/*       ++in_it, ++out_it) { */
/*     in_string = Rcpp::as< std::string >(*in_it); */
/*     out_string = ""; // The parse step doesn't return an empty string on failure */

/*     /1* Rcpp::Rcout << in_string + "\n"; *1/ */

/*     memory_input<> in_mem( in_string, "original-formula" ); */
/*     /1* parse< xltoken::root, xltoken::tokenize, tracer >( in_mem, out_string); *1/ */
/*     parse< xltoken::root, xltoken::tokenize >( in_mem, out_string); */

/*     *out_it = out_string; */
/*   } */

/*   return out; */
/*   /1* Rcpp::Rcout << "Formula is: " << formula_tokens << "\n"; *1/ */
/* } */


// [[Rcpp::export]]
Rcpp::List xl_formula_(Rcpp::CharacterVector x)
{

  std::string in_string;             // the formula from the iterator
  Rcpp::List out;               // wraps the vectors below
  std::vector<std::string> types;
  std::vector<std::string> tokens;

    in_string = Rcpp::as< std::string >(x);

    memory_input<> in_mem( in_string, "original-formula" );
    /* parse< xltoken::root, xltoken::tokenize >( in_mem, types, tokens); */
    parse< xltoken::root, xltoken::tokenize, tracer >( in_mem, types, tokens);

    out = Rcpp::List::create(
        Rcpp::_["type"] = types,
        Rcpp::_["token"] = tokens
        );

    int n = tokens.size();
    out.attr("class") = Rcpp::CharacterVector::create("tbl_df", "tbl", "data.frame");
    out.attr("row.names") = Rcpp::IntegerVector::create(NA_INTEGER, -n); // Dunno how this works (the -n part)

  return out;
}

