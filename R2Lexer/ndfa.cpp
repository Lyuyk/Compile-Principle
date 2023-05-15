/****************************************************
 * @Copyright © 2021-2023 Lyuyk. All rights reserved.
 *
 * @FileName: ndfa.cpp
 * @Brief: NFA、DFA类源文件
 * @Module Function:
 *
 * @Current Version: 1.3
 * @Author: Lyuyk
 * @Modifier: Lyuyk
 * @Finished Date: 2023/4/21
 *
 * @Version History: 1.1
 *                   1.2 增添了部分注释，提高了部分代码的可读性
 *                   1.3 current version
 *
 ****************************************************/
#include "ndfa.h"
#include <QDebug>

NDFA::NDFA()
{
    init();
}

/**
 * @brief NDFA::init
 * 类初始化函数
 */
void NDFA::init()
{
    //初始化FA状态计数
    NFAStateNum=0;
    DFAStateNum=0;
    mDFAStateNum=0;
    opCharSet.clear();
    DFAEndStateSet.clear();
    keyWordSet.clear();
    reg_keyword.clear();
    lexerCodeStr.clear();
    dividedSet->clear();

    //FA图初始化
    NFAG.startNode=NULL;
    NFAG.endNode=NULL;
    mDFAG.startState=-1;
    mDFAG.endStateSet.clear();

    //NFA节点数组初始化
    for(int i=0;i<ARR_MAX_SIZE;i++)
    {
        NFAStateArr[i].init();
        NFAStateArr[i].stateNum=i;
    }

    //DFA节点数组初始化
    for(int i=0;i<ARR_MAX_SIZE;i++)
    {
        DFAStateArr[i].init();
        DFAStateArr[i].stateNum=i;
    }

    //mDFA节点数组初始化优化
    for(int i=0;i<ARR_MAX_SIZE;i++)
    {
        mDFANodeArr[i].init();
    }

}

void NDFA::printNFA(QTableWidget *table)
{
    qDebug()<<"printNFA";
    int rowCount=NFAStateNum;//记录NFA状态数
    int epsColN=opCharSet.size()+1;//最后一列 epsilon 列号
    int colCount=opCharSet.size()+3;
    QStringList OpStrList=opCharSet.values();
    std::sort(OpStrList.begin(),OpStrList.end());
    OpStrList.push_front("状态号");
    OpStrList.push_back("epsilon");
    OpStrList.push_back("初/终态");

    //ui->plainTextEdit_console->insertPlainText("NFA states' count:"+QString::number(NFAStateNum)+'\n');
    //ui->plainTextEdit_console->insertPlainText("Operators' count:"+QString::number(OpCharSet.size())+'\n');

    qDebug()<<OpStrList;
    table->setRowCount(rowCount);
    table->setColumnCount(colCount);
    /*设置表头 行*/
    table->setHorizontalHeaderLabels(OpStrList);
    /*设置表头 列*/
    for(int row=0;row<rowCount;row++)
    {
        //状态号
        table->setItem(row,0,new QTableWidgetItem(QString::number(row)));
        //epsilon转换集合
        QString epsSetStr="";
        QSet<int>::iterator it;
        for(it=NFAStateArr[row].epsToSet.begin();it!=NFAStateArr[row].epsToSet.end();it++)
        {
            epsSetStr+=QString::number(*it)+",";
        }
        epsSetStr.chop(1);//去掉末尾逗号
        table->setItem(row,epsColN,new QTableWidgetItem(epsSetStr.left(1)));

//        qDebug()<<"eSUnion:"<<NFAStateArr[row].epsToSet;
//        qDebug()<<"epsSetStr:"+epsSetStr;

        //非epsilon转换
        int colN=OpStrList.indexOf(NFAStateArr[row].val);

        if(colN != -1)
        {
            table->setItem(row,colN,new QTableWidgetItem(QString::number(NFAStateArr[row].toState)));
        }

        qDebug()<<NFAStateArr[row].stateNum;
        qDebug()<<NFAG.startNode->stateNum;

        if(NFAStateArr[row].stateNum==NFAG.startNode->stateNum)
        {
            table->setItem(row,epsColN+1,new QTableWidgetItem("初态"));
        }

        if(NFAStateArr[row].stateNum==NFAG.endNode->stateNum)
        {
            table->setItem(row,epsColN+1,new QTableWidgetItem("终态"));
        }
    }
}

void NDFA::printDFA(QTableWidget *table)
{
    QStringList OpStrList=opCharSet.values();
    std::sort(OpStrList.begin(),OpStrList.end());
    OpStrList.push_front("状态号");
    OpStrList.push_back("初/终态");

    int rowCount=DFAStateNum;
    int colCount=OpStrList.size();
    //设置表格行列
    table->setColumnCount(colCount);
    table->setRowCount(rowCount);

    //水平表头
    table->setHorizontalHeaderLabels(OpStrList);
    //table->setItem(DFAG.startState,colCount-1,new QTableWidgetItem("初态"));

    for(int iArr=0;iArr<DFAStateNum;iArr++)
    {
        int rowN=DFAStateArr[iArr].stateNum;
        table->setItem(rowN,0,new QTableWidgetItem(QString::number(rowN)));
        //遍历DFA节点出边
        for(int i=0;i<DFAStateArr[rowN].edgeCount;i++)
        {
            //转到状态的列标
            int colN=OpStrList.indexOf(DFAStateArr[rowN].edges[i].value);
            int toStateN=DFAStateArr[rowN].edges[i].toState;
            table->setItem(rowN,colN,new QTableWidgetItem(QString::number(toStateN)));
        }

        if(DFAStateArr[rowN].isEnd)
        {
            table->setItem(rowN,colCount-1,new QTableWidgetItem("终态"));
        }

    }
}

void NDFA::printMDFA(QTableWidget *table)
{
    QStringList OpStrList=opCharSet.values();
    std::sort(OpStrList.begin(),OpStrList.end());
    //OpStrList.push_front("状态集元素");
    OpStrList.push_front("状态集号");
    OpStrList.push_back("初/终态");

    int rowCount=mDFAStateNum;
    int colCount=OpStrList.size();
    //设置表格行列
    table->setColumnCount(colCount);
    table->setRowCount(rowCount);
    //设置表头
    table->setHorizontalHeaderLabels(OpStrList);

    //输出表格内容
    table->setHorizontalHeaderLabels(OpStrList);
    table->setItem(mDFAG.startState,colCount-1,new QTableWidgetItem("初态"));

    //qDebug()<<"mDS:"<<mDFAStateNum;
    for(int iArr=0;iArr<mDFAStateNum;iArr++)
    {
        int rowN=mDFANodeArr[iArr].stateNum;

        table->setItem(rowN,0,new QTableWidgetItem(QString::number(rowN)));

        //遍历DFA节点出边
        for(int i=0;i<mDFANodeArr[rowN].edgeCount;i++)
        {
            //转到状态的列标
            int colN=OpStrList.indexOf(mDFANodeArr[rowN].edges[i].value);
            qDebug()<<"colN"<<colN;
            int toStateN=mDFANodeArr[rowN].edges[i].toState;
            table->setItem(rowN,colN,new QTableWidgetItem(QString::number(toStateN)));
        }

        if(mDFANodeArr[rowN].isEnd)
        {
            table->setItem(rowN,colCount-1,new QTableWidgetItem("终态"));
        }
    }
}

void NDFA::printLexer(QPlainTextEdit *widget)
{
    widget->clear();
    widget->setPlainText(lexerCodeStr);
}


/**
 * @brief NDFA::strToNfa
 * 将正则表达式转换为NFA
 * @param s
 * @return
 */
NDFA::NFAGraph NDFA::strToNfa(QString s)
{
    opPriorityMapInit();//运算符优先级初始化

    QStack<NFAGraph> NFAStack;//存NFA子图的栈
    QStack<QChar> opStack;//符号栈

    for(int i=0;i<s.size();i++)
    {
        switch(s[i].unicode())
        {
        case '|':
        case '&':
        {
            pushOpStackProcess(s[i],opStack,NFAStack);
            break;
        }
        case '*':
        case '+':
        case '?':
        {
            pushOpStackProcess(s[i],opStack,NFAStack);
            insConnOp(s,i,opStack,NFAStack);
            break;
        }
        case '(': opStack.push('(');break;
        case ')':
        {
            while(!opStack.empty())
            {
                if(opStack.top() != '(')
                {
                    opProcess(opStack.top(),NFAStack);
                    opStack.pop();
                }
                else break;
            }
            opStack.pop();
            insConnOp(s,i,opStack,NFAStack);
            break;
        }
        default:
        {
            QString tmpStr;
            if(s[i]=='\\')
            {
                while(s[++i]!='\\')
                {
                    if(s[i]=='`')i++;

                    tmpStr+=s[i];
                }
                opCharSet.insert(tmpStr);//顺便加入操作符集合
            }
            else {
                tmpStr=s[i];

                if(!opSet.contains(s[i]))
                    opCharSet.insert(s[i]);//若是转义字符顺便加入操作符集合
            }
            NFAGraph n=createNFA(NFAStateNum);
            NFAStateNum+=2;
            n.startNode->value=tmpStr;//生成NFA子图，非eps边
            add(n.startNode,n.endNode,tmpStr[i]);

            NFAStack.push(n);
            insConnOp(s,i,opStack,NFAStack);
        }
        }
    }
    //读完字符串，运算符站还有元素则将其全部出栈
    while(!opStack.empty())
        opProcess(opStack.pop(),NFAStack);

    return NFAStack.top();
}


/**
 * @brief NDFA::opPriorityMapInit
 * 初始化操作符优先级，数值越高优先级越大
 */
void NDFA::opPriorityMapInit()
{
    opPriorityMap['(']=0;
    opPriorityMap['|']=1;
    opPriorityMap['&']=2;
    opPriorityMap['*']=3;
    opPriorityMap['+']=3;
    opPriorityMap['?']=3;
}

void NDFA::insConnOp(QString str, int curState, QStack<QChar> &opStack, QStack<NFAGraph> &NFAStack)
{
    //判断是否两元素间加入连接符号，即栈中是否需要加入连接符号
    if(curState+1<str.size() && (!opSet.contains(str[curState+1]) || str[curState+1]=='('))
    {
        pushOpStackProcess('&',opStack,NFAStack);
    }
}

void NDFA::pushOpStackProcess(QChar opCh, QStack<QChar> &opStack, QStack<NFAGraph> &NFAStack)
{
    while(!opStack.empty())
    {
        //若栈顶原酸优先级大于当前将进栈元素，则将栈顶元素出栈并处理，再继续循环判断
        if(opStack.top()!='(' && opPriorityMap[opStack.top()]>=opPriorityMap[opCh])
        {
            opProcess(opStack.top(),NFAStack);
            opStack.pop();
        }
        else break;
    }
}

/**
 * @brief NDFA::opCharProcess
 * @param opChar
 * @param NFAStack
 * 根据运算符转换NFA处理子函数
 */
void NDFA::opProcess(QChar opChar, QStack<NFAGraph> &NFAStack)
{
    switch(opChar.unicode())
    {
    case '|':
    {
        //或运算处理
        NFAGraph n1=NFAStack.pop();//先出n1
        NFAGraph n2=NFAStack.pop();//后出n2
        NFAGraph n=createNFA(NFAStateNum);
        NFAStateNum+=2;

        add(n.startNode,n2.startNode);
        add(n.startNode,n1.startNode);
        add(n2.endNode,n.endNode);
        add(n1.endNode,n.endNode);
        NFAStack.push(n);

        break;
    }
    case '&':
    {
        //与运算处理
        NFAGraph n1=NFAStack.pop();
        NFAGraph n2=NFAStack.pop();

        add(n2.endNode,n1.startNode);

        NFAGraph n;
        n.startNode=n2.startNode;
        n.endNode=n1.endNode;

        NFAStack.push(n);
        break;
    }
    case '*':
    {
        //闭包运算处理
        NFAGraph n1=NFAStack.pop();
        NFAGraph n=createNFA(NFAStateNum);
        NFAStateNum+=2;

        add(n.startNode,n.endNode);
        add(n.startNode,n1.startNode);
        add(n1.endNode,n1.startNode);
        add(n1.endNode,n.endNode);

        NFAStack.push(n);
        break;
    }
    case '+':
    {
        //正闭包运算处理
        NFANode *newEndNode=&NFAStateArr[NFAStateNum];
        NFAStateNum++;

        NFAGraph n1=NFAStack.pop();
        add(n1.endNode,newEndNode);
        add(n1.endNode,n1.startNode);

        NFAGraph n;
        n.startNode=n1.startNode;
        n.endNode=newEndNode;

        NFAStack.push(n);
        break;
    }
    case '?':
    {
        NFANode *newStartNode=&NFAStateArr[NFAStateNum];
        NFAStateNum++;

        NFAGraph n1=NFAStack.pop();
        add(newStartNode,n1.startNode);
        add(newStartNode,n1.endNode);

        NFAGraph n;
        n.startNode=newStartNode;
        n.endNode=n1.endNode;

        NFAStack.push(n);
        break;
    }
    }
}

/**
 * @brief NDFA::get_e_closure
 * @param tmpSet
 * 求epsilon闭包
 */
void NDFA::get_e_closure(QSet<int> &tmpSet)
{
    QQueue<int> q;
    for(const auto &value: tmpSet)
        q.push_back(value);

    while(!q.empty())
    {
        int tmpFront=q.front();
        q.pop_front();

        QSet<int> eTmpSet=NFAStateArr[tmpFront].epsToSet;
        //将通过epsilon到达的节点序号放入集合中
        for(const auto &value: eTmpSet)
        {
            if(!tmpSet.contains(value))
            {
                tmpSet.insert(value);
                q.push_back(value);
            }
        }
    }
}

/**
 * @brief NDFA::getStateId
 * @param set
 * @param cur
 * @return i
 * 查询当前DFA节点属于mDFA节点的状态集号
 */
int NDFA::getStateId(QSet<int> set[], int cur)
{
    for(int i=0;i<mDFAStateNum;i++)
    {
        if(set[i].contains(cur))
            return i;
    }
}

/**
 * @brief NDFA::genLexCase
 * @param tmpList
 * @param codeStr
 * @param idx
 * @param flag
 * @return
 *生成Lexer代码的核心子函数
 */
bool NDFA::genLexCase(QList<QString> tmpList, QString &codeStr, int idx, bool flag)
{
    bool rFlag=false;
    for(int i=0;i<tmpList.size();i++)
    {
        QString tmpKey=tmpList[i];
        //字母情况
        if(tmpKey=="letter")
        {
            for(int j=0;j<26;j++)
            {
                codeStr+="\t\t\tcase \'"+QString(char('a'+j))+"\':\n";
                codeStr+="\t\t\tcase \'"+QString(char('A'+j))+"\':\n";
            }
            codeStr.chop(1);//去掉末尾字符
        }
        else if(tmpKey=="digit")
        {
            //数字情况
            for(int j=0;j<26;j++)
            {
                codeStr+="\t\t\tcase \'"+QString::number(j)+"\':\n";
            }
            codeStr.chop(1);
        }
        else if(tmpKey=="~")
        {
            rFlag=true;
            continue;
        }
        else codeStr+="\t\t\tcase \'"+tmpKey+"\':";

        if(flag)codeStr+="state = "+QString::number(mDFANodeArr[idx].mDFAEdgesMap[tmpKey])+"; ";
        codeStr+="break;\n";
    }
    return rFlag;
}

/**
 * @brief NDFA::createNFA
 * @param stateN
 * @return n
 * 建立一个初始化的NFA子图，包含开始和结尾两NFA节点
 */
NDFA::NFAGraph NDFA::createNFA(int stateN)
{
    NFAGraph n;

    n.startNode=&NFAStateArr[stateN];
    n.endNode=&NFAStateArr[stateN+1];

    return n;
}

/**
 * @brief NDFA::add
 * @param n1
 * @param n2
 * @param ch
 * n1--ch-->n2
 * NFA节点n1与n2间添加一条非epsilon边，操作符为ch
 */
void NDFA::add(NDFA::NFANode *n1, NDFA::NFANode *n2, QString ch)
{
    n1->value = ch;
    n1->toState = n2->stateNum;
}

/**
 * @brief NDFA::add
 * @param n1
 * @param n2
 * n1--eps->n2
 * NFA节点n1与n2间添加一条epsilon边，信息记录于n1节点中
 */
void NDFA::add(NDFA::NFANode *n1, NDFA::NFANode *n2)
{
    n1->epsToSet.insert(n2->stateNum);
}

/**
 * @brief NDFA::reg2NFA
 * @param regStr
 * 将正则表达式转换为NFA的主函数
 */
void NDFA::reg2NFA(QString regStr)
{
    NFAG=strToNfa(regStr);//调用转换函数
}

/**
 * @brief NDFA::NFA2DFA
 * 将NFA转换为DFA的主函数
 */
void NDFA::NFA2DFA()
{    
    QSet<int> tmpSet;
    tmpSet.insert(NFAG.startNode->stateNum);//将NFA初态节点放入集合
    get_e_closure(tmpSet);//求NFA初态节点的epsilon闭包得到DFA初态
    DFAStateArr[DFAStateNum].NFANodeSet=tmpSet;
    if(tmpSet.contains(NFAG.endNode->stateNum))
        DFAEndStateSet.insert(DFAStateNum);
    DFAStateNum++;

    QSet<QSet<int>> DFAStatesSet;//存储DFA节点包含的序号
    DFAStatesSet.insert(tmpSet);//将出台包含的序号集放入集合
    QQueue<DFANode> q;
    q.push_back(DFAStateArr[0]);//初态入队

    while(!q.empty())
    {
        DFANode *tmpDFANode=&q.front();//取出队列中一个DFA节点
        q.pop_front();

        tmpSet=tmpDFANode->NFANodeSet;//取出该DFA节点所包含的序号集
        int t_curState=tmpDFANode->stateNum;//记录当前节点序号

        for(const auto &ch: opCharSet)
        {
            QSet<int> chToSet;//能通过ch到达的节点的序号
            for(const auto &value: tmpSet)//遍历当前序号集合中的所有序号
            {
                //找出所有这样的序号
                if(NFAStateArr[value].value==ch)
                    chToSet.insert(NFAStateArr[value].toState);
            }

            if(chToSet.empty())
                continue;
            get_e_closure(chToSet);//求上得的序号集合的epsilon闭包

            if(!DFAStatesSet.contains(chToSet))
            {
                //若该DFA状态节点不存在
                //新建DFA节点
                DFAStateArr[DFAStateNum].NFANodeSet=chToSet;
                //若包含NFA终态
                if(chToSet.contains(NFAG.endNode->stateNum))
                    DFAEndStateSet.insert(DFAStateNum);
                //更新原节点的信息
                DFAStateArr[t_curState].DFAEdgeMap[ch]=DFAStateNum;
                DFAStatesSet.insert(chToSet);
                q.push_back(DFAStateArr[DFAStateNum++]);//节点入队后自增
            }
            else
            {
                for(int i=0;i<DFAStateNum;i++)
                {
                    if(DFAStateArr[i].NFANodeSet==chToSet)
                    {
                        //更新当前DFA节点状态
                        DFAStateArr[t_curState].DFAEdgeMap[ch]=i;
                        break;
                    }
                }
            }
        }
    }

}

/**
 * @brief NDFA::DFA2mDFA
 * DFA的最小化
 */
void NDFA::DFA2mDFA()
{
    mDFAStateNum=1;//未划分，状态数量为1
    for(int i=0;i<mDFAStateNum;i++)
    {
        if(!DFAStateArr[i].NFANodeSet.contains(NFAG.endNode->stateNum))
        {
            dividedSet[1].insert(DFAStateArr[i].stateNum);//非终态集合
            NFAStateNum=2;//设为2，终态与非终态
        }
        else dividedSet[0].insert(DFAStateArr[i].stateNum);
    }

    bool divFlag=true;//表示是否有新状态划分出来，有则
    while(divFlag)
    {
        divFlag=false;
        for(int i=0;i<mDFAStateNum;i++)
        {
            for(const auto &opChar: opCharSet)
            {
                int count = 0;
                stateSet t_stateSet[ARR_TEMP_SIZE];//分出的状态

                for(const auto &state: dividedSet[i])//遍历当前状态集中的所有节点
                {
                    //若当前DFA节点能通过opChar到达另外一个节点
                    if(DFAStateArr[state].DFAEdgeMap.contains(opChar))
                    {
                        //通过opChar到达的节点所属状态集合 号
                        int id=getStateId(dividedSet,DFAStateArr[state].DFAEdgeMap[opChar]);
                        bool haveSame=false;
                        for(int j=0;j<count;j++)
                        {
                            //若暂时分出来的状态号有相同的
                            if(t_stateSet[j].stateSetId==id)
                            {
                                haveSame=true;
                                t_stateSet[j].DFAStateSet.insert(state);//加入该状态
                                break;
                            }
                        }
                        if(!haveSame)
                        {
                            //若没有相同的
                            t_stateSet[count].stateSetId=id;
                            t_stateSet[count].DFAStateSet.insert(state);
                            count++;//todes
                        }
                    }
                    else
                    {
                        //若不能到达另外一个节点
                        bool haveSame=false;
                        for(int j=0;j<count;j++)
                        {
                            if(t_stateSet[j].stateSetId==-1)
                            {
                                haveSame=true;
                                t_stateSet[j].DFAStateSet.insert(state);
                                break;
                            }
                        }
                        if(!haveSame)
                        {
                            t_stateSet[count].stateSetId=-1;
                            t_stateSet[count].DFAStateSet.insert(state);
                            count++;//todes
                        }
                    }
                }
                if(count>1)
                {
                    divFlag=true;
                    for(int j=1;j<count;j++)
                    {
                        for(const auto &state: t_stateSet[j].DFAStateSet)
                        {
                            //将被分出去的DFA节点序号从原来状态中去除
                            dividedSet[i].remove(state);
                            //加入新状态
                            dividedSet[mDFAStateNum].insert(state);
                        }
                        mDFAStateNum++;
                    }
                }
            }
        }
    }

    for(int i=0;i<mDFAStateNum;i++)
    {
        mDFANodeArr[i].DFAStatesSet=dividedSet[i];
        for(const auto &state: dividedSet[i])
        {
            //若为初态
            if(state==0)
            {
                mDFAG.startState=i;
            }
            //若为终态
            if(DFAEndStateSet.contains(state))
            {
                mDFAG.endStateSet.insert(i);
            }

            //遍历所有操作符
            for(const auto &opChar: opCharSet)
            {
                if(DFAStateArr[state].DFAEdgeMap.contains(opChar))
                {
                    int id=getStateId(dividedSet,DFAStateArr[state].DFAEdgeMap[opChar]);
                    //若该边不存在，插入边
                    if(!mDFANodeArr[i].mDFAEdgesMap.contains(opChar))
                    {
                        mDFANodeArr[i].mDFAEdgesMap[opChar]=id;
                    }
                }
            }
        }
    }
}


/**
 * @brief NDFA::mDFA2Lexer
 * @return Lexer
 * 根据最小化DFA，生成词法分析程序C语言代码，返回代码字符串
 */
QString NDFA::mDFA2Lexer(QString filePath)
{
    QStringList keywordList=reg_keyword.split('|');

    QString lexCode;
    int m_state=mDFAG.startState;//最小化DFA的初态

    lexCode+="#include<stdio.h>\n";
    lexCode+="#include<stdlib.h>\n";
    lexCode+="#include<string.h>\n";
    lexCode+="#include<ctype.h>\n";
    lexCode+="#include<unordered_map>\n";
    //关键字映射map
    lexCode+="std::unordered_map<std::string, unsigned char> map;\n";
    //初始化关键字映射map
    lexCode+="void mapInitialize(FILE* output_map_fp) {\n";
    lexCode+="\tunsigned char value = 128;\n";

    for(const auto &tmp : keywordList)
    {
        lexCode+="\tmap[\""+tmp+"\"] = value;\n";
        lexCode+="\tfprintf(output_map_fp, \"%s %c\\n\", \""+tmp+"\", value++);\n";
    }
    lexCode+="}\n";
    lexCode+="void coding(FILE* input_fp,FILE* output_fp) {\n";
    lexCode+="\tchar tmp = fgetc(input_fp);\n";
    lexCode+="\tif (tmp == ' ' || tmp == '\\n' || tmp == '\\t'){\n";
    lexCode+="\t\tfprintf(output_fp, \"%c\", tmp);\n";
    lexCode+="\t\tprintf(\"%c\", tmp);\n";
    lexCode+="\t\treturn;\n";
    lexCode+="\t}\n";
    lexCode+="\tungetc(tmp, input_fp);\n";
    lexCode+="\tint state = "+QString::number(m_state)+";\n";
    lexCode+="\tbool flag = false;\n";
    lexCode+="\tbool isIndentifier = false;\n";
    lexCode+="\tbool isAnnotation = false;\n";
    lexCode+="\tstd::string value;\n";
    lexCode+="\twhile (!flag) {\n";
    lexCode+="\t\ttmp = fgetc(input_fp);\n";
    lexCode+="\t\tswitch (state) {\n";

    for(int i=0;i<mDFAStateNum;i++)
    {
        if(mDFANodeArr[i].mDFAEdgesMap.size()){
            lexCode+="\t\tcase "+QString::number(i)+": {\n";
            lexCode+="\t\t\tswitch (tmp) {\n";
            QList<QString> tmpList=mDFANodeArr[i].mDFAEdgesMap.keys();//该状态的所有边值
            if(genLexCase(tmpList,lexCode,i,1))lexCode+="\t\t\tdefault:state = "+QString::number(mDFANodeArr[i].mDFAEdgesMap["~"])+"; isAnnotation = true; break;\n";
            lexCode+="\t\t\t}\n";
            lexCode+="\t\t\tbreak;\n";
            lexCode+="\t\t}\n";
        }
    }
    lexCode+="\t\t}\n";
    lexCode+="\t\tvalue += tmp;\n";

    QList<int> stateList(mDFAG.endStateSet.begin(), mDFAG.endStateSet.end());//所有终态

    lexCode+="\t\tif (";
    for(int i=0;i<stateList.size();i++)
    {
        //要提前读一个字符判断是不是真的到终态，因为到了终态不一定是真正的终态
        int num=stateList[i];
        if(i)lexCode+="\t\telse if (";
        lexCode+="state =="+QString::number(num)+") {\n";
        lexCode+="\t\t\ttmp = fgetc(input_fp);\n";
        lexCode+="\t\t\tswitch (tmp) {\n";
        QList<QString> tmpList=mDFANodeArr[num].mDFAEdgesMap.keys();//该状态的所有边值
        genLexCase(tmpList,lexCode,num,0);
        lexCode+="\t\t\tdefault: {\n";
        lexCode+="\t\t\t\tflag=true;\n";
        if(tmpList.contains("letter"))lexCode+="\t\t\t\tisIndentifier = true;\n";
        lexCode+="\t\t\t}\n";
        lexCode+="\t\t\t}\n";
        lexCode+="\t\t\tungetc(tmp, input_fp);\n";
        lexCode+="\t\t}\n";
    }
    lexCode+="\t}\n";

    lexCode+="\tif (isIndentifier)if (map[value] != 0) {\n";
    lexCode+="\t\tfprintf(output_fp, \"%c\", map[value]);\n";
    lexCode+="\t\tprintf(\"%c\", map[value]);\n";
    lexCode+="\t\treturn;\n";
    lexCode+="\t}\n";
    lexCode+="\tif (!isAnnotation) {\n";
    lexCode+="\t\tfprintf(output_fp, value.c_str(), sizeof(value.c_str()));\n";
    lexCode+="\t\tprintf(value.c_str());\n";
    lexCode+="\t}\n";
    lexCode+="}\n";

    QFileInfo fileInfo(filePath);
    tmpFilePath=fileInfo.path();
    lexCode+="int main(int argc, char* argv[]) {\n";
    lexCode+="\tFILE* input_fp = fopen(\""+filePath+"\", \"r\");\n";
    lexCode+="\tif (input_fp == NULL) {\n";
    lexCode+="\t\tprintf(\"Failed to open input file\");\n";
    lexCode+="\t\treturn 1;\n";
    lexCode+="\t}\n";

    lexCode+="\tFILE* output_fp = fopen(\""+tmpFilePath+"/output.txt"+"\", \"w\");\n";
    lexCode+="\tif (output_fp == NULL) {\n";
    lexCode+="\t\tprintf(\"Failed to open output file\");\n";
    lexCode+="\t\tfclose(input_fp);\n";
    lexCode+="\t\treturn 1;\n";
    lexCode+="\t}\n";

    lexCode+="\tFILE* output_map_fp = fopen(\""+tmpFilePath+"/output_map.txt"+"\", \"w\");\n";
    lexCode+="\tif (output_map_fp == NULL) {\n";
    lexCode+="\t\tprintf(\"Failed to open output_map file\");\n";
    lexCode+="\t\tfclose(input_fp);\n";
    lexCode+="\t\tfclose(output_fp);\n";
    lexCode+="\t\treturn 1;\n";
    lexCode+="\t}\n";

    lexCode+="\tmapInitialize(output_map_fp);\n";
    lexCode+="\tfclose(output_map_fp);\n";
    lexCode+="\tchar c;\n";
    lexCode+="\twhile ((c=fgetc(input_fp)) != EOF) {\n";
    lexCode+="\t\tungetc(c, input_fp);\n";
    lexCode+="\t\tcoding(input_fp, output_fp);\n";
    lexCode+="\t}\n";
    lexCode+="\tfclose(input_fp);\n";
    lexCode+="\tfclose(output_fp);\n";
    lexCode+="\treturn 0;\n";
    lexCode+="}";

    lexerCodeStr=lexCode;
    return lexCode;
}
