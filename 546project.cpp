
#include "json/json.h"
#include <iostream>
#include <fstream>
#include <unordered_map>
using namespace std;

    stringbuf encode(vector<unsigned char> input) {
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
    vector<unsigned char> decode(vector<unsigned char> input) {
        vector<unsigned char> output;
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
    vector<unsigned char> delta_encode(vector<unsigned char> vec)
    {
        unsigned char last = 0;
        for (int i = 0; i < vec.size(); i++)
        {
            unsigned char current = vec[i];
            vec[i] = current - last;
            last = current;
        }
        return vec;
    }
    vector<unsigned char> delta_decode(vector<unsigned char> vec)
    {
        unsigned int last = 0;
        for (int i = 0; i < vec.size(); i++)
        {
            unsigned int  delta = vec[i];
            vec[i] = delta + last;
            last = vec[i];
        }
        return vec;
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
        vector<unsigned char> vec;
        for (int i = 0; i < stringoutput.size(); i++)
        {
            vec.push_back(stringoutput[i]);
        }
        if (!test)
            output << stringoutput;
        else {
            vector<unsigned char> vec2;
            vec = delta_encode(vec);
            string encoded = encode(vec).str();
            for (int i = 0; i < encoded.size(); i++)
            {
                vec2.push_back(encoded[i]);
            }
            vector<unsigned char> test3 = decode(vec2);
            test3 = delta_decode(test3);
            string hi = "";
            for (int i = 0; i < test3.size(); i++)
            {
                hi += test3[i];
            }
            output << hi;
        }
        output.close();
        return 0;
}