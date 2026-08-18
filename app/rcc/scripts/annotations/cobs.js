//
// COBS packets (Consistent Overhead Byte Stuffing)
//
// A zero byte delimits packets. Inside a packet each code byte points at
// the next code byte, so the chain is walked and labelled; a chain that
// runs past the delimiter is marked as a framing error.
//

decoder = {
  rows: ["packets", "codes"],
  classes: [
    {name: "packet", color: "#59a14f"},
    {name: "delimiter", color: "#e15759"},
    {name: "code", color: "#4e79a7"},
    {name: "framing error", color: "#b07aa1"}
  ],
  decode: function(bytes, offset, ctx) {
    const b = new Uint8Array(bytes)
    let i = 0
    let start = 0

    while (i < b.length) {
      if (b[i] !== 0x00) {
        ++i
        continue
      }

      if (i > start) {
        ctx.annotate(offset + start, offset + i - 1, 0, 0,
                     [(i - start) + " bytes", "P"])

        let p = start
        let ok = true
        while (p < i) {
          const code = b[p]
          if (code === 0 || p + code > i) {
            ok = false
            break
          }

          ctx.annotate(offset + p, offset + p, 1, 2, ["+" + code, String(code)])
          p += code
        }

        if (!ok)
          ctx.annotate(offset + p, offset + i - 1, 1, 3, ["framing error", "!"])
      }

      ctx.annotate(offset + i, offset + i, 0, 1, ["0x00", "0"])
      ++i
      start = i
    }

    return start
  }
}
