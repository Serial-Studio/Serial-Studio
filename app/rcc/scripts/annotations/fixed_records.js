//
// Fixed-size records
//
// The simplest framing there is: every RECORD_SIZE bytes form one record.
// Set RECORD_SIZE to the width of your struct; the first field is split
// out so a misaligned stream is obvious (the header column stops lining
// up).
//

const RECORD_SIZE = 16
const HEADER_SIZE = 2

decoder = {
  rows: ["records", "header"],
  classes: [
    {name: "record", color: "#59a14f"},
    {name: "header", color: "#4e79a7"}
  ],
  decode: function(bytes, offset, ctx) {
    const b = new Uint8Array(bytes)
    let i = 0

    while (i + RECORD_SIZE <= b.length) {
      ctx.annotate(offset + i, offset + i + RECORD_SIZE - 1, 0, 0,
                   [RECORD_SIZE + " bytes", "R"])
      ctx.annotate(offset + i, offset + i + HEADER_SIZE - 1, 1, 1,
                   ["0x" + b[i].toString(16), "H"])
      i += RECORD_SIZE
    }

    return i
  }
}
