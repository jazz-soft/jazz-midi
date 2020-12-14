#include "Midi.h"
#include <set>
#include <iostream>

void CMidiOut::MidiOut(unsigned char a,unsigned char b,unsigned char c)
{
    buff.clear();
    std::basic_string<unsigned char> data;
    data += a; data += b; data += c;
    MidiOutMsg(data);
}


bool CMidiOut::MidiOutRaw(const std::basic_string<unsigned char>& data)
{
    buff.clear();
    return MidiOutMsg(data);
}


bool CMidiOut::MidiOutLong(const std::basic_string<unsigned char>& data)
{
    size_t i;
    bool ret = false;
    buff += data;
    while (buff.length()) {
        for (i = 0; i < buff.length(); i++) if (buff[i] & 0x80 && buff[0] != 0xf7) break;
        if (i) buff = buff.substr(i);
        if (!buff.length()) break;
        unsigned char status = buff[0] & 0xf0;
        if(buff[0] >= 0xf4)	{
            ret |= MidiOutMsg(buff.substr(0, 1));
            buff = buff.substr(1);
        }
        else if(buff[0] == 0xf1 || buff[0] == 0xf3 || status == 0xc0 || status == 0xd0) {
            if (buff.length() < 2) return ret;
            if (buff[1] & 0x80) buff = buff.substr(1);
            else {
                ret |= MidiOutMsg(buff.substr(0, 2));
                buff = buff.substr(2);
            }
        }
        else if(buff[0] == 0xf2 || status == 0xe0) {
            if (buff.length() < 3) return ret;
            if (buff[1] & 0x80) buff = buff.substr(1);
            else if (buff[2] & 0x80) buff = buff.substr(2);
            else {
                ret |= MidiOutMsg(buff.substr(0, 3));
                buff = buff.substr(3);
            }
        }
        else if(status == 0x80 || status == 0x90 || status == 0xa0 || status == 0xb0) {
            if (buff.length() < 3) return ret;
            if (buff[1] & 0x80) buff = buff.substr(1);
            else if (buff[2] & 0x80) buff = buff.substr(2);
            else {
                ret |= MidiOutMsg(buff.substr(0, 3));
                buff[2] = buff[0];
                buff = buff.substr(2);
            }
        }
        else {
            for (i = 1; i < buff.length(); i++) if (buff[i] & 0x80) break;
            if (i == buff.length()) return ret;
            if (buff[i] != 0xf7) buff = buff.substr(i);
            else {
                i++;
                ret |= MidiOutMsg(buff.substr(0, i));
                buff = buff.substr(i);
            }
        }
    }
    return ret;
}


CMidiMsg* CMidi::QueryMidiIn()
{
    CMidiMsg* ret = 0;
    LockDeq();
    if (m_Deq.size()) {
        ret = m_Deq.front();
        m_Deq.pop_front();
    }
    UnlockDeq();
    return ret;
}


void CMidi::RecordMidiIn(CMidiMsg*msg)
{
    LockDeq();
    m_Deq.push_back(msg);
    UnlockDeq();
}


void CMidi::ClearDeq()
{
    LockDeq();
    for (size_t i = 0; i < m_Deq.size(); i++) delete m_Deq[i];
    m_Deq.clear();
    UnlockDeq();
}


void CMidi::OnConnect(CMidiConn::Type T, bool enable)
{
    if (T == CMidiConn::OutOn && m_OutOn == enable) return;
    if (T == CMidiConn::OutOff && m_OutOff == enable) return;
    if (T == CMidiConn::InOn && m_InOn == enable) return;
    if (T == CMidiConn::InOff && m_InOff == enable) return;
    if (enable && !m_OutOn && !m_OutOff && (T == CMidiConn::OutOn || T == CMidiConn::OutOff)) m_OutList = MidiOutList();
    if (enable && !m_InOn && !m_InOff && (T == CMidiConn::InOn || T == CMidiConn::InOff)) m_InList = MidiInList();
    if (enable && !m_OutOn && !m_OutOff && !m_InOn && !m_InOff) StartThread(OnConnectThread);
    if (T == CMidiConn::OutOn) m_OutOn = enable;
    if (T == CMidiConn::OutOff) m_OutOff = enable;
    if (T == CMidiConn::InOn) m_InOn = enable;
    if (T == CMidiConn::InOff) m_InOff = enable;
    if (!m_OutOn && !m_OutOff && !m_InOn && !m_InOff) {
        if (p_StopConnectThread) *p_StopConnectThread = true;
        p_StopConnectThread = 0;
        p_ConnectThreadLock = 0;
    }
}


void PluginConnCallback(void*);


void CMidi::OnConnectThread(CMidi* self)
{
    volatile bool stop=false;
    CMidiLock* lock=self->CreateLock();
    self->p_StopConnectThread = &stop;
    self->p_ConnectThreadLock = lock;

    while (1) {
        lock->Lock();
        if (stop) { lock->Unlock(); break; }
        std::vector<str_type> old_Out;
        std::vector<str_type> new_Out;
        std::vector<str_type> old_In;
        std::vector<str_type> new_In;
        if (self->m_OutOn || self->m_OutOff) {
            std::set<str_type> old_set, new_set;
            old_set.insert(self->m_OutList.begin(), self->m_OutList.end());
            self->m_OutList=self->MidiOutList();
            new_set.insert(self->m_OutList.begin(), self->m_OutList.end());
            if (self->m_OutOff)
                for (std::set<str_type>::iterator J = old_set.begin(); J != old_set.end(); J++)
                    if (new_set.find(*J) == new_set.end()) old_Out.push_back(*J);
            if (self->m_OutOn)
                for (std::set<str_type>::iterator J = new_set.begin(); J != new_set.end(); J++)
                    if (old_set.find(*J) == old_set.end()) new_Out.push_back(*J);
        }
        if (self->m_InOn || self->m_InOff) {
            std::set<str_type> old_set, new_set;
            old_set.insert(self->m_InList.begin(), self->m_InList.end());
            self->m_InList=self->MidiInList();
            new_set.insert(self->m_InList.begin(), self->m_InList.end());
            if (self->m_InOff)
                for (std::set<str_type>::iterator J = old_set.begin(); J != old_set.end(); J++)
                    if (new_set.find(*J) == new_set.end()) old_In.push_back(*J);
            if (self->m_InOn)
                for (std::set<str_type>::iterator J = new_set.begin(); J != new_set.end(); J++)
                    if (old_set.find(*J) == old_set.end()) new_In.push_back(*J);
        }
        if (old_Out.size() || new_Out.size() || old_In.size() || new_In.size()) {
            self->LockConnDeq();
            for (std::vector<str_type>::iterator J = old_Out.begin(); J != old_Out.end(); J++)
                self->m_ConnDeq.push_back(new CMidiConn(CMidiConn::OutOff, *J));
            for (std::vector<str_type>::iterator J = new_Out.begin(); J != new_Out.end(); J++)
                self->m_ConnDeq.push_back(new CMidiConn(CMidiConn::OutOn, *J));
            for (std::vector<str_type>::iterator J = old_In.begin(); J != old_In.end(); J++)
                self->m_ConnDeq.push_back(new CMidiConn(CMidiConn::InOff, *J));
            for (std::vector<str_type>::iterator J = new_In.begin(); J != new_In.end(); J++)
                self->m_ConnDeq.push_back(new CMidiConn(CMidiConn::InOn, *J));
            self->UnlockConnDeq();
            PluginConnCallback(self->Owner);
        }
        lock->Unlock();
        self->Sleep(100);
    }
}


CMidiConn* CMidi::GetConnEvent()
{
    CMidiConn* ret = 0;
    LockConnDeq();
    if (!m_ConnDeq.empty()) {
        ret = m_ConnDeq.front();
        m_ConnDeq.pop_front();
    }
    UnlockConnDeq();
    return ret;
}


void CMidi::DrainConnDeq()
{
    LockConnDeq();
    for (size_t i = 0; i < m_ConnDeq.size(); i++) delete m_ConnDeq[i];
    m_ConnDeq.clear();
    UnlockConnDeq();
}


void CMidi::Stop()
{
    if (p_ConnectThreadLock) {
        p_ConnectThreadLock->Lock();
        if (p_StopConnectThread) *p_StopConnectThread = true;
        p_ConnectThreadLock->Unlock();
        p_StopConnectThread = 0;
        p_ConnectThreadLock = 0;
    }
}


size_t CMidi::MidiLen(unsigned char c)
{
    switch (c & 0xf0) {
        case 0x80:
        case 0x90:
        case 0xA0:
        case 0xB0:
        case 0xE0: return 3;
        case 0xC0:
        case 0xD0: return 2;
    }
    switch (c) {
        case 0xF2: return 3;
        case 0xF1:
        case 0xF3: return 2;
    }
    return 1;
}


bool CMidiIn::Push(unsigned char c, std::vector<unsigned char>& Q, unsigned char& rs)
{
    if (c >= 0x80 && Q.size() && !(Q[0] == 0xf0 && c == 0xf7)) Q.clear(); // incomplete message
    if (c < 0x80 && !Q.size()) {
        if (rs >= 0x80) Q.push_back(rs);
        else return false;
    }
    Q.push_back(c);

    size_t m = CMidi::MidiLen(Q[0]);
    rs = m > 1 ? Q[0] : 0;

    if (Q[0] == 0xf0) { // sysex
        return c == 0xf7;
    }
    if (Q[0] == 0xf7) { // ignore
        Q.clear();
        return false;
    }
    return Q.size() >= m;
}
