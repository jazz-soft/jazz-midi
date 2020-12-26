## Windows
`nmake -f Makefile.Windows`

## MacOS
`make -f Makefile.MacOS`

## Linux
`make -f Makefile.Linux`

## Testing
`node test`

### Test messages
`["version"]` -- get version  
`["refresh"]` -- get the port list  
`["openout", "Microsoft GS Wavetable Synth"]` -- open MIDI-out port  
`["play", 144, 60, 127]` -- send MIDI message  
`["closeout"]` -- close MIDI-out port  
`["openin", "port name"]` -- open MIDI-in port  
`["closein"]` -- close MIDI-in port
