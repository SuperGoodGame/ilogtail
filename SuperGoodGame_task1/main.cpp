#include <iostream>
#include <regex>
#include <fstream>
#include <algorithm>
#include <sstream>

using namespace std;
const string indentation="  ";
const string injsonTest="in.txt";
const string outjsonTest="out.json";

// 过滤二叉树节点
class FilterNode{
public:
    FilterNode(){
        isLeaf = isRoot=0;
    }
    virtual void print(const int& level,ofstream& outfile) =0;
    static inline void printJSON(const int &level,ofstream& outfile,const string& K,const string& val);
    bool isRoot;
protected:
    bool isLeaf;

};
//叶子节点
class LeafNode:public FilterNode{
public:
    LeafNode(const string &equ);
    void print(const int& level,ofstream& outfile) final;

protected:
    string key;
    string exp;
};
//非叶子节点
class UnLeafNode:public FilterNode{
public:
    UnLeafNode(FilterNode *a,FilterNode *b,const string& op);
    void print(const int& level,ofstream& outfile) final;
protected:
    FilterNode * child[2]{};
    string oper;
};
//转换
void UnLeafNode::print(const int &levels, ofstream &outfile) {

    if(!this->isRoot) { // 根节点需要特判输出格式括号位置
        for (int i = 1; i <= levels; i++)
            outfile << indentation;
        outfile << "{\n";
    }

    int level=levels+!this->isRoot;
    printJSON(level,outfile,"operator",oper);
    outfile<<",\n";

    for(int i=1;i<=level;i++)
        outfile<<indentation;
    outfile<<"\"operands\": [\n";

    child[0]->print(level+1,outfile);
    outfile<<",\n";

    child[1]->print(level+1,outfile);
    outfile<<"\n";

    for(int i=1;i<=level;i++)
        outfile<<indentation;
    outfile<<"]";
    if(!this->isRoot) {
        outfile<<"\n";
        for (int i = 1; i <= levels; i++)
            outfile << indentation;
        outfile << "}";
    }
}

UnLeafNode::UnLeafNode(FilterNode *a, FilterNode *b,const string& op) {
    oper = op;
    child[0] = a;
    child[1] = b;
}
// 具有缩进格式的json信息打印
void FilterNode::printJSON(const int &level, ofstream &outfile, const string &K, const string &val) {
    for(int i=1;i<=level;i++)
        outfile<<indentation;
    outfile<<"\""<<K<<"\""<<": "<<"\""<<val<<"\"";
}

void LeafNode::print(const int &level,ofstream &outfile) {
    for(int i=1;i<=level;i++)
        outfile<<indentation;
    outfile<<"{\n";
    printJSON(level+1,outfile,"key",key);
    outfile<<",\n";
    printJSON(level+1,outfile,"exp",exp);
    outfile<<",\n";
    printJSON(level+1,outfile,"type","regex");
    outfile<<"\n";
    for(int i=1;i<=level;i++)
        outfile<<indentation;
    outfile<<"}";
}

LeafNode::LeafNode(const string &equ) {
    stringstream ss(equ);
    getline(ss,key,'=');
    getline(ss,exp,'=');
}

class col{
public:
    col(string injson,string outjson){
        infile.open(injson);
        outfile.open(outjson);
    }
    void getin();
    void run();
    void print();
    FilterNode* formulaFilter();
private:
    ofstream outfile;
    ifstream infile;
    string buff;
    stringstream buffs;

    FilterNode *root;
};
//处理读入的表达式
FilterNode *col::formulaFilter() {
    FilterNode * tem;
    string str;
    bool isop=0;
    stack<FilterNode*> nodeS;
    stack<string> opS;
    while(buffs>>str) {
        if(str == ")") break;
        if(isop == 0 ){
            if(str == "(") // 括号问题
                tem = formulaFilter();
            else tem = new LeafNode(str);
            nodeS.push(tem);
        }else{
            //如果新加入符号or 比栈中and 优先级低 那么要弹栈合并节点栈顶两个节点消去符号栈中的符号，直到加入符号优先级等于栈顶或栈空
            while(nodeS.empty()==0 && opS.empty() == 0 && str == "or" && opS.top() == "and" ){ // 优先级问题
                tem = nodeS.top();
                nodeS.pop();
                tem = new UnLeafNode(tem,nodeS.top(),opS.top());
                opS.pop();
                nodeS.push(tem);
            }
            opS.push(str);
        }
        isop = !isop; // 交替处理符号和原子式
    }
    while(!opS.empty()){
        tem = nodeS.top();
        nodeS.pop();
        tem = new UnLeafNode(tem,nodeS.top(),opS.top());
        opS.pop();
        nodeS.push(tem);
    }
    return nodeS.top();
}

//读入初始表达式
void col::getin() {
    getline(infile,buff);
    buffs<<buff;
}

void col::run() {
    int pos=0;
    root = formulaFilter();
    root->isRoot=1; // 为处理开头格式设置
}

// 打印转换后json
void col::print() {
    outfile<<"{\n";

    outfile<<indentation;
    outfile<<"\"filter_expression\": {\n";

    root->print(2,outfile);
    outfile<<"\n";

    outfile<<indentation;
    outfile<<"}\n";

    outfile<<"}\n";
}

int main() {
    col a(injsonTest,outjsonTest);
    a.getin();
    a.run();
    a.print();
}