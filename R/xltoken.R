#' @useDynLib xltoken
#' @importFrom Rcpp sourceCpp
NULL

#' @export
xl_check_grammar <- function() {
  xl_check_grammar_()
}

#' @export
xl_formula <- function(x) {
  xl_formula_(x)
}
