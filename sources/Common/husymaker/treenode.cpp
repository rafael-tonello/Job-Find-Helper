#include  "treenode.h" 

TreeNode TreeNode::invalidNode;


TreeNode::TreeNode(){}

TreeNode::TreeNode(TreeNode* parentP){
    this->parent = parentP;
    this->createChildNodeFunction = parentP->createChildNodeFunction;
}

bool TreeNode::isArray(){
    string childNames;
    for (auto &c: childs)
        if (c.first == "" || Utils::getOnly(c.first, "0123456789") != c.first)
            return false;

    return true;
}

TreeNode::ValueType TreeNode::getValueType(){
    if (valueType == ValueType::AUTO_DETECT)
    {
        if (string("truefalse").find(Utils::strToLower(value)) != string::npos)
            return ValueType::BOOLEAN;
        else if (Utils::getOnly(value.getString(), "0123456789-") == value.getString())
            return ValueType::INT;
        else if (Utils::getOnly(value.getString(), "0123456789-.") == value.getString())
            return ValueType::DOUBLE;
        else if ((value.getString().size() > 1) && (string("//*<!").find(value.getString()) == 0))
            return ValueType::COMMENT;
        else
            return ValueType::STRING;
    }
    else
        return valueType;
}

void TreeNode::setValueType(TreeNode::ValueType vType){
    this->valueType = vType;
}

TreeNode::NodeType TreeNode::getNodeType()
{
    if (childs.size() == 0)
        return NodeType::VALUE;
    else if(this->isArray())
        return NodeType::ARRAY;
    else
        return NodeType::OBJECT;
}


/* child  access*/
TreeNode &TreeNode::get(vector<string> objectAddress, TreeNode* parent)
{
    if (objectAddress.size() > 0)
    {
        string childName = objectAddress[0];
        objectAddress.erase(objectAddress.begin());

        if (!this->childs.count(childName))
        {
            auto tmp = this->createChildNodeFunction(parent);
            this->childs[childName] = tmp;
            this->namesInInsertionOrder.push_back(childName);
        }

        return this->childs[childName].get(objectAddress, this);

    }
    else
        return *this;
}

TreeNode &TreeNode::get(string objectAddress)
{
    objectAddress = Utils::stringReplace(objectAddress, 
        {
            {"]", ""},
            {"[", "."},
            {"::", "."},
            {":", "."},
            {"->", "."}
        }
    );

    return get(Utils::splitString(objectAddress, "."), this);
}

bool TreeNode::isValid (){
    return this->value.getString() != INVALID_VALUE;
}

/* wrapers functions for child access*/
map<string, TreeNode> &TreeNode::__getChilds(){ 
    return this->childs;
}

vector<string> &TreeNode::__getInsertOrder(){
    return this->namesInInsertionOrder;
}

int TreeNode::valueSize()
{
    return value.getString().size();
}

int TreeNode::childCount()
{
    return childs.size();
}

int TreeNode::size()
{
    if (childCount() > 0)
        return childCount();
    else
        return valueSize();

}

vector<string> TreeNode::getChildNames()
{
    return namesInInsertionOrder;
}

void TreeNode::foreachChilds(function<void(string childName, TreeNode& child)> f)
{
    for (auto &c: this->namesInInsertionOrder)
        f(c, this->childs[c]);
}

