#include  "patriciadb.h" 
 
PatriciaDB::PatriciaDB(string fileName) 
{ 
    this->fileName = fileName;
    this->db = new Trees::PatriciaFile<string>(
        fileName, 
        string("abcdefghijklmnopqrstuvxywz./:0123456789_-"), 
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
 
PatriciaDB::~PatriciaDB() 
{ 
    delete db;
}

DynamicVar PatriciaDB::get(string name, DynamicVar defaultValue){
    return this->db->get(name, defaultValue.getString());
}

void PatriciaDB::set(string name, DynamicVar value){
    this->db->set(name, value);
}

 
