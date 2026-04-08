C++ CODE STYLE

FILES & FOLDERS
- Use .cc for source and .hpp for headers.
- Use PascalCase for Class files: SensorInterface.cc.
- One class per file pair.

STRUCTURE ORDER
1. Includes.
2. Class Definition.
3. Public members.
4. Protected members.
5. Private members.
6. Out-of-line method implementations.

NAMING CONVENTIONS
- Classes: PascalCase (DataStream).
- Methods: snake_case (process_packet).
- Variables: snake_case (raw_input).
- Private members: m_ prefix (m_is_running).

BEST PRACTICES
- Use std::unique_ptr and std::shared_ptr over raw pointers.
- Use RAII for resource management.
- Prefer std::vector over C-style arrays.
- Use noexcept for critical low-level functions.
- Mark overrides with override keyword.

REFERENCE
- The Power of 10: Rules for Developing Safety-Critical Code
