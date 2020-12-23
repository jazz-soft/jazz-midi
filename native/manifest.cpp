#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

using namespace std;

int main(int argc, char* argv[])
{
    if (argc != 3) return 1;
    string src = argv[2];
    string path;
    for (size_t i = 0; i < src.length(); i++) if (src[i]=='\\') path += "\\\\"; else path += src[i];
    string json_chrome = argv[1]; json_chrome += ".chrome.json";
    string json_firefox = argv[1]; json_firefox += ".firefox.json";

    ofstream file_chrome(json_chrome, ios::out);
    file_chrome <<
"{\n\
  \"name\": \"com.jazz_soft.jazz_midi\",\n\
  \"description\": \"jazz-midi\",\n\
  \"path\": \"" << path << "\",\n\
  \"type\": \"stdio\",\n\
  \"allowed_origins\": [ \"chrome-extension://jhdoobfdaejmldnpihidjemjcbpfmbkm/\" ]\n\
}\n";
    ofstream file_firefox(json_firefox, ios::out);
    file_firefox <<
"{\n\
  \"name\": \"com.jazz_soft.jazz_midi\",\n\
  \"description\": \"jazz-midi\",\n\
  \"path\": \"" << path << "\",\n\
  \"type\": \"stdio\",\n\
  \"allowed_extensions\": [ \"jazz_midi@jazz_soft.com\" ]\n\
}\n";
    return 0;
}
