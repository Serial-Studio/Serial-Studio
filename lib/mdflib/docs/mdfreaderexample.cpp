#include <mdf/mdfreader.h>
#include <mdf/ichannelgroup.h>
#include <mdf/idatagroup.h>

using namespace mdf;

int main() {

    MdfReader reader("c:/mdf/example.mf4");  // Note the file is now open.

    // Read all blocks but not the raw data and attachments.
    // This reads in the block information into memory.
    reader.ReadEverythingButData();

    const auto* mdf_file = reader.GetFile(); // Get the file interface.
    DataGroupList dg_list;                   // Get all measurements.
    mdf_file->DataGroups(dg_list);

    // In this example, we read in all sample data and fetch all values.
    for (auto* dg4 : dg_list) {
      // Subscribers holds the sample data for a channel.
      // You should normally only subscribe on some channels.
      // We need a list to hold them.
      ChannelObserverList subscriber_list;
      const auto cg_list = dg4->ChannelGroups();
      for (const auto* cg4 : cg_list ) {
        const auto cn_list = cg4->Channels();
        for (const auto* cn4 : cn_list) {
          // Create a subscriber and add it to the temporary list
          auto sub = CreateChannelObserver(*dg4, *cg4, *cn4);
          subscriber_list.emplace_back(std::move(sub));
        }
      }

      // Now it is time to read in all samples
      reader.ReadData(*dg4); // Read raw data from file
      double channel_value = 0.0; // Channel value (no scaling)
      double eng_value = 0.0; // Engineering value
      for (auto& obs : subscriber_list) {
        for (size_t sample = 0; sample < obs->NofSamples(); ++sample) {
          const auto channel_valid = obs->GetChannelValue(sample, channel_value);
          const auto eng_valid = obs->GetEngValue(sample, eng_value);
          // You should do something with data here
        }
      }

      // Not needed in this example as we delete the subscribers,
      // but it is good practise to remove samples data from memory
      // when it is no longer needed.
      dg4->ClearData();
    }
    reader.Close(); // Close the file

}
