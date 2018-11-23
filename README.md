# Node-Caps

## What is Caps

[Caps](https://github.com/Rokid/aife-mutils) is a light, simple serialization library, support data types:
- int32
- uint32
- int64
- uint64
- string
- float
- double

## What is Node-Caps
Node-Caps is a wrapper of Caps using N-API

## Node-Caps API Reference
### Write Data
`writeInt32(num)` write int32 value into this Caps object
- `num` int32, 4 bytes

`writeUInt32(num)` write uint32 value into this Caps object
- `num` uint32, 4 bytes

`writeInt64(num)` write int64 value into this Caps object
- `num` int64, 8 bytes

`writeFloat(num)` write float value into this Caps object
- `num` float, 4 bytes

`writeDouble(num)` write int32 value into this Caps object
- `num` double number, 8 bytes

`writeCaps(capsObj)` write another Caps object into this Caps
- `capsObj` Caps object

### Read Data
`readInt32()` read out a int32 value
- return int32 value

`readUInt32()` read out a uint32 value
- return uint32 value

`readInt64()`  read out a int64 value
- return int64 value

`readFloat()`  read out a float value
- return float value

`readDouble()`  read out a double value
- return double value

`readCaps(capsObj)`  read out a Caps object
- return Caps object

### serialize and deserialize

`serialize()` serialize this Caps object into UInt8Array

`serializeForCapsObj()` serialize this Caps object into another one

`deserialize(uint8Array)` deserialize from UInt8Array

`deserializeForCapsObj(capsObj)` deserialize from another Caps object

## something you need to know
**uint64 is supported by Caps but not by Node-Caps for now**
## demo
```js
'use strict'

const Caps = require("./PATH/TO/Caps").Caps

let f = ()=>
{

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
        write.writeString("testring")
        write.writeBinary(arr)
        var capsToWrite = new Caps()
        capsToWrite.writeInt32(999)
        capsToWrite.writeString("stringincaps")
        write.writeCaps(capsToWrite)
    }
    catch (errWrite) {
        //something wrong here
    }
    var buff = write.serialize()

    var reader = new Caps()
    try {
        reader.deserialize(buff)
        console.log(reader.readInt32())// console output -123
        console.log(reader.readUInt32())// console output 123
        console.log(reader.readInt64())// console output 123456789
        console.log(reader.readString())// console output teststring
        var arr_read = reader.readBinary()
        //console output arr[0] 1 arr[1] 2 arr[2] 3 arr[3] 4 arr[4] 5
        for (var i = 0; i < arr_read.length; ++i)
            console.log(`arr[${i}] ${arr_read[i]}`)
        var capsFromCaps = reader.readCaps()
        console.log(capsFromCaps.readInt32())// console output 999
        console.log(capsFromCaps.readString())// console output stringincaps
    }
    catch (errRead) {
        //something wrong here
    }
};
f();

```
`





