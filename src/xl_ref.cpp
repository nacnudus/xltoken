#include <Rcpp.h>
#include "tao/pegtl/contrib/tracer.hpp"
#include "tao/pegtl/analyze.hpp"
#include "xlref.hpp"

// [[Rcpp::export]]
void xl_check_ref_grammar_()
{
  const size_t issues_found = tao::pegtl::analyze< xlref::root >();
}

// [[Rcpp::export]]
Rcpp::List xl_ref_(Rcpp::CharacterVector x)
{

  std::string in_string;             // the formula from the iterator
  Rcpp::List out;               // wraps the vectors below
  std::vector<std::string> token_type;
  std::vector<std::string> token;

    in_string = Rcpp::as< std::string >(x);

    memory_input<> in_mem( in_string, "original-formula" );
    parse< xlref::root, xlref::tokenize >( in_mem, token_type, token);
    /* parse< xlref::root, xlref::tokenize, tracer >( in_mem, token_type, token); */

    out = Rcpp::List::create(
        Rcpp::_["token_type"] = token_type,
        Rcpp::_["token"] = token
        );

    int n = token_type.size();
    out.attr("class") = Rcpp::CharacterVector::create("tbl_df", "tbl", "data.frame");
    out.attr("row.names") = Rcpp::IntegerVector::create(NA_INTEGER, -n); // Dunno how this works (the -n part)

  return out;
}
