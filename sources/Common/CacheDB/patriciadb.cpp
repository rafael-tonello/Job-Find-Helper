#include  "patriciadb.h" 
 
PatriciaDB::PatriciaDB(string fileName) 
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
        128
    );
}

string PatriciaDB::ensureKeyChars(string key)
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
 
PatriciaDB::~PatriciaDB() 
{ 
    delete db;
}

DynamicVar PatriciaDB::get(string name, DynamicVar defaultValue){
    return this->db->get(ensureKeyChars(name), defaultValue.getString());
}

void PatriciaDB::set(string name, DynamicVar value){
    name = ensureKeyChars(name);
    this->db->set(name, value);
}

 
