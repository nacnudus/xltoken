// Tokens must be matched in descending priority order
// The XLParser priorities are:

/* SheetQuotedToken                       =  1200; */
/* SheetToken                             =  1200; */
/* FileToken                              =  1200; */
/* ExcelRefFunctionToken                  =  1200; */
/* ExcelFunction                          =  1200; */
/* UDFToken                               =  1150; */
/* NamedRangeCombinationToken             =  1100; */ // unnecessary?
/* CellToken                              =  1000; */
/* MultipleSheetsToken                    =   100; */
/* Bool                                   =     0; */
// I think 0 is the default priority in Irony, so others that should be here
// are:
// * NumberToken
// * TextToken
// * ErrorToken
// * RefErrorToken
// * VRangeToken
// * HRangeToken
// * FilePathWindowsToken // This shouldn't be necessary, because all filenames are stored as indices e.g. [9]
/* SingleQuotedString                     =  -100; */
/* FileName                               =  -500; */  // unnecessary -- no filenames stored
/* ReservedNameToken                      =  -700; */
/* NameToken                              =  -800; */
/* SRColumnToken                          =  -900; */

#include "tao/pegtl.hpp"
#include <string>
#include <Rcpp.h>

/* namespace pegtl = tao::pegtl; */
using namespace tao::pegtl;

namespace xltoken
{

  // Symbols and operators

  struct space : one< ' ' > {};
  struct at : one< '@' > {};
  struct comma : one< ',' > {};
  struct colon : one< ':' > {};
  struct semicolon : one< ';' > {};
  struct dollar : one< '$' > {};
  struct OpenParen : one< '(' > {};
  struct CloseParen : one< ')' > {};
  struct OpenSquareParen : one< '[' > {};
  struct CloseSquareParen : one< ']' > {};
  struct OpenCurlyParen : one< '{' > {};
  struct CloseCurlyParen : one< '}' > {};
  struct exclamationMark : one< '!' > {};
  struct QuoteS : one< '\'' > {};
  struct NotQuoteS : not_one< '\'' > {};
  struct QuoteD : one< '"' > {};
  struct NotQuoteD : not_one< '"' > {};

  struct mulop : one< '*' > {};
  struct plusop : one< '+' > {};
  struct divop : one< '/' > {};
  struct minop : one< '-' > {};
  struct concatop : one< '&' > {};
  struct expop : one< '^' > {};

  struct intersectop : one< ' ' > {}; // Could be trouble, see XLParser Iron ImpliedSymbolTerminal

  struct percentop : one< '%' > {};
  struct gtop : one< '>' > {};
  struct eqop : one< '=' > {};
  struct ltop : one< '<' > {};
  struct neqop : string< '<', '>' > {};
  struct gteop : string< '>', '=' > {};
  struct lteop : string< '<', '=' > {};

  // ReservedNameToken this doesn't seem to be documented by Microsoft
  // Regex: _xlnm\.[a-zA-Z_]+
  struct ReservedNameToken
    : seq< string< '_', 'x', 'l', 'n', 'm', '.' >, plus< sor< alpha, one< '_' > > > >
  {};

  // VRangeToken and HRangeToken, referring to whole rows or whole columns
  // Regex:
  // * VRange = [$]?[A-Z]{1,4}:[$]?[A-Z]{1,4}
  // * HRange = [$]?[1-9][0-9]*:[$]?[1-9][0-9]*
  struct VRangeToken :
    seq< opt< dollar >, rep_min_max< 1, 4, upper >,
         one< ':' >,
         opt< dollar >, rep_min_max< 1, 4, upper > >
  {};
  struct HRangeToken :
    seq< opt< dollar >, range< '1', '9' >, star< digit >,
         one< ':' >,
         opt< dollar >, range< '1', '9' >, star< digit > >
  {};

  // BoolToken boolean literal "TRUE|FALSE"

  struct BoolToken
    : sor< string< 'T', 'R', 'U', 'E' >, string< 'F', 'A', 'L', 'S', 'E' > >
  {};

  // NamedRangeCombination for treating A1A1 as a name, rather than as two cells

  // **Syntax rules for names**

  // *Valid characters*
  // The first character of a name must be a letter, an underscore character
  // (_), or a backslash (\). Remaining characters in the name can be letters,
  // numbers, periods, and underscore characters.
  // XLParser allows question marks and backslashes among remaining characters,
  // because Excel secretly does.

  // Tip: You cannot use the uppercase and lowercase characters "C", "c", "R",
  // or "r" as a defined name, because they are all used as a shorthand for
  // selecting a row or column for the currently selected cell when you enter
  // them in a Name or Go To text box.
  // This needn't be checked here, because Excel never writes any to the file.

  // *Cell references disallowed*
  // Names cannot be the same as a cell reference,
  // such as Z$100 or R1C1.
  // R1C1-style cell references are converted to A1-style before being written
  // to the file.

  // *Spaces are not valid*
  // Spaces are not allowed as part of a name. Use the underscore character (_)
  // and period (.) as word separators, such as, Sales_Tax or First.Quarter.

  // *Name length*
  // A name can contain up to 255 characters.
  // This needn't be checked here, because Excel never writes any to the file.

  // *Case sensitivity*
  // Names can contain uppercase and lowercase letters. Excel does not
  // distinguish between uppercase and lowercase characters in names. For
  // example, if you created the name Sales and then create another name called
  // SALES in the same workbook, Excel prompts you to choose a unique name.

  // By experiment, the following are allowed:
  // * TRUEISH
  // * FALSEISH
  // * A\
  // * A1A1
  // * A1B1
  // * A11B

  // NameToken as in named formula
  // Start with a letter or underscore, continue with word character (letters,
  // numbers and underscore), dot or question mark
  // * first character: [\p{L}\\_]
  // * subsequent characters: [\w\\_\.\?]
  struct NameStartCharacter : sor< alpha, one< '_' >, one< '\\' > > {};
  struct NameValidCharacter
    : sor< NameStartCharacter,
           digit,
           one< '.' >,
           one< '?' > >
  {};
  struct NameToken : seq< NameStartCharacter, star< NameValidCharacter > >
  {};

  /* // XLParser defines a NamedRangeCombination token here, but I don't think it's */
  /* // different from NameValidWord.  XLParser also defines 'NameInvalidWord' */
  /* // instead of NameValidWord, but it is effectively NameValidWord.  This is a */
  /* // bit confusing, and might have to be rethought. */
  /* // */
  /* // NameValidWord matches words that begin with invalid strings, but continue */
  /* // with strings that make them valid.  For example, TRUE is invalid, but */
  /* // TRUEISH is valid.  NameValidWord matches TRUEISH. */
  /* struct NameValidWord : sor< seq< BoolToken, plus< NameValidCharacter > >, */
  /*                             seq< CellToken, */
  /*                                  minus< NameValidCharacter, digit >, */
  /*                                  star< NameValidCharacter > > > */
  /* {}; */

  // UDFToken user-defined function, regex: (_xll\.)?[\w\\.]+\(
  struct UDFToken : seq< opt< string< '_', 'x', 'l', 'l', '.' > >,
                         plus< sor< alnum, one< '.' > > >,
                         one< '(' > >
  {};

  // ExcelFunction any Excel function except those defined in
  // ExcelRefFunctionToken and ExcelConditionalRefFunctionToken.  It is broken
  // into two parts, ExcelFunction1 and ExcelFunction2, because there are too
  // many for a single line of C++ code, and then combined in ExcelFunction.
  struct ExcelFunction1
    : sor< string< 'A', 'B', 'S' >,
           string< 'A', 'C', 'C', 'R', 'I', 'N', 'T' >,
           string< 'A', 'C', 'C', 'R', 'I', 'N', 'T', 'M' >,
           string< 'A', 'C', 'O', 'S' >,
           string< 'A', 'C', 'O', 'S', 'H' >,
           string< 'A', 'D', 'D', 'R', 'E', 'S', 'S' >,
           string< 'A', 'M', 'O', 'R', 'D', 'E', 'G', 'R', 'C' >,
           string< 'A', 'M', 'O', 'R', 'L', 'I', 'N', 'C' >,
           string< 'A', 'N', 'D' >,
           string< 'A', 'R', 'E', 'A', 'S' >,
           string< 'A', 'S', 'C' >,
           string< 'A', 'S', 'I', 'N' >,
           string< 'A', 'S', 'I', 'N', 'H' >,
           string< 'A', 'T', 'A', 'N' >,
           string< 'A', 'T', 'A', 'N', '2' >,
           string< 'A', 'T', 'A', 'N', 'H' >,
           string< 'A', 'V', 'E', 'D', 'E', 'V' >,
           string< 'A', 'V', 'E', 'R', 'A', 'G', 'E' >,
           string< 'A', 'V', 'E', 'R', 'A', 'G', 'E', 'A' >,
           string< 'A', 'V', 'E', 'R', 'A', 'G', 'E', 'I', 'F' >,
           string< 'A', 'V', 'E', 'R', 'A', 'G', 'E', 'I', 'F', 'S' >,
           string< 'B', 'A', 'H', 'T', 'T', 'E', 'X', 'T' >,
           string< 'B', 'E', 'S', 'S', 'E', 'L', 'I' >,
           string< 'B', 'E', 'S', 'S', 'E', 'L', 'J' >,
           string< 'B', 'E', 'S', 'S', 'E', 'L', 'K' >,
           string< 'B', 'E', 'S', 'S', 'E', 'L', 'Y' >,
           string< 'B', 'E', 'T', 'A', 'D', 'I', 'S', 'T' >,
           string< 'B', 'E', 'T', 'A', 'I', 'N', 'V' >,
           string< 'B', 'I', 'N', '2', 'D', 'E', 'C' >,
           string< 'B', 'I', 'N', '2', 'H', 'E', 'X' >,
           string< 'B', 'I', 'N', '2', 'O', 'C', 'T' >,
           string< 'B', 'I', 'N', 'O', 'M', 'D', 'I', 'S', 'T' >,
           string< 'C', 'A', 'L', 'L' >,
           string< 'C', 'E', 'I', 'L', 'I', 'N', 'G' >,
           string< 'C', 'E', 'L', 'L' >,
           string< 'C', 'H', 'A', 'R' >,
           string< 'C', 'H', 'I', 'D', 'I', 'S', 'T' >,
           string< 'C', 'H', 'I', 'I', 'N', 'V' >,
           string< 'C', 'H', 'I', 'T', 'E', 'S', 'T' >,
           string< 'C', 'L', 'E', 'A', 'N' >,
           string< 'C', 'O', 'D', 'E' >,
           string< 'C', 'O', 'L', 'U', 'M', 'N' >,
           string< 'C', 'O', 'L', 'U', 'M', 'N', 'S' >,
           string< 'C', 'O', 'M', 'B', 'I', 'N' >,
           string< 'C', 'O', 'M', 'P', 'L', 'E', 'X' >,
           string< 'C', 'O', 'N', 'C', 'A', 'T', 'E', 'N', 'A', 'T', 'E' >,
           string< 'C', 'O', 'N', 'F', 'I', 'D', 'E', 'N', 'C', 'E' >,
           string< 'C', 'O', 'N', 'V', 'E', 'R', 'T' >,
           string< 'C', 'O', 'R', 'R', 'E', 'L' >,
           string< 'C', 'O', 'S' >,
           string< 'C', 'O', 'S', 'H' >,
           string< 'C', 'O', 'U', 'N', 'T' >,
           string< 'C', 'O', 'U', 'N', 'T', 'A' >,
           string< 'C', 'O', 'U', 'N', 'T', 'B', 'L', 'A', 'N', 'K' >,
           string< 'C', 'O', 'U', 'N', 'T', 'I', 'F' >,
           string< 'C', 'O', 'U', 'N', 'T', 'I', 'F', 'S' >,
           string< 'C', 'O', 'U', 'P', 'D', 'A', 'Y', 'B', 'S' >,
           string< 'C', 'O', 'U', 'P', 'D', 'A', 'Y', 'S' >,
           string< 'C', 'O', 'U', 'P', 'D', 'A', 'Y', 'S', 'N', 'C' >,
           string< 'C', 'O', 'U', 'P', 'N', 'C', 'D' >,
           string< 'C', 'O', 'U', 'P', 'N', 'U', 'M' >,
           string< 'C', 'O', 'U', 'P', 'P', 'C', 'D' >,
           string< 'C', 'O', 'V', 'A', 'R' >,
           string< 'C', 'R', 'I', 'T', 'B', 'I', 'N', 'O', 'M' >,
           string< 'C', 'U', 'B', 'E', 'K', 'P', 'I', 'M', 'E', 'M', 'B', 'E', 'R' >,
           string< 'C', 'U', 'B', 'E', 'M', 'E', 'M', 'B', 'E', 'R' >,
           string< 'C', 'U', 'B', 'E', 'M', 'E', 'M', 'B', 'E', 'R', 'P', 'R', 'O', 'P', 'E', 'R', 'T', 'Y' >,
           string< 'C', 'U', 'B', 'E', 'R', 'A', 'N', 'K', 'E', 'D', 'M', 'E', 'M', 'B', 'E', 'R' >,
           string< 'C', 'U', 'B', 'E', 'S', 'E', 'T' >,
           string< 'C', 'U', 'B', 'E', 'S', 'E', 'T', 'C', 'O', 'U', 'N', 'T' >,
           string< 'C', 'U', 'B', 'E', 'V', 'A', 'L', 'U', 'E' >,
           string< 'C', 'U', 'M', 'I', 'P', 'M', 'T' >,
           string< 'C', 'U', 'M', 'P', 'R', 'I', 'N', 'C' >,
           string< 'D', 'A', 'T', 'E' >,
           string< 'D', 'A', 'T', 'E', 'V', 'A', 'L', 'U', 'E' >,
           string< 'D', 'A', 'V', 'E', 'R', 'A', 'G', 'E' >,
           string< 'D', 'A', 'Y' >,
           string< 'D', 'A', 'Y', 'S', '3', '6', '0' >,
           string< 'D', 'B' >,
           string< 'D', 'C', 'O', 'U', 'N', 'T' >,
           string< 'D', 'C', 'O', 'U', 'N', 'T', 'A' >,
           string< 'D', 'D', 'B' >,
           string< 'D', 'E', 'C', '2', 'B', 'I', 'N' >,
           string< 'D', 'E', 'C', '2', 'H', 'E', 'X' >,
           string< 'D', 'E', 'C', '2', 'O', 'C', 'T' >,
           string< 'D', 'E', 'G', 'R', 'E', 'E', 'S' >,
           string< 'D', 'E', 'L', 'T', 'A' >,
           string< 'D', 'E', 'V', 'S', 'Q' >,
           string< 'D', 'G', 'E', 'T' >,
           string< 'D', 'I', 'S', 'C' >,
           string< 'D', 'M', 'A', 'X' >,
           string< 'D', 'M', 'I', 'N' >,
           string< 'D', 'O', 'L', 'L', 'A', 'R' >,
           string< 'D', 'O', 'L', 'L', 'A', 'R', 'D', 'E' >,
           string< 'D', 'O', 'L', 'L', 'A', 'R', 'F', 'R' >,
           string< 'D', 'P', 'R', 'O', 'D', 'U', 'C', 'T' >,
           string< 'D', 'S', 'T', 'D', 'E', 'V' >,
           string< 'D', 'S', 'T', 'D', 'E', 'V', 'P' >,
           string< 'D', 'S', 'U', 'M' >,
           string< 'D', 'U', 'R', 'A', 'T', 'I', 'O', 'N' >,
           string< 'D', 'V', 'A', 'R' >,
           string< 'D', 'V', 'A', 'R', 'P' >,
           string< 'E', 'D', 'A', 'T', 'E', 'E', 'F', 'F', 'E', 'C', 'T' >,
           string< 'E', 'O', 'M', 'O', 'N', 'T', 'H' >,
           string< 'E', 'R', 'F' >,
           string< 'E', 'R', 'F', 'C' >,
           string< 'E', 'R', 'R', 'O', 'R', '.', 'T', 'Y', 'P', 'E' >,
           string< 'E', 'U', 'R', 'O', 'C', 'O', 'N', 'V', 'E', 'R', 'T' >,
           string< 'E', 'V', 'E', 'N' >,
           string< 'E', 'X', 'A', 'C', 'T' >,
           string< 'E', 'X', 'P' >,
           string< 'E', 'X', 'P', 'O', 'N', 'D', 'I', 'S', 'T' >,
           string< 'F', 'A', 'C', 'T' >,
           string< 'F', 'A', 'C', 'T', 'D', 'O', 'U', 'B', 'L', 'E' >,
           string< 'F', 'A', 'L', 'S', 'E' >,
           string< 'F', 'D', 'I', 'S', 'T' >,
           string< 'F', 'I', 'N', 'D' >,
           string< 'F', 'I', 'N', 'V' >,
           string< 'F', 'I', 'S', 'H', 'E', 'R' >,
           string< 'F', 'I', 'S', 'H', 'E', 'R', 'I', 'N', 'V' >,
           string< 'F', 'I', 'X', 'E', 'D' >,
           string< 'F', 'L', 'O', 'O', 'R' >,
           string< 'F', 'O', 'R', 'E', 'C', 'A', 'S', 'T' >,
           string< 'F', 'R', 'E', 'Q', 'U', 'E', 'N', 'C', 'Y' >,
           string< 'F', 'T', 'E', 'S', 'T' >,
           string< 'F', 'V' >,
           string< 'F', 'V', 'S', 'C', 'H', 'E', 'D', 'U', 'L', 'E' >,
           string< 'G', 'A', 'M', 'M', 'A', 'D', 'I', 'S', 'T' >,
           string< 'G', 'A', 'M', 'M', 'A', 'I', 'N', 'V' >,
           string< 'G', 'A', 'M', 'M', 'A', 'L', 'N' >,
           string< 'G', 'C', 'D' >,
           string< 'G', 'E', 'O', 'M', 'E', 'A', 'N' >,
           string< 'G', 'E', 'S', 'T', 'E', 'P' >,
           string< 'G', 'E', 'T', 'P', 'I', 'V', 'O', 'T', 'D', 'A', 'T', 'A' >,
           string< 'G', 'R', 'O', 'W', 'T', 'H' >,
           string< 'H', 'A', 'R', 'M', 'E', 'A', 'N' >,
           string< 'H', 'E', 'X', '2', 'B', 'I', 'N' >,
           string< 'H', 'E', 'X', '2', 'D', 'E', 'C' >,
           string< 'H', 'E', 'X', '2', 'O', 'C', 'T' >,
           string< 'H', 'L', 'O', 'O', 'K', 'U', 'P' >,
           string< 'H', 'O', 'U', 'R' >,
           string< 'H', 'Y', 'P', 'E', 'R', 'L', 'I', 'N', 'K' >,
           string< 'H', 'Y', 'P', 'G', 'E', 'O', 'M', 'D', 'I', 'S', 'T' >,
           string< 'I', 'S', 'B', 'L', 'A', 'N', 'K' >,
           string< 'I', 'F', 'E', 'R', 'R', 'O', 'R' >,
           string< 'I', 'M', 'A', 'B', 'S' >,
           string< 'I', 'M', 'A', 'G', 'I', 'N', 'A', 'R', 'Y' >,
           string< 'I', 'M', 'A', 'R', 'G', 'U', 'M', 'E', 'N', 'T' >,
           string< 'I', 'M', 'C', 'O', 'N', 'J', 'U', 'G', 'A', 'T', 'E' >,
           string< 'I', 'M', 'C', 'O', 'S' >,
           string< 'I', 'M', 'D', 'I', 'V' >,
           string< 'I', 'M', 'E', 'X', 'P' >,
           string< 'I', 'M', 'L', 'N' >,
           string< 'I', 'M', 'L', 'O', 'G', '1', '0' >,
           string< 'I', 'M', 'L', 'O', 'G', '2' >,
           string< 'I', 'M', 'P', 'O', 'W', 'E', 'R' >,
           string< 'I', 'M', 'P', 'R', 'O', 'D', 'U', 'C', 'T' >,
           string< 'I', 'M', 'R', 'E', 'A', 'L' >,
           string< 'I', 'M', 'S', 'I', 'N' >,
           string< 'I', 'M', 'S', 'Q', 'R', 'T' >,
           string< 'I', 'M', 'S', 'U', 'B' >,
           string< 'I', 'M', 'S', 'U', 'M' >,
           string< 'I', 'N', 'F', 'O' >,
           string< 'I', 'N', 'T' >,
           string< 'I', 'N', 'T', 'E', 'R', 'C', 'E', 'P', 'T' >,
           string< 'I', 'N', 'T', 'R', 'A', 'T', 'E' >,
           string< 'I', 'P', 'M', 'T' >,
           string< 'I', 'R', 'R' >,
           string< 'I', 'S' >,
           string< 'I', 'S', 'B' >,
           string< 'I', 'S', 'E', 'R', 'R', 'O', 'R' > >
  {};

  struct ExcelFunction2
    : sor< string< 'I', 'S', 'N', 'A' >,
           string< 'I', 'S', 'N', 'U', 'M', 'B', 'E', 'R' >,
           string< 'I', 'S', 'P', 'M', 'T' >,
           string< 'J', 'I', 'S' >,
           string< 'K', 'U', 'R', 'T' >,
           string< 'L', 'A', 'R', 'G', 'E' >,
           string< 'L', 'C', 'M' >,
           string< 'L', 'E', 'F', 'T' >,
           string< 'L', 'E', 'F', 'T', 'B' >,
           string< 'L', 'E', 'N' >,
           string< 'L', 'E', 'N', 'B' >,
           string< 'L', 'I', 'N', 'E', 'S', 'T' >,
           string< 'L', 'N' >,
           string< 'L', 'O', 'G' >,
           string< 'L', 'O', 'G', '1', '0' >,
           string< 'L', 'O', 'G', 'E', 'S', 'T' >,
           string< 'L', 'O', 'G', 'I', 'N', 'V' >,
           string< 'L', 'O', 'G', 'N', 'O', 'R', 'M', 'D', 'I', 'S', 'T' >,
           string< 'L', 'O', 'O', 'K', 'U', 'P' >,
           string< 'L', 'O', 'W', 'E', 'R' >,
           string< 'M', 'A', 'T', 'C', 'H' >,
           string< 'M', 'A', 'X' >,
           string< 'M', 'A', 'X', 'A' >,
           string< 'M', 'D', 'E', 'T', 'E', 'R', 'M' >,
           string< 'M', 'D', 'U', 'R', 'A', 'T', 'I', 'O', 'N' >,
           string< 'M', 'E', 'D', 'I', 'A', 'N' >,
           string< 'M', 'I', 'D' >,
           string< 'M', 'I', 'D', 'B' >,
           string< 'M', 'I', 'N' >,
           string< 'M', 'I', 'N', 'A' >,
           string< 'M', 'I', 'N', 'U', 'T', 'E' >,
           string< 'M', 'I', 'N', 'V', 'E', 'R', 'S', 'E' >,
           string< 'M', 'I', 'R', 'R' >,
           string< 'M', 'M', 'U', 'L', 'T' >,
           string< 'M', 'O', 'D' >,
           string< 'M', 'O', 'D', 'E' >,
           string< 'M', 'O', 'N', 'T', 'H' >,
           string< 'M', 'R', 'O', 'U', 'N', 'D' >,
           string< 'M', 'U', 'L', 'T', 'I', 'N', 'O', 'M', 'I', 'A', 'L' >,
           string< 'N' >,
           string< 'N', 'A' >,
           string< 'N', 'E', 'G', 'B', 'I', 'N', 'O', 'M', 'D', 'I', 'S', 'T' >,
           string< 'N', 'E', 'T', 'W', 'O', 'R', 'K', 'D', 'A', 'Y', 'S' >,
           string< 'N', 'O', 'M', 'I', 'N', 'A', 'L' >,
           string< 'N', 'O', 'R', 'M', 'D', 'I', 'S', 'T' >,
           string< 'N', 'O', 'R', 'M', 'I', 'N', 'V' >,
           string< 'N', 'O', 'R', 'M', 'S', 'D', 'I', 'S', 'T' >,
           string< 'N', 'O', 'R', 'M', 'S', 'I', 'N', 'V' >,
           string< 'N', 'O', 'T' >,
           string< 'N', 'O', 'W' >,
           string< 'N', 'P', 'E', 'R' >,
           string< 'N', 'P', 'V' >,
           string< 'O', 'C', 'T', '2', 'B', 'I', 'N' >,
           string< 'O', 'C', 'T', '2', 'D', 'E', 'C' >,
           string< 'O', 'C', 'T', '2', 'H', 'E', 'X' >,
           string< 'O', 'D', 'D' >,
           string< 'O', 'D', 'D', 'F', 'P', 'R', 'I', 'C', 'E' >,
           string< 'O', 'D', 'D', 'F', 'Y', 'I', 'E', 'L', 'D' >,
           string< 'O', 'D', 'D', 'L', 'P', 'R', 'I', 'C', 'E' >,
           string< 'O', 'D', 'D', 'L', 'Y', 'I', 'E', 'L', 'D' >,
           string< 'O', 'R' >,
           string< 'P', 'E', 'A', 'R', 'S', 'O', 'N' >,
           string< 'P', 'E', 'R', 'C', 'E', 'N', 'T', 'I', 'L', 'E' >,
           string< 'P', 'E', 'R', 'C', 'E', 'N', 'T', 'R', 'A', 'N', 'K' >,
           string< 'P', 'E', 'R', 'M', 'U', 'T' >,
           string< 'P', 'H', 'O', 'N', 'E', 'T', 'I', 'C' >,
           string< 'P', 'I' >,
           string< 'P', 'M', 'T' >,
           string< 'P', 'O', 'I', 'S', 'S', 'O', 'N' >,
           string< 'P', 'O', 'W', 'E', 'R' >,
           string< 'P', 'P', 'M', 'T' >,
           string< 'P', 'R', 'I', 'C', 'E' >,
           string< 'P', 'R', 'I', 'C', 'E', 'D', 'I', 'S', 'C' >,
           string< 'P', 'R', 'I', 'C', 'E', 'M', 'A', 'T' >,
           string< 'P', 'R', 'O', 'B' >,
           string< 'P', 'R', 'O', 'D', 'U', 'C', 'T' >,
           string< 'P', 'R', 'O', 'P', 'E', 'R' >,
           string< 'P', 'V' >,
           string< 'Q', 'U', 'A', 'R', 'T', 'I', 'L', 'E' >,
           string< 'Q', 'U', 'O', 'T', 'I', 'E', 'N', 'T' >,
           string< 'R', 'A', 'D', 'I', 'A', 'N', 'S' >,
           string< 'R', 'A', 'N', 'D' >,
           string< 'R', 'A', 'N', 'D', 'B', 'E', 'T', 'W', 'E', 'E', 'N' >,
           string< 'R', 'A', 'N', 'K' >,
           string< 'R', 'A', 'T', 'E' >,
           string< 'R', 'E', 'C', 'E', 'I', 'V', 'E', 'D' >,
           string< 'R', 'E', 'G', 'I', 'S', 'T', 'E', 'R', '.', 'I', 'D' >,
           string< 'R', 'E', 'P', 'L', 'A', 'C', 'E' >,
           string< 'R', 'E', 'P', 'L', 'A', 'C', 'E', 'B' >,
           string< 'R', 'E', 'P', 'T' >,
           string< 'R', 'I', 'G', 'H', 'T' >,
           string< 'R', 'I', 'G', 'H', 'T', 'B' >,
           string< 'R', 'O', 'M', 'A', 'N' >,
           string< 'R', 'O', 'U', 'N', 'D' >,
           string< 'R', 'O', 'U', 'N', 'D', 'D', 'O', 'W', 'N' >,
           string< 'R', 'O', 'U', 'N', 'D', 'U', 'P' >,
           string< 'R', 'O', 'W' >,
           string< 'R', 'O', 'W', 'S' >,
           string< 'R', 'S', 'Q' >,
           string< 'R', 'T', 'D' >,
           string< 'S', 'E', 'A', 'R', 'C', 'H' >,
           string< 'S', 'E', 'A', 'R', 'C', 'H', 'B' >,
           string< 'S', 'E', 'C', 'O', 'N', 'D' >,
           string< 'S', 'E', 'R', 'I', 'E', 'S', 'S', 'U', 'M' >,
           string< 'S', 'I', 'G', 'N' >,
           string< 'S', 'I', 'N' >,
           string< 'S', 'I', 'N', 'H' >,
           string< 'S', 'K', 'E', 'W' >,
           string< 'S', 'L', 'N' >,
           string< 'S', 'L', 'O', 'P', 'E' >,
           string< 'S', 'M', 'A', 'L', 'L' >,
           string< 'S', 'Q', 'L', '.', 'R', 'E', 'Q', 'U', 'E', 'S', 'T' >,
           string< 'S', 'Q', 'R', 'T' >,
           string< 'S', 'Q', 'R', 'T', 'P', 'I' >,
           string< 'S', 'T', 'A', 'N', 'D', 'A', 'R', 'D', 'I', 'Z', 'E' >,
           string< 'S', 'T', 'D', 'E', 'V' >,
           string< 'S', 'T', 'D', 'E', 'V', 'A' >,
           string< 'S', 'T', 'D', 'E', 'V', 'P' >,
           string< 'S', 'T', 'D', 'E', 'V', 'P', 'A' >,
           string< 'S', 'T', 'E', 'Y', 'X' >,
           string< 'S', 'U', 'B', 'S', 'T', 'I', 'T', 'U', 'T', 'E' >,
           string< 'S', 'U', 'B', 'T', 'O', 'T', 'A', 'L' >,
           string< 'S', 'U', 'M' >,
           string< 'S', 'U', 'M', 'I', 'F' >,
           string< 'S', 'U', 'M', 'I', 'F', 'S' >,
           string< 'S', 'U', 'M', 'P', 'R', 'O', 'D', 'U', 'C', 'T' >,
           string< 'S', 'U', 'M', 'S', 'Q' >,
           string< 'S', 'U', 'M', 'X', '2', 'M', 'Y', '2' >,
           string< 'S', 'U', 'M', 'X', '2', 'P', 'Y', '2' >,
           string< 'S', 'U', 'M', 'X', 'M', 'Y', '2' >,
           string< 'S', 'Y', 'D' >,
           string< 'T' >,
           string< 'T', 'A', 'N' >,
           string< 'T', 'A', 'N', 'H' >,
           string< 'T', 'B', 'I', 'L', 'L', 'E', 'Q' >,
           string< 'T', 'B', 'I', 'L', 'L', 'P', 'R', 'I', 'C', 'E' >,
           string< 'T', 'B', 'I', 'L', 'L', 'Y', 'I', 'E', 'L', 'D' >,
           string< 'T', 'D', 'I', 'S', 'T' >,
           string< 'T', 'E', 'X', 'T' >,
           string< 'T', 'I', 'M', 'E' >,
           string< 'T', 'I', 'M', 'E', 'V', 'A', 'L', 'U', 'E' >,
           string< 'T', 'I', 'N', 'V' >,
           string< 'T', 'O', 'D', 'A', 'Y' >,
           string< 'T', 'R', 'A', 'N', 'S', 'P', 'O', 'S', 'E' >,
           string< 'T', 'R', 'E', 'N', 'D' >,
           string< 'T', 'R', 'I', 'M' >,
           string< 'T', 'R', 'I', 'M', 'M', 'E', 'A', 'N' >,
           string< 'T', 'R', 'U', 'E' >,
           string< 'T', 'R', 'U', 'N', 'C' >,
           string< 'T', 'T', 'E', 'S', 'T' >,
           string< 'T', 'Y', 'P', 'E' >,
           string< 'U', 'P', 'P', 'E', 'R' >,
           string< 'V', 'A', 'L', 'U', 'E' >,
           string< 'V', 'A', 'R' >,
           string< 'V', 'A', 'R', 'A' >,
           string< 'V', 'A', 'R', 'P' >,
           string< 'V', 'A', 'R', 'P', 'A' >,
           string< 'V', 'D', 'B' >,
           string< 'V', 'L', 'O', 'O', 'K', 'U', 'P' >,
           string< 'W', 'E', 'E', 'K', 'D', 'A', 'Y' >,
           string< 'W', 'E', 'E', 'K', 'N', 'U', 'M' >,
           string< 'W', 'E', 'I', 'B', 'U', 'L', 'L' >,
           string< 'W', 'O', 'R', 'K', 'D', 'A', 'Y' >,
           string< 'X', 'I', 'R', 'R' >,
           string< 'X', 'N', 'P', 'V' >,
           string< 'Y', 'E', 'A', 'R' >,
           string< 'Y', 'E', 'A', 'R', 'F', 'R', 'A', 'C' >,
           string< 'Y', 'I', 'E', 'L', 'D' >,
           string< 'Y', 'I', 'E', 'L', 'D', 'D', 'I', 'S', 'C' >,
           string< 'Y', 'I', 'E', 'L', 'D', 'M', 'A', 'T' >,
           string< 'Z', 'T', 'E', 'S', 'T' > >
  {};

  struct ExcelFunction
    : seq< sor< ExcelFunction1, ExcelFunction2 >, one< '(' > >
  {};

  // ExcelRefFunctionToken IF() or CHOOSE()
  struct ExcelRefFunctionToken
    : seq< sor< string< 'I', 'F' >,
                string< 'C', 'H', 'O', 'O', 'S', 'E' > >,
           one< '(' > >
  {};

  // ExcelConditionalRefFunctionToken INDEX() OFFSET() or INDIRECT()
  struct ExcelConditionalRefFunctionToken
    : seq< sor< string< 'I', 'N', 'D', 'E', 'X' >,
                string< 'O', 'F', 'F', 'S', 'E', 'T' >,
                string< 'I', 'N', 'D', 'I', 'R', 'E', 'C', 'T' > >,
           one< '(' > >
  {};

  // FileToken normalised filenames referred to by an index number
  // enclosed in square brackets
  struct FileToken : seq< one< '[' >, plus< digit >, one< ']' > > {};

  // NumberToken matches a straightforward decimal number, including exponents.
  struct plusminus : opt< one< '+', '-' > > {};
  struct dot : one< '.' > {};
  template< typename D >
    struct decimal : if_then_else< dot,
    plus< D >,
    seq< plus< D >, opt< dot, star< D > > > > {};
  struct e : one< 'e', 'E' > {};
  struct exponent : seq< plusminus, plus< digit > > {};
  struct NumberToken
    : seq< plusminus,
           decimal< digit >,
           opt< e, exponent > >
  {};

  // TextToken matches two QuoteD (") and anything between, i.e. character and
  // the surrounding pair of double-quotes.
  struct DoubleQuotedString : star< sor< seq< QuoteD, QuoteD >,
                                         NotQuoteD > >
  {};

  struct TextToken : if_must< QuoteD, DoubleQuotedString, QuoteD > {};

  // ErrorToken error literal "#NULL!|#DIV/0!|#VALUE!|#NAME?|#NUM!|#N/A"
  struct ErrorToken
    : sor< string< '#', 'N', 'U', 'L', 'L', '!' >,
           string< '#', 'D', 'I', 'V', '/', '0', '!' >,
           string< '#', 'V', 'A', 'L', 'U', 'E', '!' >,
           string< '#', 'N', 'A', 'M', 'E', '?' >,
           string< '#', 'N', 'U', 'M', '!' >,
           string< '#', 'N', 'A', '!' > >
  {};

  // RefErrorToken reference error literal "#REF!"
  struct RefErrorToken
    : string< '#', 'R', 'E', 'F', '!' >
  {};

  // SheetsQuotedToken, SheetsToken (one or a range of sheets)
  // * Certain characters are forbidden altogether
  // * Others require the whole name to be enclosed in single quotes
  // * Single quotes may be escaped by doubling
  struct normalSheetName;
  struct quotedSheetName;
  struct SheetsToken;
  struct SheetsQuotedToken;

  struct SheetsToken
    : seq< normalSheetName,
           sor< exclamationMark, // just one sheet
                seq< one< ':' >, // range of sheets
                     normalSheetName,
                     exclamationMark > > >
  {};

  struct SheetsQuotedToken
    : seq< quotedSheetName,
           sor< seq< one< '\'' >, // just one sheet
                     exclamationMark >,
                seq< one< ':' >,      // range of sheets
                     quotedSheetName,
                     one< '\'' >,
                     exclamationMark > > >
  {};

  struct normalSheetName
    : plus< not_one< '[', ']',
                     '\\', '/',
                     '(', ')',
                     '{', '}',
                     '<', '>',
                     '+', '-',
                     '\'', '*', ':', '?', '=', '^', '%',
                     ';', '#', '"', '&', ',', ' ', '!'> >
  {};

  struct quotedSheetName
    : star< sor< not_one< '[', ']', '\\', '/', '\'', '*', ':', '?' >,
                 string< '\'', '\'' > > >
  {};

  // CellToken cell reference, regex: [$]?[A-Z]{1,4}[$]?[1-9][0-9]*
  struct CellToken : seq< opt< dollar >, rep_min_max< 1, 4, alpha >,
                          opt< dollar >, digit, star< digit > >
  {};

  // SingleQuotedStringToken is the single-quoted equivalent of TextToken
  struct SingleQuotedString : star< sor< seq< QuoteS, QuoteS >,
                                         NotQuoteS > >
  {};
  struct SingleQuotedStringToken
    : if_must< QuoteS, SingleQuotedString, QuoteS >
  {};

  // SRColumnToken structured reference column, regex: [\w\\.]+
  struct SRColumnToken : plus< sor< alnum, one< '_' >, one< '.' > > > {};

  // Forward declarations
  struct Argument;
  struct Arguments;
  struct ArrayColumns;
  struct ArrayConstant;
  struct ArrayFormula;
  struct ArrayRows;
  struct Bool;
  struct Cell;
  struct Constant;
  struct ConstantArray;
  struct DynamicDataExchange;
  struct Error;
  struct File ;
  struct Formula;
  struct FormulaWithinParen;
  struct FormulaWithBits;
  struct FormulaWithEq;
  struct FormulaWithInOrPostfixOp;
  struct FormulaWithInfixOp;
  struct FormulaWithPostfixOp;
  struct FormulaWithPrefixOp;
  struct FunctionCall;
  struct FunctionName;
  struct HRange;
  struct InfixOp;
  struct NamedRange;
  struct Number;
  struct PostfixOp;
  struct Prefix;
  struct PrefixOp;
  struct References;
  struct Reference;
  struct ReferenceItem;
  struct ReferenceFunctionCall;
  struct RefError;
  struct RefFunctionName;
  struct ReservedName;
  struct StructuredReference ;
  struct StructuredReferenceElement ;
  struct StructuredReferenceExpression ;
  struct StructuredReferenceTable ;
  struct Text;
  struct UDFName;
  struct UDFunctionCall;
  struct Union;
  struct VRange;

  // Overall parsing rule

  struct root : sor< FormulaWithEq, FormulaWithBits, ArrayFormula > {};

  struct ArrayFormula
    : seq< OpenCurlyParen, eqop, FormulaWithBits, CloseCurlyParen >
  {};

  struct FormulaWithEq : seq< eqop, FormulaWithBits >
  {};

  struct FormulaWithBits :
    sor< seq< OpenParen,
              FormulaWithBits,
              CloseParen,
              opt< InfixOp, FormulaWithBits > >,
         seq< PrefixOp, FormulaWithBits >,
         seq< Formula,
              opt< sor< seq< PostfixOp,
                             opt< InfixOp, FormulaWithBits > >,
                        star< InfixOp, FormulaWithBits > > > > >
  {};

  struct Formula : sor< ConstantArray,
                        Constant,
                        ReservedName,
                        FunctionCall,
                        References >
  {};

  struct ReservedName : ReservedNameToken {};

  struct Constant : sor< Number, Text, Bool, Error > {};

  struct Text     : TextToken {};
  struct Number   : NumberToken {};
  struct Bool     : BoolToken {};
  struct Error    : ErrorToken {};
  struct RefError : RefErrorToken {};

  struct FunctionCall
    : sor< seq< FunctionName, opt< Arguments >, CloseParen > >
  {};

  struct FunctionName : ExcelFunction {};

  // To match an arbitrary-length list of arguments, I'm using the form
  // seq< opt< R >, star< if_must< S, R > > >
  // which is a modification of
  // seq< R, star< if_must< S, R > > >
  // (which is also known as list_must< R, S >) to allow zero arguments
  /* struct Arguments : list_tail< Argument, comma > {}; */
  struct Arguments : if_then_else< not_at< CloseParen >,
                                   list_must< Argument, comma >,
                                   success > {};

  struct Argument : opt< Formula > {}; // allows for an empty argument

  struct PrefixOp : sor< plusop, minop > {};

  struct InfixOp :
    sor< expop,
         mulop,
         divop,
         plusop,
         minop,
         concatop,
         gtop,
         eqop,
         ltop,
         neqop,
         gteop,
         lteop >
  {};

  struct PostfixOp : percentop {};

  struct References :
    seq< Reference,
         opt< sor< colon,
                   intersectop >,
              Reference > >
  {};

  struct Reference
    : sor< seq< OpenParen, Reference, CloseParen >,
           seq< Prefix, ReferenceItem >,
           DynamicDataExchange,
           ReferenceFunctionCall,
           ReferenceItem >
  {};

  struct ReferenceFunctionCall
    : sor< seq< OpenParen, Union, CloseParen >,
           seq< RefFunctionName, opt< Arguments >, CloseParen > >
  {};

  struct RefFunctionName : sor< ExcelRefFunctionToken,
                                ExcelConditionalRefFunctionToken >
  {};

  // A modification of list_must
  struct Union : seq< Reference, comma, Reference,
                      star_must< comma, Reference > >
  {};

  struct ReferenceItem
    : sor< Cell,
           VRange,
           HRange,
           RefError,
           UDFunctionCall,
           StructuredReference,
           NamedRange >
  {};

  struct UDFunctionCall : seq< UDFName, opt< Arguments >, CloseParen > {};
  struct UDFName : UDFToken {};

  struct VRange : VRangeToken {};
  struct HRange : HRangeToken {};

  struct Cell : CellToken {};

  struct File : FileToken {};

  struct DynamicDataExchange
    : seq< File, exclamationMark, SingleQuotedStringToken >
  {};

  struct NamedRange : NameToken {};

  struct Prefix // to a range, so either a sheet or a file or a combination
    : sor< seq< QuoteS,
                sor< seq< File, SheetsQuotedToken >,
                     SheetsQuotedToken > >,
           seq< File, sor< exclamationMark,
                           SheetsToken > >,
           SheetsToken >
  {};

  struct StructuredReferenceElement
    : sor< seq< OpenSquareParen, SRColumnToken, CloseSquareParen >,
           seq< OpenSquareParen, NameToken, CloseSquareParen > >
  {};

  struct StructuredReferenceTable : NameToken {};

  struct StructuredReferenceExpression
    : sor< StructuredReferenceElement,
           seq< StructuredReferenceElement, colon, StructuredReferenceElement >,
           seq< StructuredReferenceElement, comma, StructuredReferenceElement >,
           seq< StructuredReferenceElement, comma,
                StructuredReferenceElement, colon,
                StructuredReferenceElement >,
           seq< StructuredReferenceElement, comma,
                StructuredReferenceElement, comma,
                StructuredReferenceElement >,
           seq< StructuredReferenceElement, comma,
                StructuredReferenceElement, comma,
                StructuredReferenceElement, colon,
                StructuredReferenceElement > >
  {};

  struct StructuredReference
    : sor< StructuredReferenceElement,
           seq< OpenSquareParen, StructuredReferenceElement, CloseSquareParen >,
           seq< StructuredReferenceTable, StructuredReferenceElement >,
           seq< StructuredReferenceTable, OpenSquareParen, CloseSquareParen >,
           seq< StructuredReferenceTable, OpenSquareParen,
                StructuredReferenceExpression, CloseSquareParen > >
  {};

  struct ConstantArray
    : seq< OpenCurlyParen, ArrayColumns, CloseCurlyParen >
  {};

  struct ArrayColumns : list_must< ArrayRows, semicolon > {};
  struct ArrayRows : list_must< ArrayConstant, comma > {};

  struct ArrayConstant : sor< Constant, seq< PrefixOp, Number >, RefError > {};

  // Class template for user-defined actions that does
  // nothing by default.

  template<typename Rule>
    struct tokenize : nothing<Rule> {};

  // Specialisation of the user-defined action to do something when a rule
  // succeeds; is called with the portion of the input that matched the rule.

  template<> struct tokenize< root >
  {
    template< typename Input >
      static void apply( const Input & in, std::string & token_value )
      {
        token_value = in.string();
      }
  };

/*   template<> struct tokenize< FormulaWithEq > */
/*   { */
/*     template< typename Input > */
/*       static void apply( const Input & in, std::string & token_value ) */
/*       { */
/*         Rcpp::Rcout << "FormulaWithEq: " << in.string() << "\n"; */
/*       } */
/*   }; */

/*   template<> struct tokenize< Formula > */
/*   { */
/*     template< typename Input > */
/*       static void apply( const Input & in, std::string & token_value ) */
/*       { */
/*         Rcpp::Rcout << "Formula: " << in.string() << "\n"; */
/*       } */
/*   }; */

/*   template<> struct tokenize< ArrayFormula > */
/*   { */
/*     template< typename Input > */
/*       static void apply( const Input & in, std::string & token_value ) */
/*       { */
/*         Rcpp::Rcout << "ArrayFormula: " << in.string() << "\n"; */
/*       } */
/*   }; */

/*   template<> struct tokenize< References > */
/*   { */
/*     template< typename Input > */
/*       static void apply( const Input & in, std::string & token_value ) */
/*       { */
/*         Rcpp::Rcout << "References: " << in.string() << "\n"; */
/*       } */
/*   }; */

/*   template<> struct tokenize< Reference > */
/*   { */
/*     template< typename Input > */
/*       static void apply( const Input & in, std::string & token_value ) */
/*       { */
/*         Rcpp::Rcout << "Reference: " << in.string() << "\n"; */
/*       } */
/*   }; */

/*   template<> struct tokenize< Number > */
/*   { */
/*     template< typename Input > */
/*       static void apply( const Input & in, std::string & token_value ) */
/*       { */
/*         Rcpp::Rcout << "Number: " << in.string() << "\n"; */
/*       } */
/*   }; */

/*   template<> struct tokenize< Constant > */
/*   { */
/*     template< typename Input > */
/*       static void apply( const Input & in, std::string & token_value ) */
/*       { */
/*         Rcpp::Rcout << "Constant: " << in.string() << "\n"; */
/*       } */
/*   }; */

/*   template<> struct tokenize< FunctionCall > */
/*   { */
/*     template< typename Input > */
/*       static void apply( const Input & in, std::string & token_value ) */
/*       { */
/*         Rcpp::Rcout << "FunctionCall: " << in.string() << "\n"; */
/*       } */
/*   }; */

/*   template<> struct tokenize< ConstantArray > */
/*   { */
/*     template< typename Input > */
/*       static void apply( const Input & in, std::string & token_value ) */
/*       { */
/*         Rcpp::Rcout << "ConstantArray: " << in.string() << "\n"; */
/*       } */
/*   }; */

/*   template<> struct tokenize< ReservedName > */
/*   { */
/*     template< typename Input > */
/*       static void apply( const Input & in, std::string & token_value ) */
/*       { */
/*         Rcpp::Rcout << "ReservedName: " << in.string() << "\n"; */
/*       } */
/*   }; */

/*   template<> struct tokenize< ReferenceItem > */
/*   { */
/*     template< typename Input > */
/*       static void apply( const Input & in, std::string & token_value ) */
/*       { */
/*         Rcpp::Rcout << "ReferenceItem: " << in.string() << "\n"; */
/*       } */
/*   }; */

/*   template<> struct tokenize< ReferenceFunctionCall > */
/*   { */
/*     template< typename Input > */
/*       static void apply( const Input & in, std::string & token_value ) */
/*       { */
/*         Rcpp::Rcout << "ReferenceFunctionCall: " << in.string() << "\n"; */
/*       } */
/*   }; */

/*   template<> struct tokenize< NamedRange > */
/*   { */
/*     template< typename Input > */
/*       static void apply( const Input & in, std::string & token_value ) */
/*       { */
/*         Rcpp::Rcout << "NamedRange: " << in.string() << "\n"; */
/*       } */
/*   }; */

/*   template<> struct tokenize< VRange > */
/*   { */
/*     template< typename Input > */
/*       static void apply( const Input & in, std::string & token_value ) */
/*       { */
/*         Rcpp::Rcout << "VRange: " << in.string() << "\n"; */
/*       } */
/*   }; */

/*   template<> struct tokenize< HRange > */
/*   { */
/*     template< typename Input > */
/*       static void apply( const Input & in, std::string & token_value ) */
/*       { */
/*         Rcpp::Rcout << "HRange: " << in.string() << "\n"; */
/*       } */
/*   }; */

/*   template<> struct tokenize< RefError > */
/*   { */
/*     template< typename Input > */
/*       static void apply( const Input & in, std::string & token_value ) */
/*       { */
/*         Rcpp::Rcout << "RefError: " << in.string() << "\n"; */
/*       } */
/*   }; */

/*   template<> struct tokenize< UDFunctionCall > */
/*   { */
/*     template< typename Input > */
/*       static void apply( const Input & in, std::string & token_value ) */
/*       { */
/*         Rcpp::Rcout << "UDFunctionCall: " << in.string() << "\n"; */
/*       } */
/*   }; */

/*   template<> struct tokenize< StructuredReference > */
/*   { */
/*     template< typename Input > */
/*       static void apply( const Input & in, std::string & token_value ) */
/*       { */
/*         Rcpp::Rcout << "StructuredReference: " << in.string() << "\n"; */
/*       } */
/*   }; */

/*   template<> struct tokenize< NameToken > */
/*   { */
/*     template< typename Input > */
/*       static void apply( const Input & in, std::string & token_value ) */
/*       { */
/*         Rcpp::Rcout << "NameToken: " << in.string() << "\n"; */
/*       } */
/*   }; */

/*   template<> struct tokenize< FunctionName > */
/*   { */
/*     template< typename Input > */
/*       static void apply( const Input & in, std::string & token_value ) */
/*       { */
/*         Rcpp::Rcout << "FunctionName: " << in.string() << "\n"; */
/*       } */
/*   }; */

/*   template<> struct tokenize< Arguments > */
/*   { */
/*     template< typename Input > */
/*       static void apply( const Input & in, std::string & token_value ) */
/*       { */
/*         Rcpp::Rcout << "Arguments: " << in.string() << "\n"; */
/*       } */
/*   }; */

/*   template<> struct tokenize< CloseParen > */
/*   { */
/*     template< typename Input > */
/*       static void apply( const Input & in, std::string & token_value ) */
/*       { */
/*         Rcpp::Rcout << "CloseParen: " << in.string() << "\n"; */
/*       } */
/*   }; */

/*   template<> struct tokenize< normalSheetName > */
/*   { */
/*     template< typename Input > */
/*       static void apply( const Input & in, std::string & token_value ) */
/*       { */
/*         Rcpp::Rcout << "normalSheetName: " << in.string() << "\n"; */
/*       } */
/*   }; */

/*   template<> struct tokenize< quotedSheetName > */
/*   { */
/*     template< typename Input > */
/*       static void apply( const Input & in, std::string & token_value ) */
/*       { */
/*         Rcpp::Rcout << "quotedSheetName: " << in.string() << "\n"; */
/*       } */
/*   }; */

/*   template<> struct tokenize< Prefix > */
/*   { */
/*     template< typename Input > */
/*       static void apply( const Input & in, std::string & token_value ) */
/*       { */
/*         Rcpp::Rcout << "Prefix: " << in.string() << "\n"; */
/*       } */
/*   }; */

/*   template<> struct tokenize< PrefixOp > */
/*   { */
/*     template< typename Input > */
/*       static void apply( const Input & in, std::string & token_value ) */
/*       { */
/*         Rcpp::Rcout << "PrefixOp: " << in.string() << "\n"; */
/*       } */
/*   }; */

/*   template<> struct tokenize< PostfixOp > */
/*   { */
/*     template< typename Input > */
/*       static void apply( const Input & in, std::string & token_value ) */
/*       { */
/*         Rcpp::Rcout << "PostfixOp: " << in.string() << "\n"; */
/*       } */
/*   }; */

/*   template<> struct tokenize< InfixOp > */
/*   { */
/*     template< typename Input > */
/*       static void apply( const Input & in, std::string & token_value ) */
/*       { */
/*         Rcpp::Rcout << "InfixOp: " << in.string() << "\n"; */
/*       } */
/*   }; */

/*   template<> struct tokenize< ExcelFunction > */
/*   { */
/*     template< typename Input > */
/*       static void apply( const Input & in, std::string & token_value ) */
/*       { */
/*         Rcpp::Rcout << "ExcelFunction: " << in.string() << "\n"; */
/*       } */
/*   }; */

/*   template<> struct tokenize< ExcelFunction1 > */
/*   { */
/*     template< typename Input > */
/*       static void apply( const Input & in, std::string & token_value ) */
/*       { */
/*         Rcpp::Rcout << "ExcelFunction1: " << in.string() << "\n"; */
/*       } */
/*   }; */

/*   template<> struct tokenize< ExcelFunction2 > */
/*   { */
/*     template< typename Input > */
/*       static void apply( const Input & in, std::string & token_value ) */
/*       { */
/*         Rcpp::Rcout << "ExcelFunction2: " << in.string() << "\n"; */
/*       } */
/*   }; */

/*   template<> struct tokenize< ExcelRefFunctionToken > */
/*   { */
/*     template< typename Input > */
/*       static void apply( const Input & in, std::string & token_value ) */
/*       { */
/*         Rcpp::Rcout << "ExcelRefFunctionToken: " << in.string() << "\n"; */
/*       } */
/*   }; */

/*   template<> struct tokenize< Argument > */
/*   { */
/*     template< typename Input > */
/*       static void apply( const Input & in, std::string & token_value ) */
/*       { */
/*         Rcpp::Rcout << "Argument: " << in.string() << "\n"; */
/*       } */
/*   }; */

/*   template<> struct tokenize< comma > */
/*   { */
/*     template< typename Input > */
/*       static void apply( const Input & in, std::string & token_value ) */
/*       { */
/*         Rcpp::Rcout << "comma: " << in.string() << "\n"; */
/*       } */
/*   }; */

/*   template<> struct tokenize< FormulaWithinParen > */
/*   { */
/*     template< typename Input > */
/*       static void apply( const Input & in, std::string & token_value ) */
/*       { */
/*         Rcpp::Rcout << "FormulaWithinParen: " << in.string() << "\n"; */
/*       } */
/*   }; */

/*   template<> struct tokenize< FormulaWithBits > */
/*   { */
/*     template< typename Input > */
/*       static void apply( const Input & in, std::string & token_value ) */
/*       { */
/*         Rcpp::Rcout << "FormulaWithBits: " << in.string() << "\n"; */
/*       } */
/*   }; */

/*   template<> struct tokenize< FormulaWithInfixOp > */
/*   { */
/*     template< typename Input > */
/*       static void apply( const Input & in, std::string & token_value ) */
/*       { */
/*         Rcpp::Rcout << "FormulaWithInfixOp: " << in.string() << "\n"; */
/*       } */
/*   }; */

/*   template<> struct tokenize< FormulaWithPostfixOp > */
/*   { */
/*     template< typename Input > */
/*       static void apply( const Input & in, std::string & token_value ) */
/*       { */
/*         Rcpp::Rcout << "FormulaWithPostfixOp: " << in.string() << "\n"; */
/*       } */
/*   }; */

/*   template<> struct tokenize< FormulaWithPrefixOp > */
/*   { */
/*     template< typename input > */
/*       static void apply( const input & in, std::string & token_value ) */
/*       { */
/*         Rcpp::Rcout << "FormulaWithPrefixOp: " << in.string() << "\n"; */
/*       } */
/*   }; */

/*   template<> struct tokenize< Bool > */
/*   { */
/*     template< typename input > */
/*       static void apply( const input & in, std::string & token_value ) */
/*       { */
/*         Rcpp::Rcout << "Bool: " << in.string() << "\n"; */
/*       } */
/*   }; */

/*   template<> struct tokenize< Text > */
/*   { */
/*     template< typename input > */
/*       static void apply( const input & in, std::string & token_value ) */
/*       { */
/*         Rcpp::Rcout << "Text: " << in.string() << "\n"; */
/*       } */
/*   }; */

/*   template<> struct tokenize< Error > */
/*   { */
/*     template< typename input > */
/*       static void apply( const input & in, std::string & token_value ) */
/*       { */
/*         Rcpp::Rcout << "Error: " << in.string() << "\n"; */
/*       } */
/*   }; */

} // xltoken
