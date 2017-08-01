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

The function `xl_formula()` should return the formula you give it.  Behind the
scenes, it tokenizes it, and then pastes all the tokens back together into one
string, so if it doesn't return the original, then something has gone wrong.
The next is to return the parse tree, probably in a data frame, and check that
it makes sense.

### Simple tokeniser

There's another function, `xl_ref()`, which calls a much simpler parser that
only attempts to separate cell references from the rest of a formula, and
returns a data frame of tokens.  In doing so, it creates three types of tokens:
`ref`, `text` and `other`.  The `text` token is necessary because unless text is
identified, its contents might be interpreted as a cell reference.

The next steps are to construct a 'ref' object in C++ that can be kept as the
master copy of a shared formula, and then offset by x rows and y columns to
reconstruct the denormalised formulas.
