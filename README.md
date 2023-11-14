airwindows-clap-build
===

This is a script (and a couple of wrapper classes) that build the Airwindows
plugins as CLAP plugins for Linux.  The resulting "Airwindows.clap" file
contains all the plugins.


### Dependencies

[The Airwindows sources](https://github.com/airwindows/airwindows) (of course)  
[The CLAP headers](https://github.com/free-audio/clap)  
[sqs](https://github.com/stevefolta/sqs)


# Using it

Create a settings file to tell the script where the Airwindows sources are.
This can either be at
"~/.config/build-airwindows-claps/settings", or just a file called "settings"
in the current working directory.  An example:

```
airwindows-sources = "~/local/src/airwindows"
```

Then just run the "build-airwindows-claps" script.  You can give it optional
arguments to build only one plugin (`build-airwindows-clap only: Capacitor`) or
to exclude plugins (`build-airwindows-clap except: DrumSlam except: DustBunny`).

