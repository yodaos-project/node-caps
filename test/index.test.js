'use strict'

var Caps = require('../').Caps

var f = () => {
  var arr = new Uint8Array(5)
  arr[0] = 0x00
  arr[1] = 0x01
  arr[2] = 0x02
  arr[3] = 0x03
  arr[4] = 0x04
  var write = new Caps()
  console.log(write.NetWorkByteOrder)
  try {
    write.writeInt32(-123)
    write.writeUInt32(123)
    write.writeInt64(123456789)
    write.writeString('testring')
    write.writeBinary(arr)
    var capsToWrite = new Caps()
    capsToWrite.writeInt32(999)
    capsToWrite.writeString('stringincaps')
    write.writeCaps(capsToWrite)
  } catch (errWrite) {
    // something wrong here
  }
  var buff = write.serialize()

  var reader = new Caps()
  try {
    reader.deserialize(buff)
    console.log(reader.readInt32())// console output -123
    console.log(reader.readUInt32())// console output 123
    console.log(reader.readInt64())// console output 123456789
    console.log(reader.readString())// console output teststring
    var arrRead = reader.readBinary()
    // console output arr[0] 1 arr[1] 2 arr[2] 3 arr[3] 4 arr[4] 5
    for (var i = 0; i < arrRead.length; ++i) { console.log(`arr[${i}] ${arrRead[i]}`) }
    var capsFromCaps = reader.readCaps()
    console.log(capsFromCaps.readInt32())// console output 999
    console.log(capsFromCaps.readString())// console output stringincaps
  } catch (errRead) {
    // something wrong here
  }
}
f()
