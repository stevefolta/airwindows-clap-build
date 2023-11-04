{
	description = "Airwindows plugins in CLAP format";

	inputs = {
		sqs.url = "github:stevefolta/sqs";
		};

	outputs = { self, nixpkgs, sqs }:
		let
			plugin-name = "airwindows";

			# Why doesn't the flakes build system handle this automatically?!
			forAllSystems = nixpkgs.lib.genAttrs nixpkgs.lib.systems.flakeExposed;
			nixpkgsFor = forAllSystems (system: import nixpkgs { inherit system; });
		in {
			packages = forAllSystems (system: {
				default =
					nixpkgsFor.${system}.stdenv.mkDerivation {
						name = plugin-name;
						src = self;
						buildInputs = with nixpkgsFor.${system}; [
							clap sqs.packages.${system}.default
							];
						installPhase = ''
							mkdir -p $out/bin
							cp *.clap $out/bin/
							'';
						};
					});
			};
}


