# TODO: output a sensible data frame of tokens.
# * Tags aren't mutually exclusive because there is a hierarchy.
# * The hierarchy can be expressed by levels.
# * Even the formula =A1 wraps the cell reference wrapped in 'higher' tags.
# * Named references could be substituted by their definitions.
# * Parsing is inside-out, so it isn't possible to build up a formula gradually
#     via its terminals -- unless a terminal can check thfor success all the way
#     up before it writes?
# * Every non-terminal both adds itself to the tree, and increases the level, so
# that levels are counted from 0 at the deepest, up to whatever the outermost
# layer is.

# TODO: eliminate backtracking

# TODO: use a whitespace convention, e.g. this from
# PETGTL/src/example/pegtl/lua53_parse.cpp
# // In the following grammar most rules adopt the convention
# // that they take care of "internal padding", i.e. spaces
# // and comments that can occur within the rule, but not
# // "external padding", i.e. they don't start or end with
# // a rule that "eats up" all extra padding (spaces and
# // comments). In some places, where it is more efficient,
# // right padding is used.

# TODO: recognise defined names as case-insensitive.  Standardise on the first
# one?

# TODO: Are defined names an formulas in the file somewhere?  Because then they
# could be checked specifically.  No, because the parser would have to be
# compiled on the fly.

# TODO: Use the official grammar? via the paper for XLParser
# [14] Microsoft, “Excel (.xlsx) extensions to the office openxml spreadsheetml fileformat.” [Online]. Available: https://msdn.microsoft.com/en-us/library/dd922181(v=office.12).aspx

# TODO: Look harder at https://github.com/SheetJS -- does it compute anything?
# What about styles?

# Tokens, minimally adapted from XLParser
library(tabulizer)
library(tidyverse)
x <- extract_areas("../XLParser/doc/tokens.pdf")
x <- locate_areas("../XLParser/doc/tokens.pdf")
extract_tables
get_page_dims("../XLParser/doc/tokens.pdf")
595.28/2

col1right <- 190
col2right <- 333
col3right <- 475

x <- extract_tables("../XLParser/doc/tokens.pdf",
                    columns = list(c(col1right, col2right, col3right)),
                    guess = FALSE)[[1]]
colnames(x) <- x[1, ]
x <- x[-1, ]
x <- as_tibble(x)
print(x, n = Inf)
# FILENAME (file reference using name) is not needed because it isn't stored that way in the file
#
#          `Token Name`                                                              Description                                           Contents Priority
#                 <chr>                                                                    <chr>                                              <chr>    <chr>
#                  BOOL                                                          Boolean literal                                         TRUE|FALSE        0
#                  CELL                                                           Cell reference                                   $?[A-Z]+$?[0-9]+        2
#               DDECALL                                               Dynamic Data Exchange link                                       '([^ ']|")+'        0
#                 ERROR                                                            Error literal           #NULL!|#DIV/0!|#VALUE!|#NAME?|#NUM!|#N/A        0
#             ERROR-REF                                                  Reference error literal                                              #REF!        0
#        EXCEL-FUNCTION                                                  Excel built-in function              (Any entry from the function list*)\(        5
#                  FILE                                     External file reference using number                                         \[[0-9]+\]        5
#              FILEPATH                                                        Windows file path                      (4 is a note) [A-Z]:\\(4+\\)*        0
#      HORIZONTAL-RANGE                                                            Range of rows                                  $?[0-9]+:$?[0-9]+        0
#       MULTIPLE-SHEETS                                                Multiple sheet references   (2 and 3 are notes) ((2+:2+)|('(3|")+:(3|")+'))!        1
#                  NAME                                                        User Defined Name                (1 is a note) [A-Z_\\][A-Z0-9\\_1]*       -2
#         NAME-PREFIXED User defined name which starts with a string that could be another token (1 is a note) (TRUE|FALSE|[A-Z]+[0-9]+)[A-Z0-9_1]+        3
#                NUMBER         An integer, floating point or scientific notation number literal                           [0-9]+,?[0-9]*(e[0-9]+)?        0
#          REF-FUNCTION                                        Excel built-in reference function                         (INDEX|OFFSET|INDIRECT)\\(        5
#     REF-FUNCTION-COND                            Excel built-in conditional reference function                                      (IF|CHOOSE)\(        5  not necessarly a reference, could be a constant or an error
#         RESERVED-NAME                                                   An Excel reserved name                                     _xlnm\.[A-Z_]+       -1
#                 SHEET                                                  The name of a worksheet                                  (2 is a note) 2+!        5
#          SHEET-QUOTED                                                    Quoted worksheet name                                 (3 is a note) 3+'!        5
#                STRING                                                           String literal                                       "([^"]|"")*"        0
#             SR-COLUMN                                              Structured reference column                                   \[[A-Z0-9\\_]+\]       -3
#                   UDF                                                    User Defined Function       (1 is a note) (_xll\.)?[A-Z_][A-Z0-9_\\1]*\(        4
#        VERTICAL-RANGE                                                         Range of columns                                  $?[A-Z]+:$?[A-Z]+        0
#
#  * A function list is available as part of the reference implementation.
#
# Placeholder character                               Placeholder for                                   Specification
#                     1                           Extended characters       Non-control Unicode characters x80 and up
#                     2                              Sheet characters Any character except '*[]\:/?();{}#=<>&+-*/^%,␣
#                     3                     Enclosed sheet characters                   Any character except '*[]\:/?
#                     4                           Filename characters                Any character except "*[]\:/?<>|

# Priorities
# Here is the distribution of the first character of a formula
#  1     A  8620 address/sheet/namedformula/function/UDF/BOOL. `(` or `!` decides some
#  2     (  2345 openparen (subexpr?)
#  3     +  2205 prefix (infix?)
#  4     0  1158 number
#  5     -  1018 prefix (infix?)
#  6     '   955 sheet, possibly with a file [0] prefix
#  7     [   292 file
#  8     $   279 reg
#  9     _   136 UDF, sheet, or named formula
# 10  "\""    93 string
# 11     a    74 sheet or named formula
# 12     #    46 #REF! or #N/A
# 13     *     3 invalid

# 1. "A".  Could be ref/sheet/namedformula/function/UDF/BOOL.  Parse all alnum until
# a terminating character, then decide which it is.
#   * '('? function/UDF, lookup among known Excel functions to decide
#   * '!'? sheet, next must be address or namedformula
#   * '$'? address, next must be rest of address
#   * infix or ',' or eof? address or name, check address rule to decide

Rcpp::compileAttributes()
document()


file.remove("src/xltoken.so")
file.remove("src/xl_formula.o")

file.remove("src/xl_ref.o")
install(upgrade_dependencies = FALSE)

xltoken:::xl_ref("A:A")       # ref
xltoken:::xl_ref("A1")        # ref
xltoken:::xl_ref("A1:A2")     # ref
xltoken:::xl_ref("1:1")       # ref

xltoken:::xl_ref("A1:A")      # ref other other
xltoken:::xl_ref("A:A1")      # other other ref

xltoken:::xl_ref("A12B")      # other
xltoken:::xl_ref("ABS()")     # other other
xltoken:::xl_ref("LOG10()")   # other other
xltoken:::xl_ref("LOG10")     # ref
xltoken:::xl_ref("10LOG")     # other
xltoken:::xl_ref("10LOG()")   # other other
xltoken:::xl_ref("I3:M3")     # ref
xltoken:::xl_ref("A1:A2:A3")  # TODO: currently ref other ref, but should : be an operator?
xltoken:::xl_ref("ISLOGICAL(Team America:World Police)")  # valid, and shows that : is weird

xltoken::xl_check_ref_grammar()
xltoken:::xl_ref(c("A1", "N()")) # error
xltoken:::xl_ref(c("N()"))       # other other
xltoken:::xl_ref(c("\"N()\"+N()"))       # other other

library(tidyverse)
library(stringr)
# sources <- unique(readLines("./tests/testthat/formulas-distinct.txt"))
# saveRDS(sources, "sources.Rds", compress = FALSE)
sources <- readRDS("sources.Rds")

formulas <- data_frame(formula = sources,
                       ref = map(formula, xltoken::xl_ref))

# Do any of the parsed formulas differ from the original?  No, they're all okay.
formulas %>%
  # sample_n(100) %>%
  mutate(ref = map_chr(ref, ~ paste(.x$token, collapse = ""))) %>%
  filter(ref != formula)

# How do the ref-type tokens look
formulas %>%
  sample_n(100) %>%
  unnest(ref) %>%
  filter(type == "ref") %>%
  distinct(token) %>%
  print(n = Inf)

x <- tidyxl::tidy_xlsx("~/R/tidyxl/tests/testthat/examples.xlsx")$data[[1]]

definitions <-
  x %>%
  filter(!is.na(formula_group), !is.na(formula_ref)) %>%
  select(row, col, content, formula, formula_type, formula_ref, formula_group)

subordinates <-
  x %>%
  filter(!is.na(formula_group), is.na(formula_ref)) %>%
  select(row, col, content, formula, formula_type, formula_ref, formula_group)

  mutate(ref = map_chr


parsed <- xltoken::xl_formula(sources)
checks <- data_frame(sources, parsed) %>% arrange(sources)
checks %>%
  filter(parsed != sources) %>%
  pull(sources) %>%
  cat(file = "temp.txt", sep = "\n") %>%
  cat(sep = "\n")

checks %>%
  filter(parsed != sources) %>%
  pull(sources)

# InfixOp
xltoken::xl_formula("=(1*2)+3")
xltoken::xl_formula("=1*(2+3)")
xltoken::xl_formula("=1*2+3")
xltoken::xl_formula("=1*2^3")
xltoken::xl_formula("=1^2")
# PostfixOp
xltoken::xl_formula("1%")
xltoken::xl_formula("=1%*5")
xltoken::xl_formula("=(1%)*5")
xltoken::xl_formula("=(1%*5)")
xltoken::xl_formula("=1%*(5)")
# BoolToken
xltoken::xl_formula("TRUE")
xltoken::xl_formula("FALSE")
# SheetToken
xltoken::xl_formula("Sheet1!$A$1")
xltoken::xl_formula("'Sheet 1'!$A$1")
xltoken::xl_formula("'Sheet 1:Sheet 2'!$A$1")
xltoken::xl_formula("Sheet1:Sheet2!$A$1")
# UDFToken
xltoken::xl_formula("=FOO()") # NamedRange: FOO
xltoken::xl_formula("=_xll.BAR()") # NamedRange
# Formula
xltoken::xl_formula("=ABS()")
xltoken::xl_formula("=N()")
xltoken::xl_formula("=N(1)")
xltoken::xl_formula("=N(1,)")
xltoken::xl_formula("=N(1,2)")
xltoken::xl_formula("=AVERAGE()")
# Number
xltoken::xl_formula("1")
xltoken::xl_formula("-1")
xltoken::xl_formula("1.")
xltoken::xl_formula("-1.")
xltoken::xl_formula("1.1")
xltoken::xl_formula("-1.1")
xltoken::xl_formula("1.1E2")
xltoken::xl_formula("-1.1E2")
xltoken::xl_formula("1.1E-2")
# TextToken
xltoken::xl_formula("")          #
xltoken::xl_formula("''")        # '
xltoken::xl_formula("'A'")       # A
# xltoken::xl_formula("A")         # error
xltoken::xl_formula("''''")      # "
# xltoken::xl_formula("A''''")     # error
xltoken::xl_formula("'A'''")     # A'
# xltoken::xl_formula("''A''")     # error
xltoken::xl_formula("'''A'")     # 'A
# xltoken::xl_formula("''''A")     # error
xltoken::xl_formula("'A''B'")    # A'B
xltoken::xl_formula("'A''B''A'") # A'B'A
xltoken::xl_formula("")          #
# SingleQuotedStringToken
xltoken::xl_formula("\"\"")      # "
xltoken::xl_formula("\"A\"")     # A
# xltoken::xl_formula("A")         # error
xltoken::xl_formula("\"\"\"\"")  # "
# xltoken::xl_formula("A\"\"\"\"") # error
xltoken::xl_formula("\"A\"\"\"") # A"
# xltoken::xl_formula("\"\"A\"\"") # error
xltoken::xl_formula("\"\"\"A\"") # "A
# xltoken::xl_formula("\"\"\"\"A") # error
xltoken::xl_formula("\"A\"\"B\"")      # A"B
xltoken::xl_formula("\"A\"\"B\"\"A\"") # A"B"A
# ErrorToken
xltoken::xl_formula("#NULL!")
xltoken::xl_formula("#DIV/0!")
xltoken::xl_formula("#VALUE!")
xltoken::xl_formula("#NAME?")
xltoken::xl_formula("#NUM!")
xltoken::xl_formula("#N/A")
# ExcelRefFunctionToken
xltoken::xl_formula("=INDEX()")
xltoken::xl_formula("=OFFSET()")
xltoken::xl_formula("=INDIRECT()")

# ReservedNameToken
xltoken::xl_formula("_xlnm._a__dd")
# VRangeToken, HRangeToken
xltoken::xl_formula("1:42")
xltoken::xl_formula("A:BB")
# NameToken
xltoken::xl_formula("\\_.aA?")
xltoken::xl_formula("_.aA?")
# xltoken::xl_formula(".aA?") # error
xltoken::xl_formula("aA?")
# xltoken::xl_formula("?aA") # error
# RefErrorToken
xltoken::xl_formula("#REF!")
# FileNameNumericToken
xltoken::xl_formula("[0]")
# CellToken
xltoken::xl_formula("A1")
xltoken::xl_formula("$A1")
xltoken::xl_formula("A$1")
xltoken::xl_formula("$A$1")
xltoken::xl_formula("=$A$1")
