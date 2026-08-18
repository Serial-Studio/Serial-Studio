//
// SLIP framing (RFC 1055)
//
// END (0xC0) closes a packet; ESC (0xDB) escapes a literal END or ESC as
// 0xDB 0xDC or 0xDB 0xDD. The escape pairs are labelled so a stream with
// escaping trouble is visible at a glance.
//

decoder = {
  rows: ["packets", "escapes"],
  classes: [
    {name: "packet", color: "#59a14f"},
    {name: "END", color: "#e15759"},
    {name: "escape", color: "#f28e2b"}
  ],
  decode: function(bytes, offset, ctx) {
    const b = new Uint8Array(bytes)
    let i = 0
    let start = 0
    let escapes = []

    while (i < b.length) {
      if (b[i] === 0xDB) {
        if (i + 1 >= b.length)
          return start

        escapes.push(i)
        i += 2
        continue
      }

      if (b[i] !== 0xC0) {
        ++i
        continue
      }

      if (i > start)
        ctx.annotate(offset + start, offset + i - 1, 0, 0,
                     [(i - start) + " bytes", "P"])

      for (let k = 0; k < escapes.length; ++k)
        ctx.annotate(offset + escapes[k], offset + escapes[k] + 1, 1, 2, ["ESC", "E"])

      ctx.annotate(offset + i, offset + i, 0, 1, ["END", "C0"])
      escapes = []
      ++i
      start = i
    }

    return start
  }
}
