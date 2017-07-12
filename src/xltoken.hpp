#include "tao/pegtl.hpp"
#include <string>
#include <Rcpp.h>

using namespace tao::pegtl;

namespace xltoken
{

  // Symbols and operators

  struct space : one< ' ' > {};
  struct spaces : star< space > {};
  struct attoken : one< '@' > {}; // don't clash with tao::pegtl::at<
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

  // Could be trouble, see XLParser Iron ImpliedSymbolTerminal
  // Should be space, not spaces, but some spurious formulas have double-spaces
  // and it's probably better to parse them than to fail.
  // E.g. "-E144  New IP deals might require some modifications"
  struct intersectop : spaces {};

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
                         plus< sor< alnum, one< '_' >, one< '.' > > >,
                         OpenParen >
  {};

  // Excel functions declared in reverse-alphabetical order to make it search
  // for longer strings before matching shorter, similar ones.
  struct ExcelFunction1
    : sor< string< 'Z', 'T', 'E', 'S', 'T' >,
           string< 'Y', 'I', 'E', 'L', 'D', 'M', 'A', 'T' >,
           string< 'Y', 'I', 'E', 'L', 'D', 'D', 'I', 'S', 'C' >,
           string< 'Y', 'I', 'E', 'L', 'D' >,
           string< 'Y', 'E', 'A', 'R', 'F', 'R', 'A', 'C' >,
           string< 'Y', 'E', 'A', 'R' >,
           string< 'X', 'N', 'P', 'V' >,
           string< 'X', 'I', 'R', 'R' >,
           string< 'W', 'O', 'R', 'K', 'D', 'A', 'Y' >,
           string< 'W', 'E', 'I', 'B', 'U', 'L', 'L' >,
           string< 'W', 'E', 'E', 'K', 'N', 'U', 'M' >,
           string< 'W', 'E', 'E', 'K', 'D', 'A', 'Y' >,
           string< 'V', 'L', 'O', 'O', 'K', 'U', 'P' >,
           string< 'V', 'D', 'B' >,
           string< 'V', 'A', 'R', 'P', 'A' >,
           string< 'V', 'A', 'R', 'P' >,
           string< 'V', 'A', 'R', 'A' >,
           string< 'V', 'A', 'R' >,
           string< 'V', 'A', 'L', 'U', 'E' >,
           string< 'U', 'P', 'P', 'E', 'R' >,
           string< 'T', 'Y', 'P', 'E' >,
           string< 'T', 'T', 'E', 'S', 'T' >,
           string< 'T', 'R', 'U', 'N', 'C' >,
           string< 'T', 'R', 'U', 'E' >,
           string< 'T', 'R', 'I', 'M', 'M', 'E', 'A', 'N' >,
           string< 'T', 'R', 'I', 'M' >,
           string< 'T', 'R', 'E', 'N', 'D' >,
           string< 'T', 'R', 'A', 'N', 'S', 'P', 'O', 'S', 'E' >,
           string< 'T', 'O', 'D', 'A', 'Y' >,
           string< 'T', 'I', 'N', 'V' >,
           string< 'T', 'I', 'M', 'E', 'V', 'A', 'L', 'U', 'E' >,
           string< 'T', 'I', 'M', 'E' >,
           string< 'T', 'E', 'X', 'T' >,
           string< 'T', 'D', 'I', 'S', 'T' >,
           string< 'T', 'B', 'I', 'L', 'L', 'Y', 'I', 'E', 'L', 'D' >,
           string< 'T', 'B', 'I', 'L', 'L', 'P', 'R', 'I', 'C', 'E' >,
           string< 'T', 'B', 'I', 'L', 'L', 'E', 'Q' >,
           string< 'T', 'A', 'N', 'H' >,
           string< 'T', 'A', 'N' >,
           string< 'T' >,
           string< 'S', 'Y', 'D' >,
           string< 'S', 'U', 'M', 'X', 'M', 'Y', '2' >,
           string< 'S', 'U', 'M', 'X', '2', 'P', 'Y', '2' >,
           string< 'S', 'U', 'M', 'X', '2', 'M', 'Y', '2' >,
           string< 'S', 'U', 'M', 'S', 'Q' >,
           string< 'S', 'U', 'M', 'P', 'R', 'O', 'D', 'U', 'C', 'T' >,
           string< 'S', 'U', 'M', 'I', 'F', 'S' >,
           string< 'S', 'U', 'M', 'I', 'F' >,
           string< 'S', 'U', 'M' >,
           string< 'S', 'U', 'B', 'T', 'O', 'T', 'A', 'L' >,
           string< 'S', 'U', 'B', 'S', 'T', 'I', 'T', 'U', 'T', 'E' >,
           string< 'S', 'T', 'E', 'Y', 'X' >,
           string< 'S', 'T', 'D', 'E', 'V', 'P', 'A' >,
           string< 'S', 'T', 'D', 'E', 'V', 'P' >,
           string< 'S', 'T', 'D', 'E', 'V', 'A' >,
           string< 'S', 'T', 'D', 'E', 'V' >,
           string< 'S', 'T', 'A', 'N', 'D', 'A', 'R', 'D', 'I', 'Z', 'E' >,
           string< 'S', 'Q', 'R', 'T', 'P', 'I' >,
           string< 'S', 'Q', 'R', 'T' >,
           string< 'S', 'Q', 'L', '.', 'R', 'E', 'Q', 'U', 'E', 'S', 'T' >,
           string< 'S', 'M', 'A', 'L', 'L' >,
           string< 'S', 'L', 'O', 'P', 'E' >,
           string< 'S', 'L', 'N' >,
           string< 'S', 'K', 'E', 'W' >,
           string< 'S', 'I', 'N', 'H' >,
           string< 'S', 'I', 'N' >,
           string< 'S', 'I', 'G', 'N' >,
           string< 'S', 'E', 'R', 'I', 'E', 'S', 'S', 'U', 'M' >,
           string< 'S', 'E', 'C', 'O', 'N', 'D' >,
           string< 'S', 'E', 'A', 'R', 'C', 'H', 'B' >,
           string< 'S', 'E', 'A', 'R', 'C', 'H' >,
           string< 'R', 'T', 'D' >,
           string< 'R', 'S', 'Q' >,
           string< 'R', 'O', 'W', 'S' >,
           string< 'R', 'O', 'W' >,
           string< 'R', 'O', 'U', 'N', 'D', 'U', 'P' >,
           string< 'R', 'O', 'U', 'N', 'D', 'D', 'O', 'W', 'N' >,
           string< 'R', 'O', 'U', 'N', 'D' >,
           string< 'R', 'O', 'M', 'A', 'N' >,
           string< 'R', 'I', 'G', 'H', 'T', 'B' >,
           string< 'R', 'I', 'G', 'H', 'T' >,
           string< 'R', 'E', 'P', 'T' >,
           string< 'R', 'E', 'P', 'L', 'A', 'C', 'E', 'B' >,
           string< 'R', 'E', 'P', 'L', 'A', 'C', 'E' >,
           string< 'R', 'E', 'G', 'I', 'S', 'T', 'E', 'R', '.', 'I', 'D' >,
           string< 'R', 'E', 'C', 'E', 'I', 'V', 'E', 'D' >,
           string< 'R', 'A', 'T', 'E' >,
           string< 'R', 'A', 'N', 'K' >,
           string< 'R', 'A', 'N', 'D', 'B', 'E', 'T', 'W', 'E', 'E', 'N' >,
           string< 'R', 'A', 'N', 'D' >,
           string< 'R', 'A', 'D', 'I', 'A', 'N', 'S' >,
           string< 'Q', 'U', 'O', 'T', 'I', 'E', 'N', 'T' >,
           string< 'Q', 'U', 'A', 'R', 'T', 'I', 'L', 'E' >,
           string< 'P', 'V' >,
           string< 'P', 'R', 'O', 'P', 'E', 'R' >,
           string< 'P', 'R', 'O', 'D', 'U', 'C', 'T' >,
           string< 'P', 'R', 'O', 'B' >,
           string< 'P', 'R', 'I', 'C', 'E', 'M', 'A', 'T' >,
           string< 'P', 'R', 'I', 'C', 'E', 'D', 'I', 'S', 'C' >,
           string< 'P', 'R', 'I', 'C', 'E' >,
           string< 'P', 'P', 'M', 'T' >,
           string< 'P', 'O', 'W', 'E', 'R' >,
           string< 'P', 'O', 'I', 'S', 'S', 'O', 'N' >,
           string< 'P', 'M', 'T' >,
           string< 'P', 'I' >,
           string< 'P', 'H', 'O', 'N', 'E', 'T', 'I', 'C' >,
           string< 'P', 'E', 'R', 'M', 'U', 'T' >,
           string< 'P', 'E', 'R', 'C', 'E', 'N', 'T', 'R', 'A', 'N', 'K' >,
           string< 'P', 'E', 'R', 'C', 'E', 'N', 'T', 'I', 'L', 'E' >,
           string< 'P', 'E', 'A', 'R', 'S', 'O', 'N' >,
           string< 'O', 'R' >,
           string< 'O', 'D', 'D', 'L', 'Y', 'I', 'E', 'L', 'D' >,
           string< 'O', 'D', 'D', 'L', 'P', 'R', 'I', 'C', 'E' >,
           string< 'O', 'D', 'D', 'F', 'Y', 'I', 'E', 'L', 'D' >,
           string< 'O', 'D', 'D', 'F', 'P', 'R', 'I', 'C', 'E' >,
           string< 'O', 'D', 'D' >,
           string< 'O', 'C', 'T', '2', 'H', 'E', 'X' >,
           string< 'O', 'C', 'T', '2', 'D', 'E', 'C' >,
           string< 'O', 'C', 'T', '2', 'B', 'I', 'N' >,
           string< 'N', 'P', 'V' >,
           string< 'N', 'P', 'E', 'R' >,
           string< 'N', 'O', 'W' >,
           string< 'N', 'O', 'T' >,
           string< 'N', 'O', 'R', 'M', 'S', 'I', 'N', 'V' >,
           string< 'N', 'O', 'R', 'M', 'S', 'D', 'I', 'S', 'T' >,
           string< 'N', 'O', 'R', 'M', 'I', 'N', 'V' >,
           string< 'N', 'O', 'R', 'M', 'D', 'I', 'S', 'T' >,
           string< 'N', 'O', 'M', 'I', 'N', 'A', 'L' >,
           string< 'N', 'E', 'T', 'W', 'O', 'R', 'K', 'D', 'A', 'Y', 'S' >,
           string< 'N', 'E', 'G', 'B', 'I', 'N', 'O', 'M', 'D', 'I', 'S', 'T' >,
           string< 'N', 'A' >,
           string< 'N' >,
           string< 'M', 'U', 'L', 'T', 'I', 'N', 'O', 'M', 'I', 'A', 'L' >,
           string< 'M', 'R', 'O', 'U', 'N', 'D' >,
           string< 'M', 'O', 'N', 'T', 'H' >,
           string< 'M', 'O', 'D', 'E' >,
           string< 'M', 'O', 'D' >,
           string< 'M', 'M', 'U', 'L', 'T' >,
           string< 'M', 'I', 'R', 'R' >,
           string< 'M', 'I', 'N', 'V', 'E', 'R', 'S', 'E' >,
           string< 'M', 'I', 'N', 'U', 'T', 'E' >,
           string< 'M', 'I', 'N', 'A' >,
           string< 'M', 'I', 'N' >,
           string< 'M', 'I', 'D', 'B' >,
           string< 'M', 'I', 'D' >,
           string< 'M', 'E', 'D', 'I', 'A', 'N' >,
           string< 'M', 'D', 'U', 'R', 'A', 'T', 'I', 'O', 'N' >,
           string< 'M', 'D', 'E', 'T', 'E', 'R', 'M' >,
           string< 'M', 'A', 'X', 'A' >,
           string< 'M', 'A', 'X' >,
           string< 'M', 'A', 'T', 'C', 'H' >,
           string< 'L', 'O', 'W', 'E', 'R' >,
           string< 'L', 'O', 'O', 'K', 'U', 'P' >,
           string< 'L', 'O', 'G', 'N', 'O', 'R', 'M', 'D', 'I', 'S', 'T' >,
           string< 'L', 'O', 'G', 'I', 'N', 'V' >,
           string< 'L', 'O', 'G', 'E', 'S', 'T' >,
           string< 'L', 'O', 'G', '1', '0' >,
           string< 'L', 'O', 'G' >,
           string< 'L', 'N' >,
           string< 'L', 'I', 'N', 'E', 'S', 'T' >,
           string< 'L', 'E', 'N', 'B' >,
           string< 'L', 'E', 'N' >,
           string< 'L', 'E', 'F', 'T', 'B' >,
           string< 'L', 'E', 'F', 'T' >,
           string< 'L', 'C', 'M' >,
           string< 'L', 'A', 'R', 'G', 'E' >,
           string< 'K', 'U', 'R', 'T' >,
           string< 'J', 'I', 'S' >,
           string< 'I', 'S', 'P', 'M', 'T' >,
           string< 'I', 'S', 'N', 'U', 'M', 'B', 'E', 'R' >,
           string< 'I', 'S', 'N', 'A' > >
  {};

  struct ExcelFunction2
    : sor< string< 'I', 'S', 'E', 'R', 'R', 'O', 'R' >,
           string< 'I', 'S', 'B', 'L', 'A', 'N', 'K' >,
           string< 'I', 'S', 'B' >,
           string< 'I', 'S' >,
           string< 'I', 'R', 'R' >,
           string< 'I', 'P', 'M', 'T' >,
           string< 'I', 'N', 'T', 'R', 'A', 'T', 'E' >,
           string< 'I', 'N', 'T', 'E', 'R', 'C', 'E', 'P', 'T' >,
           string< 'I', 'N', 'T' >,
           string< 'I', 'N', 'F', 'O' >,
           string< 'I', 'M', 'S', 'U', 'M' >,
           string< 'I', 'M', 'S', 'U', 'B' >,
           string< 'I', 'M', 'S', 'Q', 'R', 'T' >,
           string< 'I', 'M', 'S', 'I', 'N' >,
           string< 'I', 'M', 'R', 'E', 'A', 'L' >,
           string< 'I', 'M', 'P', 'R', 'O', 'D', 'U', 'C', 'T' >,
           string< 'I', 'M', 'P', 'O', 'W', 'E', 'R' >,
           string< 'I', 'M', 'L', 'O', 'G', '2' >,
           string< 'I', 'M', 'L', 'O', 'G', '1', '0' >,
           string< 'I', 'M', 'L', 'N' >,
           string< 'I', 'M', 'E', 'X', 'P' >,
           string< 'I', 'M', 'D', 'I', 'V' >,
           string< 'I', 'M', 'C', 'O', 'S' >,
           string< 'I', 'M', 'C', 'O', 'N', 'J', 'U', 'G', 'A', 'T', 'E' >,
           string< 'I', 'M', 'A', 'R', 'G', 'U', 'M', 'E', 'N', 'T' >,
           string< 'I', 'M', 'A', 'G', 'I', 'N', 'A', 'R', 'Y' >,
           string< 'I', 'M', 'A', 'B', 'S' >,
           string< 'I', 'F', 'E', 'R', 'R', 'O', 'R' >,
           string< 'H', 'Y', 'P', 'G', 'E', 'O', 'M', 'D', 'I', 'S', 'T' >,
           string< 'H', 'Y', 'P', 'E', 'R', 'L', 'I', 'N', 'K' >,
           string< 'H', 'O', 'U', 'R' >,
           string< 'H', 'L', 'O', 'O', 'K', 'U', 'P' >,
           string< 'H', 'E', 'X', '2', 'O', 'C', 'T' >,
           string< 'H', 'E', 'X', '2', 'D', 'E', 'C' >,
           string< 'H', 'E', 'X', '2', 'B', 'I', 'N' >,
           string< 'H', 'A', 'R', 'M', 'E', 'A', 'N' >,
           string< 'G', 'R', 'O', 'W', 'T', 'H' >,
           string< 'G', 'E', 'T', 'P', 'I', 'V', 'O', 'T', 'D', 'A', 'T', 'A' >,
           string< 'G', 'E', 'S', 'T', 'E', 'P' >,
           string< 'G', 'E', 'O', 'M', 'E', 'A', 'N' >,
           string< 'G', 'C', 'D' >,
           string< 'G', 'A', 'M', 'M', 'A', 'L', 'N' >,
           string< 'G', 'A', 'M', 'M', 'A', 'I', 'N', 'V' >,
           string< 'G', 'A', 'M', 'M', 'A', 'D', 'I', 'S', 'T' >,
           string< 'F', 'V', 'S', 'C', 'H', 'E', 'D', 'U', 'L', 'E' >,
           string< 'F', 'V' >,
           string< 'F', 'T', 'E', 'S', 'T' >,
           string< 'F', 'R', 'E', 'Q', 'U', 'E', 'N', 'C', 'Y' >,
           string< 'F', 'O', 'R', 'E', 'C', 'A', 'S', 'T' >,
           string< 'F', 'L', 'O', 'O', 'R' >,
           string< 'F', 'I', 'X', 'E', 'D' >,
           string< 'F', 'I', 'S', 'H', 'E', 'R', 'I', 'N', 'V' >,
           string< 'F', 'I', 'S', 'H', 'E', 'R' >,
           string< 'F', 'I', 'N', 'V' >,
           string< 'F', 'I', 'N', 'D' >,
           string< 'F', 'D', 'I', 'S', 'T' >,
           string< 'F', 'A', 'L', 'S', 'E' >,
           string< 'F', 'A', 'C', 'T', 'D', 'O', 'U', 'B', 'L', 'E' >,
           string< 'F', 'A', 'C', 'T' >,
           string< 'E', 'X', 'P', 'O', 'N', 'D', 'I', 'S', 'T' >,
           string< 'E', 'X', 'P' >,
           string< 'E', 'X', 'A', 'C', 'T' >,
           string< 'E', 'V', 'E', 'N' >,
           string< 'E', 'U', 'R', 'O', 'C', 'O', 'N', 'V', 'E', 'R', 'T' >,
           string< 'E', 'R', 'R', 'O', 'R', '.', 'T', 'Y', 'P', 'E' >,
           string< 'E', 'R', 'F', 'C' >,
           string< 'E', 'R', 'F' >,
           string< 'E', 'O', 'M', 'O', 'N', 'T', 'H' >,
           string< 'E', 'D', 'A', 'T', 'E', 'E', 'F', 'F', 'E', 'C', 'T' >,
           string< 'D', 'V', 'A', 'R', 'P' >,
           string< 'D', 'V', 'A', 'R' >,
           string< 'D', 'U', 'R', 'A', 'T', 'I', 'O', 'N' >,
           string< 'D', 'S', 'U', 'M' >,
           string< 'D', 'S', 'T', 'D', 'E', 'V', 'P' >,
           string< 'D', 'S', 'T', 'D', 'E', 'V' >,
           string< 'D', 'P', 'R', 'O', 'D', 'U', 'C', 'T' >,
           string< 'D', 'O', 'L', 'L', 'A', 'R', 'F', 'R' >,
           string< 'D', 'O', 'L', 'L', 'A', 'R', 'D', 'E' >,
           string< 'D', 'O', 'L', 'L', 'A', 'R' >,
           string< 'D', 'M', 'I', 'N' >,
           string< 'D', 'M', 'A', 'X' >,
           string< 'D', 'I', 'S', 'C' >,
           string< 'D', 'G', 'E', 'T' >,
           string< 'D', 'E', 'V', 'S', 'Q' >,
           string< 'D', 'E', 'L', 'T', 'A' >,
           string< 'D', 'E', 'G', 'R', 'E', 'E', 'S' >,
           string< 'D', 'E', 'C', '2', 'O', 'C', 'T' >,
           string< 'D', 'E', 'C', '2', 'H', 'E', 'X' >,
           string< 'D', 'E', 'C', '2', 'B', 'I', 'N' >,
           string< 'D', 'D', 'B' >,
           string< 'D', 'C', 'O', 'U', 'N', 'T', 'A' >,
           string< 'D', 'C', 'O', 'U', 'N', 'T' >,
           string< 'D', 'B' >,
           string< 'D', 'A', 'Y', 'S', '3', '6', '0' >,
           string< 'D', 'A', 'Y' >,
           string< 'D', 'A', 'V', 'E', 'R', 'A', 'G', 'E' >,
           string< 'D', 'A', 'T', 'E', 'V', 'A', 'L', 'U', 'E' >,
           string< 'D', 'A', 'T', 'E' >,
           string< 'C', 'U', 'M', 'P', 'R', 'I', 'N', 'C' >,
           string< 'C', 'U', 'M', 'I', 'P', 'M', 'T' >,
           string< 'C', 'U', 'B', 'E', 'V', 'A', 'L', 'U', 'E' >,
           string< 'C', 'U', 'B', 'E', 'S', 'E', 'T', 'C', 'O', 'U', 'N', 'T' >,
           string< 'C', 'U', 'B', 'E', 'S', 'E', 'T' >,
           string< 'C', 'U', 'B', 'E', 'R', 'A', 'N', 'K', 'E', 'D', 'M', 'E', 'M', 'B', 'E', 'R' >,
           string< 'C', 'U', 'B', 'E', 'M', 'E', 'M', 'B', 'E', 'R', 'P', 'R', 'O', 'P', 'E', 'R', 'T', 'Y' >,
           string< 'C', 'U', 'B', 'E', 'M', 'E', 'M', 'B', 'E', 'R' >,
           string< 'C', 'U', 'B', 'E', 'K', 'P', 'I', 'M', 'E', 'M', 'B', 'E', 'R' >,
           string< 'C', 'R', 'I', 'T', 'B', 'I', 'N', 'O', 'M' >,
           string< 'C', 'O', 'V', 'A', 'R' >,
           string< 'C', 'O', 'U', 'P', 'P', 'C', 'D' >,
           string< 'C', 'O', 'U', 'P', 'N', 'U', 'M' >,
           string< 'C', 'O', 'U', 'P', 'N', 'C', 'D' >,
           string< 'C', 'O', 'U', 'P', 'D', 'A', 'Y', 'S', 'N', 'C' >,
           string< 'C', 'O', 'U', 'P', 'D', 'A', 'Y', 'S' >,
           string< 'C', 'O', 'U', 'P', 'D', 'A', 'Y', 'B', 'S' >,
           string< 'C', 'O', 'U', 'N', 'T', 'I', 'F', 'S' >,
           string< 'C', 'O', 'U', 'N', 'T', 'I', 'F' >,
           string< 'C', 'O', 'U', 'N', 'T', 'B', 'L', 'A', 'N', 'K' >,
           string< 'C', 'O', 'U', 'N', 'T', 'A' >,
           string< 'C', 'O', 'U', 'N', 'T' >,
           string< 'C', 'O', 'S', 'H' >,
           string< 'C', 'O', 'S' >,
           string< 'C', 'O', 'R', 'R', 'E', 'L' >,
           string< 'C', 'O', 'N', 'V', 'E', 'R', 'T' >,
           string< 'C', 'O', 'N', 'F', 'I', 'D', 'E', 'N', 'C', 'E' >,
           string< 'C', 'O', 'N', 'C', 'A', 'T', 'E', 'N', 'A', 'T', 'E' >,
           string< 'C', 'O', 'M', 'P', 'L', 'E', 'X' >,
           string< 'C', 'O', 'M', 'B', 'I', 'N' >,
           string< 'C', 'O', 'L', 'U', 'M', 'N', 'S' >,
           string< 'C', 'O', 'L', 'U', 'M', 'N' >,
           string< 'C', 'O', 'D', 'E' >,
           string< 'C', 'L', 'E', 'A', 'N' >,
           string< 'C', 'H', 'I', 'T', 'E', 'S', 'T' >,
           string< 'C', 'H', 'I', 'I', 'N', 'V' >,
           string< 'C', 'H', 'I', 'D', 'I', 'S', 'T' >,
           string< 'C', 'H', 'A', 'R' >,
           string< 'C', 'E', 'L', 'L' >,
           string< 'C', 'E', 'I', 'L', 'I', 'N', 'G' >,
           string< 'C', 'A', 'L', 'L' >,
           string< 'B', 'I', 'N', 'O', 'M', 'D', 'I', 'S', 'T' >,
           string< 'B', 'I', 'N', '2', 'O', 'C', 'T' >,
           string< 'B', 'I', 'N', '2', 'H', 'E', 'X' >,
           string< 'B', 'I', 'N', '2', 'D', 'E', 'C' >,
           string< 'B', 'E', 'T', 'A', 'I', 'N', 'V' >,
           string< 'B', 'E', 'T', 'A', 'D', 'I', 'S', 'T' >,
           string< 'B', 'E', 'S', 'S', 'E', 'L', 'Y' >,
           string< 'B', 'E', 'S', 'S', 'E', 'L', 'K' >,
           string< 'B', 'E', 'S', 'S', 'E', 'L', 'J' >,
           string< 'B', 'E', 'S', 'S', 'E', 'L', 'I' >,
           string< 'B', 'A', 'H', 'T', 'T', 'E', 'X', 'T' >,
           string< 'A', 'V', 'E', 'R', 'A', 'G', 'E', 'I', 'F', 'S' >,
           string< 'A', 'V', 'E', 'R', 'A', 'G', 'E', 'I', 'F' >,
           string< 'A', 'V', 'E', 'R', 'A', 'G', 'E', 'A' >,
           string< 'A', 'V', 'E', 'R', 'A', 'G', 'E' >,
           string< 'A', 'V', 'E', 'D', 'E', 'V' >,
           string< 'A', 'T', 'A', 'N', 'H' >,
           string< 'A', 'T', 'A', 'N', '2' >,
           string< 'A', 'T', 'A', 'N' >,
           string< 'A', 'S', 'I', 'N', 'H' >,
           string< 'A', 'S', 'I', 'N' >,
           string< 'A', 'S', 'C' >,
           string< 'A', 'R', 'E', 'A', 'S' >,
           string< 'A', 'N', 'D' >,
           string< 'A', 'M', 'O', 'R', 'L', 'I', 'N', 'C' >,
           string< 'A', 'M', 'O', 'R', 'D', 'E', 'G', 'R', 'C' >,
           string< 'A', 'D', 'D', 'R', 'E', 'S', 'S' >,
           string< 'A', 'C', 'O', 'S', 'H' >,
           string< 'A', 'C', 'O', 'S' >,
           string< 'A', 'C', 'C', 'R', 'I', 'N', 'T', 'M' >,
           string< 'A', 'C', 'C', 'R', 'I', 'N', 'T' >,
           string< 'A', 'B', 'S' > >
  {};

  struct ExcelFunction
    : seq< sor< ExcelFunction1, ExcelFunction2 >, OpenParen >
  {};

  // ExcelRefFunctionToken IF() or CHOOSE()
  struct ExcelRefFunctionToken
    : seq< sor< string< 'I', 'F' >,
                string< 'C', 'H', 'O', 'O', 'S', 'E' > >,
           OpenParen >
  {};

  // ExcelConditionalRefFunctionToken INDEX() OFFSET() or INDIRECT()
  struct ExcelConditionalRefFunctionToken
    : seq< sor< string< 'I', 'N', 'D', 'E', 'X' >,
                string< 'O', 'F', 'F', 'S', 'E', 'T' >,
                string< 'I', 'N', 'D', 'I', 'R', 'E', 'C', 'T' > >,
           OpenParen >
  {};

  // BoolToken boolean literal "TRUE|FALSE"

  struct BoolToken
    : seq< sor< string< 'T', 'R', 'U', 'E' >, 
                string< 'F', 'A', 'L', 'S', 'E' > >,
           not_at< NameValidCharacter > >
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
           string< '#', 'N', '/', 'A' > >
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
                          opt< dollar >, digit, star< digit >,
                          not_at< NameValidCharacter > >
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

  struct root : sor< FormulaWithEq, FormulaWithBits, ArrayFormula, spaces > {};

  struct ArrayFormula
    : seq< OpenCurlyParen, eqop, FormulaWithBits, CloseCurlyParen >
  {};

  struct FormulaWithEq : seq< eqop, FormulaWithBits >
  {};

  struct FormulaWithBits :
    sor< seq< OpenParen,
              spaces,
              FormulaWithBits,
              spaces,
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
    : sor< seq< FunctionName, spaces, opt< Arguments >, spaces, CloseParen > >
  {};

  struct FunctionName : ExcelFunction {};

  // To match an arbitrary-length list of arguments, I'm using the form
  // seq< opt< R >, star< if_must< S, R > > >
  // which is a modification of
  // seq< R, star< if_must< S, R > > >
  // (which is also known as list_must< R, S >) to allow zero arguments
  /* struct Arguments : list_tail< Argument, comma > {}; */
  // To allow commas between argument sto be followed by spaces (these appear in
  // the XLParser tests), there is an opt< space >.
  struct Arguments : if_then_else< not_at< CloseParen >,
                                   list_must< Argument, seq< comma, spaces > >,
                                   success > {};

  struct Argument
    : if_then_else< at< spaces, sor< CloseParen, comma > >, // empty
                    success,
                    FormulaWithBits > // not empty
  {};

  struct PrefixOp : sor< plusop, minop > {};

  struct InfixOp :
    seq< spaces,
         sor< expop,
              mulop,
              divop,
              plusop,
              minop,
              concatop,
              neqop,
              gteop,
              lteop,
              gtop,
              eqop,
              ltop >,
         spaces >
  {};

  struct PostfixOp : percentop {};
  //
  // Perhaps should be list_must, but fails with "-E144  New IP deals might require some modifications" because of the double space
  struct References
    : list< Reference,
            sor< colon,
                 intersectop > >
  {};

  struct Reference
    : sor< ReferenceFunctionCall,
           DynamicDataExchange,
           seq< OpenParen, spaces, Reference, spaces, CloseParen >,
           seq< Prefix, ReferenceItem >,
           ReferenceItem >
  {};

  struct ReferenceFunctionCall
    : sor< seq< OpenParen, spaces, Union, spaces, CloseParen >,
           seq< RefFunctionName, spaces, opt< Arguments >, spaces, CloseParen > >
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

  struct UDFunctionCall : seq< UDFName, opt< Arguments >, spaces, CloseParen > {};
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
    : seq< OpenSquareParen, sor< seq< SRColumnToken, CloseSquareParen >,
                                 seq< NameToken, CloseSquareParen > > >
  {};

  struct StructuredReferenceTable : NameToken {};

  struct StructuredReferenceExpression
    : seq< StructuredReferenceElement,
           opt< seq< sor< colon, comma >, StructuredReferenceElement > >,
           opt< seq< sor< colon, comma >, StructuredReferenceElement > >,
           opt< seq< colon, StructuredReferenceElement > > >
  {};

  struct StructuredReference
    : sor< seq< OpenSquareParen, StructuredReferenceElement, CloseSquareParen >,
           StructuredReferenceElement,
           seq< StructuredReferenceTable,
                sor< StructuredReferenceElement,
                     seq< OpenSquareParen,
                          sor< CloseSquareParen,
                               seq< StructuredReferenceExpression, CloseSquareParen > > > > > >
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

/*   template<> struct tokenize< SheetsToken > */
/*   { */
/*     template< typename Input > */
/*       static void apply( const Input & in, std::string & token_value ) */
/*       { */
/*         Rcpp::Rcout << "SheetsToken: " << in.string() << "\n"; */
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
