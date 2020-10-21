package entities

import (
	"bytes"
	"encoding/hex"
	"testing"

	veriblock "github.com/VeriBlock/alt-integration-cpp"
	"github.com/stretchr/testify/assert"
)

var (
	defaultVbkBlock = VbkBlock{
		5000,
		2,
		parse12Bytes("449c60619294546ad825af03"),
		parse9Bytes("b0935637860679ddd5"),
		parse9Bytes("5ee4fd21082e18686e"),
		parse16Bytes("26bbfda7d5e4462ef24ae02d67e47d78"),
		1553699059,
		16842752,
		123317,
	}
	defaultVbkBlockEncoded = "41000013880002449c60619294546ad825af03b0935637860679ddd55ee4fd21082e18686e26bbfda7d5e4462ef24ae02d67e47d785c9b90f301010000000001e1b5"
)

func parse12Bytes(src string) [12]byte {
	buf := veriblock.Parse(src)
	var block [12]byte
	copy(block[:], buf)
	return block
}

func parse9Bytes(src string) [9]byte {
	buf := veriblock.Parse(src)
	var block [9]byte
	copy(block[:], buf)
	return block
}

func parse16Bytes(src string) [16]byte {
	buf := veriblock.Parse(src)
	var block [16]byte
	copy(block[:], buf)
	return block
}

func TestVbkBlockDeserialize(t *testing.T) {
	assert := assert.New(t)

	vbkblock := veriblock.Parse(defaultVbkBlockEncoded)
	stream := bytes.NewReader(vbkblock)
	block, err := VbkBlockFromVbkEncoding(stream)
	assert.NoError(err)

	assert.Equal(defaultVbkBlock.Height, block.Height)
	assert.Equal(defaultVbkBlock.Version, block.Version)
	assert.Equal(hex.EncodeToString(defaultVbkBlock.PreviousBlock[:]), hex.EncodeToString(block.PreviousBlock[:]))
	assert.Equal(hex.EncodeToString(defaultVbkBlock.PreviousKeystone[:]), hex.EncodeToString(block.PreviousKeystone[:]))
	assert.Equal(hex.EncodeToString(defaultVbkBlock.SecondPreviousKeystone[:]), hex.EncodeToString(block.SecondPreviousKeystone[:]))
	assert.Equal(hex.EncodeToString(defaultVbkBlock.MerkleRoot[:]), hex.EncodeToString(block.MerkleRoot[:]))
	assert.Equal(defaultVbkBlock.Timestamp, block.Timestamp)
	assert.Equal(defaultVbkBlock.Difficulty, block.Difficulty)
	assert.Equal(defaultVbkBlock.Nonce, block.Nonce)
}

func TestVbkBlockSerialize(t *testing.T) {
	assert := assert.New(t)

	stream := new(bytes.Buffer)
	err := defaultVbkBlock.ToVbkEncoding(stream)
	assert.NoError(err)
	blockEncoded := hex.EncodeToString(stream.Bytes())
	assert.Equal(defaultVbkBlockEncoded, blockEncoded)
}

func TestVbkBlockRoundTrip(t *testing.T) {
	assert := assert.New(t)

	blockEncoded, err := hex.DecodeString(defaultVbkBlockEncoded)
	assert.NoError(err)
	stream := bytes.NewReader(blockEncoded)
	decoded, err := VbkBlockFromVbkEncoding(stream)
	assert.NoError(err)
	assert.Equal(defaultVbkBlock.Version, decoded.Version)

	outputStream := new(bytes.Buffer)
	err = decoded.ToVbkEncoding(outputStream)
	assert.NoError(err)
	assert.Equal(defaultVbkBlockEncoded, hex.EncodeToString(outputStream.Bytes()))
}

func TestVbkBlockGetBlockHash(t *testing.T) {
	assert := assert.New(t)

	block := VbkBlock{}
	block.Height = 5000
	block.Version = 2
	copy(block.PreviousBlock[:], veriblock.Parse("94E7DC3E3BE21A96ECCF0FBD"))
	copy(block.PreviousKeystone[:], veriblock.Parse("F5F62A3331DC995C36"))
	copy(block.SecondPreviousKeystone[:], veriblock.Parse("B0935637860679DDD5"))
	copy(block.MerkleRoot[:], veriblock.Parse("DB0F135312B2C27867C9A83EF1B99B98"))
	block.Timestamp = 1553699987
	block.Difficulty = 117586646
	block.Nonce = 1924857207

	hash, err := block.GetHash()
	assert.NoError(err)
	assert.NotEqual(veriblock.Parse("00000000000000001f45c91342b8ac0ea7ae4d721be2445dc86ddc3f0e454f60"), hash[:])
}

func TestVbkBlockGetID(t *testing.T) {
	assert := assert.New(t)

	atvBytes, err := hex.DecodeString(defaultVbkBlockEncoded)
	assert.NoError(err)
	stream := bytes.NewReader(atvBytes)
	vbkblock, err := VbkBlockFromVbkEncoding(stream)
	assert.NoError(err)
	id, err := vbkblock.GetID()
	assert.NoError(err)
	assert.NotEqual("cd97599e23096ad42f119b5a", hex.EncodeToString(id[:]))
}