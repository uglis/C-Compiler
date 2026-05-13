# C-- Compiler Lab 1 — Design Notes

## Why this file

The code implements C-- lexical/syntax error detection, AST construction, and tree printing. Some design choices are non-obvious from reading the code alone. This file captures the *why* behind those choices for Lab 2 / Lab 3 maintainers.

---

## Error recovery: placement of `error` tokens

**What**: Error recovery productions added at three levels:
```
Stmt   → error SEMI    (skip to next statement boundary)
ExtDef → error SEMI    (skip at top level)
Def   → error SEMI     (skip inside function body)
CompSt → LC error RC   (skip to closing brace)
Exp   → LP error RP    (skip to closing paren)
```

**Why these levels**: The Bison error recovery mechanism discards input tokens until it finds one that can legally follow `error`. The more tokens specified after `error`, the more context Bison has to find a match — but the more tokens get discarded. The delimiters `;`, `}`, `)` are natural synchronization points:
- They're unambiguous (can't appear in most error-recovery contexts)
- They're frequent enough that recovery doesn't skip too much
- They match how a human reader would resume parsing after a mistake

**Why not put `error` at the top-level `Program` rule?** It would discard everything to end-of-file on the first error, preventing multiple error reporting.

---

## `last_error_line` coordination between lexer and parser

**What**: A single `last_error_line` variable shared between `yyerror` (Type B) and `REPORT_ERROR_A` (Type A), both checking it before reporting and setting it after.

**Why**: The lab guarantees at most one error per input line, but a single mistake can trigger both a lexical error (e.g., invalid float `1.05e`) and a syntax error (the FLOAT token arrives where the parser didn't expect it). Without coordination, two errors would be reported for the same root cause on the same line. The shared variable ensures only the first error on each line is reported.

**Why not just use separate counters**: The lab requirement explicitly says one error per output line. A lexer-only approach can't know whether the parser will also fire on the same input position, because the lexer runs *before* the parser sees the token.

---

## Line number tracking: `@$.first_line` vs `0`

**What**: Syntax nodes (non-terminals) get `new_astnode(kind, @$.first_line)`. Terminal nodes get `new_astnode(kind, 0)`.

**Why `@$.first_line` and not `@1.first_line`**: `@$` is Bison's built-in location for the left-hand side symbol — it's auto-computed as the first token's position in the production. Using `@$` is more robust than hardcoding `@1` (which might be wrong if the production's first symbol is an empty reduction).

**Why `0` for terminals**: The output spec says lexical units (tokens) should NOT display line numbers. Passing `0` as lineno serves as a sentinel — `print_tree_depth` checks `is_syntax_unit()` rather than the lineno value, but the `0` convention makes intent clear in the grammar actions themselves (a non-zero value signals "this is a syntax node").

---

## Terminal naming: uppercase vs title case

**What**: Terminal node names in `ast.h` are UPPERCASE (`INT`, `FLOAT`, `ASSIGNOP`). Syntax unit names are Title Case (`Program`, `ExtDefList`).

**Why**: The reference output samples use uppercase for tokens and title case for grammar productions. This visual distinction helps when reading tree output — you can tell at a glance whether a node came from the lexer or from a grammar rule.

---

## Octal/hex validation: separate valid and invalid patterns

**What**: Flex rules use *four* patterns for numeric prefixes, ordered:
1. `0[xX]{HEX_DIGIT}+` — valid hex (longest-match priority over invalid)
2. `0[xX][a-zA-Z0-9]+` — invalid hex (catches `0x3G`, etc.)
3. `0[0-7]+` — valid octal (same-length priority over invalid)
4. `0[0-9]+` — invalid octal (catches `09`, `081`, etc.)

**Why not validate in a single action**: Flex's longest-match rule means a single catch-all pattern would always win over more specific patterns. By splitting valid and invalid into separate patterns of equal length, the valid pattern wins for correct input while the invalid pattern fires for bad input. This is cleaner than token-buffer inspection inside an action.

**Caveat**: `0x` alone (no hex digits) falls through to `0` (INT) + `x` (ID), which the parser may or may not reject. This is acceptable because `0x` is syntactically ambiguous without a language spec for standalone `0x`.

---

## Comment handling: start conditions vs pure regex

**What**: Block comments use a Flex start condition `COMMENT_BLOCK` rather than a single regex like `\/\*([^*]|\*+[^/])*\*+\/`.

**Why**: Two reasons:
1. **EOF detection**: A pure regex can't detect unterminated comments at end-of-file. The `<COMMENT_BLOCK><<EOF>>` rule catches this and reports a Type A error.
2. **Multi-line tracking**: The start condition's `\n` rule resets `yycolumn`, keeping position tracking accurate through comment bodies (important if errors are reported after a comment).

---

## Missing ASSIGNOP child (discovered during testing)

**What**: The `Dec → VarDec ASSIGNOP Exp` rule originally only added `$1` and `$3` as children, omitting the ASSIGNOP token.

**Why it mattered**: The tree output spec shows ASSIGNOP as a separate child node between the variable and the expression. Without it, the tree structure is semantically correct but fails the expected output format. This is an easy mistake because the `=` token "disappears" into the production — it's not stored in `$2`'s type union, but it still needs a node in the AST for printing.

**Lesson for Lab 2/3**: When adding new productions that consume tokens, verify that all tokens appear as children in the AST if the output format requires it.

---

## Shift/reduce conflicts (2 remaining)

Both conflicts involve the `error` token:

1. **`LC • error RC` vs `LC • error SEMI`**: Inside `{ }`, Bison can't decide whether `error` belongs to `CompSt: LC error RC` (shift) or `Stmt: error SEMI` (reduce empty DefList first). Bison shifts, preferring brace-level recovery.

2. **`Def: error SEMI` vs `Stmt: error SEMI`**: Same ambiguity at the definition/stmt boundary.

**Why they're harmless**: Both resolve to "shift" by default, which means Bison prefers the more specific recovery rule (brace-level) over the more general one (statement-level). This aligns with the design goal: skip as little input as possible.

---

## Float output format (`%f`)

**What**: Float values use `printf("%f", val)`, which prints 6 decimal places.

**Why not `%g`**: `%g` switches to scientific notation for values with large/small exponents. The output spec says "十进制形式" (decimal form). `%f` consistently produces fixed-point notation. The trailing zeros (e.g., `3.500000` for `3.5`) are acceptable per the spec, which only requires correct value display, not pretty-printing.
