//
// Created by leo on 22-12-7.
//

#ifndef PUNF_UNF_HEURISTIC_H
#define PUNF_UNF_HEURISTIC_H
#include "iostream"
#include "cpn.h"
#include "BA/tinyxml.h"
#include "synchronization.h"
#include "cmath"

typedef class Possible_Extend PE;
class Binding_unf;
class CUT;
typedef class Configuration Config;
class UNFPDN;
class CUTINFO;

class Heuristic{
public:
    SYNCH *synch;
    CPN *cpn;
    LTLCategory LTLtype;//LTL公式类型;
    vector<string> P,T;//target P\T
    map<index_t ,map<index_t,NUM_t>> dis2T;//t-->disOfEachT2t;
    map<index_t ,vector<pair<index_t,index_t>>> map_q2ProgressQT;
    map<index_t ,NUM_t > map_dis2acceptQ;//q->dis

    void init(SYNCH *synch,LTLCategory type);
    void cal_Heuristic(SYNCH *synch ,int number, LTLCategory type);//计算启发式相关信息；
    void extract_targetPT(int number);
    void GetLTL(ofstream &out,TiXmlElement *p);
    void cal_q2ProgressQT();//q->下一状态q'及之间的弧对应的变迁
    void cal_q2acceptQ();

    index_t get_nextE(vector<PE *> en_e,Config * C,UNFPDN *unfpdn);//获取下一加入配置的事件。
    Config *get_Config_add(Config *C, PE *pe,UNFPDN *unfpdn);
//    NUM_t DefaultBAstep=1000;//BA每步权重。
    NUM_t get_dis2acceptQ(index_t q_cur);
//LTLF
    void cal_dis2T(index_t t_idx);//计算各变迁到目标变迁的距离；
    bool update_dis(map<index_t,NUM_t> &map_dis,index_t t_idx,index_t t_pa);
    NUM_t dis2LTLF(string s);
    //guard
    NUM_t DefaultDis2G=100000;//guard变迁不可达时给一个最大的距离。因为后面还要作乘法，不能设为-1。
    NUM_t DefaultGDelay=100000;
//    NUM_t DefaultTstep=100;
    map<index_t,vector<index_t>> map_guardPath;//T0->t
    void cal_guardPath();
    NUM_t get_dis2guard(index_t t_idx);//到目标变迁路径上第一个guard成立的距离;
    Config *C_plus;
    PE* pe_next;
    index_t T_target;
    bool is_directWriteGuard=false;//判断下一变迁是否处于guard变迁的路径上，且直接赋值guard中变量。
    NUM_t dis2guard(const condition_tree& guard);//当前状态到目标guard的距离;
    void dis2CTN(condition_tree_node * guard,NUM_t &dis);//当前状态到目标guard的距离;
    void dis2CTN_Token(condition_tree_node *root,NUM_t &dis);
    void dis2CTN_SingleColor(condition_tree_node *root,token &condtk,NUM_t &dis);//有修改
    bool FetchBktV(string v,Config *C,Bucket &bkt);//获取变量对应token
    set<index_t > set_overwriteT;
    void cal_overwriteT();//通过输出弧表达式变量数获取覆盖写变迁
    bool is_DWG(string v);
    Binding_unf * get_bindingF( CTransition *transition, CUT *cut);
    Binding_unf * bindingTokenF(condition_tree_node *node, CUTINFO *multiset);

//LTLV
    NUM_t dis2LTLV(string s);
    bool FetchColor(string s,bucket &color);//获取token-count语句中库所的token

};
#endif //PUNF_UNF_HEURISTIC_H
