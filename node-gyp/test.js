var jazz = require('.');
var midi = new jazz.MIDI();
console.log('jazz:', jazz);
console.log('midi:', midi);

console.log('System:', process.platform, process.arch);

//console.log('Package version:', jazz.package.version);
console.log('jazz.version:', jazz.version);
console.log('midi.version:', midi.version);
console.log('jazz.isJazz:', jazz.isJazz);
console.log('midi.isJazz:', midi.isJazz);
console.log('jazz.MidiOutList():', jazz.MidiOutList());
console.log('midi.MidiOutList():', midi.MidiOutList());
console.log('jazz.MidiInList():', jazz.MidiInList());
console.log('midi.MidiInList():', midi.MidiInList());
console.log('jazz.Support():', jazz.Support());
console.log('midi.Support():', midi.Support());
console.log('midi.out:', jazz.Support('midi.out'));
console.log('midi.in:', jazz.Support('midi.in'));
var i;
arr_out = jazz.MidiOutList();
for(i = 0; i < arr_out.length; i++) {
 console.log('Out:', i, midi.MidiOutInfo(i));
 console.log('Out:', i, midi.MidiOutInfo(arr_out[i]));
}
arr_in = jazz.MidiInList();
for (i = 0; i < arr_in.length; i++) {
 console.log('In:', i, midi.MidiInInfo(i));
 console.log('In:', i, midi.MidiInInfo(arr_in[i]));
}

var count = 0;
var outs = jazz.MidiOutList();
var ports = [];
function test_midi_out() {
 if (count < outs.length) {
  var name = outs[count];
  count++;
  var port = new jazz.MIDI();
  if (port.MidiOutOpen(name) != name) {
   console.log('Testing:', name, '- Cannot open!');
   setTimeout(test_midi_out, 0);
  } else {
   console.log('Testing:', name, '- OK!');
   var info=jazz.MidiOutInfo(name);
   console.log('   Manufacturer:', info[1]);
   console.log('   version:', info[2]);
   port.MidiOut(0x90,60,100); port.MidiOut(0x90,64,100); port.MidiOut(0x90,67,100);
   ports.push(port);
   setTimeout(test_midi_out, 2000);
  }
 } else {
  for (var i in ports) {
   ports[i].MidiOut(0x80,60,0); ports[i].MidiOut(0x80,64,0); ports[i].MidiOut(0x80,67,0);
  }
 }
}
test_midi_out();
