# Luna

Luna is a modified version of Lua, created by a Brazilian developer [cross-sniper](https://github.com/cross-sniper). This customized Lua interpreter offers additional features and a unique variable naming scheme with `Lua_*` variables replaced by `luna_*`.

**Note**: Luna is an ongoing project, and improvements are continually being made. To build Luna, follow these steps:

1. Clone the Luna repository to your local machine.
   
   ```bash
   git clone https://github.com/cross-sniper/luna.git
   ```

2. Navigate to the Luna source directory.
   
   ```bash
   cd luna
   ```

3. Build Luna by running `make`.
   
   ```bash
   make
   ```

If you encounter issues related to the `Lua_*` variables in the codebase, you can use the `fix.py` script provided. This script will replace all instances of `Lua_*` with Luna's equivalent.

**Usage**:

Luna can be used like regular Lua with additional features and the `luna_*` naming convention for variables. Simply run the Luna interpreter in your terminal to execute Lua scripts.

Feel free to explore Luna, contribute to its development, or use it for your Lua-based projects.

Please check the [Luna repository](https://github.com/cross-sniper/luna) for more information, updates, and documentation.

**Disclaimer**: Luna is an unofficial custom version of Lua and is not affiliated with the official Lua project.

[![GitHub](https://img.shields.io/github/license/cross-sniper/luna)](https://github.com/cross-sniper/luna/blob/main/LICENSE)
