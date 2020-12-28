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
        if (buff[0] >= 0xf4)	{
            ret |= MidiOutMsg(buff.substr(0, 1));
            buff = buff.substr(1);
        }
        else if (buff[0] == 0xf1 || buff[0] == 0xf3 || status == 0xc0 || status == 0xd0) {
            if (buff.length() < 2) return ret;
            if (buff[1] & 0x80) buff = buff.substr(1);
            else {
                ret |= MidiOutMsg(buff.substr(0, 2));
                buff = buff.substr(2);
            }
        }
        else if (buff[0] == 0xf2 || status == 0xe0) {
            if (buff.length() < 3) return ret;
            if (buff[1] & 0x80) buff = buff.substr(1);
            else if (buff[2] & 0x80) buff = buff.substr(2);
            else {
                ret |= MidiOutMsg(buff.substr(0, 3));
                buff = buff.substr(3);
            }
        }
        else if (status == 0x80 || status == 0x90 || status == 0xa0 || status == 0xb0) {
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
