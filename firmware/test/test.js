// framebuffer[field][line*20 + pixel/8] |= (digitalRead(PIN_DATA0) << (pixel % 8));
// framebuffer[field][line*20 + pixel/8] |= (digitalRead(PIN_DATA1) << (pixel % 8 + 1));
// framebuffer[field][line*20 + pixel/8] |= (digitalRead(PIN_DATA2) << (pixel % 8 + 2));
// framebuffer[field][line*20 + pixel/8] |= (digitalRead(PIN_DATA3) << (pixel % 8 + 3));


const framebuffer = new Array(160/8 * 160/8);

const PIN_DATA0 = 0;
const PIN_DATA1 = 1;
const PIN_DATA2 = 0;
const PIN_DATA3 = 1;

const PIN_DATA4 = 1;
const PIN_DATA5 = 1;
const PIN_DATA6 = 0;
const PIN_DATA7 = 1;

let line = 0;
let pixel = 0;

console.log(pixel % 3);

framebuffer[line*20 + Math.floor(pixel/8)] |= (PIN_DATA0 << (pixel % 8));
framebuffer[line*20 + Math.floor(pixel/8)] |= (PIN_DATA1 << (pixel % 8 + 1));
framebuffer[line*20 + Math.floor(pixel/8)] |= (PIN_DATA2 << (pixel % 8 + 2));
framebuffer[line*20 + Math.floor(pixel/8)] |= (PIN_DATA3 << (pixel % 8 + 3));

console.log(framebuffer.map((i) => i.toString(2)));

pixel += 4;

console.log(pixel % 8);

framebuffer[line*20 + Math.floor(pixel/8)] |= (PIN_DATA4 << (pixel % 8));
framebuffer[line*20 + Math.floor(pixel/8)] |= (PIN_DATA5 << (pixel % 8 + 1));
framebuffer[line*20 + Math.floor(pixel/8)] |= (PIN_DATA6 << (pixel % 8 + 2));
framebuffer[line*20 + Math.floor(pixel/8)] |= (PIN_DATA7 << (pixel % 8 + 3));

console.log(framebuffer.map((i) => i.toString(2)));

for (let i = 0; i < 8; i++) {
    console.log((framebuffer[0] >> i) & 1);
}   

console.log(1 % 20);