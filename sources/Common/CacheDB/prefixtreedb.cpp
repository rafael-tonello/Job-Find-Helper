#include  "prefixtreedb.h" 
 
PrefixTreeDB::PrefixTreeDB(string fileName) 
{ 
    this->fileName = fileName;
    this->db = new Trees::PatriciaFile<string>(
        fileName, 
        validKeyChars, 
        128, 
        string(""), 
        [](string value, char* bufferOut, const int maxSize){
            size_t index = 0;
            for (auto &c: value) 
                bufferOut[index++] = c;
            bufferOut[index++] = 0;
        }, 
        [](const char* bufferIn, const int maxSize){
            string result;
            int pos = 0;
            while (pos < maxSize && bufferIn[pos] != 0)
                result += bufferIn[pos++];

            return result;
        },
        MAX_VALUE_SIZE
    );
}

string PrefixTreeDB::ensureKeyChars(string key)
{
    stringstream ret;
    for (auto &c: key)
    {
        if (validKeyChars.find(c) != string::npos)
            ret << c;
        else //just replace by some other valid char to prevent empty keys and reduce colision possibility (caused by remotion of chars)
            ret << validKeyChars[((int)c) % validKeyChars.size()];
    }

    return ret.str();
}
 
PrefixTreeDB::~PrefixTreeDB() 
{ 
    delete db;
}

DynamicVar PrefixTreeDB::get(string name, DynamicVar defaultValue){
    name = ensureKeyChars(name);
    auto tmp = this->db->get(name, defaultValue.getString());

    if (tmp == LIST_VALUE_IDENTIFICATION)
    {
        string completeValue = "";
        int count = stoi(this->db->get(name + ".count", "0"));
        for (int c = 0; c < count; c++)
            completeValue += this->db->get(name + "." + to_string(c), "");

        return completeValue;
    }
    else
        return tmp;
}

void PrefixTreeDB::set(string name, DynamicVar value){
    name = ensureKeyChars(name);
    if (value.getString().size() <= MAX_VALUE_SIZE)
        this->db->set(name, value);
    else
    {
        this->db->set(name, LIST_VALUE_IDENTIFICATION);
        string tmpValue = value.getString();
        int count = 0;
        string part = "";
        while (true)
        {
            if (tmpValue.size() > MAX_VALUE_SIZE)
            {
                part = tmpValue.substr(0, MAX_VALUE_SIZE);
                tmpValue = tmpValue.substr(MAX_VALUE_SIZE);
            }
            else
            {
                part = tmpValue;
                tmpValue = "";
            }

            this->db->set(name + "." + to_string(count), tmpValue);

            count++;
            if (tmpValue == "")
                break;
        }

        this->db->set(name + ".count", to_string(count));
    }
}

 
