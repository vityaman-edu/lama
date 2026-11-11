{
  description = "LaMa development shell";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";

  outputs = { nixpkgs, ... }:
    let
      systems = [ "x86_64-linux" "aarch64-darwin" ];
    in
    {
      devShells = nixpkgs.lib.genAttrs systems (system:
        let
          pkgs = import nixpkgs { inherit system; };
        in
        {
          default = pkgs.mkShell {
            packages = with pkgs; [
              cmake
              ninja
              ccache
              llvmPackages_23.clang-tools
              llvmPackages_23.clang
              python3
              ruff

              git

              gnumake
              m4
              (writeShellScriptBin "gm4" ''exec ${m4}/bin/m4 "$@"'')
              ocaml
              ocamlPackages.dune_3
              opam
              pcre2
              libffi
              pkg-config
              pkgconf
            ];

            shellHook = builtins.readFile ./flake.shell.sh;
          };
        });
    };
}
