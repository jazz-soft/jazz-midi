#include <Carbon/Carbon.h>
#include <CoreAudio/CoreAudio.h>
#include <AudioToolbox/AudioToolbox.h>
#include <sys/time.h>
#include <sstream>
#include "Midi.h"
#include "MidiMacOSX.h"

#include <iostream>

static std::string DefaultOut = "Apple DLS Synth";

CMidi* CMidi::CreateMidi(void* p) { return new CMidiMacOSX(p); }

//static void dummy(const MIDINotification*, void*) {}

CMidiMacOSX::CMidiMacOSX(void* p) : CMidi(p), midi(0)
{
    gettimeofday(&m_StartTime, 0);
    pthread_mutex_init(&m_DeqMx, 0); pthread_mutex_init(&m_ConnDeqMx, 0);
    //MIDIClientRef client; MIDIClientCreate(CFSTR("hack"), dummy, NULL, &client);
}


CMidiOutSW::CMidiOutSW()
{
    AUNode synthNode, outNode;
    NewAUGraph(&graph);
    AudioComponentDescription cd;
    cd.componentManufacturer = kAudioUnitManufacturer_Apple;
    cd.componentFlags = 0;
    cd.componentFlagsMask = 0;
    cd.componentType = kAudioUnitType_MusicDevice;
    cd.componentSubType = kAudioUnitSubType_DLSSynth;
    AUGraphAddNode (graph, &cd, &synthNode);
    cd.componentType = kAudioUnitType_Output;
    cd.componentSubType = kAudioUnitSubType_DefaultOutput;
    AUGraphAddNode (graph, &cd, &outNode);
    AUGraphOpen(graph);
    AUGraphConnectNodeInput(graph, synthNode, 0 ,outNode, 0);
    AUGraphInitialize(graph);
    AUGraphStart (graph);
    AUGraphNodeInfo(graph, synthNode, 0, &synth);
    AUGraphNodeInfo(graph, outNode, 0, &out);
    name=fromUtf8(DefaultOut);
}


CMidiOutSW::~CMidiOutSW()
{
    AUGraphStop(graph);
    DisposeAUGraph(graph);
}


CMidiOutHW::~CMidiOutHW()
{
    if(port) MIDIPortDispose(port);
}


CMidiInHW::~CMidiInHW()
{
    if (port) MIDIPortDispose(port);
}


void CMidiOutHW::MidiOut(unsigned char a, unsigned char b, unsigned char c)
{
    buff.clear();
    std::basic_string<unsigned char> data;
    int len = CMidi::MidiLen(a);
    data += a; if (len > 1) data += b; if (len > 2) data += c;
    MidiOutMsg(data);
}


bool CMidiOutHW::MidiOutMsg(const std::basic_string<unsigned char>& data)
{
    MIDITimeStamp timestamp = 0;
    int blen=data.length() + 64;
    Byte buffer[blen];
    MIDIPacketList *packetlist = (MIDIPacketList*)buffer;
    MIDIPacket *packet = MIDIPacketListInit(packetlist);
    packet = MIDIPacketListAdd(packetlist, blen, packet, timestamp, data.length(), data.c_str());
    MIDISend(port, dest, packetlist);
    return true;
}


bool CMidiOutSW::MidiOutMsg(const std::basic_string<unsigned char>& data)
{
    bool ret = false;
    if(data.length() < 4) {
        ret |= (!MusicDeviceMIDIEvent(synth, data[0], data.length() > 1? data[1] : 0, data.length() > 2 ? data[2] : 0, 0));
    }
    else if(data.length() == 8 && data[0] == 0xF0 && data[1] == 0x7F && data[2] == 0x7F && data[3] == 0x04 && data[4] == 0x01 && data[7] == 0xF7) {
        // Set volume SysEx
        unsigned x = data[6]; x <<= 7; x += data[5];
        double y = x; y /= 0x3fff;
        ret |= (!AudioUnitSetParameter(out, kHALOutputParam_Volume, kAudioUnitScope_Global, 0, y, 0));
    }
    else {
        ret |= (!MusicDeviceSysEx(synth, data.c_str(), data.length()));
    }
    return ret;
}


str_type toStr(CFStringRef r)
{
    str_type s;
    for (CFIndex i = 0; i<CFStringGetLength(r); i++) s += CFStringGetCharacterAtIndex(r, i);
    return s;
}


std::string toStdStr(CFStringRef r)
{
    std::string s;
    for (CFIndex i = 0; i<CFStringGetLength(r); i++) s += (char)CFStringGetCharacterAtIndex(r, i);
    return s;
}


std::vector<str_type> CMidiMacOSX::MidiOutList()
{
    CFStringRef S;
    std::vector<str_type> v;
    v.push_back(fromUtf8(DefaultOut));
    //CFRunLoopRef ref = CFRunLoopGetCurrent();
    ItemCount k = MIDIGetNumberOfDestinations();
    for (ItemCount i = 0; i < k; ++i) {
        MIDIEndpointRef device = MIDIGetDestination(i);
        MIDIObjectGetStringProperty(device, kMIDIPropertyDisplayName, &S);
        if (S) v.push_back(toStr(S));
    }
    return v;
}


std::vector<str_type> CMidiMacOSX::MidiInList()
{
    CFStringRef S;
    std::vector<str_type> v;
    CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, true);
    ItemCount k = MIDIGetNumberOfSources();
    for(ItemCount i = 0 ; i < k ; ++i) {
        MIDIEndpointRef device = MIDIGetSource(i);
        MIDIObjectGetStringProperty(device, kMIDIPropertyDisplayName, &S);
        if (S) v.push_back(toStr(S));
    }
    return v;
}


std::vector<str_type> GetDefaultInfo()
{
    std::vector<str_type> v;
    v.push_back(fromUtf8(DefaultOut));
    v.push_back(fromUtf8("Apple"));
    v.push_back(fromUtf8("1.0"));
    return v;
}


std::vector<str_type> GetInfo(MIDIEndpointRef& device)
{
    std::vector<str_type> v;
    CFStringRef S;
    MIDIObjectGetStringProperty(device, kMIDIPropertyDisplayName, &S);
    v.push_back(toStr(S));
    if (!MIDIObjectGetStringProperty(device, kMIDIPropertyManufacturer, &S)) v.push_back(toStr(S));
    else v.push_back(fromUtf8("Unknown"));
    SInt32 N = 0;
    MIDIObjectGetIntegerProperty(device, kMIDIPropertyDriverVersion, &N);
    std::wostringstream SS;
    if (!N || N>=256) SS << (N >> 8) << '.' << (N & 0xff);
    else SS << N << ".0";
    v.push_back(SS.str());
    return v;
}


std::vector<str_type> CMidiMacOSX::MidiOutInfo(int n)
{
    if (!n) return GetDefaultInfo();
    n--;
    MIDIEndpointRef device = MIDIGetDestination(n);
    if (device) return GetInfo(device);
    return std::vector<str_type>();
}


std::vector<str_type> CMidiMacOSX::MidiOutInfo(const char_type* name)
{
    if (fromUtf8(DefaultOut) == name) return GetDefaultInfo();
    ItemCount k = MIDIGetNumberOfDestinations();
    CFStringRef S;
    for (size_t i = 0; i < k; ++i) {
        MIDIEndpointRef device = MIDIGetDestination(i);
        MIDIObjectGetStringProperty(device, kMIDIPropertyDisplayName, &S);
        if (toStr(S) == name) return GetInfo(device);
    }
    return std::vector<str_type>();
}


std::vector<str_type> CMidiMacOSX::MidiInInfo(int n)
{
    MIDIEndpointRef device = MIDIGetSource(n);
    if (device) return GetInfo(device);
    return std::vector<str_type>();
}


std::vector<str_type> CMidiMacOSX::MidiInInfo(const char_type* name)
{
    ItemCount k = MIDIGetNumberOfSources();
    CFStringRef S;
    for (size_t i = 0; i < k; ++i) {
        MIDIEndpointRef device = MIDIGetSource(i);
        MIDIObjectGetStringProperty(device, kMIDIPropertyDisplayName, &S);
        if (toStr(S) == name) return GetInfo(device);
    }
    return std::vector<str_type>();
}


str_type CMidiMacOSX::MidiOutOpen(int n)
{
    if (n < 0 || n > MIDIGetNumberOfDestinations()) return CurrentOutName();
    if (!n) {
        SetOut(new CMidiOutSW());
        return CurrentOutName();
    }
    n--;
    
    CFStringRef S;
    MIDIPortRef port;
    MIDIEndpointRef dest = MIDIGetDestination(n);
    MIDIObjectGetStringProperty(dest, kMIDIPropertyDisplayName, &S);
    if (CurrentOutName() == toStr(S)) return CurrentOutName();
    if (!midi) MIDIClientCreate(CFSTR("Midi"), 0, 0, &midi);
    MIDIOutputPortCreate(midi,CFSTR("port"), &port);
    SetOut(new CMidiOutHW(port, dest, toStr(S).c_str()));
    return CurrentOutName();
}


str_type CMidiMacOSX::MidiOutOpen(const char_type* name)
{
    if(CurrentOutName() == name) return CurrentOutName();
    if (fromUtf8(DefaultOut) == name) {
        SetOut(new CMidiOutSW());
        return CurrentOutName();
    }

    CFStringRef S;
    MIDIPortRef port;
    MIDIEndpointRef dest;
    ItemCount i;
    ItemCount k = MIDIGetNumberOfDestinations();
    for (i = 0; i < k; ++i) {
        dest = MIDIGetDestination(i);
        MIDIObjectGetStringProperty(dest, kMIDIPropertyDisplayName, &S);
        if (toStr(S) == name) break;
    }
    if(i == k) return CurrentOutName();

    if (!midi) MIDIClientCreate(CFSTR("Midi"), 0, 0, &midi);
    MIDIOutputPortCreate(midi,CFSTR("port"), &port);
    SetOut(new CMidiOutHW(port, dest, name));
    return CurrentOutName();
}


void PluginCallback(void*, void*);


void MidiInProc(const MIDIPacketList* packetList, void* p,void*)
{
    MIDIPacket *packet = (MIDIPacket*)packetList->packet;
    int count = packetList->numPackets;
    std::vector<unsigned char> Q;
    unsigned char RS = 0;
    for (int i = 0; i < count; i++) {
        for (int j = 0; j < packet->length; j++) {
            if (CMidiIn::Push(packet->data[j], Q, RS)) {
                std::vector<unsigned char>* v = new std::vector<unsigned char>(Q);
                Q.clear();
                PluginCallback((void*)p, (void*)v);
            }
        }
        packet = MIDIPacketNext(packet);
    }
}


void CMidiInHW::ReadMidiInput(void* data, std::vector<unsigned char>& v)
{
    std::vector<unsigned char>* d = (std::vector<unsigned char>*)data;
    for (size_t i = 0; i < (*d).size(); i++) v.push_back((*d)[i]);
    delete d;
}


str_type CMidiMacOSX::MidiInOpen(int n, void* p)
{
    if (n < 0 || n >= MIDIGetNumberOfSources()) return CurrentInName();
    CFStringRef S;
    MIDIPortRef port;
    MIDIEndpointRef src=MIDIGetSource(n);
    MIDIObjectGetStringProperty(src, kMIDIPropertyDisplayName, &S);
    if (CurrentInName() == toStr(S)) return CurrentInName();
    if (!midi) MIDIClientCreate(CFSTR("Midi"), 0, 0, &midi);
    MIDIInputPortCreate(midi, CFSTR("input"), MidiInProc, p, &port);
    MIDIPortConnectSource(port, src, 0);
    SetIn(new CMidiInHW(port, src, toStr(S).c_str()));
    return CurrentInName();
}


str_type CMidiMacOSX::MidiInOpen(const char_type* name, void* p)
{
    if (CurrentInName() == name) return CurrentInName();
    CFStringRef S;
    MIDIPortRef port;
    MIDIEndpointRef src;
    ItemCount i;
    ItemCount k = MIDIGetNumberOfSources();
    for (i = 0; i < k; ++i) {
        src = MIDIGetSource(i);
        MIDIObjectGetStringProperty(src, kMIDIPropertyDisplayName, &S);
        if(toStr(S) == name) break;
    }
    if (i == k) return CurrentInName();
    if (!midi) MIDIClientCreate(CFSTR("Midi"), 0, 0, &midi);
    MIDIInputPortCreate(midi, CFSTR("input"), MidiInProc, p, &port);
    MIDIPortConnectSource(port, src, 0);
    SetIn(new CMidiInHW(port, src, name));
    return CurrentInName();
}


struct ThreadParam
{
    void(*func)(CMidi*);
    CMidi* ptr;
    ThreadParam(void(*f)(CMidi*), CMidi*p) : func(f), ptr(p) {}
};


void* ThreadWrapper(void* p)
{
    //CFRunLoopRef ref = CFRunLoopGetCurrent();
    ThreadParam* tp = (ThreadParam*)p; tp->func(tp->ptr); delete tp; return 0;
}


void CMidiMacOSX::StartThread(void(*func)(CMidi*)){ pthread_t t; pthread_create(&t, 0, ThreadWrapper, new ThreadParam(func,this));}

void CMidiMacOSX::Sleep(int ms){ usleep(ms);}
