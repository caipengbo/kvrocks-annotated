#pragma once

#include <glog/logging.h>
#include <rocksdb/listener.h>

#include "storage.h"

// 在Rocksdb中，事件监听器EventListener类包含一系列的回调函数
// 当特定的RocksDB事件发生时，比如一次落盘或者压缩工作结束，就会被调用。
// 这些回调接口可以被用于开发一些自定义功能，此处就是为了打一些LOG。
class EventListener : public rocksdb::EventListener {
 public:
  explicit EventListener(Engine::Storage *storage) : storage_(storage) {}
  ~EventListener() override = default;
  void OnFlushBegin(rocksdb::DB* db, const rocksdb::FlushJobInfo& fi) override;
  void OnFlushCompleted(rocksdb::DB *db, const rocksdb::FlushJobInfo &fi) override;
  void OnCompactionCompleted(rocksdb::DB *db, const rocksdb::CompactionJobInfo &ci) override;
  void OnBackgroundError(rocksdb::BackgroundErrorReason reason, rocksdb::Status *status) override;
  void OnTableFileDeleted(const rocksdb::TableFileDeletionInfo& info) override;
  void OnStallConditionsChanged(const rocksdb::WriteStallInfo& info) override;
  void OnTableFileCreated(const rocksdb::TableFileCreationInfo& info) override;
 private:
  Engine::Storage *storage_ = nullptr;
};
