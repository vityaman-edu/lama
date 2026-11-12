# Compilation database

`make compile_commands` configures the release build using the same settings as
`make lamavm2`, without compiling the VM. It creates or refreshes the project-root
`compile_commands.json` symlink to `build/rel/compile_commands.json`.

Keep the generated symlink ignored by Git. Validate by running the target twice
and checking that the linked file contains a valid compilation database.
