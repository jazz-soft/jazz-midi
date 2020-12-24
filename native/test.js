const child = require('child_process');
const readline = require('readline');

const app = child.spawn('jazz-midi');

app.stdout.on('data', function(data) { console.log('<= ', decode(data.toString())); });
app.stderr.on('data', function(data) { console.error('<! ', data.toString()); });
app.on('close', function(code) { console.log('child process exited with code', code); });

console.log('Press ^C to quit...');
app.stdin.write(encode(JSON.stringify(['version'])));
app.stdin.write(encode(JSON.stringify(['refresh'])));

var rl = readline.createInterface({ input: process.stdin, output: process.stdout });
rl.on('line', function(line) {
  app.stdin.write(encode(line));
}).on('SIGINT', function() {
  app.kill('SIGINT');
  process.exit();
});

function encode(s) {
  var len = s.length;
  return String.fromCharCode(len & 0xff) + String.fromCharCode((len >> 8) & 0xff) +
    String.fromCharCode((len >> 16) & 0xff) + String.fromCharCode((len >> 24) & 0xff) + s;
}

function decode(s) {
  var len = s.charCodeAt(0) + s.charCodeAt(1) * 0x100 + s.charCodeAt(2) * 0x10000 + s.charCodeAt(3) * 0x1000000;
  s = s.substr(4);
  if (len != s.length) console.log('Warning:', len, '!=', s.length);
  return s;
}
