var assert = require('assert');
var MT = require('midi-test');
var jazz = require('..');
var midi = new jazz.MIDI();

it('Info', function() {
  console.log('Node:', process.versions.node);
  console.log('process.platform:', process.platform);
  console.log('process.arch:', process.arch);
  console.log('jazz:', jazz);
  console.log('midi:', midi);
  console.log('MT:', MT);
});
