#include <sys/time.h>

class CMidiLockLnx : public CMidiLock
{
    pthread_mutex_t m_Mx;
public:
    CMidiLockLnx() { pthread_mutex_init(&m_Mx, 0); }
    ~CMidiLockLnx() { pthread_mutex_destroy(&m_Mx); }
    void Lock() { pthread_mutex_lock(&m_Mx); }
    void Unlock() { pthread_mutex_unlock(&m_Mx); }
};

struct CAlsaEntry
{
    std::string S;
    std::string M;	// Manufacturer
    int C, P;
    CAlsaEntry(const std::string&s, const std::string&m, int c, int p) : S(s), M(m), C(c), P(p) {}
    str_type Name() const;
    str_type Manufacturer() const { return fromUtf8(M); }
    str_type Version() const { return fromUtf8("0.0"); }
    bool operator<(const CAlsaEntry&) const;
};

class CMidiALSA : public CMidi
{
    snd_seq_t* m_Seq;
    pthread_mutex_t m_DeqMx;
    pthread_mutex_t m_ConnDeqMx;
public:
    timeval m_StartTime;
    CMidiALSA(void*p) : CMidi(p) {
        gettimeofday(&m_StartTime, 0);
        pthread_mutex_init(&m_DeqMx, 0); pthread_mutex_init(&m_ConnDeqMx, 0);
        snd_seq_open(&m_Seq, "default", SND_SEQ_OPEN_DUPLEX, 0);
    }
    ~CMidiALSA() {
        MidiInClose(); MidiOutClose(); ClearDeq();
        snd_seq_close(m_Seq);
        pthread_mutex_destroy(&m_DeqMx); pthread_mutex_destroy(&m_ConnDeqMx);
    }
    unsigned long Time() { timeval t; gettimeofday(&t, 0); unsigned long z = (t.tv_sec-m_StartTime.tv_sec)*1000; z += (t.tv_usec-m_StartTime.tv_usec)/1000; return z; }
    int MidiOut(unsigned char,unsigned char,unsigned char);
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
    void LockDeq() { pthread_mutex_lock(&m_DeqMx); }
    void UnlockDeq() { pthread_mutex_unlock(&m_DeqMx); }
    void LockConnDeq() { pthread_mutex_lock(&m_ConnDeqMx); }
    void UnlockConnDeq() { pthread_mutex_unlock(&m_ConnDeqMx); }
    void StartThread(void(*)(CMidi*));
    void Sleep(int);
    std::vector<CAlsaEntry> ListAll(bool in);
    CMidiLock* CreateLock() { return new CMidiLockLnx; }
};

class CMidiOutHW : public CMidiOut
{
friend class CMidiALSA;
protected:
    snd_seq_t* m_Seq;
    snd_seq_port_subscribe_t* m_Sub;
    snd_midi_event_t* m_Coder;
    unsigned char* m_Buf;
    size_t m_Sz;
    int m_Port, m_C, m_P;
    CMidiOutHW(snd_seq_t* seq, int client, int port, const char_type* n);
    virtual ~CMidiOutHW();
    void MidiOut(unsigned char, unsigned char, unsigned char);
    bool MidiOutMsg(const std::basic_string<unsigned char>& data);
};

class CMidiInHW : public CMidiIn
{
friend class CMidiALSA;
protected:
    snd_seq_t* m_Seq;
    snd_seq_port_subscribe_t* m_Sub;
    int m_Port, m_C, m_P;
    void* plugin;
    snd_rawmidi_t* handle;
    pthread_mutex_t lock;
    pthread_t thread;
    CMidiInHW(snd_seq_t* seq, int client, int port, const char_type* n, void* p);
    ~CMidiInHW();
    virtual void ReadMidiInput(void*,std::vector<unsigned char>&);
    static void* MidiInThread(void* p);
};


class CCoder
{
private:
    snd_midi_event_t* m_Coder;
public:
    CCoder(size_t sz) { snd_midi_event_new(sz, &m_Coder); }
    ~CCoder() { snd_midi_event_free(m_Coder); }
    snd_midi_event_t* Coder() { return m_Coder; }
};
