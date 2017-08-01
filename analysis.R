# Analyse the ENRON formulas for clues to parsing

library(stringi)
library(tidyverse)
library(forcats)

# sources <- unique(readLines("./tests/testthat/formulas-distinct.txt"))
# saveRDS(sources, "sources.Rds", compress = FALSE)
sources <- readRDS("sources.Rds")

# Replace all strings of alphabetic characters with A (uppercase) or a
# (lowercase), and all numeric characters with 0.
neat <-
  sources %>%
  stri_replace_all_regex("[a-z]+", "a") %>%
  stri_replace_all_regex("[A-Z]+", "A") %>%
  stri_replace_all_regex("[0-9]+", "0")

# Function to pair each character with the next.
# Trivially parallelisable, but purrr doesn't support it :-(
pair_next <- function(x, pb) {
  # Progress bar
  if (pb$i < pb$n) {
    pb$tick()$print()
  }
  characters <- strsplit(x, "")[[1]]
  n <- length(characters)
  list(formula = rep(x, n - 1),
       char = characters[-n],
       next_char = characters[-1])
}

pb <- progress_estimated(length(neat))
paired <-
  map(neat, pair_next, pb = pb) %>%
  # Convert the list of lists to a data frame
  transpose %>%
  map(unlist) %>%
  as_tibble()

# # A compiled version is faster, but with no progress bar
# pair_next_compiled <- compiler::cmpfun(pair_next)
# paired2 <-
#   map(neat, pair_next, pb = pb) %>%
#   # Convert the list of lists to a data frame
#   transpose %>%
#   map(unlist) %>%
#   as_tibble()

# saveRDS(paired, "paired.Rds", compress = FALSE)
paired <- readRDS("paired.Rds")

# data.tables aren't a drop-in replacement :-(
# paired <- as.data.table(paired)

# The first character in a formula
paired %>%
  group_by(formula) %>%
  slice(1) %>%
  ungroup %>%
  count(char) %>%
  arrange(desc(n)) %>%
  print(n = Inf)
# # A tibble: 13 x 2
#     char     n
#    <chr> <int>
#  1     A  8620 ref/sheet/namedformula/function/UDF. `(` or `!` decides some
#  2     (  2345 openparen (subexpr?)
#  3     +  2205 prefix (infix?)
#  4     0  1158 number
#  5     -  1018 prefix (infix?)
#  6     '   955 ref(sheet)
#  7     [   292 file
#  8     $   279 reg
#  9     _   136 UDF, ref(sheet), or named formula
# 10  "\""    93 string
# 11     a    74 ref(sheet) or named formula, or UDF
# 12     #    46 #REF! or #N/A
# 13     *     3 invalid

paired %>%
  group_by(formula) %>%
  slice(1) %>%
  ungroup %>%
  filter(char == "#")

data_frame(sources) %>%
  filter(stri_detect_regex(sources, "^\\(")) %>%
  # mutate(sources = stri_extract_first_regex(sources, "^\\$[A-Z]+")) %>%
  mutate(sources = stri_sub(sources, 1, 3)) %>%
  count(sources) %>%
  arrange(n) %>%
  print(n = Inf)

tidyxl::tidy_xlsx("~/R/tidyxl/tests/testthat/examples.xlsx", 1)$data[[1]] %>% tail

# All pairs in a formula
paired %>%
  group_by(char) %>%
  summarise(n = n()) %>%
  arrange(desc(n)) %>%
  print(n = Inf)

plot_data <-
  paired %>%
  count(char, next_char) %>%
  arrange(desc(n))

plot_data %>%
  ggplot(aes(next_char, char, fill = log10(n), label = next_char)) +
  geom_tile() +
  geom_text(colour = "white") +
  theme(axis.text.x = element_blank(),
        axis.ticks.x = element_blank())

plot_data %>%
  arrange(char, desc(n), next_char) %>%
  split(.$char) %>%
  walk(print, n = Inf)
