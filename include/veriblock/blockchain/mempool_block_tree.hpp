// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALTINTEGRATION_MEMPOOL_BLOCK_TREE_HPP
#define ALTINTEGRATION_MEMPOOL_BLOCK_TREE_HPP

#include <unordered_map>

#include "veriblock/blockchain/alt_block_tree.hpp"
#include "veriblock/blockchain/temp_block_tree.hpp"
#include "veriblock/mempool_relations.hpp"

namespace altintegration {

struct MemPoolBlockTree {
 private:
  template <typename payload_t>
  struct PayloadCmp {
    PayloadCmp(const MemPoolBlockTree& tree) : tree_(tree) {}

    bool operator()(const std::shared_ptr<payload_t>& p1,
                    const std::shared_ptr<payload_t>& p2) {
      return tree_.payloadCompare(*p1, *p2) < 0;
    }

   private:
    const MemPoolBlockTree& tree_;
  };

 public:
  using BtcBlockTree = typename VbkBlockTree::BtcTree;

  MemPoolBlockTree(AltBlockTree& tree);

  bool acceptVbkBlock(const std::shared_ptr<VbkBlock>& blk,
                      ValidationState& state);

  bool acceptVTB(const VTB& vtb,
                 const std::shared_ptr<VbkBlock>& containingBlock,
                 ValidationState& state);

  bool acceptATV(const ATV& atv,
                 const std::shared_ptr<VbkBlock>& blockOfProof,
                 ValidationState& state);

  bool checkContextually(const ATV& atv, ValidationState& state);

  bool checkContextually(const VTB& vtb, ValidationState& state);

  void removePayloads(const VbkBlock& blk);

  void removePayloads(const VTB& vtb);

  void removePayloads(const ATV& atv);

  /**
   * Compares ATVs for the strongly equivalence
   *
   * @param[in] first ATV
   * @param[in] second ATV
   * @return
   * Returns true if ATVs strongly equivalent, otherwise returns false
   */
  bool areStronglyEquivalent(const ATV& atv1, const ATV& atv2) const;

  /**
   * Compares VTBs for the strongly equivalence
   *
   * @param[in] first VTB
   * @param[in] second VTB
   * @return
   * Returns true if VTB strongly equivalent, otherwise returns false
   */
  bool areStronglyEquivalent(const VTB& vtb1, const VTB& vtb2) const;

  /**
   * Compares VTBs for the weakly equivalence
   *
   * @param[in] first VTB
   * @param[in] second VTB
   * @return
   * Returns true if VTBs weakly equivalent, otherwise returns false
   */
  bool areWeaklyEquivalent(const VTB& vtb1, const VTB& vtb2) const;

  /**
   * Compare two vtbs that are weakly equivalent.
   * @param[in] first VTB to compare
   * @param[in] second VTB to compare
   * @return:
   * Return positive if vtb1 is better
   * Return negative if vtb2 is better
   * Return 0 if they are strongly equal
   */
  int weaklyCompare(const VTB& vtb1, const VTB& vtb2) const;

  /**
   * Compare two vtbs.
   * @param[in] first VTB to compare
   * @param[in] second VTB to compare
   * @return:
   * Return positive if vtb1 is better
   * Return negative if vtb2 is better
   * Return 0 if they are strongly equal
   */
  int payloadCompare(const VTB& vtb1, const VTB& vtb2) const;

  /**
   * Compare two atvs.
   * @param[in] first ATV to compare
   * @param[in] second ATV to compare
   * @return:
   * Return positive if atv1 is better
   * Return negative if atv2 is better
   * Return 0 if they are strongly equal
   */
  int payloadCompare(const ATV& atv1, const ATV& atv2) const;

  TempBlockTree<VbkBlockTree>& vbk() { return temp_vbk_tree_; }

  const TempBlockTree<VbkBlockTree>& vbk() const { return temp_vbk_tree_; }

  TempBlockTree<BtcBlockTree>& btc() { return temp_btc_tree_; }

  const TempBlockTree<BtcBlockTree>& btc() const { return temp_btc_tree_; }

  AltBlockTree& alt() { return *tree_; }

  const AltBlockTree& alt() const { return *tree_; }

  void clear() {
    temp_btc_tree_.clear();
    temp_vbk_tree_.clear();
  }

 private:
  TempBlockTree<VbkBlockTree> temp_vbk_tree_;
  TempBlockTree<BtcBlockTree> temp_btc_tree_;
  std::set<std::shared_ptr<ATV>, PayloadCmp<ATV>> stored_atvs_;
  std::set<std::shared_ptr<VTB>, PayloadCmp<VTB>> stored_vtbs_;

  AltBlockTree* tree_;
};

}  // namespace altintegration

#endif  