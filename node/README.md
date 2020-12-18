# jazz-midi

### Low-level MIDI support for Node.js.

![nodejs](http://jazz-soft.github.io/img/nodejs.jpg)
![windows](http://jazz-soft.github.io/img/windows.jpg)
![mocos](http://jazz-soft.github.io/img/macos.jpg)
![linux](http://jazz-soft.github.io/img/linux.jpg)
![raspberry pi](http://jazz-soft.github.io/img/rpi.jpg)  
[![npm](https://img.shields.io/npm/v/jazz-midi.svg)](https://www.npmjs.com/package/jazz-midi)
[![npm](https://img.shields.io/npm/dt/jazz-midi.svg)](https://www.npmjs.com/package/jazz-midi)


*Note:
Although **jazz-midi** can be used [stand-alone](http://jazz-soft.net/doc/Jazz-Plugin/reference.html),
it is best to use via [jzz](https://www.npmjs.com/package/jzz) or
[web-midi-api](https://www.npmjs.com/package/web-midi-api).*

Questions and comments are welcome [here](http://jazz-soft.org).

We would really appreciate your [**support**](https://jazz-soft.net/donate)!


## Install
    npm install jazz-midi

## Dependencies
None

## MIDI-Out example

    var jazz = require('jazz-midi');
    var midi = new jazz.MIDI();

    var name = midi.MidiOutOpen(0);
    if(name){
      console.log('Default MIDI-Out port:', name);
      midi.MidiOut(0x90,60,100); midi.MidiOut(0x90,64,100); midi.MidiOut(0x90,67,100);
      setTimeout(function(){
        midi.MidiOut(0x80,60,0); midi.MidiOut(0x80,64,0); midi.MidiOut(0x80,67,0);
        midi.MidiOutClose();
        console.log('Thank you for using Jazz-MIDI!');
      }, 3000);
    } else {
      console.log('Cannot open default MIDI-Out port!');
    }

## MIDI-In callback example

    var jazz = require('jazz-midi');
    var midi = new jazz.MIDI();

    var name = midi.MidiInOpen(0, function(t, msg){
      console.log(msg);
    });
    if(name){
      console.log('Default MIDI-In port:', name);
      setTimeout(function(){
        midi.MidiInClose();
        console.log('Thank you for using Jazz-MIDI!');
      }, 5000);
    } else {
      console.log('Cannot open default MIDI-In port!');
    }

## MIDI-In polling example

    var jazz = require('jazz-midi');
    var midi = new jazz.MIDI();

    var out_name;
    var in_name;
    var current;
    var delta;

    function start_recording(){
      out_name = midi.MidiOutOpen(0);
      if(out_name==''){ console.log('No default MIDI-Out port!'); return;}
      in_name = midi.MidiInOpen(0);
      if(in_name==''){ console.log('No default MIDI-In port!'); return;}
      console.log('Recording from', in_name, '...');
      setTimeout(start_playing, 5000);
    }

    function start_playing(){
      midi.MidiInClose();
      console.log('Playing to', out_name, '...');
      current = midi.QueryMidiIn();
      if(!current){
        cleanup();
        return;
      }
      delta = midi.Time() - current[0];
      next_msg();
    }

    function next_msg(){
      if(!current){
        setTimeout(cleanup, 1000);
        return;
      }
      var wait = current[0] + delta - midi.Time();
      if(wait<=0){
        midi.MidiOutLong(current.slice(1, current.length));
        current = midi.QueryMidiIn();
        next_msg();
      }
      else setTimeout(next_msg, wait);
    }

    function cleanup(){
      midi.MidiOutClose();
      console.log('Hope you have enjoyed!');
    }

    start_recording();
