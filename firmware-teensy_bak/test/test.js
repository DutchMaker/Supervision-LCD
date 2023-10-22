const dec2bin = (dec, len) => (dec >>> 0).toString(2).padStart(len, '0')

let flex_io = 0

let cmd = 0b11011001

let high_nibble = cmd >> 4 & 0x0F
let low_nibble = cmd & 0x0F

console.log(`cmd: ${dec2bin(cmd, 8)}`)

console.log(`high bits: ${dec2bin(high_nibble, 4)}`)
console.log(`low bits: ${dec2bin(low_nibble, 4)}`)

flex_io = (low_nibble << 28 | high_nibble << 22) 

let low = flex_io & 0x0F
let high = (flex_io >> 6 & 0x0F) << 4
console.log(`flex_io low: ${dec2bin(low, 8)}`)
console.log(`flex_io high: ${dec2bin(high, 8)}`)

console.log(`command: ${dec2bin(high | low, 8)}`)

console.log(`flex_io: ${dec2bin(flex_io, 32)}`)