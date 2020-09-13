
#include "json/json.h"
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <time.h>
#include <chrono> 

using namespace std;

ofstream diceoutput;
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
    void generateUncoded()
    {
        int i = 1;
        int thisscene = 0;
        int minscene = INT_MAX;
        int currplaylength = 0;
        int longestplay = 0;
        string longestplayid;
        string shortestplayid;
        int shortestplay = INT_MAX;
        int totalscene = 0;
        string currplay = "othello";
        Json::Value res;
        string sentence;
        unordered_map<string, vector<pair<int, int>>> map;
        Json::Value root;
        ifstream stream("shakespeare-scenes.json", std::ifstream::binary);
        stream >> root;
        for (auto itr : root["corpus"]) {
            if (currplay.compare(itr["playId"].asString()) !=0)
            {
                if (currplaylength > longestplay)
                {
                    longestplay = currplaylength;
                    longestplayid = itr["playId"].asString();
                }
                if (currplaylength < shortestplay)
                {
                    longestplay = currplaylength;
                    shortestplayid = itr["playId"].asString();
                }
                currplaylength = 0;
                currplay = itr["playId"].asString();
            }
            thisscene = 0;
            string word;
            int j = 1;
            sentence = itr["text"].asString();
            istringstream stream(sentence);
            while (stream >> word)
            {
                thisscene++;
                Json::Value temp;
                temp.append(i);
                temp.append(j);
                res[word].append(temp);
                j++;
            }
            totalscene += thisscene;
            currplaylength++;
            minscene = min(thisscene, minscene);
            i++;
        }
        cout << "Shortest Scene Length: " << minscene;
        cout << "Average Scene Length: " << totalscene / 747;
        cout << "Longest Play: " << longestplayid;
        cout << "Shortest Play: " << shortestplayid;
        ofstream output;
        output.open("uncoded_output.txt");
        Json::StreamWriterBuilder builder;
        string stringoutput = Json::writeString(builder, res);
        vector<unsigned char> vec;
        for (int i = 0; i < stringoutput.size(); i++)
        {
            vec.push_back(stringoutput[i]);
        }
        string encoded = encode(vec).str();
        output << encoded;
        output.close();
    }
    void generateCoded()
    {
        int i = 1;
        Json::Value res;
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
                Json::Value temp;
                temp.append(i);
                temp.append(j);
                res[word].append(temp);
                j++;
            }
            i++;
        }
        ofstream output;
        output.open("coded_output.txt");
        Json::StreamWriterBuilder builder;
        string stringoutput = Json::writeString(builder, res);
        vector<unsigned char> vec;
        for (int i = 0; i < stringoutput.size(); i++)
        {
            vec.push_back(stringoutput[i]);
        }
        vec = delta_encode(vec);
        string encoded = encode(vec).str();
        output << encoded;
        output.close();
    }
    vector<string> getValues(Json::Value json)
    {
        vector<string> res;
        Json::ValueIterator itr;
        for (itr = json.begin();itr!=json.end();itr++)
        {
            res.push_back(itr.key().asString());
        }
        return res;
    }
    map<string,unordered_set<int>> getInfo(Json::Value json)
    {
        map<string, unordered_set<int>> m;
        unordered_set<int> s;
        m["termfrequency"].insert(json.size());
        for (int i = 0; i < json.size(); i++)
        {
            auto num = json[i][0];
            if (num.isInt())
            {
                s.insert(num.asInt());
            }
        }
        m["documentfrequency"] = s;
        return  m;
    }
    void calculateDice(map < string, map<string, unordered_set<int>>> mapping)
    {
        double maxdice = -1;
        pair<string,string> maxpair = {};
        for (auto element1 = mapping.begin(); element1 != mapping.end(); ++element1) {
            for (auto element2 = std::next(element1); element2 != mapping.end(); ++element2) {
                unordered_set<int> tempset;
                tempset.insert(element1->second["documentfrequency"].begin(), element1->second["documentfrequency"].end());
                tempset.insert(element2->second["documentfrequency"].begin(), element2->second["documentfrequency"].end());
                double a = element1->second["documentfrequency"].size();
                double b = element2->second["documentfrequency"].size();
                double ab = (a+b)-tempset.size();
                double dice = (2 * ab) / (a + b);
                if (dice > maxdice)
                {
                    maxdice = dice;
                    maxpair = { element1->first,element2->first };
                }
            }
        }
        diceoutput << maxpair.first << " " << maxpair.second << endl;
    }
    void queryRandomTerms(vector<string> values, Json::Value json, int count, int times)
    {
        ofstream output;
        output.open("chosen" + to_string(count) + ".txt");
        for(int i = 0; i < times; i++)
        {
            map < string, map<string, unordered_set<int>>> mapping;
            for (int j = 0; j < count; j++)
            {
                string word = values[rand() % values.size()];
                map<string,unordered_set<int>> m = getInfo(json[word]);
                mapping.insert({ word, m });
                output << word + ", ";
            }
            calculateDice(mapping);
            output << "\n";
        }
    }
    void evaluateIndices()
    {
        int terms;
        int repeats;
        int quit = 0;
        int choice;
        while (quit != 1) {
            cout << "how many terms do you want";
            cin >> terms;
            cout << "how many times do you want them";
            cin >> repeats;
            cout << "uncoded or coded? 1/2";
            cin >> choice;
            if (choice == 1)
            {
                string uncoded;
                string decompressed;
                ifstream input("uncoded_output.txt");
                input >> uncoded;
                vector<unsigned char> vec;
                for (int i = 0; i < uncoded.size(); i++)
                {
                    vec.push_back(uncoded[i]);
                }
                vector<unsigned char> vec3 = decode(vec);
                for (int i = 0; i < vec3.size(); i++)
                {
                    decompressed += vec3[i];
                }
                auto start = chrono::high_resolution_clock::now();
                Json::Value json;
                Json::CharReaderBuilder builder;
                Json::CharReader* reader = builder.newCharReader();
                string errors;
                bool parsingSuccessful = reader->parse(decompressed.c_str(), decompressed.c_str() + decompressed.size(), &json, &errors);
                vector<string> values = getValues(json);
                queryRandomTerms(values, json, terms, repeats);
                auto stop = chrono::high_resolution_clock::now();
                auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start);
                cout << "Time it took to run query for " << to_string(terms) << " Terms " <<
                    to_string(repeats) << " Times: " << to_string(duration.count()) << "ms" << endl;
            }
            else if (choice == 2)
            {
                auto start = chrono::high_resolution_clock::now();
                ifstream stream2("coded_output.txt", std::ifstream::binary);
                string encoded;
                stream2 >> encoded;
                vector<unsigned char> vec2;
                for (int i = 0; i < encoded.size(); i++)
                {
                    vec2.push_back(encoded[i]);
                }
                vector<unsigned char> test3 = decode(vec2);
                test3 = delta_decode(test3);
                string decoded = "";
                for (int i = 0; i < test3.size(); i++)
                {
                    decoded += test3[i];
                }
                Json::Value json;
                Json::CharReaderBuilder builder;
                Json::CharReader* reader = builder.newCharReader();
                string errors;
                bool parsingSuccessful = reader->parse(decoded.c_str(), decoded.c_str() + decoded.size(), &json, &errors);
                vector<string> values = getValues(json);
                queryRandomTerms(values, json, terms, repeats);
                auto stop = chrono::high_resolution_clock::now();
                auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start);
                cout << "Time it took to run query for " << to_string(terms) << " Terms " <<
                    to_string(repeats) << " Times: " << to_string(duration.count()) << "ms" << endl;
            }
            cout << "Press 1 to exit any other to continue" << endl;
            cin >> quit;
        }
    }
    int main() {
        diceoutput.open("highscore.txt");
        srand(time(NULL));
        int choice = 0;
        while (true) {
            cout << "Please choose what you would like to do: \n"
                << "-1: Exit \n"
                << "1: Generate Uncoded Inverted Index \n"
                << "2: Generate Coded Inverted Index \n"
                << "3: Evaluate Inverted Indices";
            cin >> choice;
            switch (choice) {
            case -1: {
                return 0;
            }
            case 1: {
                generateUncoded();
                break;
            }
            case 2: {
                generateCoded();
                break;
            }
            case 3: {
                evaluateIndices();
                break;
            }
            }
        }
        return 0;
}