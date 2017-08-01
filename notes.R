Rcpp::compileAttributes()
document()

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

# Priorities
# Here is the distribution of the first character of a formula
#  1     A  8620 address/sheet/namedformula/function/UDF. `(` or `!` decides some
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

# 1. "A".  Could be ref/sheet/namedformula/function/UDF.  Parse all alnum until
# a terminating character, then decide which it is.
#   * '('? function/UDF, lookup among known Excel functions to decide
#   * '!'? sheet, next must be address or namedformula
#   * '$'? address, next must be rest of address
#   * infix or ',' or eof? address or name, check address rule to decide

clean_dll()
install(upgrade_dependencies = FALSE)
xltoken::xl_check_grammar()
xltoken:::xl_formula("(1.3+3)%")
xltoken:::xl_formula("A1")
xltoken:::xl_formula("A1:B2")
xltoken:::xl_formula("A1 B2")
xltoken:::xl_formula("nacnudus")
xltoken:::xl_formula("(A1)*(B2)")

file.remove("src/xltoken.so")
file.remove("src/xl_formula.o")
install(upgrade_dependencies = FALSE)
xltoken::xl_formula("XNPV(Assumptions!D56,'Consultant and Enron Valuation'!B18:K18,'Consultant and Enron Valuation'!B3:K3)")

xltoken::xl_formula("DAYS360(R15,$E$3,1)")
xltoken::xl_formula("6:6")
xltoken::xl_formula("IF(E2=21:21,E$22:E$23,\" \")")
xltoken:::xl_formula("DeptSales[@Commission Amount]") # TODO: didn't appear in test formulas
xltoken:::xl_formula("SUM(Table1[col1],Table1[@col2],4)")
xltoken:::xl_formula("SUM(Table1[col1],Table1[[#Headers],[col2]],4)")
xltoken:::xl_formula("IF(R13C3>DATE(2002,1,6),0,IF(ISERROR(R41C[2]),0,IF(R13C3>=R[2]C2:R[6]C[3],0, IF(AND(R[23]C11>=55,R[24]C[11]>=20),R53C3,0))))")

xltoken:::xl_formula("\"Hello, \"\"World\"\"\"&\"I can see you\"")
xltoken:::xl_formula("'C:\\users\\My Documents\\[border.xlsx]Sheet1'!$A$1")
xltoken:::xl_formula("[1]'Sheet1'!$A$1")
xltoken:::xl_formula("'[1]Sheet1'!$A$1")


xltoken::xl_formula("(((1000+500+500+500)*4.33*15)*CMF)/SM134Units")
xltoken::xl_formula("SUM(A1:B1:C1)")
xltoken::xl_formula("DType = \"pre\"")
xltoken::xl_formula("DType= \"pre\"")
xltoken::xl_formula("DType =\"pre\"")
xltoken::xl_formula("DType=\"pre\"")
xltoken::xl_formula("DATE(YEAR(UnderStart), MAX(MONTH(UnderStart), EmbeddedFirstMonth) + (Q8 - 1) * 12 /EmbeddedFrequency, 1)")
xltoken::xl_formula("-E144 * E145")
xltoken::xl_formula("-E144  New IP deals might require some modifications")
xltoken::xl_formula("1/100*[2]!BLP(J22,REF!B2,,,[1]!'GJTB3MO Index,[PX_LAST]') ")
xltoken::xl_formula("(YEAR(I14)-YEAR(C4))*12+MONTH(I14)-MONTH(C4)+IF(MONTH(I14)-MONTH(C4)>=0, IF((DAY(I14)-DAY(C4))>25, 1,( IF(DAY(I14)-DAY(C4)<-5, -1,0))), IF((DAY(I14)-DAY(C4))>25, 1, IF((DAY(I14)-DAY(C4))<-25, -1,0)))")
xltoken::xl_formula("SUM( X67:X68)")
xltoken::xl_formula("IF(TRUEMODEL=\"TRUE MODEL\",0.5,RAND())")
xltoken::xl_formula("IF(SUM(L49:L54)=0,0,$N$41-(SUMIF($A$10:$A$40,IF($L$49=\" \",$A$49,\" \"),$N$10:$N$40))-(SUMIF($A$10:$A$40,IF($L$50=\" \",$A$50,\" \"),$N$10:$N$40))-(SUMIF($A$10:$A$40,IF( $L$51=\" \",$A$51,\" \"),$N$10:$N$40))-(SUMIF($A$10:$A$40,IF($L$52=\" \",$A$52,\" \"),$N$10:$N$40))-(SUMIF($A$10:$A$40,IF($L$53=\" \",$A$53,\" \"),$N$10:$N$40))-(SUMIF($A$10:$A$40,IF($L$54=\" \",$A$54,\" \"),$N$10:$N$40)))")
xltoken::xl_formula("IF( 0,TRUE,FALSE)")
xltoken::xl_formula("IF(Scope!G24=\"yes\",\"not used\",\"Assumes that gas compression is not required.\")")

file.remove("src/xltoken.so")
file.remove("src/xl_formula.o")
install(upgrade_dependencies = FALSE)
library(tidyverse)
library(stringr)
# sources <- unique(readLines("./tests/testthat/formulas-distinct.txt"))
# saveRDS(sources, "sources.Rds", compress = FALSE)
sources <- readRDS("sources.Rds")
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
