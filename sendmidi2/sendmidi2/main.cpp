#include <iostream>
#include <iomanip>
#include <CoreMIDI/CoreMIDI.h>
#include <CoreFoundation/CoreFoundation.h>

int main()
{
    MIDIClientRef client = 0;
    MIDIPortRef outputPort = 0;

    OSStatus status;

    // MIDI Clientを作成
    status = MIDIClientCreate(
        CFSTR("UMP Sender"),
        nullptr,
        nullptr,
        &client);

    if (status != noErr)
    {
        std::cerr
            << "MIDIClientCreate failed: "
            << status << std::endl;
        return 1;
    }

    // MIDI 2.0 Output Portを作成
    MIDIOutputPortCreate(
        client,
        CFSTR("UMP Output"),
        &outputPort);

    if (status != noErr)
    {
        std::cerr
            << "MIDIOutputPortCreate failed: "
            << status << std::endl;
        return 1;
    }

    // MIDI Destinationの数を取得
    ItemCount destinationCount =
        MIDIGetNumberOfDestinations();

    if (destinationCount == 0)
    {
        std::cerr
            << "No MIDI destination found."
            << std::endl;
        return 1;
    }

    // MIDI Destination一覧を表示
    for (ItemCount i = 0;
         i < destinationCount;
         ++i)
    {
        MIDIEndpointRef destination =
            MIDIGetDestination(i);

        CFStringRef name = nullptr;

        MIDIObjectGetStringProperty(
            destination,
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

    // Destinationを選択
    int destinationNumber;

    std::cout << "Select MIDI Destination: ";
    std::cin >> destinationNumber;

    if (destinationNumber < 0 ||
        destinationNumber >=
            static_cast<int>(destinationCount))
    {
        std::cerr
            << "Invalid MIDI Destination."
            << std::endl;
        return 1;
    }

    MIDIEndpointRef destination =
        MIDIGetDestination(destinationNumber);

    // --------------------------------
    // MIDI 2.0 UMPを作成
    // --------------------------------

    uint32_t ump[2];

    // MIDI 2.0 Note On
    //
    // MT      = 4
    // Group   = 0
    // Status  = 9 (Note On)
    // Channel = 0
    // Note    = 60
    //
    // Velocity = 0xFFFF
    //
    ump[0] =
        0x40903C00;

    ump[1] =
        0xFFFF0000;

    // UMPを表示
    std::cout
        << "Sending UMP: 0x"
        << std::hex
        << std::setw(8)
        << std::setfill('0')
        << ump[0]
        << " "
        << std::setw(8)
        << ump[1]
        << std::dec
        << std::endl;

    // --------------------------------
    // MIDIEventListを作成
    // --------------------------------

    MIDIEventList eventList = {};

    MIDIEventPacket *packet =
        MIDIEventListInit(
            &eventList,
            kMIDIProtocol_2_0);

    packet = MIDIEventListAdd(
        &eventList,
        sizeof(eventList),
        packet,
        0,
        2,
        ump);

    if (packet == nullptr)
    {
        std::cerr
            << "MIDIEventListAdd failed."
            << std::endl;
        return 1;
    }

    // --------------------------------
    // 送信
    // --------------------------------

    status = MIDISendEventList(
        outputPort,
        destination,
        &eventList);

    if (status != noErr)
    {
        std::cerr
            << "MIDISendEventList failed: "
            << status
            << std::endl;
        return 1;
    }

    std::cout
        << "MIDI 2.0 UMP sent."
        << std::endl;

    // 後片付け
    MIDIPortDispose(outputPort);
    MIDIClientDispose(client);

    return 0;
}
