#include "veriblock/entities/altpoptx.hpp"

namespace altintegration {

AltPopTx AltPopTx::fromVbkEncoding(ReadStream& stream) {
  AltPopTx alt_pop_tx;
  alt_pop_tx.version = stream.readBE<int32_t>();
  alt_pop_tx.vbk_context = readArrayOf<VbkBlock>(
      stream,
      0,
      MAX_CONTEXT_COUNT_ALT_PUBLICATION,
      (VbkBlock(*)(ReadStream&))VbkBlock::fromVbkEncoding);

  alt_pop_tx.atv = ATV::fromVbkEncoding(stream);
  alt_pop_tx.vtbs = readArrayOf<VTB>(stream,
                                     0,
                                     MAX_CONTEXT_COUNT_ALT_PUBLICATION,
                                     (VTB(*)(ReadStream&))VTB::fromVbkEncoding);

  return alt_pop_tx;
}

AltPopTx AltPopTx::fromVbkEncoding(Slice<const uint8_t> raw_bytes) {
  ReadStream stream(raw_bytes);
  return fromVbkEncoding(stream);
}

void AltPopTx::toVbkEncoding(WriteStream& stream) const {
  stream.writeBE<int32_t>(version);
  writeSingleBEValue(stream, vbk_context.size());
  for (const auto& b : vbk_context) {
    b.toVbkEncoding(stream);
  }
  atv.toVbkEncoding(stream);
  writeSingleBEValue(stream, vtbs.size());
  for (const auto& vtb : vtbs) {
    vtb.toVbkEncoding(stream);
  }
}

std::vector<uint8_t> AltPopTx::toVbkEncoding() const {
  WriteStream stream;
  toVbkEncoding(stream);
  return stream.data();
}

}  // namespace altintegration
