// fuzzytest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string_view>
#include <mdf/mdfreader.h>
#include <mdf/ichannelgroup.h>
#include <mdf/idatagroup.h>

using namespace mdf;

int main()
{
  constexpr std::string_view kTestFile = "K:/test/mdf/crashfile.mf4";  
    MdfReader reader(kTestFile.data());  // Open the specified file

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

    return 0;
}


// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
