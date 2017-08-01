# xltoken

Another experimental R package for parsing (really just tokenising) Excel
formulas in R. It's mainly an attempted port of
[XLParser](https://github.com/spreadsheetlab/XLParser) from C# (Irony parser
generator) to C++/Rcpp (PEGTL parser generator).  The advantages of XLParser
are:

* It isn't handwritten -- it's a grammar for a parser-generator
* Its grammar is designed for analysis rather than calculation, so is relatively
    simple.

The disadvantage is that the grammar is designed for an LALR parser generator
rather than a PEG, which in my utterly unexpert experience means that
backtracking isn't such a problem for the original grammar because it can be
handled by priorities, whereas it becomes a big problem for me using a PEG
parser generator.

The end goal is to de-normalise shared formulas in tidyxl, where there is an
[issue](https://github.com/nacnudus/tidyxl/issues/7) that explains all this much
better.
