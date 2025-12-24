/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include "convertersamplequeue.h"

#include "dg4block.h"
#include "dt4block.h"
#include "hl4block.h"
#include "dl4block.h"
#include "dz4block.h"
#include "cn4block.h"

namespace mdf::detail {

ConverterSampleQueue::ConverterSampleQueue(MdfWriter& writer,
                                       IDataGroup& data_group)
    : Writer4SampleQueue(writer, data_group) {

}

ConverterSampleQueue::~ConverterSampleQueue() {
  SampleQueue::Reset();
}

void ConverterSampleQueue::TrimQueue() {
  // No need to trim the queue as all samples should be stored onto file.
}

void ConverterSampleQueue::SaveQueue(std::unique_lock<std::mutex>& lock) {
  if (writer_.CompressData()) {
    SaveQueueCompressed(lock);
    return;
  }
  auto* dg4 = dynamic_cast<Dg4Block*>(&data_group_);
  if (dg4 == nullptr) {
    return;
  }

  lock.unlock(); // OK to add samples to the queue
  // Save uncompressed data in last DG/DT block

  // File should be open at this point
  if (!IsOpen()) {
    lock.lock();
    return;
  }
  if (dg4->FilePosition() <= 0) {
    dg4->Write(*writer_.file_); // Flush out data
  }

  auto* dt4 = dg4->CreateOrGetDt4(*writer_.file_);
  if (dt4 == nullptr) {
    return;
  }

  SetLastPosition(*writer_.file_);

  const auto id_size = dg4->RecordIdSize();

  lock.lock(); // Lock the sample queue while flushing out to file

  while (!IsEmpty() ) {
    // Write all samples last to file
    SampleRecord sample = GetSample();

    auto* cg4 = dg4->FindCgRecordId(sample.record_id);
    if (cg4 == nullptr) {
      continue;
    }
    lock.unlock();

    auto* vlsd_group = sample.vlsd_data ?
                                        dg4->FindCgRecordId(sample.record_id + 1) : nullptr;
    // The next group must have the VLSD flag set otherwise it's not a VLSD group.
    if (vlsd_group != nullptr && (vlsd_group->Flags() & CgFlag::VlsdChannel) == 0) {
      vlsd_group = nullptr;
    }
    auto* cn4 = sample.vlsd_data && vlsd_group == nullptr ?
                                                          cg4->FindSdChannel() : nullptr;
    // If the sample holds VLSD data, save this data first and then update
    // the data index. VLSD data is stored in SD or CG. A dirty trick is that
    // the VLSD CG must have the next record_id
    if (vlsd_group != nullptr) {
      // Store as a VLSD record
      const auto vlsd_index = vlsd_group->WriteVlsdSample(*writer_.file_, id_size,
                                                          sample.vlsd_buffer);
      // Update the sample buffer with the new vlsd_index. Index is always
      // last 8 bytes in sample buffer
      const LittleBuffer buff(vlsd_index);
      if (sample.record_buffer.size() >= 8) {
        const auto index_pos =
            static_cast<int>(sample.record_buffer.size() - 8);
        std::copy(buff.cbegin(), buff.cend(),
                  std::next(sample.record_buffer.begin(), index_pos) );
      }
    } else if (cn4 != nullptr) {
      // Store as SD data on VLSD channel
      const auto vlsd_index = cn4->WriteSdSample(sample.vlsd_buffer);
      // Update the sample buffer with the new vlsd_index. Index is always
      // last 8 bytes in sample buffer
      const LittleBuffer buff(vlsd_index);
      if (sample.record_buffer.size() >= 8) {
        const auto index_pos =
            static_cast<int>(sample.record_buffer.size() - 8);
        std::copy(buff.cbegin(), buff.cend(),
                  std::next(sample.record_buffer.begin(), index_pos) );
      }
    }

    cg4->WriteSample(*writer_.file_, id_size, sample.record_buffer);
    lock.lock();
  }

  lock.unlock();
  // Save cuurent (last) byte position
  const auto last_position = GetFilePosition(*writer_.file_);
  dg4->Write(*writer_.file_); // Flush out data

  const uint64_t block_length = last_position - dt4->FilePosition();
  dt4->UpdateBlockSize(*writer_.file_, block_length);

  lock.lock();
}

void ConverterSampleQueue::CleanQueue(std::unique_lock<std::mutex>& lock) {

  if (writer_.CompressData()) {
    CleanQueueCompressed(lock, true);
    return;
  }
  SaveQueue(lock);
}

void ConverterSampleQueue::SaveQueueCompressed(std::unique_lock<std::mutex>& lock) {
  const auto nof_dz = CalculateNofDzBlocks();
  if (nof_dz < 2) {
    // Only save full 4MB DZ blocks
    return;
  }
  CleanQueueCompressed(lock, false); // Saves one DZ block only
}

void ConverterSampleQueue::CleanQueueCompressed(std::unique_lock<std::mutex>& lock,
                                        bool finalize) {
  // Save compressed data in last DG block by appending HL/DL and DZ/DT blocks
  constexpr size_t buffer_max = 4'000'000;
  if (IsEmpty()) {
    // Nothing to save to the file.
    return;
  }

  auto* dg4 = dynamic_cast<Dg4Block*>(&data_group_);
  if (dg4 == nullptr) {
    return;
  }

  auto* hl4 = dg4->CreateOrGetHl4();
  if (hl4 == nullptr) {
    return;
  }

  lock.unlock();
  if (!IsOpen()) {
    lock.lock();
    return;
  }

  SetLastPosition(*writer_.file_);

  std::vector<uint8_t> buffer;
  buffer.reserve(4'000'000);

  // Create DL block
  auto dl4 = std::make_unique<Dl4Block>();
  dl4->Flags(0);
  size_t dz_count = 0;
  dl4->Offset(dz_count, offset_);

  const auto id_size = dg4->RecordIdSize();

  lock.lock();

  while (!IsEmpty()) {
    SampleRecord sample = GetSample();

    lock.unlock(); // Need to unlock the sample queue while file operation

    size_t max_index = sample.record_buffer.size()
                       + dg4->RecordIdSize()
                       + buffer.size();
    if (sample.vlsd_data ) {
      max_index += sample.vlsd_buffer.size() + 4 + id_size;
    }

    // Check if this DZ block is full (4MB). If break out.
    if (max_index >= buffer_max) {
      // If the measurement should be finalized, we need to create a new
      // DZ block and put the remaining samples there. If not we can break
      // here and assume that the next cyclic call will handle the next DZ
      // block.
      if (finalize || QueueSize() > buffer_max) {
        // Purge the buffer to a DZ block and add it to the last DL block
        auto dz4 = std::make_unique<Dz4Block>();
        dz4->OrigBlockType("DT");
        dz4->Type(Dz4ZipType::Deflate);
        dz4->Data(buffer);
        dl4->Offset(dz_count, offset_);
        ++dz_count;
        offset_ += buffer.size();
        dl4->DataBlockList().push_back(std::move(dz4));

        buffer.clear();
        buffer.shrink_to_fit();
        buffer.reserve(buffer_max);

      } else {
        lock.lock(); // Lock the sample queue when leaving the while loop.
        PushSample(sample);
        break;
      }
    }
    // The following handling is similar as with uncompressed data but instead
    // of saving to file, we need to save it to a temporary buffer.
    auto* cg4 = dg4->FindCgRecordId(sample.record_id);
    if (cg4 == nullptr) {
      // Should not happen but lost sample is
      lock.lock();
      continue;
    }

    // If the sample have vlsd data, it could be stored in the next VLSD CG or
    // in a SD block.
    auto* vlsd_group = sample.vlsd_data ?
                                        dg4->FindCgRecordId(sample.record_id + 1) : nullptr;
    // The next group must have the VLSD flag set otherwise it's not a VLSD group.
    if (vlsd_group != nullptr &&
        (vlsd_group->Flags() & CgFlag::VlsdChannel) == 0) {
      vlsd_group = nullptr;
    }
    // Needs a pointer to the channel in case of VLSD SD storage
    auto* cn4 = sample.vlsd_data && vlsd_group == nullptr ?
                                                          cg4->FindSdChannel() : nullptr;
    // If the sample holds VLSD data, save this data first and then update
    // the data index. VLSD data is stored in SD or CG. A dirty trick is that
    // the VLSD CG must have the next record_id

    if (vlsd_group != nullptr) {
      // Store as a VLSD record
      const auto vlsd_index = vlsd_group->WriteCompressedVlsdSample(buffer,
                                                                    id_size,
                                                                    sample.vlsd_buffer);
      // Update the sample buffer with the new vlsd_index. Index is always
      // last 8 bytes in sample buffer
      const LittleBuffer buff(vlsd_index);
      if (sample.record_buffer.size() >= 8) {
        const auto index_pos =
            static_cast<int>(sample.record_buffer.size() - 8);
        std::copy(buff.cbegin(), buff.cend(),
                  std::next(sample.record_buffer.begin(), index_pos) );
      }
    } else if (cn4 != nullptr) {
      // Store as SD data on VLSD channel
      const auto vlsd_index = cn4->WriteSdSample(sample.vlsd_buffer);
      // Update the sample buffer with the new vlsd_index. Index is always
      // last 8 bytes in sample buffer
      const LittleBuffer buff(vlsd_index);
      if (sample.record_buffer.size() >= 8) {
        const auto index_pos =
            static_cast<int>(sample.record_buffer.size() - 8);
        std::copy(buff.cbegin(), buff.cend(),
                  std::next(sample.record_buffer.begin(), index_pos) );
      }
    }
    cg4->WriteCompressedSample(buffer, id_size, sample.record_buffer);
    lock.lock();
  }

  lock.unlock();

  if (!buffer.empty()) {
    if (buffer.size() > 100) {
      auto dz4 = std::make_unique<Dz4Block>();
      dz4->OrigBlockType("DT");
      dz4->Type(Dz4ZipType::Deflate);
      dz4->Data(buffer);
      auto& block_list = dl4->DataBlockList();
      block_list.push_back(std::move(dz4));
    } else {
      auto dt4 = std::make_unique<Dt4Block>();
      dt4->Data(buffer);
      auto& block_list = dl4->DataBlockList();
      block_list.push_back(std::move(dt4));
    }
    dl4->Offset(dz_count, offset_);
    ++dz_count;
    offset_ += buffer.size();
  }
  hl4->DataBlockList().push_back(std::move(dl4));


  dg4->Write(*writer_.file_); // Flush out data
  lock.lock();

  hl4->ClearData(); // Remove temp data
}
/*
void ConverterSampleQueue::ConverterThread() {
  do {
    // Wait on stop condition
    std::unique_lock lock(locker_);
    sample_event_.wait_for(lock, 10s);
    switch (write_state_) {
      case WriteState::Init: {
        TrimQueue();  // Purge the queue using pre-trig time
        break;
      }
      case WriteState::StartMeas: {
        //        MDF_TRACE() << "Start " << sample_queue_.size();
        SaveQueue(lock);  // Save the contents of the queue to file
        break;
      }

      case WriteState::StopMeas: {
        //        MDF_TRACE() << "Stop " << sample_queue_.size();
        CleanQueue(lock);
        break;
      }

      default:
        break;
    }
  } while (!stop_thread_);
  {
    //    MDF_TRACE() << "Stopping " << sample_queue_.size();
    {
      std::unique_lock lock(locker_);
      CleanQueue(lock);
    }
  }
}
 */
}  // namespace mdf::detail