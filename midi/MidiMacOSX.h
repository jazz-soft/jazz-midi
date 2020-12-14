class CMidiLockLnx : public CMidiLock
{
    pthread_mutex_t m_Mx;
public:
    CMidiLockLnx(){ pthread_mutex_init(&m_Mx, 0);}
    ~CMidiLockLnx(){ pthread_mutex_destroy(&m_Mx);}
    void Lock(){ pthread_mutex_lock(&m_Mx);}
    void Unlock(){ pthread_mutex_unlock(&m_Mx);}
};

class CMidiMacOSX : public CMidi
{
    pthread_mutex_t m_DeqMx;
    pthread_mutex_t m_ConnDeqMx;
public:
    timeval m_StartTime;
    MIDIClientRef midi;
    CMidiMacOSX(void* p);
    ~CMidiMacOSX(){ MidiInClose(); MidiOutClose(); ClearDeq(); if(midi) MIDIClientDispose(midi); pthread_mutex_destroy(&m_DeqMx); pthread_mutex_destroy(&m_ConnDeqMx);}
    unsigned long Time(){ timeval t; gettimeofday(&t, 0); unsigned long z = (t.tv_sec-m_StartTime.tv_sec)*1000; z += (t.tv_usec-m_StartTime.tv_usec)/1000; return z;}
    std::vector<str_type> MidiOutList();
    std::vector<str_type> MidiInList();
    std::vector<str_type> MidiOutInfo(int);
    std::vector<str_type> MidiOutInfo(const char_type*);
    std::vector<str_type> MidiInInfo(int);
    std::vector<str_type> MidiInInfo(const char_type*);
    str_type MidiOutOpen(int);
    str_type MidiOutOpen(const char_type*);
    str_type MidiInOpen(int,void*);
    str_type MidiInOpen(const char_type*,void*);
    void LockDeq(){ pthread_mutex_lock(&m_DeqMx);}
    void UnlockDeq(){pthread_mutex_unlock(&m_DeqMx);}
    void LockConnDeq(){ pthread_mutex_lock(&m_ConnDeqMx);}
    void UnlockConnDeq(){ pthread_mutex_unlock(&m_ConnDeqMx);}
    void StartThread(void(*)(CMidi*));
    void Sleep(int);
    CMidiLock* CreateLock(){ return new CMidiLockLnx;}
};

class CMidiOutHW : public CMidiOut
{
    friend class CMidiMacOSX;
protected:
    MIDIPortRef port;
    MIDIEndpointRef dest;
    CMidiOutHW(MIDIPortRef p,MIDIEndpointRef d,const char_type* n) : port(p), dest(d) { name=n;}
    ~CMidiOutHW();
    void MidiOut(unsigned char,unsigned char,unsigned char);
    bool MidiOutMsg(const std::basic_string<unsigned char>&);
};

class CMidiOutSW : public CMidiOut
{
    friend class CMidiMacOSX;
protected:
    AUGraph graph;
    AudioUnit synth;
    AudioUnit out;
    CMidiOutSW();
    ~CMidiOutSW();
    bool MidiOutMsg(const std::basic_string<unsigned char>&);
};

class CMidiInHW : public CMidiIn
{
    friend class CMidiMacOSX;
protected:
    MIDIPortRef port;
    MIDIEndpointRef src;
    CMidiInHW(MIDIPortRef p,MIDIEndpointRef s,const char_type* n) : port(p), src(s) { name=n;}
    ~CMidiInHW();
    void ReadMidiInput(void*,std::vector<unsigned char>&);
};
