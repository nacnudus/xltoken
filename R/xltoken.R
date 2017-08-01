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

#' @export
xl_check_ref_grammar <- function() {
  xl_check_ref_grammar_()
}

#' @export
xl_ref <- function(x) {
  xl_ref_(x)
}
