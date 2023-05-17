#include "bnfp.h"


BNFP::BNFP()
{


}

void BNFP::InitGrammar()
{
    m_startChar='\0';
    m_tmrSet.clear();
    m_nonTmrSet.clear();
    m_GM_productionMap.clear();

    QString grammarString=ui->plainTextEdit_edit->toPlainText();
    QStringList productions = grammarString.split("\n");

    m_startChar=productions[0].at(0);

    for(const auto &subProduction: qAsConst(productions))
    {
        if(subProduction.simplified().isEmpty())continue;//略去空行

        QStringList productionContent = subProduction.split("->");//分离左右部
        QChar productionL = productionContent[0].simplified().at(0);//左部
        QStringList productionR_list=productionContent.at(1).split("|");//右部

        for(const auto &productionR: qAsConst(productionR_list))
        {
            for(const auto&ch: productionR)
            {
                if(isTerminator(ch))
                {
                    m_tmrSet.insert(ch);
                }
            }

            if(productionR.simplified()=="@")
            {
                m_GM_productionMap[productionL].insert("");
            }
            else
            {
                m_GM_productionMap[productionL].insert(productionR.simplified());
            }
        }
    }

    SimplifyGrammar();
}

/**
 * @brief BNFP::isTerminator
 * @param s
 * @return
 * 判断是否为终结符
 */
bool BNFP::isTerminator(QString s)
{
    return m_tmrSet.contains(s);
}

/**
 * @brief BNFP::isNonTerminator
 * @param s
 * @return
 * 判断是否为非终结符
 */
bool BNFP::isNonTerminator(QString s)
{
    return m_nonTmrSet.contains(s);
}

/**
 * @brief BNFP::isProductionR_terminable
 * 判断右部是否可终结
 * @param nonTmrSet
 * @param productionR
 * @return
 */
bool BNFP::isProductionR_terminable(const QSet<QChar> &nonTmrSet, const QString &productionR)
{
    for(const auto c: productionR)
    {
        if(isNonTerminator(c) and not nonTmrSet.contains(c)) return false;
    }
    return true;
}

bool BNFP::isProductionR_reachable(const QSet<QChar> &nonTmrSet, const QSet<QChar> &tmrSet, const QString &productionR)
{
    for(const auto &c: productionR)
    {
        if((isNonTerminator(c) and not nonTmrSet.contains(c)) or
                (isTerminator(c) and not tmrSet.contains(c)))
        {
            return false;
        }
    }
    return true;
}



void BNFP::SimplifyGrammar()
{
    //1.消除形如U->U等有害规则
    const QList<QString> &tmp_GM_productionMap_keys=m_GM_productionMap.keys();
    for(const auto &tmp_productionL: tmp_GM_productionMap_keys)
    {
        m_GM_productionMap[tmp_productionL].remove(tmp_productionL);
    }

    //2.消除不可终结的无用符号以及无用产生式
    QSet<QString> tmp_nonTmrSet;
    QMap<QChar, QSet<QString>> tmp_productions;

    qsizetype preNonTmrSetSize=tmp_nonTmrSet.size();
    qsizetype curNonTmrSetSize=std::numeric_limits<qsizetype>::max();

    while(preNonTmrSetSize != curNonTmrSetSize)
    {
        preNonTmrSetSize=curNonTmrSetSize;

        const QList<QString> &tmp_GM_productionMap_keys=m_GM_productionMap.keys();
        for(const auto &tmp_productionL: tmp_GM_productionMap_keys)
        {
            if(tmp_nonTmrSet.contains(tmp_productionL)) continue;

            for(const auto &tmp_productionR: qAsConst(m_GM_productionMap[tmp_productionL]))
            {
                if(isProductionR_terminable(tmp_nonTmrSet, tmp_productionR))
                {
                    tmp_nonTmrSet.insert(tmp_productionL);//有一个产生式符合就结束判断
                    break;
                }
            }
        }

        curNonTmrSetSize=tmp_nonTmrSet.size();//更新当前非终结符数量
    }

    //4.遍历产生式，对于每一条产生式，如果它的右部是可以终结的，就将该条产生式加入产生式集合映射中；
    for(const auto &tmp_productionL: qAsConst(tmp_nonTmrSet))
    {
        for(const auto &tmp_productionR: m_GM_productionMap.value(tmp_productionL))
        {
            if(isProductionR_terminable(tmp_nonTmrSet, tmp_productionR))
            {
                tmp_productions[tmp_productionL].insert(tmp_productionR);
            }
        }
    }

    //5.更新非终结符集合与产生式集合；
    m_nonTmrSet=tmp_nonTmrSet;
    m_GM_productionMap=tmp_productions;

    //c.消除不可达的无用符号以及无用产生式；
    tmp_nonTmrSet.clear();
    tmp_productions.clear();
    QSet<QChar> tmp_tmrSet;
    tmp_nonTmrSet.insert(m_startChar);

    preNonTmrSetSize=tmp_nonTmrSet.size();
    curNonTmrSetSize=std::numeric_limits<qsizetype>::max();
    qsizetype curTmrSetSize=std::numeric_limits<qsizetype>::max();
    qsizetype preTmrSetSize=tmp_tmrSet.size();


    while(preNonTmrSetSize != curNonTmrSetSize ||
          preTmrSetSize != curTmrSetSize)
    {
        preNonTmrSetSize=curNonTmrSetSize;
        preTmrSetSize=curTmrSetSize;

        const QList<QChar> &tmp_GM_productionMap_keys=m_GM_productionMap.keys();
        for(const auto &tmp_productionL: tmp_GM_productionMap_keys)
        {
            if(!m_nonTermSet.contains(tmp_productionL))continue;

            for(const auto &tmp_productionR: qAsConst(m_GM_productionMap[tmp_productionL]))
            {
                for(const auto &c: tmp_productionR)
                {
                    if(isNonTerminator(c))
                    {
                        tmp_nonTmrSet.insert(c);
                    }
                    else
                    {
                        tmp_tmrSet.insert(c);
                    }
                }
            }

            curNonTmrSetSize=tmp_nonTmrSet.size();
            curTmrSetSize=tmp_tmrSet.size();
        }

        //4.
        for(const auto &tmp_productionL: qAsConst(tmp_nonTmrSet))
        {
            for(const auto &tmp_productionR: m_GM_productionMap.value(tmp_productionL))
            {
                if(isProductionR_reachable(tmp_nonTmrSet, tmp_tmrSet, tmp_productionR))
                {
                    tmp_productions[tmp_productionL].insert(tmp_productionR);
                }
            }
        }

        m_GM_productionMap=tmp_productions;
        m_nonTmrSet=tmp_nonTmrSet;
        m_tmrSet=tmp_tmrSet;
    }

}



/**
 * @brief BNFP::printGrammar
 * @param e
 * 文法输出主函数
 */
void BNFP::PrintGrammar(QPlainTextEdit *e)
{
    e->clear();

    QString grammarString;

    const QList<QString> &t_GM_productionMap_keys=m_GM_productionMap.keys();
    for(const QString &tmp_productionL: t_GM_productionMap_keys)
    {
        QString tmp_productionRStr;
        for(const QString &tmp_productionR: qAsConst(m_GM_productionMap[tmp_productionL]))
        {
            if(tmp_productionR.isEmpty())
            {
                tmp_productionRStr += "| @ ";
            }
            else
            {
                tmp_productionRStr += " | " + tmp_productionR;
            }
        }
        QString pLine=QString(tmp_productionL)+" -> "+tmp_productionRStr.remove(0,2);//字符串处理去掉首个右部'|'
        grammarString += (pLine+'\n');
    }

    e->insertPlainText(grammarString);
}

/**
 * @brief BNFP::eliminateLRecursion
 * 消除文法左递归
 */
void BNFP::EliminateLRecursion()
{
    //给非终结符规定顺序
    QList<QChar> tmp_nonTmrList;
    for(const auto &tmp_nonTmr: qAsConst(m_nonTmrSet))
    {
        tmp_nonTmrList.push_back(tmp_nonTmr);
    }

    //按照该顺序遍历所有非终结符
    for(auto curChar{tmp_nonTmrList.begin()};curChar != tmp_nonTmrList.end(); curChar++)
    {
        for(auto preChar{tmp_nonTmrList.begin()};preChar != curChar; preChar++)
        {
            replaceL(*preChar, *curChar);
        }

        //消除直接左递归，定义不含左递归的产生式右部集合
        QSet<QString> tmp_nonLRecursionRSet;
        //包含左递归的产生式右部的集合
        QSet<QString> tmp_LRecursionRSet;

        for(const auto &tmp_productionR: qAsConst(m_GM_productionMap[*curChar]))
        {
            if(tmp_productionR.isEmpty() || *curChar != tmp_productionR[0])
            {
                tmp_nonLRecursionRSet.insert(tmp_productionR);
            }//去除左递归对应的符号
            else
            {
                QString tmpR=tmp_productionR;
                tmpR.remove(0,1);//去掉第一个字符
                tmp_LRecursionRSet.insert(tmpR);
            }
        }

        //若非终结符产生式不包含左递归则跳过判断
        if(tmp_LRecursionRSet.isEmpty()) continue;

        //否则申请新的非终结符
        QChar newNonTmr=getNewTmr();

        //对于 A-> Aa|b形式，先添加B->aB|ɛ 这样一条产生式
        for(const auto &tmp_productionR: tmp_LRecursionRSet)
        {
            m_GM_productionMap[newNonTmr].insert(tmp_productionR+newNonTmr);
        }
        m_GM_productionMap[newNonTmr].insert("");

        //移除原来的产生式"A->Aa|b"
        m_GM_productionMap.remove(*curChar);

        // 添加 A->bB这样一条产生式
        for(const auto &tmp_productionR: tmp_nonLRecursionRSet)
        {
            m_GM_productionMap[*curChar].insert(tmp_productionR+newNonTmr);
        }
    }


    SimplifyGrammar();
}

void BNFP::replaceL(const QChar &replaceC, const QChar &replaced)
{
    QSet<QString> tmp_replacedProductionRSet = m_GM_productionMap[replaced];

    m_GM_productionMap.remove(replaced);

    for(const auto &tmp_replaceProductionR: tmp_replacedProductionRSet)
    {
        if(tmp_replaceProductionR.isEmpty() || replaceC != tmp_replaceProductionR[0])
        {
            m_GM_productionMap[replaced].insert(tmp_replaceProductionR);
            continue;
        }

        QSet<QString> replacedResult=replaceL(replaceC,tmp_replaceProductionR);

        for(const auto &replacedC: qAsConst(replacedResult))
            m_GM_productionMap[replaced].insert(replacedC);
    }


}

QSet<QString> BNFP::replaceL(const QChar &replaceC, const QString &productionR)
{
    QSet<QString> resSet;

    for(const auto &replaceCProductionR: m_GM_productionMap[replaceC])
    {
        QString replaced=productionR;
        replaced.replace(0,1,replaceCProductionR);
        resSet.insert(replaced);
    }
    return resSet;
}


QChar BNFP::getNewTmr()
{
    QSet<QChar> tmp_availableNonTmrSet;
    for(char c='A';c<='Z';c++)
    {
        tmp_availableNonTmrSet.insert(c);
    }
    tmp_availableNonTmrSet -= m_nonTmrSet;
    if(tmp_availableNonTmrSet.isEmpty())
    {
        qDebug()<<"[文法化简]:非终结符超限，申请失败";

        //throw QString{ "[文法化简]:非终结符超限，申请失败" };
    }

    return *m_nonTmrSet.insert(*tmp_availableNonTmrSet.begin());
}

/**
 * @brief BNFP::eliminateLCommonFactor
 */
void BNFP::EliminateLCommonFactor()
{
    QStack<QChar> tmp_nonTmrStk;

    for(const auto &tmp_nonTmr: m_GM_productionMap.keys())
    {
        tmp_nonTmrStk.push_back(tmp_nonTmr);
    }
    while(!tmp_nonTmrStk.isEmpty())
    {
        QChar tmp_productionL=tmp_nonTmrStk.pop();

        QSet<QString> tmp_replaceResSet;
        for(const auto &tmp_productionR: m_GM_productionMap[tmp_productionL])
        {
            tmp_replaceResSet |= replaceLNonTmr(tmp_productionR);
        }

        m_GM_productionMap.remove(tmp_productionL);

        if(tmp_replaceResSet.contains(""))
            m_GM_productionMap[tmp_productionL].insert("");

        QSet<QChar> commonPrefixes=getRPPrefixes(tmp_replaceResSet);

        for(const auto &commonPrefix: commonPrefixes)
        {
            QSet<QString> suffixes=findPrefixProductions(commonPrefix,tmp_replaceResSet);

            if(suffixes.size()>1)
            {
                QChar newNonTmr=getNewTmr();

                m_GM_productionMap[newNonTmr]=suffixes;

                m_GM_productionMap[tmp_productionL].insert(QString(commonPrefix)+newNonTmr);

                tmp_nonTmrStk.push_back(newNonTmr);
            }
            else
            {
                m_GM_productionMap[tmp_productionL].insert(commonPrefix + *suffixes.begin());
            }
        }
    }

    SimplifyGrammar();
}

QSet<QString> BNFP::replaceLNonTmr(const QString &productionR)
{
    if(productionR.isEmpty() || isTerminator(productionR[0]))
    {
        return {productionR};
    }

    QSet<QString> resSet;

    QStack<QString> tmpStk;
    tmpStk.push_back(productionR);

    //推导次数限制
    short count=4;
    qDebug()<<'-';
    while(!tmpStk.isEmpty())
    {
        qDebug()<<1;
        if(--count==0)
        {
            qDebug()<<"推导超限（4次），无法消除左公因子";
            throw QString{"推导超限（4次），无法消除左公因子"};

        }

        //带入栈中的数据处理
        QString curProduction=tmpStk.pop();
        QSet<QString> newResSet=replaceL(curProduction.at(0),curProduction);

        for(const auto &newProductionR: qAsConst(newResSet))
        {
            if(newProductionR.isEmpty() || isTerminator(newProductionR[0]))
            {
                resSet.insert(newProductionR);
            }
            else
            {
                tmpStk.push_back(newProductionR);
            }
        }
    }

    return resSet;
}

QSet<QChar> BNFP::getRPPrefixes(const QSet<QString> &productionRSet)
{
    QSet<QChar> prefixes;

    for(const auto &productionR: productionRSet)
    {
        if(productionR.isEmpty())continue;
        prefixes.insert(productionR[0]);
    }

    return prefixes;
}

QSet<QString> BNFP::findPrefixProductions(const QChar &prefix, const QSet<QString> &productionRSet)
{
    QSet<QString> preProductionSet;

    for(const auto &productionR: productionRSet)
    {
        if(!productionR.isEmpty() && prefix==productionR[0])
        {
            QString tmp=productionR;
            tmp.remove(0,1);
            preProductionSet.insert(tmp);
        }
    }

    return preProductionSet;
}

QSet<QString> BNFP::getFirstSet(const QChar &nonTmr)
{
    QSet<QString> firstSet;

    for(const auto &tmp_productionR: m_GM_productionMap[nonTmr])
    {
        firstSet |= getFirstSet(tmp_productionR);
    }

    return firstSet;
}

QSet<QString> BNFP::getFirstSet(const QString &productionR)
{
    if(productionR.isEmpty())
        return {""};

    QSet<QString> firstSet;

    for(const auto &c: productionR)
    {
        if(isTerminator(c))
            return firstSet |= c;

        firstSet |= getFirstSet(c);

        if(!firstSet.contains(""))
            return firstSet;

        firstSet.remove("");
    }
    return firstSet |= "";
}

QMap<QChar, QSet<QString> > BNFP::GetFirstSet()
{
    QMap<QChar, QSet<QString>> resMap;

    for(const auto &tmp_productionL: m_firstSetMap.keys())
    {
        for(const auto &tmp_productionRSet: m_firstSetMap[tmp_productionL])
        {
            resMap[tmp_productionL] |= tmp_productionRSet;
        }
    }
    return resMap;
}

QMap<QChar, QSet<QChar> > BNFP::GetFollowSet()
{
    return m_followSetMap;
}

void BNFP::firstNfollowSet()
{
    m_followSetMap.clear();
    m_firstSetMap.clear();
    QList<QPair<QChar, QChar>> pendingFollow;

    followSetMap[startChar].insert('$');

    for(const auto &tmp_productionL: m_nonTmrSet)//非终结符
    {
        for(const auto &tmp_productionR: GM_productionMap[tmp_productionL])//对于右部每一条产生式
        {
            firstSetMap[tmp_productionL][tmp_productionR]=getFirstSet(tmp_productionR);

            for(int idx=0;idx<tmp_productionR.length();++idx)//对于每一条产生式逐个
            {
                if(isNonTerminator(tmp_productionR[idx]))//若为非终结符则开求
                {
                    QString tmp=tmp_productionR;
                    tmp.remove(0,idx+1);
                    QSet<QString> tmp_firstSet=getFirstSet(tmp);

                    //remove epsilon
                    for(const auto &tmr: tmp_firstSet)//后面的first集元素除了eps外全加进去
                    {
                        if(!tmr.isEmpty())
                            followSetMap[tmp_productionR[idx]] |= tmr[0];
                    }

                    if(tmp_firstSet.contains(""))
                    {
                        pendingFollow.push_back({tmp_productionL, tmp_productionR[idx]});
                    }
                }
            }
        }
    }

    bool finished=false;
    while(!finished)
    {
        finished=true;
        for(const auto &[from, to]: pendingFollow)
        {
            qsizetype size=m_followSetMap[to].size();
            m_followSetMap[to] |= m_followSetMap[from];

            if(size != m_followSetMap[to].size())
                finished=false;
        }
    }
}

