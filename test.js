var MT = require('midi-test');
var src = new MT.MidiSrc('VIRTUAL MIDI-In');
var dst = new MT.MidiDst('VIRTUAL MIDI-Out');
if (!src.connect() || !dst.connect()) {
  console.error('Cannot open MIDI port!');
  process.exit(1);
}
console.log('Created VIRTUAL MIDI-In and VIRTUAL MIDI-Out!');
console.log('Enter MIDI message (e.g. 90 40 7f) or ^C to exit...');

const rl = require('readline').createInterface({
  input: process.stdin,
  output: process.stdout
});
rl.on('line', function(input) {
  var msg = parse(input.trim());
  if (msg) {
    src.emit(msg);
    console.log('MIDI sent:', format(msg));
  }
  rl.prompt();
});
rl.prompt();

dst.receive = function(msg) { console.log('MIDI received:', format(msg)); };

function format(msg) {
  return msg;
}

function parse(input) {
  if (!input) return;
  var arr = input.split(/\s+/);
  var msg = [];
  for (var i = 0; i < arr.length; i++) {
    var x = parseInt(arr[i], 16);
    if (isNaN(x) || x < 0 || x > 255 || arr[i].search(/[^0-9A-Fa-f]/) != -1) {
      console.log('Bad MIDI value:', arr[i]);
      return;
    }
    msg.push(x);
  }
  return msg;
}