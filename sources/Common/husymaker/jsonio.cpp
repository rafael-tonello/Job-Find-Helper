#include  "jsonio.h" 

string JsonIO::escape(string source)
{
    return Utils::stringReplace(source,
        {
            {"\"", "\\\""},
            {"\n", "\\n"},
            {"\r", "\\r"},
            {"\t", "\\t"},
        }
    );
}

string JsonIO::unescape(string source)
{
    return Utils::stringReplace(source,
        {
            {"\\\"", "\""},
            {"\\n", "\n"},
            {"\\r", "\r"},
            {"\\t", "\t"},
        }
    );
}

TreeNode JsonIO::parseJson(string json, TreeNode *rootNode, string parentName, bool tryParseInvalidJson)
{
    enum ParseStates { findingStart, readingName, waitingKeyValueSep, findValueStart, prepareArray, readingContentString, readingContentNumber, readingContentSpecialWord, readingComment };
    
    TreeNode _root = TreeNode();
    TreeNode &root = _root;
    if (rootNode != nullptr)
        root = *rootNode;


    if (json == "null")
    {
        return TreeNode::invalidNode;
    }


    TreeNode *currentObject = &root;


    if (parentName != "")
        currentObject = &(root[parentName]);

    //currentObject->parser_name = parentName;

    ParseStates state = ParseStates::findValueStart;

    bool ignoreNextChar = false;
    stringstream currentStringContent;
    stringstream currentNumberContent;
    stringstream currentSpecialWordContent;
    stringstream currentChildName;
    stringstream currentComment;

    int commentsCount = 0;

    int currLine = 1;
    int currCol = 1;

    int max = json.size();
    char curr = ' ';

    string commentType = "";
    string commentName = "";
    char oldCurr = '\0';

    for (int cont = 0; cont < max; cont++)
    {
        oldCurr = curr;
        curr = json[cont];

        currCol++;
        if (curr == '\n')
        {
            currCol = 1;
            currLine++;
        }

        switch (state)
        {
            case ParseStates::findingStart:
                if (curr == '"')
                {
                    if (currentObject->getNodeType() == TreeNode::NodeType::ARRAY)
                        state = ParseStates::prepareArray;
                    else
                        state = ParseStates::readingName;
                    currentChildName.str("");
                }
                else if (curr == ','/* || (curr == '[') || (curr == '{')*/)
                {
                    if (currentObject->getNodeType() == TreeNode::NodeType::ARRAY)
                        state = ParseStates::prepareArray;
                }
                else if ((curr == '}') || (curr == ']'))
                {
                    if (parentName.find('.') != string::npos)
                    {
                        parentName = parentName.substr(0, parentName.find_last_of('.'));
                        if (currentObject != NULL)
                            currentObject = currentObject->parent;
                            //currentObject = (TreeNode&)(*((TreeNode*)(atoll(currentObject.getTag("parentAddr", "0").c_str()))));
                    }
                    else
                    {
                        parentName = "";
                        currentObject = &root;
                    }
                }
                else if (curr == '/')
                {
                    currentComment.str("");
                    commentType = "";
                    state = ParseStates::readingComment;
                    clog << "Warning: Json does not officially supports comments. Using Javascript comment system." << endl;
                }
                break;
            case ParseStates::readingName:
                if (curr == '"')
                {
                    state = ParseStates::waitingKeyValueSep;
                    currentObject = &((*currentObject)[currentChildName.str()]);
                    currentObject->setTag("parser_name", currentChildName.str());
                    parentName = parentName + (parentName != "" ? "." : "") + currentChildName.str();
                    currentChildName.str("");

                }
                else
                    currentChildName << curr;
                break;
            case ParseStates::waitingKeyValueSep:
                if (curr == ':')
                    state = ParseStates::findValueStart;
                break;
            case ParseStates::findValueStart:
                if (curr == '"')
                {
                    state = ParseStates::readingContentString;
                    currentStringContent.str("");
                }
                else if (curr == '{')
                {
                    state = ParseStates::findingStart;
                }
                else if (curr == '[')
                    state = ParseStates::prepareArray;
                else if (string("0123456789-+.").find(curr) != string::npos)
                {
                    state = ParseStates::readingContentNumber;
                    currentNumberContent.str("");
                    cont--;
                    currCol--;
                }
                else if (string("untfUNTF").find(curr) != string::npos)
                {
                    state = ParseStates::readingContentSpecialWord;
                    currentSpecialWordContent.str("");
                    cont--;
                    currCol--;
                }
                else if (curr == ']')
                {
                    //delete currenObject
                    auto temp = currentObject;


                    if (parentName.find('.') != string::npos)
                    {
                        parentName = parentName.substr(0, parentName.find_last_of('.'));
                        currentObject = currentObject->parent;
                    }
                    else
                    {
                        parentName = "";
                        currentObject = &root;
                    }

                    currentObject->__getChilds().erase(temp->getTag("parser_name", ""));

                    cont--;
                    currCol--;
                    state = ParseStates::findingStart;
                }
                else if (string(" \t\r\n").find(curr) == string::npos)
                {
                    string errorMessage = "SintaxError at line "+to_string(currLine) + " and column "+to_string(currCol) + ". Expected ' '(space), \t, \r or \n, but found "+curr+".";
                    if(!tryParseInvalidJson)
                        throw errorMessage;
                }
                break;

            case ParseStates::prepareArray:
                //state = "findingStart";
                currentChildName.str("");
                currentChildName << to_string(currentObject->__getChilds().size());
                currentObject = &((*currentObject)[currentChildName.str()]);
                currentObject->setTag("parser_name", currentChildName.str());
                parentName = parentName + (parentName != "" ? "." : "") + currentChildName.str();
                state = ParseStates::findValueStart;
                cont--;
                currCol--;
                break;
            case ParseStates::readingContentString:
                if (ignoreNextChar)
                {
                    currentStringContent << curr;
                    ignoreNextChar = false;
                }
                else if (curr == '\\')
                {
                    ignoreNextChar = true;
                    currentStringContent << curr;
                }
                else if (curr == '"')
                {
                    currentObject->value = unescape(currentStringContent.str());
                    currentObject->setValueType(TreeNode::ValueType::STRING);

                    currentStringContent.str("");


                    //return to parent Object
                    if (parentName.find('.') != string::npos)
                    {
                        parentName = parentName.substr(0, parentName.find_last_of('.'));
                        currentObject = currentObject->parent;
                    }
                    else
                    {
                        parentName = "";
                        currentObject = &root;
                    }

                    state = ParseStates::findingStart;

                }
                else
                    currentStringContent << curr;
                break;
            case ParseStates::readingContentNumber:
                if (string("0123456789.-+").find(curr) != string::npos)
                    currentNumberContent << curr;
                else
                {
                    currentObject->value = currentNumberContent.str();
                    currentObject->setValueType(TreeNode::ValueType::DOUBLE);
                    currentNumberContent.str("");

                    //return to parent Object
                    if (parentName.find('.') != string::npos)
                    {
                        parentName = parentName.substr(0, parentName.find_last_of('.'));
                        currentObject = currentObject->parent;
                    }
                    else
                    {
                        parentName = "";
                        currentObject = &root;
                    }

                    cont--;
                    state = ParseStates::findingStart;
                }

                break;
            case ParseStates::readingContentSpecialWord:
                if (string("truefalseundefinednulTRUEFALSEUNDEFINEDNUL").find(curr) != string::npos)
                    currentSpecialWordContent<<curr;
                else
                {
                    string strTemp = currentSpecialWordContent.str();
                    std::transform(strTemp.begin(), strTemp.end(), strTemp.begin(), ::tolower);
                    if ((strTemp == "true") ||
                        (strTemp == "false") ||
                        (strTemp == "null") ||
                        (strTemp == "undefined"))
                    {
                        currentObject->value = strTemp;
                        currentObject->setValueType(TreeNode::ValueType::BOOLEAN);
                        currentSpecialWordContent.str("");

                        //return to parent Object
                        if (parentName.find('.') != string::npos)
                        {
                            parentName = parentName.substr(0, parentName.find_last_of('.'));
                            currentObject = currentObject->parent;
                        }
                        else
                        {
                            parentName = "";
                            currentObject = &root;
                        }

                        cont--;
                        state = ParseStates::findingStart;
                    }
                    else
                    {
                        string errorMessage = "Invalid simbol at line " + to_string(currLine) + " and column " + to_string(currCol) + ": " + currentSpecialWordContent.str();
                        if (!tryParseInvalidJson)
                            throw errorMessage;
                    }
                }

            break;
            case ParseStates::readingComment:
                //identify comment type
                if (commentType == "")
                {
                    if (curr == '/')
                    {
                        commentType = "line";
                        currentComment << "//";
                    }
                    else
                    {
                        commentType = "block";
                        currentComment << "/*";
                    }
                }
                else
                {
                    if ((commentType == "line" && curr == '\n') || (commentType == "block" && oldCurr == '*' && curr == '/'))
                    {
                        //add the current comment to the current parent (comment is a special SOType)
                        state = ParseStates::findingStart;

                        if (commentType == "block")
                            currentComment << "/";

                        currentComment << endl;

                        currentObject->value = currentComment.str();
                        currentObject->setValueType(TreeNode::ValueType::COMMENT);
                        currentComment.str("");
                    }
                    else
                    {
                        currentComment << curr;
                    }

                }
                

            break;
        }
    }

    return root;
}

string JsonIO::createIdentation(bool format, int identLevel, string prefix, string sufix)
{
    if (format)
    {
        stringstream s;
        while (identLevel-- > 0)
            s << "    ";

        return prefix + s.str() + sufix;
    }
    return "";
}

string JsonIO::serializeNodeArray(TreeNode& node, bool format,  bool enableCommentSupport, int identLevel)
{
    stringstream result;
    result << "[";

    int index = 0;
    auto &nodeChilds = node.__getChilds();
    int max = nodeChilds.size();
    for (auto &c: node.__getInsertOrder())
    {
        if (nodeChilds[c].getValueType() != TreeNode::ValueType::COMMENT  || enableCommentSupport)
        {
            result << createIdentation(format, identLevel);
            result << exportJson(nodeChilds[c], format, enableCommentSupport, identLevel+1);

            if (index++ < max-1)
            {
                if (nodeChilds[c].getValueType() != TreeNode::ValueType::COMMENT)
                    result << "," << (format? " " : "");
            }
        }
    }
    result <<  createIdentation(format, identLevel-1) << "]";

    return result.str();
}

string JsonIO::serializeNodeObject(TreeNode& node, bool format, bool enableCommentSupport, int identLevel)
{
    stringstream result;
    auto &nodeChilds = node.__getChilds();
    int max = nodeChilds.size();

    result << "{";

    int index = 0;
    for (auto &c: node.__getInsertOrder())
    {
        result << createIdentation(format, identLevel);


        if (nodeChilds[c].getValueType() != TreeNode::ValueType::COMMENT)
            result << "\""+c+"\":" << (format? " " : "") ;

        if (nodeChilds[c].getValueType() != TreeNode::ValueType::COMMENT || enableCommentSupport)
        {
            result << exportJson(nodeChilds[c], format, enableCommentSupport, identLevel+1);

            if (index++ < max-1)
            {
                if (nodeChilds[c].getValueType() != TreeNode::ValueType::COMMENT)
                    result << "," << (format? " " : "");
            }
        }
    }
    result << createIdentation(format, identLevel-1) << "}" ;
    return result.str();
}

string JsonIO::serializeNodeValue(TreeNode& node)
{
    switch(node.getValueType()){
        case TreeNode::ValueType::STRING:
            return "\""+escape(node.value.getString())+"\"";
        break;
        case TreeNode::ValueType::INT:
            return node.value.getString();
        break;
        case TreeNode::ValueType::DOUBLE:
            return Utils::stringReplace(node.value.getString(), ",", ".");
        break;
        case TreeNode::ValueType::BOOLEAN:
            return node.value.getBool() ? "true":"false";
        break;
        case TreeNode::ValueType::NULLVALUE:
            return "null";
        break;
        case TreeNode::ValueType::COMMENT:
            clog << "Warning: JSON does not officialy supports comments. Using Javascript comment style" << endl;
            return "/*" + node.value.getString() + "*/";
        break;
    }

    return "undefined";
}

string JsonIO::exportJson(TreeNode& node, bool format, bool enableCommentSupport, int identLevel)
{
    auto nodeType = node.getNodeType();

    stringstream result;

    if (nodeType == TreeNode::NodeType::VALUE)
        result << serializeNodeValue(node);
    else if (nodeType == TreeNode::NodeType::ARRAY)
        result << serializeNodeArray(node, format, enableCommentSupport, identLevel);
    else
        result << serializeNodeObject(node, format, enableCommentSupport, identLevel);

    return result.str();
}

string JsonIO::exportJson(TreeNode& node, bool formatedOutput, bool enableCommentsSupport)
{
    return exportJson(node, formatedOutput, enableCommentsSupport, 1);
}

string JsonIO::toString(TreeNode& node, bool formatedOutput, bool enableCommentsSupport)
{
    return exportJson(node, formatedOutput, enableCommentsSupport);
}
