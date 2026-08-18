//
// Modbus RTU frames
//
// RTU delimits frames by an idle gap on the wire, which a byte stream no
// longer carries, so frame length is derived from the function code. The
// common function codes are covered; anything else falls back to scanning
// for the next plausible address byte.
//
// LENGTHS holds the response lengths; a request for the same function is
// always 8 bytes (address, function, 4 data bytes, 2 CRC).
//

decoder = {
  rows: ["frames", "fields"],
  classes: [
    {name: "address", color: "#4e79a7"},
    {name: "function", color: "#59a14f"},
    {name: "data", color: "#76b7b2"},
    {name: "CRC", color: "#f28e2b"},
    {name: "exception", color: "#e15759"}
  ],
  decode: function(bytes, offset, ctx) {
    const b = new Uint8Array(bytes)
    const COUNTED = [0x01, 0x02, 0x03, 0x04, 0x0C, 0x11, 0x14, 0x15, 0x17]
    let i = 0

    while (i + 4 <= b.length) {
      const fn = b[i + 1]
      let total = 8

      if ((fn & 0x80) !== 0)
        total = 5
      else if (COUNTED.indexOf(fn) >= 0)
        total = 3 + b[i + 2] + 2

      if (i + total > b.length)
        return i

      ctx.annotate(offset + i, offset + i, 0, 0, ["addr " + b[i], String(b[i])])
      ctx.annotate(offset + i + 1, offset + i + 1, 1, (fn & 0x80) !== 0 ? 4 : 1,
                   [(fn & 0x80) !== 0 ? "exception" : "fn " + fn, String(fn & 0x7F)])

      if (total > 4)
        ctx.annotate(offset + i + 2, offset + i + total - 3, 1, 2,
                     [(total - 4) + " bytes", "D"])

      ctx.annotate(offset + i + total - 2, offset + i + total - 1, 0, 3, ["CRC", "C"])
      i += total
    }

    return i
  }
}
