#include <vector>
#include <string>
#include <deque>
#include "char_type.h"

class CMidiLock
{
public:
    virtual ~CMidiLock() {}
    virtual void Lock() = 0;
    virtual void Unlock() = 0;
};


class CMidiIn
{
friend class CMidi;
protected:
    str_type name;
    virtual ~CMidiIn() {}
    virtual void ReadMidiInput(void*, std::vector<unsigned char>&) = 0;
    virtual void Start() {};
public:
    static bool Push(unsigned char, std::vector<unsigned char>&, unsigned char&);
};


class CMidiOut
{
friend class CMidi;
protected:
    str_type name;
    std::basic_string<unsigned char> buff;
    virtual ~CMidiOut() {}
    virtual void MidiOut(unsigned char, unsigned char, unsigned char);
    virtual bool MidiOutLong(const std::basic_string<unsigned char>&);
    virtual bool MidiOutRaw(const std::basic_string<unsigned char>&);
    virtual bool MidiOutMsg(const std::basic_string<unsigned char>&) = 0;
    virtual void Stop() { for (unsigned char c = 0xb0; c < 0xc0; c++) MidiOut(c, 123, 0); }	// all notes off
};


struct CMidiMsg
{
    unsigned long T;
    std::vector<unsigned char> V;
};


class CMidi
{
protected:
    void* Owner;
    CMidiIn* In;
    CMidiOut* Out;
    std::vector<str_type> m_OutList;
    std::vector<str_type> m_InList;
public:
    static CMidi* CreateMidi(void*);
    static size_t MidiLen(unsigned char);
    CMidi(void*p) : Owner(p), In(0), Out(0) {}
    virtual ~CMidi() {}
    virtual std::vector<str_type> MidiOutList() = 0;
    virtual std::vector<str_type> MidiInList() = 0;
    virtual std::vector<str_type> MidiOutInfo(int) = 0;
    virtual std::vector<str_type> MidiOutInfo(const char_type*) = 0;
    virtual std::vector<str_type> MidiInInfo(int) = 0;
    virtual std::vector<str_type> MidiInInfo(const char_type*) = 0;
    virtual str_type MidiOutOpen(int) = 0;
    virtual str_type MidiOutOpen(const char_type*) = 0;
    virtual str_type MidiInOpen(int, void*) = 0;
    virtual str_type MidiInOpen(const char_type*, void*) = 0;
    void MidiOut(unsigned char a1, unsigned char a2, unsigned char a3) { if (!Out) MidiOutOpen(0); if (Out) Out->MidiOut(a1, a2, a3); }
    void MidiOutClose() { if (Out) { Out->Stop(); delete Out; Out = 0; }}
    void MidiInClose() { if (In) { delete In; In = 0; }}
    str_type CurrentOutName() { return Out ? Out->name : L""; }
    str_type CurrentInName() { return In ? In->name : L""; }
    void SetOut(CMidiOut*p) { MidiOutClose(); Out = p; }
    void SetIn(CMidiIn*p) { MidiInClose(); In = p; p->Start(); }
    virtual unsigned long Time() = 0;
    bool MidiOutLong(const std::basic_string<unsigned char>& s) { return Out ? Out->MidiOutLong(s) : 0; }
    bool MidiOutRaw(const std::basic_string<unsigned char>& s) { return Out ? Out->MidiOutRaw(s) : 0; }
    void ReadMidiInput(void* p, std::vector<unsigned char>& v) { if (In) In->ReadMidiInput(p, v); }
    virtual void StartThread(void(*fun)(CMidi*)) = 0;
    virtual void Sleep(int) = 0;
    virtual CMidiLock* CreateLock() = 0;
};
