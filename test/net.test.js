var net = require('net')
var assert = require('assert')
var Caps = require('../').Caps

var server = net.createServer((socket) => {
  var bufs = []
  socket.on('data', data => {
    bufs.push(data)
  })
  socket.on('end', () => {
    var data = Buffer.concat(bufs)
    var caps = new Caps()
    caps.deserialize(data)
    assert(caps.readString(), 'foo')
    socket.end()
    server.close()
  })
})
server.listen(0, () => {
  var port = server.address().port
  var cli = net.connect(port, '127.0.0.1', () => {
    var caps = new Caps()
    caps.writeString('foo')
    var data = caps.serialize()
    cli.write(data)
    cli.end()
  })
})
