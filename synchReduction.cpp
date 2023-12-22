//
// Created by leo on 23-8-12.
//

#include "synchReduction.h"

extern string tid_str;
extern string controlflowArcExp(string arcexp);
extern double get_timeSub(struct timespec time1, struct timespec time2);

void mip0() {
    try {

        // Create an environment
        GRBEnv env = GRBEnv(true);
        env.set("LogFile", "mip1.log");
        env.start();

        // Create an empty model
        GRBModel model = GRBModel(env);

        // Create variables
        GRBVar x = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "x");
        GRBVar y = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "y");
        GRBVar z = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "z");

        // Set objective: maximize x + y + 2 z
        model.setObjective(x + y + 2 * z, GRB_MAXIMIZE);

        // Add constraint: x + 2 y + 3 z <= 4
        model.addConstr(x + 2 * y + 3 * z <= 4, "c0");

        // Add constraint: x + y >= 1
        model.addConstr(x + y >= 1, "c1");

        // Optimize model
        model.optimize();

        cout << x.get(GRB_StringAttr_VarName) << " "
             << x.get(GRB_DoubleAttr_X) << endl;
        cout << y.get(GRB_StringAttr_VarName) << " "
             << y.get(GRB_DoubleAttr_X) << endl;
        cout << z.get(GRB_StringAttr_VarName) << " "
             << z.get(GRB_DoubleAttr_X) << endl;

        cout << "Obj: " << model.get(GRB_DoubleAttr_ObjVal) << endl;

    } catch(GRBException e) {
        cout << "Error code = " << e.getErrorCode() << endl;
        cout << e.getMessage() << endl;
    } catch(...) {
        cout << "Exception during optimization" << endl;
    }

}

GUROBI::GUROBI(CPN *cpn) {
    this->cpn=cpn;
    matrix.resize(cpn->get_placecount(),vector<int>(cpn->get_transcount()));
    pre_ti.resize(cpn->get_transcount(),vector<int>(cpn->get_placecount()));
    m0.resize(cpn->get_placecount());
    //m0 initialization
    for(auto i=0;i<cpn->get_placecount();i++){
        auto p=cpn->findP_byindex(i);
        if(p->getMultiSet().getcolorcount())
            m0[i]=1;
    }

    //matrix initialization
    cal_matrix();
}

void GUROBI::cal_matrix() {
    vector<vector<int>> output(cpn->get_placecount(),vector<int>(cpn->get_transcount()));
    vector<vector<int>> input(cpn->get_placecount(),vector<int>(cpn->get_transcount()));
    //计算输出矩阵
    for(auto i=0;i<cpn->get_transcount();i++){
        auto tran=cpn->findT_byindex(i);
        for(auto j=0;j<tran->get_consumer().size();j++){
            int row=tran->get_consumer()[j].idx;//place_idx
            int col=i;//t_idx
            output[row][col]=1;
        }
    }
    //计算输入矩阵
    for(auto i=0;i<cpn->get_transcount();i++){
        auto tran=cpn->findT_byindex(i);
        for(auto j=0;j<tran->get_producer().size();j++){
            int row=tran->get_producer()[j].idx;//place_idx
            int col=i;//t_idx
            input[row][col]=1;
        }
    }

    //A+ — A- = matrix
    for (int i = 0; i < cpn->get_placecount(); i++) {
        for (int j = 0; j < cpn->get_transcount(); j++) {
            matrix[i][j] = output[i][j] - input[i][j];
        }
    }

    //save pre_ti = input^T
    for(auto col=0;col<input.size();col++){
        for(auto row=0;row<input[col].size();row++)
            pre_ti[row][col]=input[col][row];
    }
}

bool GUROBI::mip0(NUM_t p_idx)
{
    try {

        // Create an environment
        GRBEnv env = GRBEnv(true);
        env.set("LogFile", "mip1.log");
        env.start();

        // Create an empty model
        GRBModel model = GRBModel(env);

        // Create variables
        GRBVar x = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "x");
        GRBVar y = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "y");
        GRBVar z = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "z");

        // Set objective: maximize x + y + 2 z
        model.setObjective(x + y + 2 * z, GRB_MAXIMIZE);

        // Add constraint: x + 2 y + 3 z <= 4
        {
            GRBLinExpr cons;
            cons += x;
            cons += 2 * y;
            cons += 3 * z;
            model.addConstr(cons <= 4, "c0");
        }
        // Add constraint: x + y >= 1
        {
            GRBLinExpr cons;
            cons += x;
            cons += y;
            model.addConstr(cons >= 1, "c1");
        }
        // Optimize model
        model.optimize();

        cout << x.get(GRB_StringAttr_VarName) << " "
             << x.get(GRB_DoubleAttr_X) << endl;
        cout << y.get(GRB_StringAttr_VarName) << " "
             << y.get(GRB_DoubleAttr_X) << endl;
        cout << z.get(GRB_StringAttr_VarName) << " "
             << z.get(GRB_DoubleAttr_X) << endl;

        cout << "Obj: " << model.get(GRB_DoubleAttr_ObjVal) << endl;

    } catch(GRBException e) {
        cout << "Error code = " << e.getErrorCode() << endl;
        cout << e.getMessage() << endl;
    } catch(...) {
        cout << "Exception during optimization" << endl;
    }

    return 0;
}

bool GUROBI::mip(NUM_t p_idx) {
    try {
        // Create an environment
        GRBEnv env = GRBEnv(true);
        env.set("LogFile", "mip1.log");
        // 禁用控制台输出
        env.set(GRB_IntParam_OutputFlag, 0);  // 将输出标志设置为0以禁用控制台输出


        env.start();

        // Create an empty model
        GRBModel model = GRBModel(env);

        // Create exclude p_idx
        set<int> exclude_pidx;
        exclude_pidx.insert(P_imp.begin(),P_imp.end());
        exclude_pidx.emplace(p_idx);

        // Create variables
        vector<GRBVar> x(cpn->get_placecount());
        for(auto i=0;i<cpn->get_placecount();i++){
            if(exclude_pidx.find(i)!=exclude_pidx.end())
                x[i]=model.addVar(0.0, 0.0, 0.0, GRB_BINARY, "x"+ to_string(i));
            else
                x[i]=model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "x"+ to_string(i));
        }
        // Add constraint: Y^T · N ≤ l_p
        vector<int> l_p=matrix[p_idx];

        for(auto i_col=0;i_col<cpn->get_transcount();i_col++){
            GRBLinExpr cons;
            for(auto i_x=0;i_x<x.size();i_x++){
                cons+=matrix[i_x][i_col] * x[i_x];
            }
            model.addConstr(cons <= l_p[i_col], "c"+ to_string(i_col));
        }

        // Add constraint: Y^T ·(M_0 -pre(t_i)) ≤ M_0(p)-1
        //不等式对任意t_i都需要成立
        auto p=cpn->findP_byindex(p_idx);
        int target=p->getMultiSet().getcolorcount()-1;

        vector<int> t_i;
        if(varP.find(p->getid())==varP.end() &&p->get_consumer().empty())
            return false;
        for(auto i_con=0;i_con<p->get_consumer().size();i_con++){
            t_i.emplace_back(p->get_consumer()[i_con].idx);
        }

        for(auto i_ti=0;i_ti<t_i.size();i_ti++){
            //pre(t_i)
            vector<int> pre_t_i=pre_ti[t_i[i_ti]];

            //M_0-pre(t_i)
            vector<int> m0_pre_ti(cpn->get_placecount());
            for(auto i=0;i<m0_pre_ti.size();i++)
                m0_pre_ti[i]=m0[i]-pre_t_i[i];

            // Add constraint: Y^T ·(M_0 -pre(t_i)) ≤ M_0(p)-1
            GRBLinExpr cons;
            for(auto i_x=0;i_x<x.size();i_x++){
                cons+=x[i_x] *m0_pre_ti[i_x];
            }
            model.addConstr(cons <= target, "cp"+ to_string(i_ti));
        }
        // 设置输出级别为0以禁用输出
        model.getEnv().set(GRB_IntParam_OutputFlag, 0);

        // Optimize model
        model.optimize();


        if (model.get(GRB_IntAttr_Status) == GRB_OPTIMAL || model.get(GRB_IntAttr_Status) == GRB_SUBOPTIMAL) {
//            for(auto i=0;i<x.size();i++){
//                if(x[i].get(GRB_DoubleAttr_X)) {
//                    auto p_y=cpn->findP_byindex(i);
//                    cout << p_y->getid() << ", ";
//                }
//            }
//            cout<<endl;
            return true;
        } else {
//            cout<<endl;
            return false;
        }

    } catch(GRBException e) {
        cout << "Error code = " << e.getErrorCode() << endl;
        cout << e.getMessage() << endl;
    } catch(...) {
        cout << "Exception during optimization" << endl;
    }

}

void GUROBI::get_varP() {
    for(auto p_idx=0;p_idx<cpn->get_placecount();p_idx++) {
        auto p=cpn->findP_byindex(p_idx);
        //若是控制库所或执行库所，则不是变量库所
        if(p->getiscontrolP() ||p->getisexecuted())
            continue;
        //若前后集均为空，则为变量库所（但没有语句修改其值）
        if(p->get_producer().empty() &&p->get_consumer().empty()) {
            varP.emplace(p->getid());
            continue;
        }
        //根据输入弧的类型判断变量库所。
        if(p->get_producer().empty())
            throw "Reduction::get_varP";
        auto arcType = p->get_producer()[0].arcType;
        if (arcType == readArc || arcType == writeArc || arcType == readwrite
            || arcType == allocwrite || arcType == remain) {
            varP.emplace(p->getid());
        }
    }
}

void Reduction::get_dead_PT(vector<NUM_t> &dead_P, set<NUM_t> &dead_T) {
    //根据初始token和输入变迁找死库所
    for(NUM_t i=0;i<cpn->get_placecount();i++){
        CPlace *p = &cpn->getplacearr()[i];
        if(p->getMultiSet().getcolorcount())
            continue;
        auto &p_pro=p->get_producer();
        if(p_pro.empty()) {
            dead_P.emplace_back(i);
            continue;
        }
//        bool has_pro=false;
//        for(auto j:p_pro){
//            if(!j.is_deleted){
//                has_pro=true;
//                break;
//            }
//        }
//        if(!has_pro)
//            dead_P.emplace_back(i);
    }

    //根据上一轮死库所找死库所和死变迁
    for(auto i=0;i<dead_P.size();i++){
        auto p=&cpn->getplacearr()[dead_P[i]];
        auto &p_con=p->get_consumer();
        for(auto icon:p_con){
            auto t_idx=icon.idx;
            auto t=&cpn->gettransarr()[t_idx];
            auto &t_pro=t->get_producer();
            bool is_dead=false;
            for(auto ipro:t_pro){
                if(exist_in(dead_P,ipro.idx)) {
                    is_dead = true;
                    break;
                }
            }
            if(!is_dead)
                continue;
            dead_T.emplace(t_idx);
            //找死库所
            auto &t_con=t->get_consumer();
            for(auto itcon:t_con){
                auto pp_idx=itcon.idx;
                if(exist_in(dead_P,pp_idx))
                    continue;

                auto pp=&cpn->getplacearr()[pp_idx];
                auto &pp_pro=pp->get_producer();
                is_dead= true;
                for(auto ippro:pp_pro){
                    if(dead_T.find(ippro.idx)==dead_T.end()){
                        is_dead=false;
                        break;
                    }
                }
                if(is_dead){
                    dead_P.emplace_back(pp_idx);
                }
            }
        }
    }


}

void Reduction::remove_PT(vector<NUM_t> &dead_P,const set<NUM_t>& dead_T) {
    vector<string> dead_P_s,dead_T_s;
    for(auto i:dead_P){
        auto &p=cpn->getplacearr()[i];
        dead_P_s.emplace_back(p.getid());
    }
    for(auto i:dead_T){
        auto &t=cpn->gettransarr()[i];
        dead_T_s.emplace_back(t.getid());
    }

    //modify mapPlace,place
    auto placecount=cpn->get_placecount();
    for(NUM_t i=0,j=0;i<placecount;i++){
        if(!exist_in(dead_P,i)) {
            cpn->place[j]=cpn->place[i];
            cpn->mapPlace.find(cpn->place[i].getid())->second=j;
            j++;
            continue;
        }
        cpn->placecount--;
        auto p=&cpn->getplacearr()[i];
        auto iter=cpn->mapPlace.find(p->getid());
        cpn->mapPlace.erase(iter);
        if(p->getiscontrolP() && p->getisexecuted())
            cpn->exePlacecount--;
        else if(p->getiscontrolP() && !p->getisexecuted())
            cpn->ctlPlacecount--;
        else {
            if(cpn->varPlacecount>0)
            cpn->varPlacecount--;
        }
    }

    //modify mapTransition,transition
    auto transcount=cpn->get_transcount();
    for(NUM_t i=0,j=0;i<transcount;i++){
        if(dead_T.find(i)==dead_T.end()) {
            cpn->transition[j]=cpn->transition[i];
            cpn->mapTransition.find(cpn->transition[i].getid())->second=j;
            j++;
            continue;
        }
        cpn->transitioncount--;
        auto t=&cpn->gettransarr()[i];
        auto iter=cpn->mapTransition.find(t->getid());
        cpn->mapTransition.erase(iter);
    }

    //modify arc
    auto arccount=cpn->arccount;
    for(unsigned int i=0,j=0;i<arccount;i++)
    {
        if(cpn->arc[i].getisp2t())
        {
            if(!exist_in(dead_P_s,cpn->arc[i].getsrc()) && !exist_in(dead_T_s,cpn->arc[i].gettgt())) {
                cpn->arc[j++] = cpn->arc[i];
            }else{
                cpn->arccount--;
            }
        }
        else {
            if (!exist_in(dead_T_s, cpn->arc[i].getsrc()) && !exist_in(dead_P_s, cpn->arc[i].gettgt())){
                cpn->arc[j++] = cpn->arc[i];
            }else{
                cpn->arccount--;
            }
        }
    }
    cpn->set_producer_consumer();

    update_synch();
}

void Reduction:: update_synch() {
    //q_begin_idx
    int i=0;
    for(;i<cpn->get_placecount();i++){
        auto &p=cpn->getplacearr()[i];
        if(p.getid()[0]=='Q') {
            synch->q_begin_idx = i;
            break;
        }
    }
    //q_count
    synch->q_count=0;
    for(;i<cpn->get_placecount();i++){
        auto &p=cpn->getplacearr()[i];
        if(p.getid()[0]!='Q')
            break;
        synch->q_count++;
    }
    //ba_t_begin_idx
    i=0;
    for(;i<cpn->get_transcount();i++){
        auto &t=cpn->gettransarr()[i];
        if(t.getid()[0]=='u' ||t.getid()[0]=='I') {
            synch->ba_t_begin_idx = i;
            break;
        }
    }
    //ba_t_count;  I_idx;  CP_tran
    synch->ba_t_count=0;
    synch->I_idx.clear();
    synch->CP_tran.clear();
    for(;i<cpn->get_transcount();i++){
        auto &t=cpn->gettransarr()[i];
        if(t.getid()[0]=='I'){
            synch->I_idx.emplace(i);
        }
        else if(t.getid()[0]=='C'&&t.getid()[1]=='P'){
            synch->CP_tran.emplace(i);
        }
        else if(t.getid()[0]!='u')
            break;
        synch->ba_t_count++;
    }

}

void Reduction::update_cpn() {
    //set_thread_enterT
    for(auto i=cpn->set_thread_enterT.begin();i!=cpn->set_thread_enterT.end();){
        if(cpn->mapTransition.find(*i)!=cpn->mapTransition.end()){
            i++;
            continue;
        }
        i=cpn->set_thread_enterT.erase(i);
    }
    //map_thread2beginP_endP
    for(auto i=cpn->map_thread2beginP_endP.begin();i!=cpn->map_thread2beginP_endP.end();){
        if(cpn->mapTransition.find(i->first.front())!=cpn->mapTransition.end()){
            i++;
            continue;
        }
        i=cpn->map_thread2beginP_endP.erase(i);
    }
    //map_call_enterP2exitP
    for(auto i=cpn->map_call_enterP2exitP.begin();i!=cpn->map_call_enterP2exitP.end();){
        if(cpn->mapPlace.find(i->first)!=cpn->mapPlace.end()){
            i++;
            continue;
        }
        i=cpn->map_call_enterP2exitP.erase(i);
    }
}

void Reduction::remove_deadPT(){
    vector<NUM_t > dead_P;
    set<NUM_t > dead_T;
    get_dead_PT(dead_P,dead_T);
    remove_PT(dead_P,dead_T);
    //死库所规则下可以更新cpn中相关信息
    update_cpn();
//    cpn->print_CPN("cpn_redu");

}

void Reduction::extractExVarPlace() {
    vector<pair<string,set<string>>> varW2R;

    //根据变迁提取相关变量
    //guard相关变量都需排除
    //赋值语句中保留写变量和读变量关系，后续分析
    for(auto i=0;i<cpn->get_transcount();i++){
        auto t=cpn->findT_byindex(i);
        auto t_con=t->get_consumer();
        //若有guard，则为条件语句，提取所有变量库所。
        if(t->get_hasguard()) {
            for (auto j: t_con) {
                if(j.is_deleted)
                    continue;
                if(j.arcType==readArc){
                    auto p=cpn->findP_byindex(j.idx);
                    ExVarPlace.emplace(p->getid());
                }
            }
        }else{//若没有guard，则可能为赋值语句，提取写变量和读变量
            string varW;
            set<string> varR;
            bool is_assign=false;
            for (auto j: t_con) {
                if(j.is_deleted)
                    continue;
                if(j.arcType==readArc){
                    auto p=cpn->findP_byindex(j.idx);
                    varR.emplace(p->getid());
                }else if(j.arcType==readwrite ||j.arcType==writeArc){
                    auto p=cpn->findP_byindex(j.idx);
                    varW=p->getid();
                    is_assign=true;
                }
            }
            if(is_assign){
                varW2R.emplace_back(varW,varR);
            }
        }
    }
    auto size=-1;
    while(size!=ExVarPlace.size()) {
        size=ExVarPlace.size();
        for (auto i: varW2R) {
            //判断赋值语句写变迁与guard中相关
            if (ExVarPlace.find(i.first) == ExVarPlace.end())
                continue;

            //若写变迁与guard相关，则读变迁也都与guard相关
            for (auto j: i.second) {
                ExVarPlace.emplace(j);
            }
        }
    }
}


void Reduction::remove_implicitP() {
    extractExVarPlace();

    shared_ptr<GUROBI>gurobi = make_shared<GUROBI>(cpn);
    gurobi->get_varP();
    for(auto p_idx=0;p_idx<cpn->get_placecount();p_idx++) {

        auto p=cpn->findP_byindex(p_idx);
//        if(ExVarPlace.find(p->getid())!=ExVarPlace.end())
//            continue;
//        if(p->getisexecuted())
//            continue;
        auto p_id=p->getid();
        if(is_exclude_P(p_id) || is_exclude_P_chgCon(p_id))
            continue;
        if(gurobi->mip(p_idx)) {
            gurobi->P_imp.emplace_back(p_idx);
        }
    }
    remove_PT(gurobi->P_imp);
}

vector<index_t> Reduction::get_p_pro(index_t p_idx){
    vector<index_t > ts;
    auto p=cpn->findP_byindex(p_idx);
    auto pPro=&p->get_producer();
    for(auto i=0;i<pPro->size();i++){
        ts.emplace_back((*pPro)[i].idx);
    }
    return ts;
}

vector<index_t> Reduction::get_p_con(index_t p_idx){
    vector<index_t > ts;
    auto p=cpn->findP_byindex(p_idx);
    auto pCon=&p->get_consumer();
    for(auto i=0;i<pCon->size();i++){
        ts.emplace_back((*pCon)[i].idx);
    }
    return ts;
}

vector<index_t> Reduction::get_t_pro(index_t t_idx){
    vector<index_t > ps;
    auto t=cpn->findT_byindex(t_idx);
    auto tPro=&t->get_producer();
    for(auto i=0;i<tPro->size();i++){
        ps.emplace_back((*tPro)[i].idx);
    }
    return ps;
}

vector<index_t> Reduction::get_t_con(index_t t_idx){
    vector<index_t > ps;
    auto t=cpn->findT_byindex(t_idx);
    auto tCon=&t->get_consumer();
    for(auto i=0;i<tCon->size();i++){
        ps.emplace_back((*tCon)[i].idx);
    }
    return ps;
}

void Reduction::merge_Abstraction(vector<string> ps_id) {
    //排除不可约简库所
    if(contain_exclude_P(ps_id))
        return;

    vector<index_t> ps;
    for(auto i:ps_id){
        auto p_idx=cpn->mapPlace[i];
        ps.emplace_back(p_idx);
    }

    auto p_idx=ps.front();
    auto us= get_p_pro(p_idx);
    auto t_idx= *get_p_con(p_idx).begin();

    auto t=cpn->findT_byindex(t_idx);
    auto t_pro= get_t_pro(t_idx);
    vector<index_t> extra_p;
    for(auto i=0;i<t_pro.size();i++){
        if(exist_in(ps,t_pro[i]))
            continue;
        extra_p.emplace_back(t_pro[i]);
    }

    //连接(•t ∩ Pred) × (•p ∩ Tred)
    for(auto i=0;i<extra_p.size();i++){
        auto p=cpn->findP_byindex(extra_p[i]);
        auto source_id=p->getid();
        auto target_id=t->getid();
        for(auto j=0;j<us.size();j++){
            auto u=cpn->findT_byindex(us[j]);
            auto target_new_id=u->getid();
            changeArc(source_id,target_id,source_id,target_new_id);
        }
    }

    //连接(•p ∩ Tred) × t•
    auto t_con= get_t_con(t_idx);
    for(auto i=0;i<t_con.size();i++){
        auto p=cpn->findP_byindex(t_con[i]);
        auto source_id=t->getid();
        auto target_id=p->getid();
        for(auto j=0;j<us.size();j++){
            auto u=cpn->findT_byindex(us[j]);
            auto source_new_id=u->getid();
            changeArc(source_id,target_id,source_new_id,target_id);
        }
    }

    set<index_t> ts;
    ts.emplace(t_idx);
    remove_PT(ps,ts);
}

void Reduction::changeArc(std::string source, std::string target, std::string source_new, std::string target_new) {
    for(unsigned int i=0;i<cpn->arccount;i++) {
        auto arc=cpn->getarcarr();
        if ((arc[i].getsrc() == source || source == "")
            && (arc[i].gettgt() == target || target == "")) {
            arc[i].change_src_tgt(source_new,target_new);
//            for(unsigned int j=i;j<arccount-1;j++){
//                arc[j] = arc[j+1];
//            }
//            arccount--;
//            i--;
        }
    }
}

void Reduction::copyArc(std::string source, std::string target, std::string source_new, std::string target_new) {
    CArc *aa;
    aa = cpn->findArc_bysrctgt(source,target);
    if(aa==NULL){
        cout<< "Redu::copyArc can't find source arc!"<<endl;
        exit(-1);
    }
    if(aa->getdeleted())
        return;
    CArc *bb;
    bb = &cpn->arc[cpn->arccount++];
    bb->InitArc(source_new,target_new,aa->getisp2t(),aa->getexp(),aa->getArctype());

    //若拷贝变量弧，需添加下述过程。
//    if (bb->getexp() != "") {
//        string _P = bb->getrelated_P();
//        auto pp = cpn->findP_byid(_P);
//        auto ps = sorttable.find_productsort(pp->getsid());
//        cpn->Add_Variable(bb->getroot(), pp->gettid(), pp->getsid(), 0, ps.get_sortnum() - 1);
//    }
}

void Reduction::Abstraction_Rule() {
    // •p ！= ∅, p• = {t}
    // ∀u ∈ •p: u• = {p}
    // t• ！= ∅
    // M0(p) = 0

    set<index_t > visited;
    vector<vector<string>> PS;
    //遍历库所p
    for(auto p_idx=0;p_idx<cpn->get_placecount();p_idx++){

        if(visited.find(p_idx)!=visited.end())
            continue;
        vector<index_t> us;//pPro

        //获取p的前集变迁ts
        us= get_p_pro(p_idx);
        //检查前集非空
        if(us.empty())
            continue;
        vector<index_t > ps;

        //检查us的后集库所ps相同
        {
            auto iter = us.begin();
            ps = get_t_con(*iter);

            bool samep = true;
            while (++iter != us.end()) {
                vector<index_t> ps2 = get_t_con(*iter);
                sort(ps.begin(),ps.end());
                sort(ps2.begin(),ps2.end());
                if (ps != ps2) {
                    samep = false;
                    break;
                }
            }
            if (!samep)
                continue;
        }

        //检查ps的前集变迁等于us
        {
            auto iter = ps.begin();
            auto us1 = get_p_pro(*iter);

            bool sameUs = true;
            while (++iter != ps.end()) {
                vector<index_t> us2 = get_p_pro(*iter);
                sort(us.begin(),us.end());
                sort(us2.begin(),us2.end());
                if (us1 != us2) {
                    sameUs = false;
                    break;
                }
            }
            if (!sameUs)
                continue;
        }

        //检查ps的后集等于t
        {
            auto iter = ps.begin();
            auto ts = get_p_con(*iter);
            //检查t唯一
            if(ts.size()!=1)
                continue;

            auto t=*ts.begin();
            bool samet=true;
            while (++iter != ps.end()) {
                vector<index_t> ts2 = get_p_con(*iter);
                if (ts != ts2) {
                    samet = false;
                    break;
                }
            }
            if (!samet)
                continue;
        }

        //检查M0(ps)=0
        bool hastoken=false;
        for(auto p_i=0;p_i<ps.size();p_i++){
            auto p=cpn->findP_byindex(ps[p_i]);
            if(p->getMultiSet().tokennum_unf()){
                hastoken= true;
                break;
            }
        }
        if(hastoken)
            continue;

        //检查完成
        vector<string> ps_id;
        for(auto i=0;i<ps.size();i++) {
            auto p=cpn->findP_byindex(ps[i]);
            ps_id.emplace_back(p->getid());
        }
        PS.emplace_back(ps_id);
    }

    //网约简
    for(auto i=0;i<PS.size();i++){
        merge_Abstraction(PS[i]);
    }

}

//void Reduction::mergeUT_Pre_Agglomeration(vector<index_t> us, index_t p_idx, index_t t_idx) {
//    auto t_pro= get_t_pro(t_idx);
//    auto t=cpn->findT_byindex(t_idx);
//    for(auto i=0;i<t_pro.size();i++){
//        auto p_src=cpn->findP_byindex(t_pro[i]);
//        auto source=p_src->getid();
//        auto target=t->getid();
//        for(auto j=0;j<us.size();j++){
//            auto u=cpn->findT_byindex(us[j]);
//            auto target_new=u->getid();
//            changeArc(source,target,source,target_new);
//        }
//    }
//    vector<index_t> ps;
//    set<index_t >ts;
//    ps.emplace_back(p_idx);
//    ts.emplace(t_idx);
//    remove_PT(ps,ts);
//}

//void Reduction::Pre_Agglomeration_Rule() {
//    //•p = {t}, p• != ∅
//    //•t != ∅, t• = {p}
//    //M0(p) = 0
//
//    vector<index_t> PS;
//    //遍历库所p
//    for(auto p_idx=0;p_idx<cpn->get_placecount();p_idx++){
//        //检查p前集等于t
//        auto ts=get_p_pro(p_idx);
//        if(ts.size()!=1)
//            continue;
//
//        //检查p的后集不为空
//        auto us= get_p_con(p_idx);
//        if(us.empty())
//            continue;
//
//        //检查t的前集不为空
//        auto t_idx=ts.front();
//        auto t_pro= get_t_pro(t_idx);
//        if(t_pro.empty())
//            continue;
//
//        //检查t的后集为p；
//        auto t_con= get_t_con(t_idx);
//        if(t_con.size()!=1)
//            continue;
//
//        //检查M0(p)=0
//        auto p=cpn->findP_byindex(p_idx);
//        if(p->getMultiSet().tokennum())
//            continue;
//
//        //检查完成
//        PS.emplace_back(p_idx);
//    }
//
//    //网约简
//    for(auto i=0;i<PS.size();i++){
//        auto p_idx=PS[i];
//        auto us= get_p_pro(p_idx);
//        auto t_idx= *get_p_con(p_idx).begin();
//        mergeUT_Pre_Agglomeration(us,p_idx,t_idx);
//    }
//}

//void Reduction::mergeUT_Post_Agglomeration(index_t p_idx) {
//
//}

//void Reduction::Post_Agglomeration_Rule() {
//    // •p != ∅, p• != ∅
//    // ∀t ∈ p•: •t = {p}
//    // M0(p) = 0
//
//    vector<index_t> PS;
//    //遍历库所p
//    for(auto p_idx=0;p_idx<cpn->get_placecount();p_idx++){
//        //检查p前集非空
//        auto ts=get_p_pro(p_idx);
//        if(ts.empty())
//            continue;
//
//        //检查p的后集不为空
//        auto us= get_p_con(p_idx);
//        if(us.empty())
//            continue;
//
//        //检查t的前集等于p
//        {
//            bool equal = true;
//            for (auto i = 0; i < us.size(); i++) {
//                auto t_pro = get_t_pro(us[i]);
//                if (t_pro.size() != 1) {
//                    equal = false;
//                    break;
//                }
//            }
//            if (!equal)
//                continue;
//        }
//
//        //检查M0(p)=0
//        auto p=cpn->findP_byindex(p_idx);
//        if(p->getMultiSet().tokennum())
//            continue;
//
//        //检查完成
//        PS.emplace_back(p_idx);
//    }
//
//    //网约简
//    for(auto i=0;i<PS.size();i++){
//        auto p_idx=PS[i];
//        auto us= get_p_pro(p_idx);
//        auto t_idx= *get_p_con(p_idx).begin();
//        mergeUT_Post_Agglomeration(p_idx);
//    }
//}

void Reduction::T_Reduction() {
    // •t1 = •t2
    // t1• = t2•
    //附加条件：G(t1)=G(t2)

    set<index_t > visited;
    vector<vector<string>> TS;
    //遍历变迁t
    for(auto t_idx=0;t_idx<cpn->get_transcount();t_idx++){
        if(visited.find(t_idx)!=visited.end())
            continue;

        // •t1 = •t2
        auto t_bro= get_bro_Tran(t_idx);

        visited.insert(t_bro.begin(),t_bro.end());
        visited.emplace(t_idx);

        vector<string> ts;
        auto t_id=cpn->findT_byindex(t_idx)->getid();
        ts.emplace_back(t_id);
        for(auto i:t_bro){
            auto t_bro_id=cpn->findT_byindex(i)->getid();
            ts.emplace_back(t_bro_id);
        }
        TS.emplace_back(ts);
    }

    //网约简
    for(auto i=0;i<TS.size();i++){
        merge_T_Reduction(TS[i]);
    }
}

vector<index_t > Reduction::get_bro_Tran(index_t t_idx) {
    vector<index_t> ts_Bro;
    auto t_pro= get_t_pro(t_idx);
    for(auto i:t_pro){
        auto p_con= get_p_con(i);
        for(auto j:p_con){
            if(j!=t_idx && !exist_in(ts_Bro,j))
                ts_Bro.emplace_back(j);
        }
    }
    return ts_Bro;
}

void Reduction::merge_T_Reduction(vector<std::string> ts) {
    set<index_t > TS;
    //保留第一个变迁，删除其他变迁
    for(auto i=1;i<ts.size();i++){
        auto t_idx=cpn->mapTransition[ts[i]];
        TS.emplace(t_idx);
    }
    vector<index_t> PS;
    remove_PT(PS,TS);
}

bool contain_vec(vector<index_t> vec1,vector<index_t> vec2){
    if(vec1.size()<vec2.size())
        return false;

    std::sort(vec1.begin(), vec1.end());
    std::sort(vec2.begin(), vec2.end());
    for(auto i=0;i<vec2.size();i++){
        if(vec1[i]!=vec2[i])
            return false;
    }
    return true;
}

void Reduction::Post_Reduction() {
    // If Σ¬ϕ has two distinct transitions u and t
    // such that •t ！= t•, •t ⊆ u•, and ∀p ∈ •t, p• = {t},
    // then by Post-Reduction Rule,
    // we can obtain the resulting synchronization
    // Σ′¬ϕ = (P, T, F ′, M_0, I, L),
    //where F ′ = (F \({u} × •t)) ∪ ({u} × t•).
    vector<pair<vector<string>, string>> u2t;
    set<string> visited;
    //遍历变迁t
    for(auto t_idx=0;t_idx<cpn->get_transcount();t_idx++){
        auto t_id=cpn->findT_byindex(t_idx)->getid();
        if(visited.find(t_id)!=visited.end())
            continue;

        //•t ！= t•
        auto t_pro=get_t_pro(t_idx);
        auto t_con= get_t_con(t_idx);
        if(t_pro.size()==t_con.size() && contain_vec(t_pro,t_con))
            continue;

        //•t ⊆ u•
        set<index_t > ps_pro;
        for(auto i=0;i<t_pro.size();i++){
            auto p_pro= get_p_pro(t_pro[i]);
            ps_pro.insert(p_pro.begin(),p_pro.end());
        }
        vector<index_t> us;
        for(auto i=ps_pro.begin();i!=ps_pro.end();i++){
            auto u_con= get_t_con(*i);
            if(contain_vec(u_con,t_pro))
                us.emplace_back(*i);
        }
        if(us.empty())
            continue;

        //and ∀p ∈ •t, p• = {t}
        bool oneCon= true;
        for(auto i:t_pro){
            auto p_con= get_p_con(i);
            if(p_con.size()!=1) {
                oneCon = false;
                break;
            }
        }
        if(!oneCon)
            continue;

        //附加条件:t没有guard
        auto t=cpn->findT_byindex(t_idx);
        if(t->get_hasguard())
            continue;

        vector<string> us_id;
        for(auto i:us){
            auto u_id=cpn->findT_byindex(i)->getid();
            us_id.emplace_back(u_id);
        }

        bool is_visited=false;
        for(auto i:us_id){
            if(visited.find(i)!=visited.end()){
                is_visited=true;
                break;
            }
        }
        if(is_visited)
            continue;
        visited.emplace(t_id);
        visited.insert(us_id.begin(),us_id.end());
        u2t.emplace_back(us_id,t_id);
    }

    //网约简
    for(auto i:u2t){
        merge_Post_Reduction(i.first,i.second);
    }
}

void Reduction::merge_Post_Reduction(vector<string> us_id,string t_id) {
    if(is_exclude_Post_Reduction(us_id,t_id))
        return;

    auto t_idx=cpn->mapTransition[t_id];
    auto t_pro= get_t_pro(t_idx);
    auto t_con= get_t_con(t_idx);
    for(auto i=0;i<us_id.size();i++){
        auto u_idx=cpn->mapTransition[us_id[i]];
        auto u=cpn->findT_byindex(u_idx);
        auto u_con= get_t_con(u_idx);
        auto src=u->getid();
        for(auto j=0;j<u_con.size();j++){
            if(exist_in(t_pro,u_con[j])){
                auto tgt=cpn->findP_byindex(u_con[j])->getid();
                cpn->delete_arc(src,tgt);
            }
        }
        //Add_Arc(source_T[i], executed_P, controlflowArcExp(tid_str), false, executed,normal);
        for(auto j=0;j<t_con.size();j++){
            auto tgt=cpn->findP_byindex(t_con[j])->getid();
            copyArc(t_id,tgt,src,tgt);
        }
    }
    cpn->set_producer_consumer();
}

bool Reduction::is_exclude_Post_Reduction(vector<std::string> us_id, std::string t_id) {
    for(auto i:us_id){
        if(is_exclude_T_chgCon(i))
            return true;
    }
    auto t_idx=cpn->mapTransition[t_id];
    auto t_pro= get_t_pro(t_idx);
    for(auto i:t_pro){
        auto p_id=cpn->findP_byindex(i)->getid();
        if(is_exclude_P_chgPro(p_id))
            return true;
    }
    return false;
}

void Reduction::Pre_Reduction() {
    // If Σ¬ϕ has a transition t such that
    // •t ∩ t• = ∅, ∀u ∈ PreTran(t), u• ⊂ •t,
    // ∀p ∈ •t, •p ！= ∅, p• = {t}, and M (p) = 0
    vector<pair<vector<string>, string>> u2t;
    //遍历变迁t
    for(auto t_idx=0;t_idx<cpn->get_transcount();t_idx++){
        //•t ∩ t• = ∅
        auto t_pro=get_t_pro(t_idx);
        auto t_con= get_t_con(t_idx);
        bool inter=false;
        for(auto i:t_pro){
            if(exist_in(t_con,i)){
                inter=true;
                break;
            }
        }
        if(inter)
            continue;

        //∀u ∈ PreTran(t), u• ⊂ •t
        bool cover= true;
        auto preTran= get_PreTran(t_idx);
        for(auto iu:preTran){
            auto u_con= get_t_con(iu);
            if(!contain_vec(t_pro,u_con)){
                cover= false;
                break;
            }
        }
        if(!cover){
            continue;
        }

        // ∀p ∈ •t, •p ！= ∅, p• = {t}, and M (p) = 0
        bool check=true;
        for(auto i:t_pro){
            auto p=cpn->findP_byindex(i);
            auto p_pro=p->get_producer();
            if(p_pro.empty()){
                check=false;
                break;
            }
            auto p_con=p->get_consumer();
            if(p_con.size()!=1){
                check=false;
                break;
            }
            if(p->has_token()){
                check=false;
                break;
            }
        }
        if(!check)
            continue;

        vector<string> preTran_id;
        for(auto i:preTran){
            auto preT_id=cpn->findT_byindex(i)->getid();
            preTran_id.emplace_back(preT_id);
        }
        auto t_id=cpn->findT_byindex(t_idx)->getid();
        u2t.emplace_back(preTran_id,t_id);
    }

    //网约简
    for(auto i:u2t){
        merge_Pre_Reduction(i.first,i.second);
    }
}

vector<index_t> Reduction::get_PreTran(index_t t_idx) {
    vector<index_t> preTran;
    auto t_pro=cpn->findT_byindex(t_idx)->get_producer();
    for(auto i:t_pro){
        auto p_pro=cpn->findP_byindex(i.idx)->get_producer();
        for(auto j:p_pro){
            if(!exist_in(preTran,j.idx))
                preTran.emplace_back(j.idx);
        }
    }
    return preTran;
}

void Reduction::merge_Pre_Reduction(vector<string> us_id, string t_id) {
    if(is_exclude_Pre_Reduction(us_id,t_id))
        return;

    vector<index_t >us;
    for(auto i:us_id){
        auto u_idx=cpn->mapTransition[i];
        us.emplace_back(u_idx);
    }
    auto t_idx=cpn->mapTransition[t_id];

    auto u_combin=findComb(us,t_idx);
    auto t_pro= get_t_pro(t_idx);

    //添加变迁集合D
    for(auto i:u_combin){
        add_Tran_Pre_Reduction(i,t_idx);
    }

    //删除库所、变迁
    auto ps=t_pro;
    set<index_t > ts;
    ts.insert(us.begin(),us.end());
    ts.emplace(t_idx);
    remove_PT(ps,ts);
}

void Reduction::add_Tran_Pre_Reduction(vector<index_t> us, index_t t_idx) {
    CTransition *tt = &cpn->transition[cpn->transitioncount++];
    auto id="T"+ to_string(id_count++);
    auto t=cpn->findT_byindex(t_idx);
    tt->Init_Transition(id,t->get_guard().getexp(),t->getRow());
    cpn->mapTransition.insert(make_pair(id,cpn->transitioncount-1));

    tt->isabort=t->isabort;

    //add consumer arc
    auto t_con= cpn->findT_byindex(t_idx)->get_consumer();
    for(auto i:t_con){
        auto tgt=cpn->findP_byindex(i.idx)->getid();
        copyArc(t->getid(),tgt,tt->getid(),tgt);
    }
    //add produce arc
    for(auto i:us){
        auto tgt=cpn->findT_byindex(i)->getid();
        auto u_pro=cpn->findT_byindex(i)->get_producer();
        for(auto j:u_pro){
            auto src=cpn->findP_byindex(j.idx)->getid();
            copyArc(src,tgt,src,tt->getid());
        }
    }
}

bool Reduction::is_exclude_Pre_Reduction(vector<std::string> us_id, std::string t_id) {
    if(is_exclude_T_chgPro(t_id))
        return true;
    return false;
}

void findCombUtil(vector<vector<index_t>>& us_con,vector<vector<index_t>>& res,vector<index_t>& currentU,vector<index_t>& currentPlace, NUM_t targetSUM, NUM_t index){
    //如果当前组合的工作数等于目标工作数，保存该组合
    if(currentPlace.size()==targetSUM){
        res.emplace_back(currentU);
        return;
    }

    // 如果当前索引越界，返回
    if(index>=us_con.size()){
        return;
    }

    // 尝试包含当前u的后集
    bool inter=false;
    for(auto i:us_con[index]){
        if(exist_in(currentPlace,i)){
            inter=true;
            break;
        }
    }
    if(!inter){
        currentU.emplace_back(index);
        NUM_t size_ori=currentPlace.size();
        currentPlace.insert(currentPlace.end(),us_con[index].begin(),us_con[index].end());
        findCombUtil(us_con,res,currentU,currentPlace,targetSUM,index+1);
        // 回溯
        currentU.pop_back();
        currentPlace.erase(currentPlace.begin()+size_ori,currentPlace.end());
    }

    // 尝试不包含当前u的后集
    findCombUtil(us_con,res,currentU,currentPlace,targetSUM,index+1);
}

vector<vector<index_t>> Reduction::findComb(vector<index_t> us, index_t t_idx) {
    vector<vector<index_t>> us_con;
    for(auto i:us){
        auto u_con= get_t_con(i);
        us_con.emplace_back(u_con);
    }
    auto t_pro= get_t_pro(t_idx);

    vector<vector<index_t>> u_combin;
    vector<index_t> currentU,currentPlace;
    findCombUtil(us_con,u_combin,currentU,currentPlace,t_pro.size(),0);
    return u_combin;
}
void Reduction::Post_A_Rule() {
    //If Σ¬ϕ has a place p such that •p != ∅,
    //p• != ∅, ∀t ∈ p•, •t = {p} and p !∈ t•
    vector<string> PS;
    //遍历库所p
    for(index_t p_idx=0;p_idx<cpn->get_placecount();p_idx++){
        //检查p前集非空
        auto p_pro = get_p_pro(p_idx);
        if (p_pro.empty())
            continue;

        //检查p的后集非空
        auto p_con = get_p_con(p_idx);
        if (p_con.empty())
            continue;
        //检查t的前集等于p
        {
            bool equal = true;
            for (auto i = 0; i < p_con.size(); i++) {
                auto t_pro = get_t_pro(p_con[i]);
                if (t_pro.size() != 1) {
                    equal = false;
                    break;
                }
            }
            if (!equal)
                continue;
        }

        //检查p ！∈ t•
        {
            bool iscontain = false;
            for (auto i = 0; i < p_con.size(); i++) {
                auto t_con = get_t_con(p_con[i]);
                if (exist_in(t_con,p_idx)) {
                    iscontain = true;
                    break;
                }
            }
            if (iscontain)
                continue;
        }

        //检查完成
        auto p_id=cpn->findP_byindex(p_idx)->getid();
        PS.emplace_back(p_id);
    }

    //网约简
    for(auto i=0;i<PS.size();i++){
        auto p_id=PS[i];
        merge_Post_A_Rule(p_id);
    }
}

void Reduction::merge_Post_A_Rule(std::string p_id) {
    //排除不可约简库所
    if(is_exclude_P(p_id))
        return;

    auto p_idx=cpn->mapPlace[p_id];
    auto us= get_p_pro(p_idx);
    auto t_idx= *get_p_con(p_idx).begin();

    for(auto i:us){
        add_Tran_Post_A_Rule(i,t_idx);
    }
    set<index_t > ts;
    ts.insert(us.begin(),us.end());

    vector<NUM_t> ps;
    remove_PT(ps,ts);
}

void Reduction::add_Tran_Post_A_Rule(index_t u_idx, index_t t_idx) {
    CTransition *tt = &cpn->transition[cpn->transitioncount++];
    auto id="T"+ to_string(id_count++);
    auto t=cpn->findT_byindex(t_idx);
    tt->Init_Transition(id,t->get_guard().getexp(),t->getRow());
    cpn->mapTransition.insert(make_pair(id,cpn->transitioncount-1));

    tt->isabort=t->isabort;

    //add consumer arc
    auto t_con= cpn->findT_byindex(t_idx)->get_consumer();
    for(auto i:t_con){
        auto tgt=cpn->findP_byindex(i.idx)->getid();
        copyArc(t->getid(),tgt,tt->getid(),tgt);
    }
    auto t_pro=t->get_producer().front().idx;
    auto u=cpn->findT_byindex(u_idx);
    auto u_con=u->get_consumer();
    for(auto i:u_con){
        if(t_pro==i.idx)
            continue;
        auto tgt=cpn->findP_byindex(i.idx)->getid();
        copyArc(u->getid(),tgt,tt->getid(),tgt);
    }
    //add produce arc
    auto tgt=cpn->findT_byindex(u_idx)->getid();
    auto u_pro=cpn->findT_byindex(u_idx)->get_producer();
    for(auto j:u_pro){
        auto src=cpn->findP_byindex(j.idx)->getid();
        copyArc(src,tgt,src,tt->getid());
    }
}

void Reduction::Pre_A_Rule() {
    // If Σ¬ϕ has a place p such that
    // •p != ∅, p• != ∅, ∀t ∈ •p, t• = {p} and p !∈ •t,

    vector<string> PS;
    //遍历库所p
    for(index_t p_idx=0;p_idx<cpn->get_placecount();p_idx++){
        //•p != ∅
        auto p_pro= get_p_pro(p_idx);
        if(p_pro.empty())
            continue;

        //p• != ∅
        auto p_con= get_p_con(p_idx);
        if(p_con.empty())
            continue;

        //∀t ∈ •p, t• = {p} and p !∈ •t
        bool satisfied=true;
        for(auto i:p_pro){
            auto t_con= get_t_con(i);
            if(t_con.size()!=1){
                satisfied=false;
                break;
            }
            auto t_pro= get_t_pro(i);
            if(exist_in(t_pro,p_idx)){
                satisfied=false;
                break;
            }
        }
        if(!satisfied)
            continue;

        auto p_id=cpn->findP_byindex(p_idx)->getid();
        PS.emplace_back(p_id);
    }

    //网约简
    for(auto i=0;i<PS.size();i++){
        merge_Pre_A_Rule(PS[i]);
    }
}

void Reduction::merge_Pre_A_Rule(std::string p_id) {
    //排除不可约简库所
    if(is_exclude_P(p_id))
        return;

    auto p_idx=cpn->mapPlace[p_id];
    auto p=cpn->findP_byindex(p_idx);

    auto ts= get_p_pro(p_idx);
    auto us= get_p_con(p_idx);

    if(p->has_token()) {
        for (auto i: us) {
            auto t_id=cpn->findT_byindex(i)->getid();
            if (is_exclude_T_CallEnterPCon(t_id))
                return;
        }
    }

    for(auto i:ts){
        for(auto j:us)
            add_Tran_Pre_A_Rule(i,j);
    }
    set<index_t > Ts;
    Ts.insert(ts.begin(),ts.end());

    vector<NUM_t> ps;
    remove_PT(ps,Ts);
}

void Reduction::add_Tran_Pre_A_Rule(index_t t_idx, index_t u_idx) {
    CTransition *tt = &cpn->transition[cpn->transitioncount++];
    auto id="T"+ to_string(id_count++);
    auto t=cpn->findT_byindex(t_idx);
    tt->Init_Transition(id,t->get_guard().getexp(),t->getRow());
    cpn->mapTransition.insert(make_pair(id,cpn->transitioncount-1));
    tt->isabort=t->isabort;

    //add consumer arc
    auto u=cpn->findT_byindex(u_idx);
    auto u_con= cpn->findT_byindex(u_idx)->get_consumer();
    for(auto i:u_con){
        auto tgt=cpn->findP_byindex(i.idx)->getid();
        copyArc(u->getid(),tgt,tt->getid(),tgt);
    }

    //add produce arc
    auto tgt=t->getid();
    auto t_pro=t->get_producer();
    for(auto i:t_pro){
        auto src=cpn->findP_byindex(i.idx)->getid();
        copyArc(src,tgt,src,tt->getid());
    }
    auto t_con=t->get_consumer().front().idx;
    auto u_pro=u->get_producer();
    for(auto i:u_pro){
        if(t_con==i.idx)
            continue;
        auto src=cpn->findP_byindex(i.idx)->getid();
        copyArc(src,u->getid(),src,tt->getid());
    }
}

bool Reduction::is_exclude_T_CallEnterPCon(std::string t) {
    if(exclude_T_CallEnterPCon.find(t)!=exclude_T_CallEnterPCon.end())
        return true;
    return false;
}

void Reduction::local_redu() {
    Abstraction_Rule();
//    Pre_Agglomeration_Rule();
//    Post_Agglomeration_Rule();
//    T_Reduction();//不存在该情况
    Post_Reduction();
    Pre_Reduction();
    Post_A_Rule();
//    Pre_A_Rule();
//    update_synch();
}

bool Reduction::is_exclude_P(string p) {
    if(exclude_P.find(p)!=exclude_P.end())
        return true;
    return false;
}

bool Reduction::is_exclude_T(string t) {
    if(exclude_T.find(t)!=exclude_T.end())
        return true;
    return false;
}

bool Reduction::is_exclude_P_chgPro(std::string p) {
    if(exclude_P_chgPro.find(p)!=exclude_P_chgPro.end())
        return true;
    return false;
}

bool Reduction::is_exclude_P_chgCon(std::string p) {
    if(exclude_P_chgCon.find(p)!=exclude_P_chgCon.end())
        return true;
    return false;
}

bool Reduction::is_exclude_T_chgPro(std::string t) {
    if(exclude_T_chgPro.find(t)!=exclude_T_chgPro.end())
        return true;
    return false;
}

bool Reduction::is_exclude_T_chgCon(std::string t) {
    if(exclude_T_chgCon.find(t)!=exclude_T_chgCon.end())
        return true;
    return false;
}

bool Reduction::contain_exclude_P(vector<string> p_vec) {
    for(auto i:p_vec){
        if(is_exclude_P(i))
            return true;
    }
    return false;
}

void Reduction::get_exclude_PT() {
    //排除function的入口Place和出口Place
    auto f_map=cpn->f_table.get_f_map();
    for(auto i=f_map.begin();i!=f_map.end();i++){
        auto beginP_id=i->second.begin_place;
        exclude_P.emplace(beginP_id);
        auto endP_id=i->second.end_place;
        exclude_P.emplace(endP_id);

        auto p_con= get_p_con(cpn->mapPlace[beginP_id]);
        for(auto j:p_con) {
            auto t_id=cpn->findT_byindex(j)->getid();
            exclude_T_chgPro.emplace(t_id);
        }
//        auto p_con= get_p_con(cpn->mapPlace[endP_id]);
//        for(auto j:p_con) {
//            auto t_id=cpn->findT_byindex(j)->getid();
//            exclude_T_chgPro.emplace(t_id);
//        }
    }

    //set_thread_enterT
    exclude_T_chgCon.insert(cpn->set_thread_enterT.begin(),cpn->set_thread_enterT.end());
    for(auto i:cpn->set_thread_enterT){
        auto t_idx=cpn->mapTransition[i];
        auto t_con= get_t_con(t_idx);
        for(auto j:t_con){
            auto p_id=cpn->findP_byindex(j)->getid();
            exclude_P_chgPro.emplace(p_id);
        }
    }

    //map_thread2beginP_endP
    for(auto i=cpn->map_thread2beginP_endP.begin();i!=cpn->map_thread2beginP_endP.end();i++){
        for(auto j:i->second)
            exclude_P.emplace(j);

        //UNFPDN::cal_call
//        exclude_P_chgPro.emplace(i->second.front());
//        auto p_idx=cpn->mapPlace[i->second.front()];
//        auto p_pro= get_p_pro(p_idx);
//        for(auto j:p_pro){
//            auto t_id=cpn->findT_byindex(j)->getid();
//            exclude_T_chgCon.emplace(t_id);
//        }
    }

    //map_call_enterP2exitP
    for(auto i=cpn->map_call_enterP2exitP.begin();i!=cpn->map_call_enterP2exitP.end();i++){
        exclude_P.emplace(i->first);
        exclude_P.emplace(i->second);

        exclude_P_chgCon.emplace(i->first);
        exclude_P_chgCon.emplace(i->second);
        auto p_con= get_p_con(cpn->mapPlace[i->first]);
        for(auto j:p_con){
            auto t_id=cpn->findT_byindex(j)->getid();
            exclude_T_chgPro.emplace(t_id);
            exclude_T_CallEnterPCon.emplace(t_id);
        }
        p_con= get_p_con(cpn->mapPlace[i->second]);
        for(auto j:p_con){
            auto t_id=cpn->findT_byindex(j)->getid();
            exclude_T_chgPro.emplace(t_id);
        }
//        auto p_pro= get_p_pro(cpn->mapPlace[i->first]);
//        for(auto j:p_pro){
//            auto t_id=cpn->findT_byindex(j)->getid();
//            exclude_T.emplace(t_id);
//        }
//        p_pro= get_p_pro(cpn->mapTransition[i->second]);
//        for(auto j:p_pro){
//            auto t_id=cpn->findT_byindex(j)->getid();
//            exclude_T.emplace(t_id);
//        }
    }
}

void Reduction::init() {
    id_count=cpn->transitioncount;
    get_exclude_PT();
}

void Reduction::reduce(){
    auto p_count_ori=cpn->get_placecount();
    auto t_count_ori=cpn->get_transcount();
    auto p_var_ori=cpn->get_varplacecount();

//    local_redu();
//    remove_deadPT();
    clock_t red_start=clock();
    remove_implicitP();
    clock_t red_end=clock();
    cout<<endl<<"time_red: "<<(red_end-red_start)/1000000.0;
//    cpn->print_CPN("cpn_ba");
    cpn->cal_t2thread_init();
    cout<<endl<<"PDNet reduced"<<endl
    <<"placenum: "<<p_count_ori<<" -> "<<cpn->get_placecount()<<endl
    <<"varplacenum: "<<p_var_ori<<" -> "<<cpn->get_varplacecount()<<endl
    <<"transnum: "<<t_count_ori<<" -> "<<cpn->get_transcount()<<endl;

}
