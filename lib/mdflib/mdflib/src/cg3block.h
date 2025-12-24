
#pragma once
#include <memory>
#include <string>
#include <vector>

#include "cn3block.h"
#include "mdf/ichannelgroup.h"
#include "mdf/idatagroup.h"
#include "mdfblock.h"
#include "sr3block.h"
#include "tx3block.h"
#include "dgrange.h"
namespace mdf::detail {
class Cg3Block : public MdfBlock, public IChannelGroup {
 public:
  using Cn3List = std::vector<std::unique_ptr<Cn3Block>>;
  using Sr3List = std::vector<std::unique_ptr<Sr3Block>>;

  [[nodiscard]] int64_t Index() const override;
  [[nodiscard]] std::string BlockType() const override {
    return MdfBlock::BlockType();
  }
  void Name(const std::string& name) override;
  [[nodiscard]] std::string Name() const override;

  void Description(const std::string& description) override;
  [[nodiscard]] std::string Description() const override;

  [[nodiscard]] uint64_t NofSamples() const override;
  void NofSamples(uint64_t nof_samples) override;

  void RecordId(uint64_t record_id) override;
  [[nodiscard]] uint64_t RecordId() const override;

  [[nodiscard]] std::vector<IChannel*> Channels() const override;
  [[nodiscard]] IChannel* CreateChannel() override;

  [[nodiscard]] const IChannel* GetXChannel(
      const IChannel& reference) const override;

  [[nodiscard]] std::string Comment() const override;
  MdfBlock* Find(int64_t index) const override;
  void GetBlockProperty(BlockPropertyList& dest) const override;
  uint64_t Read(std::streambuf& buffer) override;
  uint64_t Write(std::streambuf& buffer) override;

  void ReadCnList(std::streambuf& buffer);
  void ReadSrList(std::streambuf& buffer);

  void ReadData(std::streambuf& buffer) const;


  uint16_t RecordSize() const { return size_of_data_record_; }

  void AddCn3(std::unique_ptr<Cn3Block>& cn3);
  const Cn3List& Cn3() const { return cn_list_; }

  const Sr3List& Sr3() const { return sr_list_; }

  [[nodiscard]] std::vector<uint8_t>& SampleBuffer() const {
    return sample_buffer_;
  }
  uint64_t ReadDataRecord(std::streambuf& buffer, const IDataGroup& notifier) const;
  uint64_t ReadRangeDataRecord(std::streambuf& buffer, const IDataGroup& notifier,
                             DgRange& range) const;
  [[nodiscard]] bool PrepareForWriting();

  void ClearData() override;

  [[nodiscard]] const IDataGroup* DataGroup() const override;

 private:
  uint16_t record_id_ = 0;
  uint16_t nof_channels_ = 0;
  uint16_t size_of_data_record_ = 0;
  uint32_t nof_records_ = 0;

  std::string comment_;
  Cn3List cn_list_;
  Sr3List sr_list_;


};
}  // namespace mdf::detail
