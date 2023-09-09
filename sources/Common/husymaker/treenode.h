#ifndef __TREENODE__H__ 
#define __TREENODE__H__ 

#include <map>
#include <vector>
#include <string>
#include <DynamicVar.h>
#include <TaggedObject.h>
#include <utils.h>


using namespace std;

//#define INVALID_VALUE "__w__a__s__nev__er__s__e__t___";
#define INVALID_VALUE "undefined"
class TreeNode: public TaggedObject{
public:
    enum ValueType{STRING, INT, DOUBLE, BOOLEAN, NULLVALUE, COMMENT, AUTO_DETECT };
    enum NodeType{ARRAY, OBJECT, VALUE};
    using CreateNodF = function<TreeNode(TreeNode*)>;
protected:
    map<string, TreeNode> childs;
    vector<string> namesInInsertionOrder;
    ValueType valueType = ValueType::AUTO_DETECT;

    bool isArray();

    /**
     * @brief this function allow createaton derivations of TreeNode and reuse its internal child search function
     */
    CreateNodF createChildNodeFunction = [](TreeNode* parentNode){
        /*auto tmp = TreeNode(parentNode);
        tmp.createChildNodeFunction = parentNode->createChildNodeFunction;

        return tmp;*/

        return TreeNode(parentNode);
    };
public:
    static TreeNode invalidNode;

    TreeNode* parent = nullptr;
    DynamicVar value = INVALID_VALUE;

    TreeNode(TreeNode* parentP);
    TreeNode();

    ValueType getValueType();
    void setValueType(ValueType vType);
    
    NodeType getNodeType();
    
    /* child  access*/
    TreeNode &get(vector<string> objectAddress, TreeNode* parent);

    TreeNode &get(string objectAddress);
    static function<TreeNode(TreeNode* parentNode)> __createNode;


    bool isValid ();

    /* wrapers functions for child access*/
    map<string, TreeNode> &__getChilds();
    vector<string> &__getInsertOrder();

    int valueSize();

    int childCount();
    /**
     * @brief returns the count of childs if the noce contains childs or the 'value' size (interpreted as string)
     * 
     * @return int 
     */
    int size();
    vector<string> getChildNames();

    void foreachChilds(function<void(string childName, TreeNode& child)> f);

    TreeNode& operator[] (string name)
    {
        return get(name);
    }

    TreeNode& operator[] (uint ArrayIndex)
    {
        return get(to_string(ArrayIndex));
        /*string index_s = to_string(index);
        if (this->childs.count(index_s))
            return childs[index_s];

        //return get(to_string(index)); //work only if current node is an array (objects with names like "0", "1", ...)
        int curr = 0;
        for (auto &c: namesInInsertionOrder)
        {
            if (curr == index)
                return childs[c];
            curr++;
        }

        return TreeNode::invalidNode;*/
    }
    
    /*operator string(){
        return value.getString(); 
    }
    
    operator int(){
        return value.getInt(); 
    }
    operator int64_t(){
        return value.getInt64(); 
    }
    operator uint64_t(){
        return value.getUint64(); 
    }
    operator double(){
        return value.getDouble();
    }
    operator bool(){
        return value.getBool(); 
    }
    operator const char*(){
        return value.getString().c_str(); 
    }
    operator signed long long(){
        return value.getInt64(); 
        }
    operator unsigned long long(){
        return value.getUint64(); 
    }
    */
    TreeNode& operator=(const string& _value)
    {
        this->value = _value;
        return *this;
    }

    TreeNode& operator=(const int& _value)
    {
        this->value = _value;
        return *this;
    }

    TreeNode& operator=(const uint& _value)
    {
        this->value = _value;
        return *this;
    }

    TreeNode& operator=(const int64_t& _value)
    {
        this->value = _value;
        return *this;
    }

    TreeNode& operator=(const uint64_t& _value)
    {
        this->value = _value;
        return *this;
    }

    TreeNode& operator=(const double& _value)
    {
        this->value = _value;
        return *this;
    }

    TreeNode& operator=(const bool& _value)
    {
        this->value = _value;
        return *this;
    }

    TreeNode& operator=(const char* _value)
    {
        this->value = string(_value);
        return *this;
    }

    TreeNode& operator=(DynamicVar _value)
    {
        this->value = _value;
        return *this;
    }
};


 
#endif 
 