#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <sstream>
#include "../midi/Midi.h"
#include "../midi/version.h"

using namespace std;

CMidi* Midi = 0;
CMidiLock* Lock = 0;

union ULen
{
    unsigned char c[4];
    unsigned long n;
};

string ReadMsg()
{	string s;
    ULen len; len.n = 0;
    for (size_t i = 0; i < 4; i++) {
        int n = getchar();
        if (n == EOF) exit(0);
        len.c[i] = (unsigned char)n;
    }
    for (size_t i = 0; i < len.n; i++) {
        int n = getchar();
        if (n == EOF) exit(0);
        s += (unsigned char)n;
    }
    return s;
}

void SendMsg(string s)
{
    ULen len;
    len.n = s.length();
    Lock->Lock();
    for (size_t i = 0; i < 4; i++) putchar(len.c[i]);
    for (size_t i = 0; i < s.length(); i++) putchar(s[i]);
    fflush(stdout);
    Lock->Unlock();
}

string Quote(string s)
{
    string str = "\"";
    str += s;
    str += "\"";
    return str;
}

struct CToken
{
    string S;
    unsigned int N;
    CToken(const string& s) : S(s), N(0) {}
    CToken(const unsigned int n) : N(n) {}
};

vector<CToken> Tokenize(const string& s)
{
    vector<CToken> v;
    if (s.length() < 1 || s[0] != '[') return v;
    bool txt = false;
    bool num = false;
    bool bsl = false;
    string str;
    unsigned int n;
    for (size_t i = 1; i < s.length(); i++) {
        char c = s[i];
        if (!num && !txt) {
            if (c == '\"') txt = true;
            else if (c >= '0' && c <= '9') { num = true; n = c - '0'; }
            continue;
        }
        if (txt) {
            if (c == '\\' && !bsl) { bsl = true; continue; }
            if (c == '\"' && !bsl) {
                v.push_back(CToken(str));
                str.clear(); txt = false;
                continue;
            }
            str += c;
            bsl = false;
            continue;
        }
        if (num) {
            if (c < '0' || c > '9') {
                v.push_back(CToken(n));
                num = false;
                continue;
            }
            n = n * 10 + c - '0';
            continue;
        }
    }
    return v;
}

int main()
{
    string req;
    Midi = CMidi::CreateMidi(0);
    Lock = Midi->CreateLock();
    while (true) {
        req = ReadMsg();
        vector<CToken> ttt = Tokenize(req);
        if (ttt.empty()) continue;
        string& str = ttt[0].S;

        if (str == "version") SendMsg("[\"version\"," + Quote(VERSION) + "]");
        else if (str == "refresh") {
            string s = "[\"refresh\",{\"ins\":[";
            for (size_t i = 0; ; i++) {
                vector<str_type> v = Midi->MidiInInfo(i);
                if (v.size()<3) break;
                s += i ? ",{" : "{";
                s += "\"name\":" + Quote(toUTF8(v[0])) + ",\"manufacturer\":" + Quote(toUTF8(v[1])) + ",\"version\":" + Quote(toUTF8(v[2])) + "}";
            }
            s += "],\"outs\":[";
            for (size_t i = 0; ; i++) {
                vector<str_type> v = Midi->MidiOutInfo(i);
                if (v.size()<3) break;
                s += (i ? ",{" : "{");
                s += "\"name\":" + Quote(toUTF8(v[0])) + ",\"manufacturer\":" + Quote(toUTF8(v[1])) + ",\"version\":" + Quote(toUTF8(v[2])) + "}";
            }
            s += "]}]";
            SendMsg(s);
        }
        else if (str == "openout") {
            if (ttt.size() < 2) continue;
            str_type dev = Midi->MidiOutOpen(fromUTF8<str_type>(ttt[1].S).c_str());
            SendMsg("[\"openout\"," + Quote(toUTF8(dev)) + "]");
        }
        else if (str == "openin") {
            if (ttt.size() < 2) continue;
            str_type dev = Midi->MidiInOpen(fromUTF8<str_type>(ttt[1].S).c_str(), 0);
            SendMsg("[\"openin\"," + Quote(toUTF8(dev)) + "]");
        }
        else if (str == "play") {
            basic_string<unsigned char> v;
            for (size_t i = 1; i<ttt.size(); i++) v.push_back((unsigned char)ttt[i].N);
            Midi->MidiOutRaw(v);
        }
        else if (str == "closeout") Midi->MidiOutClose();
        else if (str == "closein") Midi->MidiInClose();
        else if (str == "watch") {
            Midi->OnConnect(CMidiConn::OutOn, true);
            Midi->OnConnect(CMidiConn::OutOff, true);
            Midi->OnConnect(CMidiConn::InOn, true);
            Midi->OnConnect(CMidiConn::InOff, true);
        }
        else if (str == "unwatch") {
            Midi->OnConnect(CMidiConn::OutOn, false);
            Midi->OnConnect(CMidiConn::OutOff, false);
            Midi->OnConnect(CMidiConn::InOn, false);
            Midi->OnConnect(CMidiConn::InOff, false);
        }
    }
}

void PluginCallback(void* dummy, void* ptr)
{
    vector<unsigned char> v;
    Midi->ReadMidiInput(ptr, v);
    if (!v.size()) return;
    stringstream s; s << "[\"midi\"," << Midi->Time();
    for (size_t i = 0; i<v.size(); i++) s << "," << (unsigned int)v[i];
    s << "]";
    SendMsg(s.str());
}

void PluginConnCallback(void*)
{
    SendMsg("[\"connection\"]");
}
