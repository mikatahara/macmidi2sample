#include <iostream>
#include <iomanip>
#include <CoreMIDI/CoreMIDI.h>

// MIDI 2.0受信コールバック
static void processMIDIEvent(
    const MIDIEventList *eventList)
{
    if (eventList == nullptr)
        return;

    const MIDIEventPacket *packet =
        &eventList->packet[0];

    for (UInt32 i = 0;
         i < eventList->numPackets;
         ++i)
    {
        for (UInt32 w = 0;
             w < packet->wordCount;
             ++w)
        {
            uint32_t word = packet->words[w];

            std::cout
                << "0x"
                << std::hex
                << std::setw(8)
                << std::setfill('0')
                << word
                << std::dec
                << " ";
        }
        std::cout
            << std::endl;

        packet = MIDIEventPacketNext(packet);
    }
}

int main()
{
    MIDIClientRef client = 0;
    MIDIPortRef inputPort = 0;

    OSStatus status;

    // MIDI Clientを作成
    status = MIDIClientCreateWithBlock(
        CFSTR("UMP Receiver"),
        &client,
        nullptr);

    if (status != noErr)
    {
        std::cerr
            << "MIDIClientCreateWithBlock failed: "
            << status << std::endl;
        return 1;
    }

    // MIDI 2.0用Input Port
    status = MIDIInputPortCreateWithProtocol(
        client,
        CFSTR("UMP Input"),
        kMIDIProtocol_2_0,
        &inputPort,
        ^(const MIDIEventList *eventList,
          void *srcConnRefCon) {
          processMIDIEvent(eventList);
        });

    if (status != noErr)
    {
        std::cerr
            << "MIDIInputPortCreateWithProtocol failed: "
            << status << std::endl;
        return 1;
    }

    // MIDI Sourceの数を取得
    ItemCount sourceCount =
        MIDIGetNumberOfSources();

    std::cout
        << "MIDI Sources: "
        << sourceCount
        << std::endl;

    for (ItemCount i = 0; i < sourceCount; ++i)
    {

        MIDIEndpointRef source = MIDIGetSource(i);

        CFStringRef name = nullptr;

        MIDIObjectGetStringProperty(
            source,
            kMIDIPropertyName,
            &name);

        char nameBuffer[256] = {};

        if (name != nullptr)
        {
            CFStringGetCString(
                name,
                nameBuffer,
                sizeof(nameBuffer),
                kCFStringEncodingUTF8);

            CFRelease(name);
        }

        std::cout
            << "[" << i << "] "
            << nameBuffer
            << std::endl;
    }

    // Source番号を入力
    int isourcenum;

    std::cout << "Select MIDI Source: ";
    std::cin >> isourcenum;
    {
        
        MIDIEndpointRef source = MIDIGetSource(isourcenum);

        // 最初のMIDI Sourceに接続
        status = MIDIPortConnectSource(
            inputPort,
            source,
            nullptr);

        if (status != noErr)
        {
            std::cerr
                << "MIDIPortConnectSource failed: "
                << status << std::endl;
            return 0;
        }
    }

    std::cout << std::endl;
    std::cout << "Waiting for MIDI 2.0 UMP..." << std::endl;
    std::cout << "Press Ctrl+C to exit." << std::endl;

    // プロセスを終了させない
    while (true)
    {
        CFRunLoopRunInMode(
            kCFRunLoopDefaultMode,
            1.0,
            false);
    }

    return 0;
}
