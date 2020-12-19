class CMidiLockWin : public CMidiLock
{
    HANDLE m_Mx;
public:
    CMidiLockWin() { m_Mx = CreateMutex(0, 0, 0); }
    ~CMidiLockWin() { CloseHandle(m_Mx); }
    void Lock() { WaitForSingleObject(m_Mx, INFINITE); }
    void Unlock() { ReleaseMutex(m_Mx); }
};

class CMidiWin32 : public CMidi
{
    HANDLE m_DeqMx;
    HANDLE m_ConnDeqMx;
public:
    HWND m_hWnd;
    unsigned long m_StartTime;
    CMidiWin32(void*p) : CMidi(p) { m_StartTime=timeGetTime(); m_DeqMx=CreateMutex(0,0,0); m_ConnDeqMx=CreateMutex(0,0,0); }
    ~CMidiWin32() { MidiInClose(); MidiOutClose(); ClearDeq(); CloseHandle(m_DeqMx); CloseHandle(m_ConnDeqMx); }
    unsigned long Time() { return timeGetTime()-m_StartTime; }
    int MidiOut(unsigned char, unsigned char, unsigned char);
    std::vector<str_type> MidiOutList();
    std::vector<str_type> MidiInList();
    std::vector<str_type> MidiOutInfo(int);
    std::vector<str_type> MidiOutInfo(const char_type*);
    std::vector<str_type> MidiInInfo(int);
    std::vector<str_type> MidiInInfo(const char_type*);
    str_type MidiOutOpen(int);
    str_type MidiOutOpen(const char_type*);
    str_type MidiInOpen(int, void*);
    str_type MidiInOpen(const char_type*, void*);
    void LockDeq() { WaitForSingleObject(m_DeqMx,INFINITE); }
    void UnlockDeq() { ReleaseMutex(m_DeqMx); }
    void LockConnDeq() { WaitForSingleObject(m_ConnDeqMx,INFINITE); }
    void UnlockConnDeq() { ReleaseMutex(m_ConnDeqMx); }
    void StartThread(void(*)(CMidi*));
    void Sleep(int);
    CMidiLock* CreateLock() { return new CMidiLockWin; }
};

class CMidiOutHW : public CMidiOut
{
friend class CMidiWin32;
protected:
    HMIDIOUT handle;
    CMidiOutHW(HMIDIOUT h,const char_type* n) : handle(h) { name=n; }
    ~CMidiOutHW() { midiOutClose(handle); }
    bool MidiOutMsg(const std::basic_string<unsigned char>& data);
};

class CMidiOutSW : public CMidiOut
{
friend class CMidiWin32;
protected:
    HMIDIOUT handle;
    CMidiOutSW(HMIDIOUT h, const char_type* n) : handle(h) { name = n; }
    ~CMidiOutSW() { midiOutClose(handle); }
    bool MidiOutMsg(const std::basic_string<unsigned char>& data);
};

#define MIDIINNUM 4
#define MIDIINSIZE 256
class CMidiInHW : public CMidiIn
{
friend class CMidiWin32;
protected:
    HMIDIIN handle;
    MIDIHDR hdr[MIDIINNUM];
    char* bfr[MIDIINNUM];
    CMidiInHW(HMIDIIN h, const char_type* n) : handle(h) { name = n; }
    ~CMidiInHW() { Stop(); }
    void Start();
    void Stop();
    virtual void ReadMidiInput(void*, std::vector<unsigned char>&);
};

struct CAsyncMidiInData
{	CMidiWin32* obj;
    unsigned long time;
    void* data;
    CAsyncMidiInData(CMidiWin32* o, unsigned long t, void* p) : obj(o), time(t), data(p) {}
};
