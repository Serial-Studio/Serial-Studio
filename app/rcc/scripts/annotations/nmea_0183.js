//
// NMEA 0183 sentences: $TALKER,fields*CS<CR><LF>
//
// Labels the sentence type, its payload and the checksum, and marks a
// sentence whose checksum does not match the XOR of the bytes between
// $ and *.
//

decoder = {
  rows: ["sentence", "fields"],
  classes: [
    {name: "type", color: "#4e79a7"},
    {name: "payload", color: "#59a14f"},
    {name: "checksum", color: "#f28e2b"},
    {name: "bad checksum", color: "#e15759"}
  ],
  decode: function(bytes, offset, ctx) {
    const b = new Uint8Array(bytes)
    let i = 0

    while (i < b.length) {
      while (i < b.length && b[i] !== 0x24)
        ++i

      if (i >= b.length)
        return i

      const start = i
      let star = -1
      let j = i + 1
      while (j < b.length && b[j] !== 0x0A) {
        if (b[j] === 0x2A)
          star = j

        ++j
      }

      if (j >= b.length)
        return start

      if (star < 0 || star + 2 >= j) {
        i = j + 1
        continue
      }

      let sum = 0
      for (let k = start + 1; k < star; ++k)
        sum ^= b[k]

      const hex = String.fromCharCode(b[star + 1], b[star + 2])
      const ok = parseInt(hex, 16) === sum

      let comma = start + 1
      while (comma < star && b[comma] !== 0x2C)
        ++comma

      const type = String.fromCharCode.apply(null, b.subarray(start + 1, comma))
      ctx.annotate(offset + start, offset + comma - 1, 0, 0, [type, type.substring(2)])
      ctx.annotate(offset + comma, offset + star - 1, 1, 1,
                   [(star - comma) + " bytes", "F"])
      ctx.annotate(offset + star, offset + star + 2, 0, ok ? 2 : 3,
                   [ok ? "CRC " + hex : "BAD " + hex, ok ? "*" : "X"])

      i = j + 1
    }

    return i
  }
}
