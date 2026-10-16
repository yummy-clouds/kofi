source code (.ml)
   ↓ Lexer        → tokens
   ↓ Parser       → AST (tree)
   ↓ Semantic     → checked AST (types, scopes)
   ↓ IR (optional)→ simple intermediate code
   ↓ Codegen      → assembly (.s)
   ↓ gcc/as + ld  → executable