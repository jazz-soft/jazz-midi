template<class T> std::string toUTF8(const T& in)
{   std::string out;
    for(size_t i=0;i<in.length();i++)
    {	unsigned int c=0; c|=in[i];
        if(c<=0x7f)
        {	out+=(char)c; continue;
        }
        if(c<=0x7ff)
        {	out+=(char)((c>>6)|0xc0);
            out+=(char)((c&0x3f)|0x80);
            continue;
        }
        if(c<=0xffff)
        {	out+=(char)((c>>12)|0xe0);
            out+=(char)(((c>>6)&0x3f)|0x80);
            out+=(char)((c&0x3f)|0x80);
            continue;
        }
        if(c<=0x1fffff)
        {	out+=(char)((c>>18)|0xf0);
            out+=(char)(((c>>12)&0x3f)|0x80);
            out+=(char)(((c>>6)&0x3f)|0x80);
            out+=(char)((c&0x3f)|0x80);
            continue;
        }
        if(c<=0x3ffffff)
        {	out+=(char)((c>>24)|0xf8);
            out+=(char)(((c>>18)&0x3f)|0x80);
            out+=(char)(((c>>12)&0x3f)|0x80);
            out+=(char)(((c>>6)&0x3f)|0x80);
            out+=(char)((c&0x3f)|0x80);
            continue;
        }
        if(c<=0x7fffffff)
        {	out+=(char)((c>>30)|0xfc);
            out+=(char)(((c>>24)&0x3f)|0x80);
            out+=(char)(((c>>18)&0x3f)|0x80);
            out+=(char)(((c>>12)&0x3f)|0x80);
            out+=(char)(((c>>6)&0x3f)|0x80);
            out+=(char)((c&0x3f)|0x80);
            continue;
        }
    }
    return out;
}

template<class T> T fromUTF8(const std::string& in)
{	T out;
    for(size_t i=0;i<in.length();i++)
    {	if(!(in[i]&0x80))
        {	out+=static_cast<typename T::value_type>(in[i]); continue;
        }
        if((in[i]&0xe0)==0xc0)
        {	typename T::value_type w=0; w|=(in[i]&0x1f);
            if(i+1>=in.length() || (in[i+1]&0xc0)!=0x80) continue;
            i++; w<<=6; w|=(in[i]&0x3f);
            out+=w; continue;
        }
        if((in[i]&0xf0)==0xe0)
        {	typename T::value_type w=0; w|=(in[i]&0x0f);
            if(i+1>=in.length() || (in[i+1]&0xc0)!=0x80) continue;
            i++; w<<=6; w|=(in[i]&0x3f);
            if(i+1>=in.length() || (in[i+1]&0xc0)!=0x80) continue;
            i++; w<<=6; w|=(in[i]&0x3f);
            out+=w; continue;
        }
        if((in[i]&0xf8)==0xf0)
        {	typename T::value_type w=0; w|=(in[i]&0x07);
            if(i+1>=in.length() || (in[i+1]&0xc0)!=0x80) continue;
            i++; w<<=6; w|=(in[i]&0x3f);
            if(i+1>=in.length() || (in[i+1]&0xc0)!=0x80) continue;
            i++; w<<=6; w|=(in[i]&0x3f);
            if(i+1>=in.length() || (in[i+1]&0xc0)!=0x80) continue;
            i++; w<<=6; w|=(in[i]&0x3f);
            out+=w; continue;
        }
        if((in[i]&0xfc)==0xf8)
        {	typename T::value_type w=0; w|=(in[i]&0x03);
            if(i+1>=in.length() || (in[i+1]&0xc0)!=0x80) continue;
            i++; w<<=6; w|=(in[i]&0x3f);
            if(i+1>=in.length() || (in[i+1]&0xc0)!=0x80) continue;
            i++; w<<=6; w|=(in[i]&0x3f);
            if(i+1>=in.length() || (in[i+1]&0xc0)!=0x80) continue;
            i++; w<<=6; w|=(in[i]&0x3f);
            if(i+1>=in.length() || (in[i+1]&0xc0)!=0x80) continue;
            i++; w<<=6; w|=(in[i]&0x3f);
            out+=w; continue;
        }
        if((in[i]&0xfe)==0xfc)
        {	typename T::value_type w=0; w|=(in[i]&0x01);
            if(i+1>=in.length() || (in[i+1]&0xc0)!=0x80) continue;
            i++; w<<=6; w|=(in[i]&0x3f);
            if(i+1>=in.length() || (in[i+1]&0xc0)!=0x80) continue;
            i++; w<<=6; w|=(in[i]&0x3f);
            if(i+1>=in.length() || (in[i+1]&0xc0)!=0x80) continue;
            i++; w<<=6; w|=(in[i]&0x3f);
            if(i+1>=in.length() || (in[i+1]&0xc0)!=0x80) continue;
            i++; w<<=6; w|=(in[i]&0x3f);
            if(i+1>=in.length() || (in[i+1]&0xc0)!=0x80) continue;
            i++; w<<=6; w|=(in[i]&0x3f);
            out+=w; continue;
        }
    }
    return out;
}

typedef wchar_t char_type;
typedef std::wstring str_type;

inline std::string toUtf8(const str_type&s){ return toUTF8<str_type>(s);}
inline str_type fromUtf8(const std::string&s){ return fromUTF8<str_type>(s);}
