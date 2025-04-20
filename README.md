# Tekken Mesh Mapper
Quick simplistic tool to solve the key/value pair issue when working with Unreal assets exported by umodel.

It reads your PSK files and the adjacent materials referenced (../Materials/example.mat) to define all the Unreal asset keys with real path names relative to the base of your assets folder.

This may get more QoL updates in the future as the larger project I'm working on scales and I get annoyed with this tool.


## Usage
You can build using the build.bat script if you want. It uses g++ but really it's just two files, use whatever compiler you want.

Create a "in" folder and drop your Unreal assets in there with the file structure they will have during runtime in your application.
Example:
```
in
├── game
└── engine
```

After that, just run the tool.
```
packer.exe [flag: --auto]
```

The auto flag is recommended as it prevents you from having to manually define every key referenced in by your mesh.

If you run with the auto flag, the only time you'll be asked to define a key is if there is no filename in your directory that matches it or if there are multiple filenames that matches it.

When you're finished there will be a .skmap file in the directory next to the .psk which should be a simple enough format to figure out.
