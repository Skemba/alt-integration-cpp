#include "veriblock/entities/vtb.hpp"

#include <gtest/gtest.h>

#include "util/literals.hpp"

using namespace VeriBlock;

static const std::string defaultVtbEncoded =
    "02046002011667ff0a897e5d512a0b6da2f41c479867fe6b3a4cae2640000013350002a793"
    "c872d6f6460e90bed62342bb968195f8c515d3eed7277a09efac4be99f95f0a15628b06ba3"
    "b44c0190b5c0495c9b8acd0701c5235ebbbe9c02011b01000000010ce74f1fb694a001eebb"
    "1d7d08ce6208033f5bf7263ebad2de07bbf518672732000000006a47304402200cf4998aba"
    "1682abeb777e762807a9dd2635a0b77773f66491b83ee3c87099ba022033b7ca24dc520915"
    "b8b0200cbdcf95ba6ae866354585af9c53ee86f27362ebec012103e5baf0709c395a82ef0b"
    "d63bc8847564ac201d69a8e6bf448d87aa53a1c431aaffffffff02b7270d00000000001976"
    "a9148b9ea8545059f3a922457afd14ddf3855d8b109988ac0000000000000000536a4c5000"
    "0013350002a793c872d6f6460e90bed62342bb968195f8c515d3eed7277a09efac4be99f95"
    "f0a15628b06ba3b44c0190b5c0495c9b8acd0701c5235ebbbe9cd4e943efe1864df0421661"
    "5cf92083f40000000002019f040000067b040000000c040000000400000020204d66077fdf"
    "24246ffd6b6979dfedef5d46588654addeb35edb11e993c131f61220023d1abe8758c6f917"
    "ec0c65674bbd43d66ee14dc667b3117dfc44690c6f5af120096ddba03ca952af133fb06307"
    "c24171e53bf50ab76f1edeabde5e99f78d4ead202f32cf1bee50349d56fc1943af84f2d2ab"
    "da520f64dc4db37b2f3db20b0ecb572093e70120f1b539d0c1495b368061129f30d35f9e43"
    "6f32d69967ae86031a275620f554378a116e2142f9f6315a38b19bd8a1b2e6dc31201f2d37"
    "a058f03c39c06c200824705685ceca003c95140434ee9d8bbbf4474b83fd4ecc2766137db9"
    "a44d7420b7b9e52f3ee8ce4fbb8be7d6cf66d33a20293f806c69385136662a74453fb16220"
    "1732c9a35e80d4796babea76aace50b49f6079ea3e349f026b4491cfe720ad17202d9b57e9"
    "2ab51fe28a587050fd82abb30abd699a5ce8b54e7cd49b2a827bcb9920dcba229acdc6b7f0"
    "28ba756fd5abbfebd31b4227cd4137d728ec5ea56c457618202cf1439a6dbcc1a35e96574b"
    "ddbf2c5db9174af5ad0d278fe92e06e4ac349a42500000c020134f09d43659eb53982d9afb"
    "444b96fa4bb58c037d2914000000000000000000ce0b1a9a77dd0db127b5df4bc368cd6ac2"
    "99a9747d991ec2dacbc0b699a2e4a5b3919b5c6c1f2c1773703bc001035000008020fc61cc"
    "9d4eac4b2d14761a4d06af8a9ef073dcd7fb5e0d000000000000000000a31508d4b101d0ad"
    "11e43ef9419c23fc277f67edae83c598ee70866dbcef5e25268b9b5c6c1f2c17e11874af50"
    "000040203f8e3980304439d853c302f6e496285e110e251251531300000000000000000039"
    "a72c22268381bd8d9dcfe002f472634a24cf0454de8b50f89e10891e5ffb1de08d9b5c6c1f"
    "2c1744290a925000000020baa42e40345a7f826a31d37db1a5d64b67b72732477422000000"
    "000000000000a33ad6be0634647b26633ab85fa8de258480bbb25e59c68e48bb0b608b1236"
    "2b10919b5c6c1f2c1749c4d1f0473045022100f4dce45edcc6bfc4a1f44ef04e47e90a348e"
    "fd471f742f18b882ac77a8d0e89e0220617cf7c4a22211991687b17126c1bb007a3b2a25c5"
    "50f75d66b857a8fd9d75e7583056301006072a8648ce3d020106052b8104000a03420004b3"
    "c10470c8e8e426f1937758d9fb5e97a1891176cb37d4c12d4af4107b1aa3e8a8a754c06a22"
    "760e44c60642fba883967c19740d5231336326f7962750c8df990400000000040000000d20"
    "2a014e88ed7ab65cdfaa85daeab07eea6cba5e147f736edd8d02c2f9ddf0dec60400000006"
    "205b977ea09a554ad56957f662284044e7d37450ddadf7db3647712f59693997872020d0a3"
    "d873eeeee6a222a75316dce60b53ca43eaea09d27f0ece897303a53ae920c06fe913dca5dc"
    "2736563b80834d69e6dfdf1b1e92383ea62791e410421b6c1120049f68d350eeb8b3df630c"
    "8308b5c8c2ba4cd6210868395b084af84d19ff0e9020000000000000000000000000000000"
    "00000000000000000000000000000000002036252dfc621de420fb083ad9d8767cba627edd"
    "eec64e421e9576cee21297dd0a40000013700002449c60619294546ad825af03b093563786"
    "0679ddd55ee4fd21082e18686eb53c1f4e259e6a0df23721a0b3b4b7ab5c9b9211070211ca"
    "f01c3f010100";

TEST(VTB, Deserialize) {
  const auto vtbBytes = ParseHex(defaultVtbEncoded);
  auto stream = ReadStream(vtbBytes);
  auto decoded = VTB::fromVbkEncoding(stream);

  EXPECT_EQ(decoded.transaction.address, Address(AddressType::STANDARD, "VE6MJFzmGdYdrxC8o6UCovVv7BdhdX"));
}

TEST(VTB, RoundTrip) {
  auto vtbBytes = ParseHex(defaultVtbEncoded);
  auto stream = ReadStream(vtbBytes);
  auto decoded = VTB::fromVbkEncoding(stream);
  EXPECT_EQ(decoded.transaction.address,
            Address(AddressType::STANDARD, "VE6MJFzmGdYdrxC8o6UCovVv7BdhdX"));

  WriteStream outputStream;
  decoded.toVbkEncoding(outputStream);
  auto txBytes = outputStream.data();
  auto txReEncoded = HexStr(txBytes);
  EXPECT_EQ(txReEncoded, defaultVtbEncoded);
}