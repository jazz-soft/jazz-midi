## Windows
`nmake -f Makefile.Windows`

## MacOS
`make -f Makefile.MacOS`

## Linux
`make -f Makefile.Linux`

## Testing
`node test`

`["version"]` -- get version  
`["refresh"]` -- get the port list  
`["openout", "Microsoft GS Wavetable Synth"]` -- open port  
`["play",144,60,127]` -- play
`["closeout"]` -- close port  
`["openin", "port name"]` -- open port  
`["closein"]` -- close port  
