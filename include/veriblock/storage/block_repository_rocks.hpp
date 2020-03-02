#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCK_REPOSITORY_ROCKS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCK_REPOSITORY_ROCKS_HPP_

#include <rocksdb/db.h>

#include <set>

#include "veriblock/blob.hpp"
#include "veriblock/serde.hpp"
#include "veriblock/storage/block_repository.hpp"
#include "veriblock/storage/db_error.hpp"
#include "veriblock/strutil.hpp"

namespace VeriBlock {

//! column family type
using cf_handle_t = rocksdb::ColumnFamilyHandle;

template <typename Block>
struct CursorRocks : public Cursor<typename Block::hash_t, Block> {
  //! stored block type
  using stored_block_t = Block;
  //! block has type
  using hash_t = typename Block::hash_t;

  CursorRocks(std::shared_ptr<rocksdb::DB> db,
              std::shared_ptr<cf_handle_t> hashBlockHandle)
      : _db(db), _hashBlockHandle(hashBlockHandle) {
    auto iterator =
        _db->NewIterator(rocksdb::ReadOptions(), _hashBlockHandle.get());
    _iterator = std::unique_ptr<rocksdb::Iterator>(iterator);
  }

  void seekToFirst() override { _iterator->SeekToFirst(); }
  void seek(const hash_t& key) override {
    std::string blockHash(reinterpret_cast<const char*>(key.data()),
                          key.size());
    _iterator->Seek(blockHash);
  }
  void seekToLast() override { _iterator->SeekToLast(); }
  bool isValid() const override { return _iterator->Valid(); }
  void next() override { _iterator->Next(); }
  void prev() override { _iterator->Prev(); }

  hash_t key() const override {
    if (!isValid()) {
      throw std::out_of_range("invalid cursor");
    }
    auto key = _iterator->key();
    return hash_t(key.ToString());
  }

  stored_block_t value() const override {
    if (!isValid()) {
      throw std::out_of_range("invalid cursor");
    }
    auto value = _iterator->value();
    return stored_block_t::fromRaw(value.ToString());
  }

 private:
  std::shared_ptr<rocksdb::DB> _db;
  std::shared_ptr<cf_handle_t> _hashBlockHandle;
  std::unique_ptr<rocksdb::Iterator> _iterator;
};

template <typename Block>
struct WriteBatchRocks : public WriteBatch<Block> {
  //! stored block type
  using stored_block_t = Block;
  //! block has type
  using hash_t = typename Block::hash_t;
  //! block height type
  using height_t = typename Block::height_t;

  WriteBatchRocks(std::shared_ptr<rocksdb::DB> db,
                  std::shared_ptr<cf_handle_t> hashBlockHandle)
      : _db(db), _hashBlockHandle(hashBlockHandle) {}

  void put(const stored_block_t& block) override {
    std::string blockHash(reinterpret_cast<const char*>(block.getHash().data()),
                          block.getHash().size());
    std::string blockBytes = block.toRaw();
    rocksdb::Status s =
        _batch.Put(_hashBlockHandle.get(), blockHash, blockBytes);
    if (!s.ok() && !s.IsNotFound()) {
      throw db::DbError(s.ToString());
    }
  }

  void removeByHash(const hash_t& hash) override {
    std::string blockHash(reinterpret_cast<const char*>(hash.data()),
                          hash.size());
    rocksdb::Status s = _batch.Delete(_hashBlockHandle.get(), blockHash);
    if (!s.ok() && !s.IsNotFound()) {
      throw db::DbError(s.ToString());
    }
  }

  void clear() override { _batch.Clear(); }

  void commit() override {
    rocksdb::Status s = _db->Write(rocksdb::WriteOptions(), &_batch);
    if (!s.ok() && !s.IsNotFound()) {
      throw db::DbError(s.ToString());
    }
    clear();
  }

 private:
  std::shared_ptr<rocksdb::DB> _db;
  std::shared_ptr<cf_handle_t> _hashBlockHandle;
  rocksdb::WriteBatch _batch{};
};

template <typename Block>
class BlockRepositoryRocks : public BlockRepository<Block> {
  //! stored block type
  using stored_block_t = Block;
  //! block has type
  using hash_t = typename Block::hash_t;
  //! block height type
  using height_t = typename Block::height_t;
  //! iterator type
  using cursor_t = Cursor<hash_t, stored_block_t>;

 public:
  BlockRepositoryRocks() = default;

  BlockRepositoryRocks(std::shared_ptr<rocksdb::DB> db,
                       std::shared_ptr<cf_handle_t> hashBlockHandle)
      : _db(db), _hashBlockHandle(hashBlockHandle) {}

  bool put(const stored_block_t& block) override {
    stored_block_t outBlock{};
    bool existing = getByHash(block.getHash(), &outBlock);

    std::string blockHash(reinterpret_cast<const char*>(block.getHash().data()),
                          block.getHash().size());
    std::string blockBytes = block.toRaw();

    rocksdb::Status s = _db->Put(
        rocksdb::WriteOptions(), _hashBlockHandle.get(), blockHash, blockBytes);
    if (!s.ok()) {
      throw db::DbError(s.ToString());
    }
    return existing;
  }

  bool getByHash(const hash_t& hash, stored_block_t* out) const override {
    std::string blockHash(reinterpret_cast<const char*>(hash.data()),
                          hash.size());
    std::string dbValue{};
    rocksdb::Status s = _db->Get(
        rocksdb::ReadOptions(), _hashBlockHandle.get(), blockHash, &dbValue);
    if (!s.ok()) {
      if (s.IsNotFound()) return false;
      throw db::DbError(s.ToString());
    }

    *out = stored_block_t::fromRaw(dbValue);
    return true;
  }

  size_t getManyByHash(Slice<const hash_t> hashes,
                       std::vector<stored_block_t>* out) const override {
    size_t found = 0;
    for (const hash_t& hash : hashes) {
      stored_block_t outBlock{};
      if (!getByHash(hash, &outBlock)) {
        /// TODO: some information about non-existing block
        continue;
      }
      found++;
      if (out) {
        out->push_back(outBlock);
      }
    }
    return found;
  }

  bool removeByHash(const hash_t& hash) override {
    stored_block_t outBlock{};
    bool existing = getByHash(hash, &outBlock);
    if (!existing) return false;

    std::string blockHash(reinterpret_cast<const char*>(hash.data()),
                          hash.size());
    rocksdb::Status s =
        _db->Delete(rocksdb::WriteOptions(), _hashBlockHandle.get(), blockHash);
    if (!s.ok() && !s.IsNotFound()) {
      throw db::DbError(s.ToString());
    }
    return true;
  }

  void clear() override {
    // call BlockRepositoryRocksManager.clear() instead
    return;
  }

  std::unique_ptr<WriteBatch<stored_block_t>> newBatch() override {
    return std::unique_ptr<WriteBatchRocks<stored_block_t>>(
        new WriteBatchRocks<stored_block_t>(_db, _hashBlockHandle));
  }

  std::shared_ptr<cursor_t> newCursor() override {
    return std::make_shared<CursorRocks<Block>>(_db, _hashBlockHandle);
  }

 private:
  std::shared_ptr<rocksdb::DB> _db{};
  std::shared_ptr<cf_handle_t> _hashBlockHandle{};
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCK_REPOSITORY_ROCKS_HPP_