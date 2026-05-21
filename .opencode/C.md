C CODE STYLE

FILES & FOLDERS
- Use .c for implementations and .h for headers.
- Use snake_case for filenames: buffer_manager.c.
- Use Header Guards (#ifndef, #define).
- Keep files focused; modularize logic into separate translation units.

STRUCTURE ORDER
1. Documentation/License header.
2. Includes (Standard libs first, then local headers).
3. Macros and Constants.
4. Type Definitions (typedef struct/enum).
5. Static Function Prototypes.
6. Public Function Definitions.
7. Static Function Definitions.

NAMING CONVENTIONS
- Functions: snake_case (init_system).
- Variables: snake_case (buffer_size).
- Constants/Macros: SCREAMING_SNAKE_CASE (MAX_RETRIES).
- Types: snake_case with _t suffix (config_t).

MEMORY & SAFETY
- Check every malloc/calloc return for NULL.
- Initialize all variables.
- Explicitly free memory; set pointers to NULL after free.
- Use const for read-only parameters.
- Avoid goto except for unified function cleanup.

REFERENCE
- The Power of 10: Rules for Developing Safety-Critical Code
