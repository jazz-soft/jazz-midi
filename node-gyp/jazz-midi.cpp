#define NAPI_EXPERIMENTAL
#include <node_api.h>
#include <iostream>
#include <memory>
#include <map>
#include "version.h"
#include "Midi.h"

#define XX(x) if (x) return 0;

class CStaticMidi
{
public:
    CMidi* DI;
    std::unique_ptr<napi_ref> InOnCall;
    std::unique_ptr<napi_ref> InOffCall;
    std::unique_ptr<napi_ref> OutOnCall;
    std::unique_ptr<napi_ref> OutOffCall;
    bool ConnListening;
    CStaticMidi() : DI(0), ConnListening(false) { DI = CMidi::CreateMidi(this); }
    ~CStaticMidi() { delete DI; InOnCall.reset(); InOffCall.reset(); OutOnCall.reset(); OutOffCall.reset(); }
};
CStaticMidi MI;

class CPlugin
{
public:
    std::unique_ptr<napi_ref> MidiInCall;
    bool MidiInArray;
    bool Listening;
    CMidi* Midi;
    CPlugin() : Listening(0), Midi(0) { Midi = CMidi::CreateMidi(&MI); }
    ~CPlugin() { MidiInCall.reset(); delete Midi; }
};

struct CEvent
{
    CPlugin* Plg;
    CMidiMsg* Msg;
    CEvent() : Plg(0), Msg(0) {}
};

std::string read_utf8(napi_env env, napi_value obj)
{
    napi_value value;
    if (napi_coerce_to_string(env, obj, &value)) return "";
    std::string str;
    size_t sz = 16;
    size_t len = 16;
    while (sz - len < 8) { // consider utf8 surrogate pairs
        sz *= 2;
        char* buf = new char[sz];
        if (napi_get_value_string_utf8(env, value, buf, sz, &len)) return "";
        str = buf;
        delete[] buf;
    }
    return str;
}

bool read_int32(napi_env env, napi_value obj, int32_t* result)
{
    bool ok = false;
    int32_t n;
    napi_value num;
    XX (napi_coerce_to_number(env, obj, &num));
    XX (napi_get_value_int32(env, num, &n));
    napi_value test;
    XX (napi_create_int32(env, n, &test));
    XX (napi_strict_equals(env, num, test, &ok));
    if (ok) *result = n;
    //else std::cout << "expected a number!";
    return ok;
}

napi_value Support(napi_env env, napi_callback_info args)
{
    static const char* funcs[] = {
        "ClearMidiIn",
        "MidiInClose",
        "MidiInInfo",
        "MidiInList",
        "MidiInOpen",
        "MidiOut",
        "MidiOutClose",
        "MidiOutInfo",
        "MidiOutList",
        "MidiOutLong",
        "MidiOutOpen",
        "MidiOutRaw",
        "Support",
        "Time"
    };
    static const char* props[] = {
        "midi.in",
        "midi.out",
        "midi.synth",
        "build.64bit"
    };
    static size_t sz = sizeof(funcs) / sizeof(funcs[0]);
    static size_t sz1 = sizeof(props) / sizeof(props[0]);
    size_t argc = 1;
    napi_value argv[1];
    napi_value arr;
    napi_value value;
    XX (napi_get_cb_info(env, args, &argc, argv, 0, 0));
    if (!argc) {
        XX (napi_create_array(env, &arr));
        for (size_t i = 0; i < sz; i++) {
            XX (napi_create_string_utf8(env, funcs[i], NAPI_AUTO_LENGTH, &value));
            XX (napi_set_element(env, arr, i, value));
        }
        return arr;
    }
    XX (napi_coerce_to_string(env, argv[0], &value));
    std::string s = read_utf8(env, value);
    bool found = false;
    for (size_t i = 0; i < sz && !found; i++) if (funcs[i] == s) found = true;
    for (size_t i = 0; i < sz1 && !found; i++) if (props[i] == s) found = true;
    XX (napi_get_boolean(env, found, &value));
    return value;
}

napi_value Time(napi_env env, napi_callback_info args)
{
    napi_value value;
    XX (napi_create_int64(env, MI.DI->Time(), &value));
    return value;
}

napi_value MidiOutList(napi_env env, napi_callback_info args)
{
    std::vector<str_type> v = MI.DI->MidiOutList();
    napi_value arr;
    napi_value value;
    XX (napi_create_array(env, &arr));
    for (size_t i = 0; i < v.size(); i++) {
        XX (napi_create_string_utf8(env, toUtf8(v[i]).c_str(), NAPI_AUTO_LENGTH, &value));
        XX (napi_set_element(env, arr, i, value));
    }
    return arr;
}

napi_value MidiInList(napi_env env, napi_callback_info args)
{
    std::vector<str_type> v = MI.DI->MidiInList();
    napi_value arr;
    napi_value value;
    XX (napi_create_array(env, &arr));
    for (size_t i = 0; i < v.size(); i++) {
        XX (napi_create_string_utf8(env, toUtf8(v[i]).c_str(), NAPI_AUTO_LENGTH, &value));
        XX (napi_set_element(env, arr, i, value));
    }
    return arr;
}

napi_value MidiOutInfo(napi_env env, napi_callback_info args)
{
    std::vector<str_type> v;
    size_t argc = 1;
    napi_value argv[1];
    napi_value arr;
    napi_value value;
    XX (napi_get_cb_info(env, args, &argc, argv, 0, 0));
    if (argc) {
        XX (napi_coerce_to_string(env, argv[0], &value));
        std::string s = read_utf8(env, value);
        v = MI.DI->MidiOutInfo(fromUtf8(s).c_str());
        if (v.empty()) {
            int32_t n;
            napi_value num;
            XX (napi_coerce_to_number(env, argv[0], &num));
            XX (napi_get_value_int32(env, num, &n));
            if (n > 0 || s == "0") v = MI.DI->MidiOutInfo(n);
        }
    }
    XX (napi_create_array(env, &arr));
    for (size_t i = 0; i < v.size(); i++) {
        XX (napi_create_string_utf8(env, toUtf8(v[i]).c_str(), NAPI_AUTO_LENGTH, &value));
        XX (napi_set_element(env, arr, i, value));
    }
    return arr;
}

napi_value MidiInInfo(napi_env env, napi_callback_info args)
{
    std::vector<str_type> v;
    size_t argc = 1;
    napi_value argv[1];
    napi_value arr;
    napi_value value;
    XX (napi_get_cb_info(env, args, &argc, argv, 0, 0));
    if (argc) {
        XX (napi_coerce_to_string(env, argv[0], &value));
        std::string s = read_utf8(env, value);
        v = MI.DI->MidiInInfo(fromUtf8(s).c_str());
        if (v.empty()) {
            int32_t n;
            napi_value num;
            XX (napi_coerce_to_number(env, argv[0], &num));
            if (read_int32(env, num, &n)) v = MI.DI->MidiInInfo(n);
        }
    }
    XX (napi_create_array(env, &arr));
    for (size_t i = 0; i < v.size(); i++) {
        XX (napi_create_string_utf8(env, toUtf8(v[i]).c_str(), NAPI_AUTO_LENGTH, &value));
        XX (napi_set_element(env, arr, i, value));
    }
    return arr;
}

static const char* err_midi_message = "MIDI message";
static const char* err_midi_callback = "MIDI callback";
static const char* err_expected_byte = "expected a value between 0 and 255";
static const char* err_expected_array = "expected an Array";
static const char* err_expected_function = "expected a Function";

//// MidiOut

napi_value MidiOut(napi_env env, napi_callback_info args)
{
    size_t argc = 3;
    napi_value self;
    napi_value argv[3];
    napi_value value;
    CPlugin* Plg;
    XX (napi_get_cb_info(env, args, &argc, argv, &self, 0));
    XX (napi_unwrap(env, self, (void**)&Plg));
    if (argc) {
        int32_t n;
        unsigned char a[3] = { 0, 0, 0 };
        for (size_t i = 0; i < argc; i++) {
            if (!read_int32(env, argv[i], &n)) napi_throw_type_error(env, err_midi_message, err_expected_byte);
            else if (n < 0 || n > 255) napi_throw_range_error(env, err_midi_message, err_expected_byte);
            else a[i] = (unsigned char)n;
        }
        Plg->Midi->MidiOut(a[0], a[1], a[2]);
    }
    XX (napi_get_undefined(env, &value));
    return value;
}

napi_value MidiOutOpen(napi_env env, napi_callback_info args)
{
    size_t argc = 1;
    napi_value self;
    napi_value argv[1];
    napi_value value;
    CPlugin* Plg;
    XX (napi_get_cb_info(env, args, &argc, argv, &self, 0));
    XX (napi_unwrap(env, self, (void**)&Plg));
    std::string s;
    if (argc) {
        int32_t n;
        XX (napi_coerce_to_number(env, argv[0], &value));
        if (read_int32(env, value, &n)) {
            s = toUtf8(Plg->Midi->MidiOutOpen(n));
        }
        else {
            XX (napi_coerce_to_string(env, argv[0], &value));
            s = read_utf8(env, value);
            str_type name = fromUtf8(s);
            s = toUtf8(Plg->Midi->MidiOutOpen(name.c_str()));
        }
    }
    else {
        s = toUtf8(Plg->Midi->CurrentOutName());
    }
    XX (napi_create_string_utf8(env, s.c_str(), NAPI_AUTO_LENGTH, &value));
    return value;
}

napi_value MidiOutClose(napi_env env, napi_callback_info args)
{
    napi_value self;
    napi_value value;
    CPlugin* Plg;
    XX (napi_get_cb_info(env, args, 0, 0, &self, 0));
    XX (napi_unwrap(env, self, (void**)&Plg));
    Plg->Midi->MidiOutClose();
    XX (napi_get_undefined(env, &value));
    return value;
}

napi_value MidiOutLong(napi_env env, napi_callback_info args)
{
    size_t argc = 1;
    napi_value self;
    napi_value argv[1];
    napi_value value;
    bool bbb;
    uint32_t len;
    int32_t n;
    CPlugin* Plg;
    XX (napi_get_cb_info(env, args, &argc, argv, &self, 0));
    XX (napi_unwrap(env, self, (void**)&Plg));
    XX (napi_is_array(env, argv[0], &bbb));
    if (!bbb) {
        napi_throw_type_error(env, err_midi_message, err_expected_array);
    }
    XX (napi_get_array_length(env, argv[0], &len));
    if (len) {
        std::basic_string<unsigned char> s;
        for (size_t i = 0; i < len; i++) {
            XX (napi_get_element(env, argv[0], i, &value));
            if (!read_int32(env, value, &n)) napi_throw_type_error(env, err_midi_message, err_expected_byte);
            else if (n < 0 || n > 255) napi_throw_range_error(env, err_midi_message, err_expected_byte);
            else s += (unsigned char)n;
        }
        Plg->Midi->MidiOutLong(s);
    }
    XX (napi_get_undefined(env, &value));
    return value;
}

napi_value MidiOutRaw(napi_env env, napi_callback_info args)
{
    size_t argc = 1;
    napi_value self;
    napi_value argv[1];
    napi_value value;
    bool bbb;
    uint32_t len;
    int32_t n;
    CPlugin* Plg;
    XX (napi_get_cb_info(env, args, &argc, argv, &self, 0));
    XX (napi_unwrap(env, self, (void**)&Plg));
    XX (napi_is_array(env, argv[0], &bbb));
    if (!bbb) {
        napi_throw_type_error(env, err_midi_message, err_expected_array);
    }
    XX (napi_get_array_length(env, argv[0], &len));
    if (len) {
        std::basic_string<unsigned char> s;
        for (size_t i = 0; i < len; i++) {
            XX (napi_get_element(env, argv[0], i, &value));
            if (!read_int32(env, value, &n)) napi_throw_type_error(env, err_midi_message, err_expected_byte);
            else if (n < 0 || n > 255) napi_throw_range_error(env, err_midi_message, err_expected_byte);
            else s += (unsigned char)n;
        }
        Plg->Midi->MidiOutRaw(s);
    }
    XX (napi_get_undefined(env, &value));
    return value;
}


//// MidiIn

std::map<void*, napi_ref> Callbacks;
napi_threadsafe_function TSF;
napi_value MidiThread(napi_env env, napi_callback_info args) { return 0; }
void FinalizeThread(napi_env env, void* finalize_data, void* finalize_hint) { /* std::cout << "Finalize!!!\n"; */ }

void EventReceived(napi_env env, napi_value js_callback, void* context, void* data)
{
    napi_value func;
    napi_value undef;
    napi_value value;
    CEvent* Evt = (CEvent*)data;
    if (Evt->Msg) {
        CPlugin* Plg = Evt->Plg;
        napi_value arr[4];
        if (Plg->MidiInCall) {
//std::cout << "MIDI CALL IS NOT AVAILABLE!\n";
          if (Plg->MidiInArray) {
              if (napi_get_reference_value(env, *Plg->MidiInCall, &func)) return;
              if (napi_get_undefined(env, &undef)) return;
              if (napi_create_uint32(env, Evt->Msg->T, &arr[0])) return;
              if (napi_create_array(env, &arr[1])) return;
              for (size_t i = 0; i < Evt->Msg->V.size(); i++) {
                  if (napi_create_uint32(env, Evt->Msg->V[i], &value)) return;
                  if (napi_set_element(env, arr[1], i, value)) return;
              }
              napi_call_function(env, undef, func, 2, arr, &value);
          }
          else if (Evt->Msg->V.size() <= 3) {
              if (napi_get_reference_value(env, *Plg->MidiInCall, &func)) return;
              if (napi_get_undefined(env, &undef)) return;
              if (napi_create_uint32(env, Evt->Msg->T, &arr[0])) return;
              for (size_t i = 0; i < 3; i++) {
                  if (napi_create_uint32(env, i < Evt->Msg->V.size() ? Evt->Msg->V[i] : 0, &arr[i + 1])) return;
              }
              napi_call_function(env, undef, func, 4, arr, &value);
          }
        }
        delete Evt->Msg;
    }
    delete Evt;
}

size_t connect_count = 0;

void connect_thread(napi_env env)
{
    if (!connect_count) {
        napi_value value;
        napi_value func;
        if (napi_create_function(env, 0, 0, MidiThread, 0, &func)) return;
        if (napi_create_string_utf8(env, "no_name", NAPI_AUTO_LENGTH, &value)) return;
        if (napi_create_threadsafe_function(env, func, 0, value, 128, 1, 0, FinalizeThread, 0, EventReceived, &TSF)) return;
    }
    connect_count++;
}

void disconnect_thread(napi_env env)
{
    connect_count--;
    if (!connect_count) {
        napi_release_threadsafe_function(TSF, napi_tsfn_release);
    }
}

void PluginCallback(void* inst, void* ptr)
{
    CPlugin* Plg = (CPlugin*)inst;
    std::vector<unsigned char> v;
    Plg->Midi->ReadMidiInput(ptr, v);
    CMidiMsg* Msg = new CMidiMsg;
    Msg->T = Plg->Midi->Time();
    for (size_t i = 0; i < v.size(); i++) Msg->V.push_back(v[i]);
    CEvent* Evt = new CEvent();
    Evt->Plg = Plg;
    Evt->Msg = Msg;
    napi_call_threadsafe_function(TSF, Evt, napi_tsfn_nonblocking);
}

napi_value MidiInOpen(napi_env env, napi_callback_info args)
{
    size_t argc = 2;
    napi_value self;
    napi_value argv[2];
    napi_value value;
    CPlugin* Plg;
    std::string s;
    XX (napi_get_cb_info(env, args, &argc, argv, &self, 0));
    XX (napi_unwrap(env, self, (void**)&Plg));
    if (argc >= 2) {
        int32_t n;
        napi_valuetype type;
        XX (napi_typeof(env, argv[1], &type));
        if (type != napi_function) napi_throw_type_error(env, err_midi_callback, err_expected_function);
        XX (napi_get_named_property(env, argv[1], "length", &value));
        XX (napi_get_value_int32(env, value, &n));
        if (n == 2) Plg->MidiInArray = true;
        else if (n == 4) Plg->MidiInArray = false;
        else napi_throw_type_error(env, err_midi_callback, "callback function must have 2 or 4 arguments");
        if (Plg->MidiInCall) {
            XX (napi_delete_reference(env, *Plg->MidiInCall));
        }
        else Plg->MidiInCall.reset(new napi_ref);
        XX (napi_create_reference(env, argv[1], 1, &*Plg->MidiInCall));

        XX (napi_coerce_to_number(env, argv[0], &value));
        if (read_int32(env, value, &n)) {
            s = toUtf8(Plg->Midi->MidiInOpen(n, Plg));
        }
        else {
            XX (napi_coerce_to_string(env, argv[0], &value));
            s = read_utf8(env, value);
            str_type name = fromUtf8(s);
            s = toUtf8(Plg->Midi->MidiInOpen(name.c_str(), Plg));
        }
    }
    else {
        s = toUtf8(Plg->Midi->CurrentInName());
    }
    if (s.empty()) {
        if (Plg->Listening) { disconnect_thread(env); Plg->Listening = false; }
    }
    else {
        if (!Plg->Listening) { connect_thread(env); Plg->Listening = true; }
    }
    XX (napi_create_string_utf8(env, s.c_str(), NAPI_AUTO_LENGTH, &value));
    return value;
}

napi_value MidiInClose(napi_env env, napi_callback_info args)
{
    napi_value self;
    napi_value value;
    CPlugin* Plg;
    XX (napi_get_cb_info(env, args, 0, 0, &self, 0));
    XX (napi_unwrap(env, self, (void**)&Plg));
    Plg->Midi->MidiInClose();
    if (Plg->MidiInCall) {
        XX (napi_delete_reference(env, *Plg->MidiInCall));
        Plg->MidiInCall.reset();
    }
    if (Plg->Listening) { disconnect_thread(env); Plg->Listening = false; }
    XX (napi_get_undefined(env, &value));
    return value;
}

//// MIDI object

napi_ref MIDI_ctor;

void destroy(napi_env env, void* data, void* hint) { /* std::cout << "delete!\n"; */ delete (CPlugin*)data; }

napi_value MIDI(napi_env env, napi_callback_info args)
{
    size_t argc = 1;
    napi_value argv[1];
    napi_value self;
    napi_value value;
    bool bbb;
    XX (napi_get_cb_info(env, args, &argc, argv, &self, 0));
    XX (napi_get_reference_value(env, MIDI_ctor, &value));
    XX (napi_instanceof(env, self, value, &bbb));
    if (!bbb) {
        XX (napi_new_instance(env, value, argc, argv, &self));
        return self;
    }
    CPlugin* Plg = new CPlugin();
    XX (napi_wrap(env, self, Plg, destroy, 0, 0));
    return self;
}

#define STATIC_FUNC(func) { #func, 0, func, 0, 0, 0, napi_enumerable, 0 }
#define EXPORT_FUNC(func) \
  XX (napi_create_function(env, #func, NAPI_AUTO_LENGTH, func, 0, &value)) \
  XX (napi_set_named_property(env, exports, #func, value))

napi_value init(napi_env env, napi_value exports)
{
    napi_value value;

    napi_property_descriptor Midi[] = {
        STATIC_FUNC(MidiInClose),
        STATIC_FUNC(MidiInInfo),
        STATIC_FUNC(MidiInList),
        STATIC_FUNC(MidiInOpen),
        STATIC_FUNC(MidiOut),
        STATIC_FUNC(MidiOutClose),
        STATIC_FUNC(MidiOutInfo),
        STATIC_FUNC(MidiOutList),
        STATIC_FUNC(MidiOutLong),
        STATIC_FUNC(MidiOutOpen),
        STATIC_FUNC(MidiOutRaw),
        STATIC_FUNC(Support),
        STATIC_FUNC(Time)
    };

    XX (napi_define_class(env, "MIDI", NAPI_AUTO_LENGTH, MIDI, 0, sizeof Midi / sizeof Midi[0], Midi, &value));
    XX (napi_create_reference(env, value, 1, &MIDI_ctor));
    XX (napi_set_named_property(env, exports, "MIDI", value));

    XX (napi_get_boolean(env, true, &value));
    XX (napi_set_named_property(env, exports, "isJazz", value));
    XX (napi_create_string_utf8(env, VERSION, NAPI_AUTO_LENGTH, &value));
    XX (napi_set_named_property(env, exports, "version", value));

    EXPORT_FUNC(MidiInInfo);
    EXPORT_FUNC(MidiInList);
    EXPORT_FUNC(MidiOutInfo);
    EXPORT_FUNC(MidiOutList);
    EXPORT_FUNC(Support);
    EXPORT_FUNC(Time);
    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, init)
