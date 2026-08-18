//
// Annotation decoder
//
// - rows: the lanes drawn in the Track tab
// - classes: the colors and names of the labels
// - decode(): called with every new chunk of bytes,
//   returns how many bytes it consumed
//

decoder = {
  rows: ["bytes", "packets"],
  classes: [
    {name: "payload", color: "#59a14f"},
    {name: "newline", color: "#e15759"}
  ],
  decode: function(bytes, offset, ctx) {
    const b = new Uint8Array(bytes)
    let i = 0
    while (i < b.length) {
      const start = i
      while (i < b.length && b[i] !== 0x0A) ++i
      if (i >= b.length)
        return start

      ctx.annotate(offset + start, offset + i - 1, 1, 0,
                   [(i - start) + " bytes", "P"])
      ctx.annotate(offset + i, offset + i, 0, 1, ["LF", "n"])
      ++i
    }

    return i
  }
}
