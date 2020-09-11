
#include "json/json.h"
#include <iostream>
#include <fstream>
#include <unordered_map>
using namespace std;

    stringbuf encode(vector<int> input) {
        stringbuf output;
    for (int i : input) {
        while (i >= 128) {
            output.sputc(i & 0x7F);
            unsigned int temp = i;
            temp >>= 7;
            i = temp;
            }
        output.sputc(i | 0x80);
        }
    return output;
    }
    vector<char> decode(vector<char> input) {
        vector<char> output;
        for (int i = 0; i < input.size(); i++) {
            int position = 0;
            int result = ((int)input[i] & 0x7F);
            while ((input[i] & 0x80) == 0) {
                i += 1;
                position += 1;
                int unsignedByte = ((int)input[i] & 0x7F);
                result |= (unsignedByte << (7 * position));
            }
            output.push_back(result);
        }
        return output;
    }
    int main() {
        int i = 1;
        bool test = true;
        string sentence;
        unordered_map<string, vector<pair<int, int>>> map;
        Json::Value root;
        ifstream stream("shakespeare-scenes.json", std::ifstream::binary);
        stream >> root;
        for (auto itr : root["corpus"]) {
            string word;
            int j = 1;
            sentence = itr["text"].asString();
            istringstream stream(sentence);
            while (stream >> word)
            {
                map[word].push_back({ i,j });
                j++;
            }
            i++;
        }
        Json::Value res;
        ofstream output;
        output.open("output.txt");
        for (auto itr : map)
        {
            Json::Value temp;
            for (int i = 0; i < itr.second.size(); i++)
            {
                temp.append(to_string(itr.second[i].first) + "," + to_string(itr.second[i].second));
            }
            res[itr.first].append(temp);
        }
        Json::StreamWriterBuilder builder;
        string stringoutput = Json::writeString(builder, res);
        vector<int> vec;
        vec.push_back(stringoutput[0]);
        unsigned int last = 0;
        for (int i = 0; i < stringoutput.size(); i++)
        {
            vec.push_back(stringoutput[i]);
        }
        if (!test)
            output << stringoutput;
        else {
            vector<char> vec2;
            string encoded = encode(vec).str();
            vec2.push_back(encoded[0]);
            unsigned char last2 = 0;
            for (int i = 0; i < encoded.size(); i++)
            {
                vec2.push_back(encoded[i]);
            }
            vector<char> test3 = decode(vec2);
            string hi = "";
            for (int i = 0; i < encoded.size(); i++)
            {
                hi += test3[i];
            }
            output << hi;
        }
        output.close();
        return 0;
}