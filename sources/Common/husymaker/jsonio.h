#ifndef __JSONIO__H__ 
#define __JSONIO__H__ 

#include <string>
#include <treenode.h>
 
class JsonIO{
    private: 
    /**
     * @brief Convert some literal chars by their escaped equivalents ones (line breaks by \n's, tabs characters by \t's, ...)
     * 
     * @param source the source string
     * @return string the escaped string (with escaped chars)
     */
    static string escape(string source);
    
    private: 
    /**
     * @brief Internal Function. Convert escaped chars to their literal ones (\n by a line break, \t by a tab character, ...)
     * 
     * @param source the source string
     * @return string the unescaped string (string with literal chars)
     */
    static string unescape(string source);
    
    public: 
    /**
     * @brief Read a json text and create a TreeNode structure
     * 
     * @param json The text to be parsed
     * @param rootNode An optional root node to receive the output structure. If informe, a copy of this will be returned by the function.
     * @param parentName An address (name) of a parent object inside rootNode (if provided)
     * @param tryParseInvalidJson If true, the function will try to parse JSON even it contains sintax errors
     * @return TreeNode 
     */
    static TreeNode parseJson(string json, TreeNode *rootNode = nullptr, string parentName = "", bool tryParseInvalidJson = true);

    private: 
    /**
     * @brief Internal Function. Create a Identation sequence
     * 
     * @param format If true, the identation text will be returned. If false, the function will return ""
     * @param identLevel The level of the identation (likely the number of tabs, or number of "    " sequences)
     * @param prefix A prefix to be inserted at begin of identation text
     * @param sufix A sufix to be inserted after at end of the identation text
     * @return string 
     */
    static string createIdentation(bool format, int identLevel, string prefix = "\n", string sufix = "");

    private: 
    /**
     * @brief Internal Function. Serialize the chidls of a node as a JSON array([1, 2, 3, 4, 5, ...])
     * 
     * @param node the node with the childs
     * @param format indicates if the output should be formated (with identationa nd line breaks) or not
     * @param enableCommentSupport Indicates if the comments are allowed or not for the output
     * @param identLevel the level of identation (see 'createIdentation' method)
     * @return string 
     */
    static string serializeNodeArray(TreeNode& node, bool format,  bool enableCommentSupport, int identLevel);

    private: 
    /**
     * @brief Internal Function. Serializes the childs of a node as a JSON object ({"v1":1, "v2":2, ...})
     * 
     * @param node The node iwth the childs to be serialized
     * @param format indicates if the output should be formated (with identationa nd line breaks) or not
     * @param enableCommentSupport Indicates if the comments are allowed or not for the output
     * @param identLevel the level of identation (see 'createIdentation' method)
     * @return string 
     */
    static string serializeNodeObject(TreeNode& node, bool format, bool enableCommentSupport, int identLevel);

    private: 
    /**
     * @brief Internal Function. Serialize a node primitive value (for nodes with no childs)
     * 
     * @param node The node whose value will be serialized
     * @return string 
     */
    static string serializeNodeValue(TreeNode& node);

    private: 
    /**
     * @brief Export the the Node tree to a json text
     * 
     * @param node Is the root node to be exported
     * @param format If true, the output will be a formated and idented text, otherwise, the output will be and compated json text
     * @param enableCommentSupport If true, the output will be an JSON with comments (using Cpp like, or javascript, format).
     * @param identLevel The current identation level
     * @return string 
     */
    static string exportJson(TreeNode& node, bool format, bool enableCommentSupport, int identLevel);

    
    public: 
    /**
     * @brief Export the the Node tree to a json text
     * 
     * @param node Is the root node to be exported
     * @param formatedOutput If true, the output will be a formated and idented text, otherwise, the output will be and compated json text
     * @param enableCommentSupport If true, the output will be an JSON with comments (using Cpp like, or javascript, format).
     * @return string 
     */
    static string exportJson(TreeNode& node, bool formatedOutput = true, bool enableCommentsSupport = true);

    public:
    /**
     * @brief Just a wraper method. Redirects its execution to exportJson method. Export the the Node tree to a json text
     * 
     * @param node Is the root node to be exported
     * @param formatedOutput If true, the output will be a formated and idented text, otherwise, the output will be and compated json text
     * @param enableCommentSupport If true, the output will be an JSON with comments (using Cpp like, or javascript, format).
     * @return string 
     */
    static string toString(TreeNode& node, bool formatedOutput = true, bool enableCommentsSupport = true);

    
};
 
#endif 
