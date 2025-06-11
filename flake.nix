{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs =
    {
      nixpkgs,
      flake-utils,
      ...
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
        lib = pkgs.lib;
      in
      {
        packages.default = pkgs.rustPlatform.buildRustPackage rec {
          pname = "mprisctl";
          version = "1.0.1";

          src = pkgs.fetchFromGitHub {
            owner = "amedoeyes";
            repo = "mprisctl";
            rev = version;
            hash = "sha256-jA4wLTxsewcOgkJp12MKYDfm1fDjkeglNa88qAWwMNY=";
          };

          cargoLock = {
            lockFile = "${src}/Cargo.lock";
          };

          nativeBuildInputs = [ pkgs.installShellFiles ];

          postInstall = ''
            installShellCompletion --cmd mprisctl \
              --bash <($out/bin/mprisctl completions bash) \
              --fish <($out/bin/mprisctl completions fish) \
              --zsh <($out/bin/mprisctl completions zsh)
          '';

          meta = {
            description = "Command-line tool to interact with MPRIS compatible media players.";
            homepage = "https://github.com/amedoeyes/mprisctl";
            license = lib.licenses.agpl3Only;
          };
        };

        devShells.default = pkgs.mkShell {
          buildInputs = with pkgs; [
            rustc
            cargo
          ];
        };
      }
    );
}
