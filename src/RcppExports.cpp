// Generated by using Rcpp::compileAttributes() -> do not edit by hand
// Generator token: 10BE3573-1514-4C36-9D1C-5A225CD40393

#include <Rcpp.h>

using namespace Rcpp;

// xl_check_grammar_
void xl_check_grammar_();
RcppExport SEXP xltoken_xl_check_grammar_() {
BEGIN_RCPP
    Rcpp::RNGScope rcpp_rngScope_gen;
    xl_check_grammar_();
    return R_NilValue;
END_RCPP
}
// xl_formula_
Rcpp::CharacterVector xl_formula_(Rcpp::CharacterVector x);
RcppExport SEXP xltoken_xl_formula_(SEXP xSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< Rcpp::CharacterVector >::type x(xSEXP);
    rcpp_result_gen = Rcpp::wrap(xl_formula_(x));
    return rcpp_result_gen;
END_RCPP
}

static const R_CallMethodDef CallEntries[] = {
    {"xltoken_xl_check_grammar_", (DL_FUNC) &xltoken_xl_check_grammar_, 0},
    {"xltoken_xl_formula_", (DL_FUNC) &xltoken_xl_formula_, 1},
    {NULL, NULL, 0}
};

RcppExport void R_init_xltoken(DllInfo *dll) {
    R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
    R_useDynamicSymbols(dll, FALSE);
}