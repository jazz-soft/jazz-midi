const child = require('child_process');
const readline = require('readline');

var buff = Buffer.alloc(0);

const app = child.spawn('./jazz-midi');
app.stdout.on('data', read);
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

function read(data) {
  var len;
  var str;
  buff = Buffer.concat([buff, data]);
  while (buff.length >= 4) {
    len = buff[0] + buff[1] * 0x100 + buff[2] * 0x10000 + buff[3] * 0x1000000 + 4;
    if (buff.length < len) break;
    str = buff.subarray(4, len);
    buff = buff.subarray(len);
    console.log('<=', str.toString());
  }
}
