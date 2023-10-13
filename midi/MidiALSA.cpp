#include <alsa/asoundlib.h>
#include <pthread.h>
#include "Midi.h"
#include "MidiALSA.h"
#include <map>
#include <algorithm>
#include <memory>
#include <iostream>

using namespace std;

CMidi* CMidi::CreateMidi(void*p) { return new CMidiALSA(p); }

str_type CAlsaEntry::Name() const
{
    return fromUtf8(S);
}


vector<CAlsaEntry> CMidiALSA::ListAll(bool in)
{
    vector<CAlsaEntry> E;
    if (!m_Seq) return E;
    map<string, string> M;
    int card = -1;
    while (!snd_card_next(&card) && card!=-1) {
        snd_ctl_t *ctl;
        string hw = "hw:" + to_string(card);
        if (snd_ctl_open(&ctl, hw.c_str(), 0)) continue;
        snd_ctl_card_info_t* card_info;
        snd_ctl_card_info_alloca(&card_info);
        string man;
        if (!snd_ctl_card_info(ctl, card_info)) {
            string name = snd_ctl_card_info_get_name(card_info);
            string long_name = snd_ctl_card_info_get_longname(card_info);
            size_t z = long_name.find(name);
            if (z && z != string::npos) {
                long_name = long_name.substr(0, z);
                while (long_name.size() && isspace(long_name[long_name.size() - 1])) long_name = long_name.substr(0, long_name.size() - 1);
                if (long_name.size()) man = long_name;
            }
            M[name] = man;
        }
        int device = -1;
        while (!snd_ctl_rawmidi_next_device(ctl, &device) && device != -1) {
            snd_rawmidi_info_t *info;
            snd_rawmidi_info_alloca(&info);
            memset(info, 0, snd_rawmidi_info_sizeof());
            snd_rawmidi_info_set_device(info, device);
            snd_rawmidi_info_set_stream(info, in ? SND_RAWMIDI_STREAM_INPUT : SND_RAWMIDI_STREAM_OUTPUT);
            snd_ctl_rawmidi_info(ctl, info);
            int subs = snd_rawmidi_info_get_subdevices_count(info);
            for (int sub = 0; sub < subs; sub++) {
                snd_rawmidi_info_set_subdevice(info, sub);
                if (snd_ctl_rawmidi_info(ctl, info)) continue;
                string name = snd_rawmidi_info_get_subdevice_name(info);
                if (!name.length()) {
                    name = snd_rawmidi_info_get_name(info);
                    M[name] = man;
                    break;
                }
                M[name] = man;
            }
        }
        snd_ctl_close(ctl);
    }
    snd_config_update_free_global();

    snd_seq_client_info_t *cinfo;
    snd_seq_client_info_alloca( &cinfo );
    snd_seq_port_info_t *pinfo;
    snd_seq_port_info_alloca( &pinfo );
    snd_seq_client_info_set_client(cinfo, -1);
    while (snd_seq_query_next_client(m_Seq, cinfo) >= 0) {
        int client = snd_seq_client_info_get_client(cinfo);
        snd_seq_port_info_set_client(pinfo, client);
        snd_seq_port_info_set_port(pinfo, -1);
        while (snd_seq_query_next_port(m_Seq, pinfo) >= 0) {
            int port = snd_seq_port_info_get_port(pinfo);
            unsigned int type = snd_seq_port_info_get_type(pinfo);
            if (type & (SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_SYNTHESIZER)) {
                unsigned int cap = snd_seq_port_info_get_capability(pinfo);
                if (in) {
                    if (!(cap & (SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ))) continue;
                }
                else {
                    if (!(cap & (SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE))) continue;
                }
                string name = snd_seq_port_info_get_name(pinfo);
                E.push_back(CAlsaEntry(name, M[name], client, port));
            }
        }
    }
    return E;
}


CMidiOutHW::CMidiOutHW(snd_seq_t* seq, int client, int port, const char_type* n)
    : m_Seq(seq), m_Sz(32), m_C(-1), m_P(-1)
{
    name = n;
    m_Buf = new unsigned char[m_Sz];
    snd_midi_event_new(m_Sz, &m_Coder);
    snd_midi_event_init(m_Coder);
    snd_seq_addr_t sender, receiver;
    receiver.client = client;
    receiver.port = port;
    sender.client = snd_seq_client_id(m_Seq);
    m_Port = snd_seq_create_simple_port(seq, "my_port", SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ, SND_SEQ_PORT_TYPE_MIDI_GENERIC|SND_SEQ_PORT_TYPE_APPLICATION);
    sender.port = m_Port;
    snd_seq_port_subscribe_malloc(&m_Sub);
    snd_seq_port_subscribe_set_sender(m_Sub, &sender);
    snd_seq_port_subscribe_set_dest(m_Sub, &receiver);
    if (!snd_seq_subscribe_port(m_Seq, m_Sub)) {
        m_C = client; m_P = port;
    }
}


CMidiOutHW::~CMidiOutHW()
{
    snd_seq_unsubscribe_port(m_Seq, m_Sub);
    snd_seq_port_subscribe_free(m_Sub);
    snd_midi_event_free(m_Coder);
    snd_seq_delete_port(m_Seq, m_Port);
    delete[] m_Buf;
}


void CMidiOutHW::MidiOut(unsigned char a, unsigned char b, unsigned char c)
{
    std::basic_string<unsigned char> data;
    int len = CMidi::MidiLen(a);
    data += a; if (len > 1) data += b; if (len > 2) data += c;
    MidiOutMsg(data);
}


bool CMidiOutHW::MidiOutMsg(const std::basic_string<unsigned char>& data)
{
    if (data.size() > m_Sz) {
        m_Sz = data.size();
        delete[] m_Buf;
        m_Buf = new unsigned char[m_Sz];
        snd_midi_event_resize_buffer(m_Coder, m_Sz);
    }
    snd_seq_event_t ev;
    snd_seq_ev_clear(&ev);
    snd_seq_ev_set_source(&ev, m_Port);
    snd_seq_ev_set_subs(&ev);
    snd_seq_ev_set_direct(&ev);
    for (size_t i = 0; i < data.size(); i++) m_Buf[i] = data[i];
    snd_midi_event_encode(m_Coder, m_Buf, data.size(), &ev);
    snd_seq_event_output(m_Seq, &ev);
    snd_seq_drain_output(m_Seq);
    return !!data.size();
}


std::vector<str_type> CMidiALSA::MidiOutList()
{
    std::vector<str_type> V;
    std::vector<CAlsaEntry> E = ListAll(false);
    for (int i = 0; i < E.size(); i++) V.push_back(E[i].Name());
    return V;
}


std::vector<str_type> CMidiALSA::MidiInList()
{
    std::vector<str_type> V;
    std::vector<CAlsaEntry> E = ListAll(true);
    for (int i = 0; i < E.size(); i++) V.push_back(E[i].Name());
    return V;
}


std::vector<str_type> CMidiALSA::MidiOutInfo(int n)
{
    std::vector<str_type> V;
    std::vector<CAlsaEntry> E = ListAll(false);
    if (n >= 0 && n < E.size()) {
        V.push_back(E[n].Name());
        V.push_back(E[n].Manufacturer());
        V.push_back(E[n].Version());
    }
    return V;
}


std::vector<str_type> CMidiALSA::MidiOutInfo(const char_type*name)
{
    std::vector<str_type> V;
    std::vector<CAlsaEntry> E = ListAll(false);
    for (int n = 0; n < E.size(); n++) {
        if (E[n].Name() != name) continue;
        V.push_back(E[n].Name());
        V.push_back(E[n].Manufacturer());
        V.push_back(E[n].Version());
        break;
    }
    return V;
}


std::vector<str_type> CMidiALSA::MidiInInfo(int n)
{
    std::vector<str_type> V;
    std::vector<CAlsaEntry> E = ListAll(true);
    if (n >= 0 && n < E.size()) {
        V.push_back(E[n].Name());
        V.push_back(E[n].Manufacturer());
        V.push_back(E[n].Version());
    }
    return V;
}


std::vector<str_type> CMidiALSA::MidiInInfo(const char_type*name)
{
    std::vector<str_type> V;
    std::vector<CAlsaEntry> E = ListAll(true);
    for (int n = 0; n < E.size(); n++) {
        if (E[n].Name() != name) continue;
        V.push_back(E[n].Name());
        V.push_back(E[n].Manufacturer());
        V.push_back(E[n].Version());
        break;
    }
    return V;
}


str_type CMidiALSA::MidiOutOpen(int n)
{
    std::vector<CAlsaEntry> E = ListAll(false);
    if (n < E.size()) {
        CMidiOutHW* port = new CMidiOutHW(m_Seq, E[n].C, E[n].P, E[n].Name().c_str());
        if (port->m_C >= 0 || port->m_P >= 0) SetOut(port);
        else delete port;
    }
    return CurrentOutName();
}


str_type CMidiALSA::MidiOutOpen(const char_type*name)
{
    if (CurrentOutName() == name) return CurrentOutName();
    std::vector<CAlsaEntry> E = ListAll(false);
    int n;
    for (n = 0; n < E.size(); n++) if (E[n].Name() == name) break;
    if (n < E.size()) {
        CMidiOutHW* port = new CMidiOutHW(m_Seq, E[n].C, E[n].P, E[n].Name().c_str());
        if (port->m_C >= 0 || port->m_P >= 0) SetOut(port);
        else delete port;
    }
    return CurrentOutName();
}


void PluginCallback(void*, void*);


void* CMidiInHW::MidiInThread(void* p)
{
    CMidiInHW* self = (CMidiInHW*)p;
    snd_seq_t* Seq = self->m_Seq;
    pthread_mutex_t& lock = self->lock;

    size_t SZ = 32;
    unique_ptr<unsigned char[]> B(new unsigned char[SZ]);
    unique_ptr<CCoder> C(new CCoder(SZ));
    std::vector<unsigned char> Q;
    unsigned char RS = 0;
    snd_seq_event_t* ev;
    while (snd_seq_event_input(Seq, &ev) >= 0) {
        if (ev->type == SND_SEQ_EVENT_PORT_SUBSCRIBED || ev->type == SND_SEQ_EVENT_PORT_UNSUBSCRIBED) continue;
        if (ev->type == SND_SEQ_EVENT_SYSEX && ev->data.ext.len > SZ) {
            SZ = ev->data.ext.len;
            B.reset(new unsigned char[SZ]);
            snd_midi_event_resize_buffer(C->Coder(), SZ);
        }
        int n = snd_midi_event_decode(C->Coder(), B.get(), SZ, ev);
        snd_seq_free_event(ev);
        for (int i = 0; i < n; i++) {
            if (CMidiIn::Push(B[i], Q, RS)) {
                pthread_mutex_lock(&lock);
                std::vector<unsigned char>* v = new std::vector<unsigned char>(Q);
                Q.clear();
                PluginCallback(self->plugin, (void*)v);
                pthread_mutex_unlock(&lock);
            }
        }
    }
    return 0;
}


void CMidiInHW::ReadMidiInput(void* data, std::vector<unsigned char>& v)
{
    std::vector<unsigned char>*d=(std::vector<unsigned char>*)data;
    for (int i = 0; i < (*d).size(); i++) v.push_back((*d)[i]);
    delete d;
}


str_type CMidiALSA::MidiInOpen(int n, void* p)
{
    std::vector<CAlsaEntry> E = ListAll(true);
    if (n < E.size()) {
        CMidiInHW* port = new CMidiInHW(m_Seq, E[n].C, E[n].P, E[n].Name().c_str(), p);
        if (port->m_C >= 0 || port->m_P >= 0) SetIn(port);
        else delete port;
    }
    return CurrentInName();
}


str_type CMidiALSA::MidiInOpen(const char_type*name, void* p)
{
    if (CurrentInName() == name) return CurrentInName();
    std::vector<CAlsaEntry> E = ListAll(true);
    int n; 
    for (n = 0; n < E.size(); n++) if (E[n].Name() == name) break;
    if (n < E.size()) {
        CMidiInHW* port = new CMidiInHW(m_Seq, E[n].C, E[n].P, E[n].Name().c_str(), p);
        if (port->m_C >= 0 || port->m_P >= 0) SetIn(port);
        else delete port;
    }
    return CurrentInName();
}


CMidiInHW::CMidiInHW(snd_seq_t* seq, int client, int port, const char_type* n, void* p)
    : m_Seq(seq), m_C(-1), m_P(-1), plugin(p), have_thread(false)
{
    if (!m_Seq) return;
    name = n;
    snd_seq_addr_t sender, receiver;
    sender.client = client;
    sender.port = port;
    receiver.client = snd_seq_client_id(m_Seq);
    m_Port = snd_seq_create_simple_port(seq, "my_port", SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE, SND_SEQ_PORT_TYPE_MIDI_GENERIC|SND_SEQ_PORT_TYPE_APPLICATION);
    receiver.port = m_Port;
    snd_seq_port_subscribe_malloc(&m_Sub);
    snd_seq_port_subscribe_set_sender(m_Sub, &sender);
    snd_seq_port_subscribe_set_dest(m_Sub, &receiver);
    if (!snd_seq_subscribe_port(m_Seq, m_Sub)) {
        m_C = client; m_P = port;
        pthread_mutex_init(&lock, 0);
        pthread_create(&thread, 0, MidiInThread, this);
        have_thread = true;
    }
}


CMidiInHW::~CMidiInHW()
{
    if (!m_Seq) return;
    snd_seq_unsubscribe_port(m_Seq, m_Sub);
    snd_seq_port_subscribe_free(m_Sub);
    snd_seq_delete_port(m_Seq, m_Port);
    if(have_thread) {
        pthread_mutex_lock(&lock);
        pthread_cancel(thread);
        pthread_mutex_unlock(&lock);
        pthread_mutex_destroy(&lock);
    }
}


struct ThreadParam
{
    void(*func)(CMidi*);
    CMidi* ptr;
    ThreadParam(void(*f)(CMidi*),CMidi*p) : func(f), ptr(p) {}
};


void* ThreadWrapper(void* p) { ThreadParam* tp=(ThreadParam*)p; tp->func(tp->ptr); delete tp; return 0; }
void CMidiALSA::StartThread(void(*func)(CMidi*)) { pthread_t t; pthread_create(&t, 0, ThreadWrapper, new ThreadParam(func,this)); }
void CMidiALSA::Sleep(int ms) { usleep(ms); }
