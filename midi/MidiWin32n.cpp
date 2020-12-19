#include <Windows.h>
#include <Mmsystem.h>
#include "Midi.h"
#include "MidiWin32.h"

CMidi* CMidi::CreateMidi(void*p) { return new CMidiWin32(p); }

bool CMidiOutHW::MidiOutMsg(const std::basic_string<unsigned char>& data)
{
    if (data.length()<4) return !midiOutShortMsg(handle, *(unsigned*)data.c_str());
    bool ret = false;
    MIDIHDR midiHdr;
    midiHdr.lpData = (char*)data.c_str();
    midiHdr.dwBufferLength = data.length();
    midiHdr.dwFlags = 0;
    if (!midiOutPrepareHeader(handle, &midiHdr, sizeof(MIDIHDR))) {
        ret=!midiOutLongMsg(handle, &midiHdr, sizeof(MIDIHDR));
        while(MIDIERR_STILLPLAYING == midiOutUnprepareHeader(handle, &midiHdr, sizeof(MIDIHDR)));
    }
    return ret;
}

bool CMidiOutSW::MidiOutMsg(const std::basic_string<unsigned char>& data)
{
    if (data.length()<4) return !midiOutShortMsg(handle, *(unsigned*)data.c_str());
    bool ret = false;
    MIDIHDR midiHdr;
    midiHdr.lpData = (char*)data.c_str();
    midiHdr.dwBufferLength = data.length();
    midiHdr.dwFlags = 0;
    if (!midiOutPrepareHeader(handle, &midiHdr, sizeof(MIDIHDR))) {
        ret=!midiOutLongMsg(handle, &midiHdr, sizeof(MIDIHDR));
        while(MIDIERR_STILLPLAYING == midiOutUnprepareHeader(handle, &midiHdr, sizeof(MIDIHDR)));
    }
    return ret;
}

std::vector<str_type> CMidiWin32::MidiOutList()
{
    MIDIOUTCAPS caps;
    std::vector<str_type> v;
    int n = midiOutGetNumDevs();
    for (int i = 0; i < n; i++)
        if (!midiOutGetDevCaps(i, &caps, sizeof(MIDIOUTCAPS))) v.push_back(caps.szPname);
    return v;
}

std::vector<str_type> CMidiWin32::MidiInList()
{
    MIDIINCAPS caps;
    std::vector<str_type> v;
    int n = midiInGetNumDevs();
    for (int i = 0; i < n; i++)
        if (!midiInGetDevCaps(i, &caps, sizeof(MIDIINCAPS))) v.push_back(caps.szPname);
    return v;
}

std::vector<std::wstring> GetInfo(void*p);

std::vector<std::wstring> CMidiWin32::MidiOutInfo(int n)
{
    MIDIOUTCAPS2 caps;
    if (!midiOutGetDevCaps(n, (MIDIOUTCAPS*)&caps, sizeof(MIDIOUTCAPS2))) return GetInfo(&caps);
    return std::vector<std::wstring>();
}

std::vector<std::wstring> CMidiWin32::MidiOutInfo(const wchar_t*name)
{
    MIDIOUTCAPS2 caps;
    int n = midiOutGetNumDevs();
    std::wstring s = name;
    for (int i = 0; i < n; i++) if (!midiOutGetDevCaps(i, (MIDIOUTCAPS*)&caps, sizeof(MIDIOUTCAPS2)) && s == caps.szPname) return GetInfo(&caps);
    return std::vector<std::wstring>();
}

std::vector<std::wstring> CMidiWin32::MidiInInfo(int n)
{
    MIDIINCAPS2 caps;
    if (!midiInGetDevCaps(n, (MIDIINCAPS*)&caps, sizeof(MIDIINCAPS2))) return GetInfo(&caps);
    return std::vector<std::wstring>();
}

std::vector<std::wstring> CMidiWin32::MidiInInfo(const wchar_t*name)
{
    MIDIINCAPS2 caps;
    int n = midiInGetNumDevs();
    std::wstring s = name;
    for (int i = 0; i < n; i++) if (!midiInGetDevCaps(i, (MIDIINCAPS*)&caps, sizeof(MIDIINCAPS2)) && s == caps.szPname) return GetInfo(&caps);
    return std::vector<std::wstring>();
}

str_type CMidiWin32::MidiOutOpen(int n)
{
    MIDIOUTCAPS caps;
    if (midiOutGetDevCaps(n, &caps, sizeof(MIDIOUTCAPS))) return CurrentOutName();
    str_type s = caps.szPname;
    if (s == CurrentOutName()) return CurrentOutName();
    HMIDIOUT h;
    if (!n) {
        if(!midiOutOpen(&h, n, 0, 0, CALLBACK_NULL)) SetOut(new CMidiOutSW(h, s.c_str()));
        return CurrentOutName();
    }
    if (!midiOutOpen(&h, n, 0, 0, CALLBACK_NULL)) SetOut(new CMidiOutHW(h, s.c_str()));
    return CurrentOutName();
}

str_type CMidiWin32::MidiOutOpen(const char_type*name)
{
    MIDIOUTCAPS caps;
    if (CurrentOutName() == name) return CurrentOutName();
    int n = midiOutGetNumDevs();
    int i;
    for (i = 0; i < n; i++) {
        if(midiOutGetDevCaps(i, &caps, sizeof(MIDIOUTCAPS))) continue;
        if (str_type(name) == caps.szPname) break;
    }
    if (i == n) return CurrentOutName();
    HMIDIOUT h;
    if (!i) {
        if(!midiOutOpen(&h, i, 0, 0, CALLBACK_NULL)) SetOut(new CMidiOutSW(h, name));
        return CurrentOutName();
    }
    if (!midiOutOpen(&h, i, 0, 0, CALLBACK_NULL)) SetOut(new CMidiOutHW(h, name));
    return CurrentOutName();
}

void PluginCallback(void*, void*);

struct CMidiInData
{
    void* plugin;
    HMIDIIN handle;
    UINT wMsg;
    DWORD_PTR dwParam1;
    DWORD_PTR dwParam2;
    CMidiInData(void*p, HMIDIIN h, UINT m, DWORD_PTR p1, DWORD_PTR p2) : plugin(p), handle(h), wMsg(m), dwParam1(p1), dwParam2(p2) {}
};

static void CALLBACK MidiInCallback(HMIDIIN handle, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    if (wMsg == MM_MIM_OPEN || wMsg == MM_MIM_CLOSE) return;
    if (wMsg == MIM_LONGDATA) {
        MIDIHDR* hdr=(MIDIHDR*)dwParam1;
        if (!hdr->dwBytesRecorded) return;
    }
    PluginCallback((void*)dwInstance, (void*)new CMidiInData((void*)dwInstance, handle,wMsg, dwParam1, dwParam2));
}

void CMidiInHW::ReadMidiInput(void*p, std::vector<unsigned char>&v)
{
    CMidiInData* data = (CMidiInData*)p;
    if (data->wMsg == MIM_DATA) {
        v.push_back(data->dwParam1 & 0xff);
        v.push_back((data->dwParam1 >> 8) & 0xff);
        v.push_back((data->dwParam1 >> 16) & 0xff);
    }
    else if (data->wMsg == MIM_LONGDATA) {
        MIDIHDR* hdr=(MIDIHDR*)data->dwParam1;
        for (size_t i = 0; i < hdr->dwBytesRecorded; i++) {
            v.push_back(hdr->lpData[i]);
        }
        midiInAddBuffer(data->handle, hdr, sizeof(MIDIHDR));
    }
    else if (data->wMsg!=MM_MIM_OPEN && data->wMsg!=MM_MIM_CLOSE) {
        char buff[256]; sprintf(buff, "MIDI message: %x", data->wMsg); MessageBoxA(0, buff, "Jazz-Plugin", IDOK);
    }
    delete data;
}

std::wstring CMidiWin32::MidiInOpen(int n, void*p)
{
    MIDIINCAPS caps;
    if (midiInGetDevCaps(n, &caps, sizeof(MIDIINCAPS))) return CurrentInName();
    std::wstring s = caps.szPname;
    if (s == CurrentInName()) return CurrentInName();
    HMIDIIN h;
    if (!midiInOpen(&h, n, (DWORD_PTR)MidiInCallback, (DWORD_PTR)p, CALLBACK_FUNCTION)) {
        SetIn(new CMidiInHW(h, s.c_str()));
    }
    return CurrentInName();
}

std::wstring CMidiWin32::MidiInOpen(const wchar_t*name, void*p)
{
    MIDIINCAPS caps;
    if (CurrentInName() == name) return CurrentInName();
    int n = midiInGetNumDevs();
    int i;
    for (i=0; i<n; i++) {
        if(midiInGetDevCaps(i, &caps, sizeof(MIDIINCAPS))) continue;
        if (std::wstring(name) == caps.szPname) break;
    }
    if (i == n) return CurrentInName();
    HMIDIIN h;
    if (!midiInOpen(&h, i, (DWORD_PTR)MidiInCallback, (DWORD_PTR)p, CALLBACK_FUNCTION)) {
        SetIn(new CMidiInHW(h, name));
    }
    return CurrentInName();
}

void CMidiInHW::Start()
{
    for (int i = 0; i < MIDIINNUM; i++) {
        bfr[i]=new char[MIDIINSIZE];
        hdr[i].lpData=bfr[i];
        hdr[i].dwBufferLength=MIDIINSIZE;
        hdr[i].dwFlags = 0;
        midiInPrepareHeader(handle, &hdr[i], sizeof(MIDIHDR));
        midiInAddBuffer(handle, &hdr[i], sizeof(MIDIHDR));
    }
    midiInStart(handle);
}

void CMidiInHW::Stop()
{
    midiInStop(handle);
    midiInReset(handle);
    for (int i = 0; i < MIDIINNUM; i++) {
        midiInUnprepareHeader(handle, &hdr[i], sizeof(MIDIHDR));
        delete[] bfr[i];
    }
    midiInClose(handle);
}

struct ThreadParam
{
    void(*func)(CMidi*);
    CMidi* ptr;
    ThreadParam(void(*f)(CMidi*), CMidi*p) : func(f), ptr(p) {}
};

DWORD WINAPI ThreadWrapper(LPVOID p) 
{
    ThreadParam* tp = (ThreadParam*)p;
    tp->func(tp->ptr);
    delete tp; return 0;
}

void CMidiWin32::StartThread(void(*func)(CMidi*)) { CreateThread(0, 0,ThreadWrapper, new ThreadParam(func,this), 0, 0); }
void CMidiWin32::Sleep(int ms) { ::Sleep(ms); }
