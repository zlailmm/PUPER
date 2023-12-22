//
// Created by leo on 23-8-12.
//

#ifndef PUNF_SYNCHREDUCTION_H
#define PUNF_SYNCHREDUCTION_H
#include "synchronization.h"
#include "gurobi_c++.h"
#include "algorithm"
#include "base.h"
using namespace std;

//LinearProgramming
class GUROBI{
public:
    CPN *cpn;
    vector<vector<int>> matrix;//P*T
    vector<vector<int>> pre_ti;//T*P
    vector<int> m0;
    set<string> varP;
    vector<NUM_t> P_imp;//保存求得的隐库所，用于求后续隐库所时排除。

    GUROBI(CPN *cpn);
    void cal_matrix();//计算关联矩阵
    bool mip(NUM_t p_idx);
    bool mip0(NUM_t p_idx);
    void get_varP();
};

class Reduction{
public:
    SYNCH *synch;
    CPN* cpn;
    Reduction(SYNCH *synch){
        this->synch=synch;
        this->cpn=synch->cpn_synch;
        init();
    };
    void init();
    void reduce();

    set<string> exclude_P;//需要排除的P。包括函数入口P和出口P（后续分线程需要）
    set<string> exclude_T;//需要排除的T。
    set<string> exclude_P_chgPro;//更改库所输入弧时需要排除的T。
    set<string> exclude_P_chgCon;//更改库所输出弧时需要排除的T。
    set<string> exclude_T_chgPro;//更改变迁输入弧时需要排除的T。
    set<string> exclude_T_chgCon;//更改变迁输出弧时需要排除的T。


    void get_exclude_PT();
    bool is_exclude_P(string p_id);
    bool is_exclude_T(string t_id);
    bool is_exclude_P_chgPro(string p_id);
    bool is_exclude_P_chgCon(string p_id);
    bool is_exclude_T_chgPro(string t_id);
    bool is_exclude_T_chgCon(string t_id);

    bool contain_exclude_P(vector<string> p_vec);

    void remove_PT(vector<NUM_t> &dead_P,const set<NUM_t> &dead_T={});
    //死变迁规则
    void remove_deadPT();
    void get_dead_PT(vector<NUM_t> &dead_P,set<NUM_t> &dead_T);
    void update_synch();//更新合成网相关索引
    void update_cpn();//更新cpn相关索引
    //隐库所规则
    void remove_implicitP();

    set<string> ExVarPlace;
    void extractExVarPlace();
    //局部约简规则
    void local_redu();
    NUM_t id_count;

    void Abstraction_Rule();//抽象规则
    void merge_Abstraction(vector<string> ps_id);
//    void Pre_Agglomeration_Rule();//前聚集规则
//    void mergeUT_Pre_Agglomeration(vector<index_t> us,index_t p_idx,index_t t_idx);
//    void Post_Agglomeration_Rule();//后聚集规则
//    void mergeUT_Post_Agglomeration(index_t p_idx);
    void T_Reduction();//T约简规则-不存在该情况
    vector<index_t > get_bro_Tran(index_t t_idx);
    void merge_T_Reduction(vector<string> ts);

    void Post_Reduction();//后约简规则
    void merge_Post_Reduction(vector<string> us_id,string t_id);
    bool is_exclude_Post_Reduction(vector<string> us_id,string t_id);

    void Pre_Reduction();//前约简规则
    vector<index_t> get_PreTran(index_t t_idx);
    void merge_Pre_Reduction(vector<string> us_id,string t_id);
    vector<vector<index_t >> findComb(vector<index_t> us,index_t t_idx);
    void add_Tran_Pre_Reduction(vector<index_t> us,index_t t_idx);//以u,t为模板添加变迁
    bool is_exclude_Pre_Reduction(vector<string> us_id,string t_id);

    void Post_A_Rule();//后A规则
    void merge_Post_A_Rule(string p_id);
    void add_Tran_Post_A_Rule(index_t u_idx,index_t t_idx);//以u,t为模板添加变迁

    void Pre_A_Rule();//前A规则
    void merge_Pre_A_Rule(string p_id);
    void add_Tran_Pre_A_Rule(index_t t_idx,index_t u_idx);//以u,t为模板添加变迁
    set<string> exclude_T_CallEnterPCon;//更改库所输入弧时需要排除的T。
    bool is_exclude_T_CallEnterPCon(string t_id);

    //Auxiliary
    vector<index_t> get_p_pro(index_t p_idx);//获取p.producer
    vector<index_t> get_p_con(index_t p_idx);//获取p.consumer
    vector<index_t> get_t_pro(index_t t_idx);//获取t.consumer
    vector<index_t> get_t_con(index_t t_idx);//获取t.consumer
    void changeArc(string source,string target,string source_new,string target_new);
    void copyArc(string source,string target,string source_new,string target_new);
};

#endif //PUNF_SYNCHREDUCTION_H
