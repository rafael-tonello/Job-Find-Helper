#ifndef __PATRICIA_HPP_
#define __PATRICIA_HPP_

#include <iostream>
#include <functional>
#include <string>
#include <fstream>
#include <vector>
#include <map>
#include <mutex>
#include "file.h"
#include <string.h>


namespace Trees {

    #define forS(vName,from,to, body) for(auto vName = from; vName < to; vName ++)body
    #define for0to(vName, to, body) for(auto vName = 0; vName < to; vName ++)body

    #define ADDRESS_t int

    using namespace std;

    template <class T>
    using SerializeFunc = function<void (T value, char* bufferOut, const int maxValueSize)>;

    template<class T>
    using DeserializeFunc = function<T(const char* bufferIn, const int maxValueSize)>;


    template<class T>
    struct Node{
        //map<string, T> childs;
        ADDRESS_t childsAddress[256];
        string key;
        T value;
        bool valueAlreadySet = false;
    };

    template <typename T>
    class PatriciaFile
    {
    public:
        SerializeFunc<T> serialize;
        DeserializeFunc<T> deserialize;

    private:

        int maxKeySize;
        int maxValueSize;
        string allowedCharsForName;
        CFile file;
        T emptyValue;
        mutex accessLocker;

        Node<T> createEmptyNode()
        {
            Node<T> ret;
            ret.key = "";
            ret.value = emptyValue;
            ret.valueAlreadySet = false;
            /*auto bytes = this->allowedCharsForName.size() * sizeof(ADDRESS_t);
            ret.childsAddress = new ADDRESS_t[bytes];*/

            for (int i = 0; i < this->allowedCharsForName.size(); i++)
                ret.childsAddress[i] = 0;

            return ret;
        }

        //this files writes the initial data ina new file. This function will be called only if
        //a new file needs to be created
        void prepareNewFile()
        {
            Node<T> rootNode = this->createEmptyNode();
            rootNode.key = "";

            this->file.seek(0);
            //this->file.seekp(0);
            //write the file header
            //"KWPF100   "
            // ||||||||
            // ||||||+----> Improvments and bugs
            // |||||+-----> New functionalities
            // ||||+------> Major version
            // |||+-------> File
            // ||+--------> Patricia
            // |+---------> kiWiisco
            // +----------> Kiwiisco
            string header = "KWPF100   ";
            this->file.write(header.c_str(), header.size());

            //create and write the root node
            this->writeNode(rootNode);

        }


        /* this function receives a char and returns its index in the 'allowedCharsForName'.
         * For instance, if allowd chars are "0123456789ABCDEF" and this functions receives the
         * char 'C' to find its index, the value '12' will be returned */
        size_t chartToChildIndex(char ch)
        {
            auto ret = this->allowedCharsForName.find(ch);
            if (ret == string::npos)
            {
                string error = "Invalid char in key name: ";
                error += ch;
                error += " (";
                error += std::to_string((int)ch);
                error += "). Allowed chars are: '"+this->allowedCharsForName+"'";
                throw std::runtime_error(error);
            }

            return ret;
        }

        //return the equality between two strings, for instance:
        // to strings 'banana' and 'bacana', returns 'ba'.
        string getEquality(string str1, string str2)
        {
            string less, bigger;
            string ret = "";
            //determines the less and the biggest value
            if (str1.size() > str2.size())
            {
                less = str1;
                bigger = str2;
            }
            else
            {
                less = str2;
                bigger = str1;
            }

            for0to(i, less.size(), {
                if (less[i] == bigger[i])
                    ret += less[i];
                else
                    break;
            });

            return ret;
        }

        //the lastValid parameters is a output that is used by function "search". It's the
        //last valid node found during search in the three. It argumen exists to just one
        //method with the Patricia Tree algorithm. 'set' and 'get' functions ignores this parameter

        Node<T> getRootNode()
        {
            this->file.seek(10);
            //this->file.seekp(10);
            Node<T> rootNode = this->readNode();
            return rootNode;
        }

        Node<T> locateNode(string name, bool readOnly, bool &sucess, Node<T> &lastValidNode)
        {
            if (name != "")
            {
                //read the root node
                this->file.seek(10);
                //this->file.seekp(10);
                Node<T> rootNode = this->readNode();
                ADDRESS_t nextRecordAddress;
                Node<T> currentNode;
                lastValidNode = rootNode;

                //locate the address of first letter
                char firstLetter = name[0];
                auto indexOnChilds = this->chartToChildIndex(firstLetter);
                nextRecordAddress = rootNode.childsAddress[indexOnChilds];

                //chec if the address is a invalid record
                if (nextRecordAddress == 0)
                {
                    if (!readOnly)
                    {
                        //create a new record in the end of the file and upate the root node with the address of this new record
                        {
                            //create record
                            auto newNode = this->createEmptyNode();
                            newNode.key = name;

                            //write the new node to end of the file
                            this->file.seekToEnd();
                            //this->file.seekp(0, ios_base::end);

                            //save the addres of the new node
                            auto addressOfNewNode = this->file.tell();
                            this->writeNode(newNode);

                            //update the root node
                            rootNode.childsAddress[indexOnChilds] = addressOfNewNode;
                            this->file.seek(10);
                            //this->file.seekp(10, ios_base::beg);

                            this->writeNode(rootNode);

                            //point to new record and return
                            this->file.seek(addressOfNewNode);
                            sucess = true;
                            return newNode;
                        }
                    }
                    else
                    {
                        sucess = false;
                        //return false;
                    }
                }
                else
                {
                    //!!!!recursive functions can be used here
                    return locateNode_readNext(10, nextRecordAddress, name, readOnly, sucess, lastValidNode);

                }
            }
            sucess = false;
            //return false;
            return createEmptyNode();
        }

        Node<T> locateNode_readNext(ADDRESS_t parentNodeAddress, ADDRESS_t nextRecordAddress, string name, bool readOnly, bool &sucess, Node<T> &lastValidNode)
        {

            Node<T> node;
            string equality;

            //chec if the address of next record is a valid address
            if (nextRecordAddress != 0)
            {
                //jump to next record and read it
                 this->file.seek(nextRecordAddress);
                //this->file.seekp(nextRecordAddress);
                node = this->readNode();
                //checks if current node is which the function is looking for
                if (node.key == name)
                {
                    lastValidNode = node;
                    //just seek the file to address and return (the node was located)
                    sucess = true;
                    this->file.seek(nextRecordAddress);
                    return node;
                    //this->file.seekp(nextRecordAddress);
                }
                else
                {
                    //find the equality between current node and the 'name'
                    equality = getEquality(name, node.key);

                    if (equality.size() == node.key.size())
                    {
                        //just a security check (this never can be false)
                        if (name.size() > node.key.size())
                        {
                            //search in child nodes
                            char searchBy = name[node.key.size()];
                            auto indexInChilds = this->chartToChildIndex(searchBy);
                            auto addr = node.childsAddress[indexInChilds];

                            if (addr > 0)
                            {
                                lastValidNode = node;
                                return this->locateNode_readNext(nextRecordAddress, addr, name, readOnly, sucess, lastValidNode);
                            }
                            else
                            {
                                if (!readOnly)
                                {
                                    //if not found, create a new one
                                    {
                                        auto newNode = this->createEmptyNode();
                                        newNode.key = name;
                                        //append the new node to the file
                                        this->file.seekToEnd();
                                        //this->file.seekp(0, ios_base::end);
                                        auto newRecordAddress = this->file.tell();
                                        this->writeNode(newNode);
                                        //udate the current node
                                        {
                                            //set the address of the new child
                                            node.childsAddress[indexInChilds] = newRecordAddress;
                                            //points to the current node (in the file)
                                            this->file.seek(nextRecordAddress);
                                            //this->file.seekp(nextRecordAddress);
                                            //rewrite the current node to the file
                                            this->writeNode(node);
                                            //resseks to the new node and return true
                                            this->file.seek(newRecordAddress);
                                            //this->file.seekp(newRecordAddress);
                                            sucess = true;
                                            return newNode;
                                        }
                                    }
                                }
                                else
                                {
                                    sucess = false;
                                    return createEmptyNode();
                                    //return false;
                                }
                            }
                        }
                        else
                            throw std::runtime_error("Bug in the patricia tree alghoritm");
                    }
                    else //if (equality.size() < node.key.size())
                    {
                        if (!readOnly)
                        {
                            //create a new parent node (rotates the tree)
                            {

                                Node<T> newChild;
                                Node<T> newParent;
                                //create the node
                                newParent = this->createEmptyNode();
                                newParent.key = equality;

                                string ret ="newParent";

                                //add current node as child
                                char searchBy = node.key[newParent.key.size()];
                                auto indexInChilds = this->chartToChildIndex(searchBy);
                                newParent.childsAddress[indexInChilds] = nextRecordAddress;

                                //checks if new parent node is the same as 'name' or a new child must be created
                                auto finalFilePositionForReturn = 0;
                                if (equality != name)
                                {
                                    newChild = this->createEmptyNode();
                                    newChild.key = name;

                                    //add new child to end of file (and update the newParent)
                                    this->file.seekToEnd();
                                    //this->file.seekp(0, ios_base::end);
                                    finalFilePositionForReturn = this->file.tell();

                                    searchBy = newChild.key[newParent.key.size()];
                                    indexInChilds = this->chartToChildIndex(searchBy);
                                    newParent.childsAddress[indexInChilds] = this->file.tell();

                                    this->writeNode(newChild);

                                    ret = "newChild";
                                }

                                //create the record in the end of the file
                                this->file.seekToEnd();
                                //this->file.seekp(0, ios_base::end);
                                if (finalFilePositionForReturn == 0)
                                    finalFilePositionForReturn = this->file.tell();

                                auto newParentAddress = this->file.tell();
                                this->writeNode(newParent);

                                //update the old parent of current node
                                {
                                    //read the old parent node
                                    this->file.seek(parentNodeAddress);
                                    //this->file.seekp(parentNodeAddress);
                                    auto oldParent = this->readNode();

                                    //update the information of the old parent node
                                    searchBy = newParent.key[oldParent.key.size()];
                                    indexInChilds = this->chartToChildIndex(searchBy);
                                    oldParent.childsAddress[indexInChilds] = newParentAddress;

                                    //rewerite the old parent
                                    this->file.seek(parentNodeAddress);
                                    //this->file.seekp(parentNodeAddress);
                                    this->writeNode(oldParent);
                                }

                                //point file handle to target node
                                this->file.seek(finalFilePositionForReturn);
                                //this->file.seekp(finalFilePositionForReturn);
                                sucess = true;

                                if (ret == "newParent")
                                {
                                    return newParent;
                                }
                                else
                                {
                                    return newChild;
                                }
                            }
                        }
                        else
                        {
                            sucess = false;
                            return createEmptyNode();
                            //return false;
                        }
                    }
                }
            }
            sucess = false;
            return createEmptyNode();
            //return false;
        }

        char *readBuffer = NULL;
        char *readBufferValue = NULL;
        int readBufferSize = 0;
        int childsTotalBytes = 0;
        Node<T> readNode()
        {
            Node<T> result = createEmptyNode();
            string temp = "";
            if (readBuffer == NULL)
            {
                readBufferSize =    this->maxKeySize +
                                    this->maxValueSize +
                                    this->allowedCharsForName.size() * sizeof(ADDRESS_t) +
                                    1; //valueAlreadySet property

                childsTotalBytes = sizeof(ADDRESS_t) * this->allowedCharsForName.size();

                readBuffer = new char[readBufferSize];
                readBufferValue = new char[maxValueSize];
            }

            int currBufferIndex = 0;
            ADDRESS_t tempAddr = 0;
            char* tempAddrAsArr = (char*)(&tempAddr);

            //read the name, the value and the 'alreadySet' from  the file
            {
                this->file.read(readBuffer, this->maxKeySize + this->maxValueSize +1);

                //read the name from the file

                bool continueInserting = true;
                for0to(i, this->maxKeySize, {
                    if (continueInserting)
                    {
                        if (readBuffer[currBufferIndex] != 0)
                            result.key += readBuffer[currBufferIndex];
                        else
                            continueInserting = false;

                    }
                    currBufferIndex++;
                });

                //read the value from the file
                for0to(i, this->maxValueSize, {
                    readBufferValue[i] = readBuffer[currBufferIndex++];
                });
                result.value = this->deserialize(readBufferValue, this->maxValueSize);
                result.valueAlreadySet = readBuffer[currBufferIndex++] == 1;
            }

            //read child addresses from the file
            this->file.read((char*)result.childsAddress, childsTotalBytes);

            return result;
        }

        char* writeBuffer = NULL;
        char* weriteBufferValue = NULL;
        int writeBufferSize = 0;

        void writeNode(Node<T> node)
        {
            //prepare the buffer to receive the node data
            if (writeBuffer == NULL)
            {
                writeBufferSize =    this->maxKeySize +
                                    this->maxValueSize +
                                    this->allowedCharsForName.size() * sizeof(ADDRESS_t) +
                                    1; //valueAlreadySet

                writeBuffer = new char[writeBufferSize];

                weriteBufferValue = new char[maxValueSize];
                childsTotalBytes = sizeof(ADDRESS_t) * this->allowedCharsForName.size();


            }

            //write the name, the value and the 'alreadySet' to the file
            {
                int currBufferIndex = 0;
                //add the name to the buffer
                for0to(i, this->maxKeySize, {
                    writeBuffer[currBufferIndex++] = i < node.key.size() ? node.key[i] : 0;
                });

                //add the value to the buffer
                //string valueAsString = this->serialize(node.value);

                this->serialize(node.value, weriteBufferValue, this->maxValueSize);
                for0to(i, this->maxValueSize, {
                    writeBuffer[currBufferIndex++] = weriteBufferValue[i];
                });

                //werite valueAlreadySet property to the buffer
                writeBuffer[currBufferIndex++] = node.valueAlreadySet == true ? 1 : 0;


                this->file.write(writeBuffer, this->maxKeySize + this->maxValueSize + 1);
            }

            //write the childs to the byte
            this->file.write((char*)node.childsAddress, childsTotalBytes);
        }

        vector<tuple<string, T>> search_getChildNames(string search, Node<T> node, bool returnValues = true, int maxResults = -1, int maxDeep = -1, int currDeep = 0, int currResults = 0){
            vector<tuple<string, T>> ret;
            Node<T> tempChild;
            vector<tuple<string, T>> childNamesAndValues;

            if (maxDeep > 0 && currDeep > maxDeep)
                return ret;

            if (node.valueAlreadySet && node.key.find(search) != string::npos)
            {

                ret.push_back({node.key, returnValues ? node.value : this->emptyValue});

                currResults++;
                if (maxResults > 0 && currResults >= maxResults)
                    return ret;
            }

            //read child nodes
            for (int cn = 0; cn < this->allowedCharsForName.size(); cn++)
            {
                auto n = node.childsAddress[cn];
                //checks if current address is valid
                if (n > 0)
                {
                    this->file.seek(n);
                    tempChild = this->readNode();

                    if (tempChild.key.find(search) != string::npos)
                    {
                        childNamesAndValues = search_getChildNames(search, tempChild, returnValues, maxResults, maxDeep, currDeep + 1, currResults);
                        for (auto currKey: childNamesAndValues)
                        {
                            ret.push_back(currKey);
                            currResults++;
                            if (maxResults > 0 && currResults >= maxResults)
                                return ret;
                        }
                    }
                }
            }
            return ret;
        };

        void init(
            string filename,
            string allowedCharsForName,
            int maxKeySize,
            int maxValueSize,
            T emptyValue,
            SerializeFunc<T> serialize,
            DeserializeFunc<T> deserialize
        ){
            this->setSerializeFunc(serialize);
            this->setDeserializeFunc(deserialize);
            this->maxKeySize = maxKeySize;
            this->maxValueSize = maxValueSize > -1 ? maxValueSize : sizeof(T);
            this->allowedCharsForName = allowedCharsForName;
            this->emptyValue = emptyValue;

            if(this->file.open(filename))
            {
                //checks if is a new file
                if (this->file.fileSize() < 10)
                {
                    this->prepareNewFile();
                }
            }
            else
            {
                string error = "Could not open the file ";
                error += filename;
                throw std::runtime_error(error);
            }

        };
    public:
        PatriciaFile(
            string filename,
            string allowedCharsForName,
            int maxKeySize,
            T emptyValue,
            SerializeFunc<T> serialize,
            DeserializeFunc<T> deserialize,
            int maxValueSize = -1
        ){
            this->init(filename, allowedCharsForName, maxKeySize, maxValueSize, emptyValue, serialize, deserialize);
        };

        PatriciaFile(
            string filename,
            string allowedCharsForName,
            int maxKeySize,
            T emptyValue,
            int maxValueSize = -1
        ){
            this->init(filename, allowedCharsForName, maxKeySize, maxValueSize, emptyValue,
                [](T val, char* bufOut, const int maxValueSize){
                    /*char* bufP = (char*)&val;
                    for (int c = 0; c < sizeof(T) && c < maxValueSize; c++)
                        bufOut[c] = bufP[c];*/
                    memcpy(bufOut, &val, maxValueSize);
                },
                [this](const char* bufIn, const int maxValueSize){
                    /*float ret;
                    char* retP = (char*)&ret;
                    for (int c = 0; c < maxValueSize; c++)
                        retP[c] = bufIn[c];

                    return ret;*/
                    T ret;
                    memcpy(&ret, bufIn, maxValueSize);
                    return ret;
                }
            );
        };

        ~PatriciaFile()
        {
            //this->file.flush();
            this->file.close();
        }

        void setSerializeFunc(SerializeFunc<T> func)
        {
            this->serialize = func;
        }

        void setDeserializeFunc(DeserializeFunc<T> funct)
        {
            this->deserialize = funct;
        }

        void set(string name, T value)
        {
            if (name == "")
                throw std::runtime_error("The name canno be empty");
            accessLocker.lock();

            bool sucess = false;
            Node<T> lastValid_Ignore = createEmptyNode();
            Node<T> node = this->locateNode(name, false, sucess, lastValid_Ignore);
            if (sucess)
            {
                node.value = value;
                node.valueAlreadySet = true;
                this->writeNode(node);
                accessLocker.unlock();
            }
            else
            {
                accessLocker.unlock();
                throw std::runtime_error("An error as occured during a setting operation.");
            }
        }

        T get(string name, T defaultValue)
        {
            if (name == "")
                throw std::runtime_error("The name cannot be empty");

            accessLocker.lock();
            bool sucess = false;
            Node<T> lastValid_Ignore = createEmptyNode();
            Node<T> node = this->locateNode(name, true, sucess, lastValid_Ignore);
            auto ret = node.value;
            if (sucess)
            {
                accessLocker.unlock();
                return ret;
            }
            else
            {
                accessLocker.unlock();
                return defaultValue;
            }
        }

        vector<string> getChildNames(string parentName)
        {
            Node<T> node;
            bool sucess = false;
            if (parentName == "")
            {
                node = this->getRootNode();
                sucess = true;
            }
            else
            {
                Node<T> lastValid_Ignore = createEmptyNode();
                node = this->locateNode(parentName, true, sucess, lastValid_Ignore);
            }

            if (sucess)
            {
                vector<string> result;
                accessLocker.lock();
                for (auto &c: node.childsAddress)
                {
                    if (c != 0)
                    {
                        file.seek(c);
                        Node<T> childNode = this->readNode();
                        result.push_back(childNode.key);
                    }
                }

                accessLocker.unlock();

                return result;
            }
            else
                return {};
        }


        vector<tuple<string, T>> search(string search, bool returnValues = true, int maxResults = -1, int maxDeep = -1)
        {
            if (search == "")
                throw std::runtime_error("The name canno be empty");

            accessLocker.lock();

            vector<tuple<string, T>> ret;
            bool sucess = false;
            Node<T> lastValidNode;
            lastValidNode.key = "";
            //auto tmp = this->locateNode(search, true, sucess, lastValidNode);

            //sucess argument is not used here. Instead, it is used the 'lastValidNode'
            //if (lastValidNode.key != "")
            {
                //scrolls the tree getting all valid child names
                ret = this->search_getChildNames(search, lastValidNode, returnValues, maxResults, maxDeep);
            }
            //else
            //    return {};
            accessLocker.unlock();
            return ret;
        }
    };

    //PrefixThreeFile<double>("/media/externalmemory/cache.db", "0123456789.", 100, [](double value){ return std::to_string(value);}, [](string value){return std::atof(value);})
}

#endif
