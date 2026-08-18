//
// MAVLink v2 packets
//
// 0xFD | len | incompat | compat | seq | sysid | compid | msgid(3) |
// payload(len) | checksum(2), plus a 13-byte signature when the
// incompatibility flag 0x01 is set.
//

decoder = {
  rows: ["packets", "fields"],
  classes: [
    {name: "header", color: "#4e79a7"},
    {name: "payload", color: "#59a14f"},
    {name: "checksum", color: "#f28e2b"},
    {name: "signature", color: "#b07aa1"}
  ],
  decode: function(bytes, offset, ctx) {
    const b = new Uint8Array(bytes)
    let i = 0

    while (i < b.length) {
      while (i < b.length && b[i] !== 0xFD)
        ++i

      if (i + 10 > b.length)
        return i

      const len = b[i + 1]
      const signed = (b[i + 2] & 0x01) !== 0
      const total = 12 + len + (signed ? 13 : 0)
      if (i + total > b.length)
        return i

      const msgid = b[i + 7] | (b[i + 8] << 8) | (b[i + 9] << 16)
      ctx.annotate(offset + i, offset + i + 9, 0, 0, ["msg " + msgid, String(msgid)])

      if (len > 0)
        ctx.annotate(offset + i + 10, offset + i + 9 + len, 1, 1, [len + " bytes", "P"])

      ctx.annotate(offset + i + 10 + len, offset + i + 11 + len, 1, 2, ["CRC", "C"])
      if (signed)
        ctx.annotate(offset + i + 12 + len, offset + i + total - 1, 1, 3, ["signature", "S"])

      i += total
    }

    return i
  }
}
