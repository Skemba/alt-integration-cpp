// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCKCHAIN_UTIL_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCKCHAIN_UTIL_HPP_

#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/blockchain/chain.hpp>
#include <veriblock/validation_state.hpp>

namespace altintegration {

template <typename Block>
void determineBestChain(Chain<Block>& currentBest, BlockIndex<Block>& indexNew);

template <typename Block, typename ChainParams>
uint32_t getNextWorkRequired(const BlockIndex<Block>& prevBlock,
                             const Block& block,
                             const ChainParams& params);

template <typename Block>
ArithUint256 getBlockProof(const Block& block);

template <typename Block>
int64_t getMedianTimePast(const BlockIndex<Block>& prev);

template <typename Block, typename ChainParams>
bool checkBlockTime(const BlockIndex<Block>& prev,
                    const Block& block,
                    ValidationState& state,
                    const ChainParams& param);

template <typename Block, typename ChainParams>
bool contextuallyCheckBlock(const BlockIndex<Block>& prev,
                            const Block& block,
                            ValidationState& state,
                            const ChainParams& params,
                            bool shouldVerifyNextWork = true);

template <typename Block, typename Storage>
void removePayloadsFromIndex(Storage& storage,
                             BlockIndex<Block>& index,
                             const CommandGroup& cg);

template <typename ProtectedBlockTree>
bool recoverEndorsements(ProtectedBlockTree& ed_,
                         Chain<typename ProtectedBlockTree::index_t>& chain,
                         typename ProtectedBlockTree::index_t& toRecover,
                         ValidationState& state) {
  std::vector<std::function<void()>> actions;
  auto& containingEndorsements = toRecover.getContainingEndorsements();
  actions.reserve(containingEndorsements.size());
  auto& ing = ed_.getComparator().getProtectingBlockTree();

  for (const auto& p : containingEndorsements) {
    auto& id = p.first;
    auto& e = *p.second;

    if (id != e.id) {
      return state.Invalid(
          "bad-id", fmt::format("Key={}, Id={}", HexStr(id), HexStr(e.id)));
    }

    auto* endorsed = ed_.getBlockIndex(e.endorsedHash);
    if (!endorsed) {
      return state.Invalid(
          "no-endorsed",
          fmt::format("Can not find endorsed block in {}", e.toPrettyString()));
    }

    if (chain[endorsed->getHeight()] == nullptr ||
        chain[endorsed->getHeight()]->getHash() != e.endorsedHash ||
        endorsed->getHash() != e.endorsedHash) {
      return state.Invalid(
          "bad-endorsed",
          fmt::format("Endorsed block does not match {}", e.toPrettyString()));
    }

    if (e.containingHash != toRecover.getHash()) {
      return state.Invalid("bad-containing",
                           fmt::format("Containing block does not match {}",
                                       e.toPrettyString()));
    }

    auto* blockOfProof = ing.getBlockIndex(e.blockOfProof);
    if (!blockOfProof) {
      return state.Invalid(
          "bad-blockofproof",
          fmt::format("Block Of Proof %s does not exist in SP chain",
                      HexStr(e.blockOfProof)));
    }

    // make sure it is accessible in lambda
    auto* endorsement = &e;

    // delay execution. this ensures atomic changes - if any of endorsemens fail
    // validation, no 'action' is actually executed.
    actions.push_back([endorsed, blockOfProof, endorsement] {
      auto& by = endorsed->endorsedBy;
      VBK_ASSERT_MSG(std::find(by.begin(), by.end(), endorsement) == by.end(),
                     "same endorsement is added to endorsedBy second time");
      by.push_back(endorsement);

      auto& bop = blockOfProof->blockOfProofEndorsements;
      VBK_ASSERT_MSG(
          std::find(bop.begin(), bop.end(), endorsement) == bop.end(),
          "same endorsement is added to blockOfProof second time");
      bop.push_back(endorsement);
    });
  }

  // all actions have been validated
  // commit changes
  for (auto& f : actions) {
    f();
  }

  return true;
}

template <typename Block>
void assertBlockCanBeRemoved(const Block& block);

template <typename Block>
void assertBlockSanity(const Block& block) {
  VBK_ASSERT_MSG(block.getHash() != block.getPreviousBlock(),
                 "Previous block hash should NOT be equal to the current block "
                 "hash: %s. A collision in altchain hash?",
                 HexStr(block.getHash()));
}

template <typename Tree, typename Pop>
void payloadToCommands(Tree& tree,
                       const Pop& pop,
                       const std::vector<uint8_t>& containingHash,
                       std::vector<CommandPtr>& cmds);

struct PopData;

template <typename Tree, typename PayloadsT>
std::vector<CommandGroup> payloadsToCommandGroups(
    Tree& tree,
    const PayloadsT& pop,
    const std::vector<uint8_t>& containinghash);

template <typename Tree, typename Pop>
void vectorPopToCommandGroup(Tree& tree,
                             const std::vector<Pop>& pop,
                             const std::vector<uint8_t>& containingHash,
                             std::vector<CommandGroup>& cgs) {
  const auto& pl = tree.getPayloadsIndex();
  for (const auto& b : pop) {
    CommandGroup cg;
    cg.payload_type_name = &Pop::name();
    cg.id = b.getId().asVector();
    cg.valid = pl.getValidity(containingHash, cg.id);
    payloadToCommands(tree, b, containingHash, cg.commands);

    cgs.push_back(std::move(cg));
  }
}

template <typename BlockTree>
bool areOnSameChain(const typename BlockTree::block_t& blk1,
                    const typename BlockTree::block_t& blk2,
                    const BlockTree& tree) {
  auto* blk_index1 = tree.getBlockIndex(blk1.getHash());
  auto* blk_index2 = tree.getBlockIndex(blk2.getHash());

  VBK_ASSERT_MSG(blk_index1, "unknown block %s", blk1.toPrettyString());
  VBK_ASSERT_MSG(blk_index2, "unknown block %s", blk2.toPrettyString());

  if (blk_index1->getHeight() > blk_index2->getHeight()) {
    return blk_index1->getAncestor(blk_index2->getHeight()) == blk_index2;
  } else {
    return blk_index2->getAncestor(blk_index1->getHeight()) == blk_index1;
  }
}

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCKCHAIN_UTIL_HPP_
