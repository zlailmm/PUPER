//
// Created by leo on 22-12-7.
//

#include "unfolding.h"


extern char LTLFfile[];
extern char LTLVfile[];
extern char LTLCfile[];

extern bool is_BoolOperator(string s);
extern type JudgeConstType(string value);
extern Bucket operate(const Bucket &lbk,string Operator);
extern Bucket operate(const Bucket &lbk,const Bucket &rbk,string Operator);
extern void BindingVariable(const Binding_unf *binding, CPN *cpn);
extern Binding_unf *bindingcid_unf(Product_t cid, SORTID sid, condition_tree_node *tokennode);
extern void BindingVariable(const Binding_unf *binding, CPN *cpn);

template<class T>
NUM_t disOfV(T v1, T v2) {
    return abs(v1 - v2) ;
}

void Heuristic::init(SYNCH *synch, LTLCategory type) {
    this->synch=synch ;cpn=synch->cpn_synch;this->LTLtype=type;
    cal_overwriteT();
}

void Heuristic::cal_Heuristic(SYNCH *synch, int number, LTLCategory type) {
    init(synch, type);
    extract_targetPT(number);

    //计算各变迁到准则变迁的距离
    if (LTLtype == LTLC)
        return;
    else if (LTLtype == LTLF) {
        for (auto i = T.begin(); i != T.end(); i++) {
            auto t_idx = cpn->mapTransition.find(*i)->second;
            cal_dis2T(t_idx);
        }
    }

    //LTLF guard
    cal_guardPath();

    //计算各BA库所到可接受状态库所的距离；
    cal_q2acceptQ();

    //计算各BA库所的前进输出变迁
    cal_q2ProgressQT();

//    //test
//    auto map=dis2T.begin()->second;
//    for(auto i=map.begin();i!=map.end();i++){
//        cout<<i->first<<" : "<<i->second<<endl;
//    }
}

void Heuristic::GetLTL(ofstream &out, TiXmlElement *p) {
    string value = p->Value();
    if (value == "all-paths") {
        GetLTL(out, p->FirstChildElement());
    } else if (value == "globally") {
        out << "G(";
        GetLTL(out, p->FirstChildElement());
        out << ")";
    } else if (value == "finally") {
        out << "F(";
        GetLTL(out, p->FirstChildElement());
        out << ")";
    } else if (value == "next") {
        out << "X(";
        GetLTL(out, p->FirstChildElement());
        out << ")";
    } else if (value == "negation") {
        out << "!(";
        GetLTL(out, p->FirstChildElement());
        out << ")";
    } else if (value == "conjunction" || value == "disjunction") {
        TiXmlElement *m, *n;
        m = p->FirstChildElement();
        GetLTL(out, m);
        m = m->NextSiblingElement();
        while (m) {
            if (value == "conjunction")
                out << "&&";
            else
                out << "||";
            GetLTL(out, m);

            m = m->NextSiblingElement();
        }
    } else if (value == "until") {
        TiXmlElement *m, *n;
        m = p->FirstChildElement();
        n = m->NextSiblingElement();
        string mValue = m->Value();
        string nValue = n->Value();
        if (mValue != "before") {
            cerr << "Error in XML file! The element until\'s first child is not before!" << endl;
            exit(-1);
        }
        if (nValue != "reach") {
            cerr << "Error in XML file! The element until\'s second child is not reach!" << endl;
            exit(-1);
        }
        out << "(";
        GetLTL(out, m->FirstChildElement());
        out << ")U(";
        GetLTL(out, n->FirstChildElement());
        out << ")";
    } else if (value == "less" || value == "equality" || value == "lesseq") {
        TiXmlElement *m, *n;
        m = p->FirstChildElement();
        n = m->NextSiblingElement();
        string mValue = m->Value();
        string nValue = n->Value();
        out << "{";
        if (mValue == "token-value") {
            out << "token-value(";
            TiXmlElement *left = m->FirstChildElement();
            while (left != NULL) {
                out << left->GetText();
                string temp = left->GetText();
                P.push_back(temp);
                if (temp == "")
                    break;
                left = left->NextSiblingElement();
            }
            out << ")";
        } else if (mValue == "int-constant"
                   || mValue == "real-constant"
                   || mValue == "string-constant") {
            out << m->GetText();
        } else {
            cerr << "Error in XML file about the element integer-le!" << endl;
            exit(-1);
        }
        if (value == "lesseq")
            out << "<=";
        else if (value == "less")
            out << "<";
        if (value == "equality")
            out << "==";
        if (nValue == "token-value") {
            out << "token-value(";
            TiXmlElement *right = n->FirstChildElement();
            while (right != NULL) {
                out << right->GetText();
                string temp = right->GetText();
                P.push_back(temp);
                right = right->NextSiblingElement();
            }
            out << ")";
        } else if (nValue == "int-constant"
                   || nValue == "real-constant"
                   || nValue == "string-constant") {
            out << n->GetText();
        } else {
            cerr << "Error in XML file about the element integer-le!" << endl;
            exit(-1);
        }
        out << "}";
    } else if (value == "is-fireable") {
        TiXmlElement *m;
        m = p->FirstChildElement();
        out << "is-fireable{";
        while (m != NULL) {
            string Value = m->Value();

            if (Value == "transition") {
                string temp = m->GetText();
                T.push_back(temp);
                out << m->GetText() << ",";
            } else {
                cerr << "Error in XML file! One of the element is-firability\'s child is not transition!" << endl;
                exit(-1);
            }
            m = m->NextSiblingElement();
        }
        out << "}";
    }
}

void Heuristic::extract_targetPT(int number) {

    TiXmlDocument *doc;
    string file;
    if (LTLtype == LTLC)
        file = LTLCfile;
    else if (LTLtype == LTLF)
        file = LTLFfile;
    else if (LTLtype == LTLV)
        file = LTLVfile;
    doc = new TiXmlDocument(file.c_str());
    if (!doc->LoadFile()) {
        cerr << doc->ErrorDesc() << endl;
    }
    file = "result.txt";
    ofstream out(file.c_str(), ios::out | ios::app);

    TiXmlElement *root = doc->RootElement();
    if (root == NULL) {
        cerr << "Failed to load file: no root element!" << endl;
        doc->Clear();
    }

    TiXmlElement *p = root->FirstChildElement();
    for (int i = 1; i < number; ++i) {
        p = p->NextSiblingElement();
    }

    TiXmlElement *id = p->FirstChildElement("id");

    TiXmlElement *formula = p->FirstChildElement("formula");


    GetLTL(out, formula->FirstChildElement());

    out.close();
}

void Heuristic::cal_dis2T(index_t t_target) {
    vector<type_T2Call> searched;
    map<index_t, NUM_t> map_dis;//T->dis

    for (auto i = cpn->map_t_call2thread.begin(); i != cpn->map_t_call2thread.end(); i++) {
        if (i->first.first == t_target)
            searched.emplace_back(i->first);
    }

    for (unsigned int i = 0; i < searched.size(); i++) {
        auto t_pa_idx = searched[i].first;
        CTransition *tran = cpn->findT_byindex(t_pa_idx);
        bool control_permit = true;//路径到达函数入口或线程创建处
        for (unsigned int j = 0; j < tran->get_producer().size(); j++) {
            CPlace *place = &cpn->getplacearr()[tran->get_producer()[j].idx];
            for (unsigned int k = 0; k < place->get_producer().size(); k++) {
                auto arcType = place->get_producer()[k].arcType;
                if (arcType == executed || arcType == call_exit) {
                    auto callStk = searched[i].second;
                    auto t_idx = place->get_producer()[k].idx;
                    auto exitT = cpn->map_call_exitT2enterT.find(tran->getid());
                    if (exitT != cpn->map_call_exitT2enterT.end()) {
                        auto enterT = cpn->map_call_exitT2enterT.find(tran->getid())->second;
                        vector<string> enterTs;
                        enterTs.emplace_back(enterT);
                        callStk.emplace_back(enterTs);
                    }
                    if (update_dis(map_dis, t_idx, t_pa_idx)) {
                        searched.emplace_back(t_idx, callStk);
                        control_permit = false;
                    }
                }
            }
        }
        if (control_permit) {
            for (unsigned int j = 0; j < tran->get_producer().size(); j++) {
                CPlace *place = &cpn->getplacearr()[tran->get_producer()[j].idx];
                for (unsigned int k = 0; k < place->get_producer().size(); k++) {
                    auto arcType = place->get_producer()[k].arcType;
                    auto t_idx = place->get_producer()[k].idx;
                    auto t_id = cpn->findT_byindex(t_idx)->getid();
                    if ((arcType == control || arcType == call_enter) && exist_in(searched[i].second.back(), t_id)) {
                        auto callStk = searched[i].second;//用于匹配调用变迁的callStack
                        callStk.pop_back();
                        //若callStk为空，则是从线程跳出到main函数（或另一线程）。
                        if(callStk.empty()){
                            for(auto i_tCall=cpn->map_t_call2thread.begin();i_tCall!=cpn->map_t_call2thread.end();i_tCall++){
                                if(i_tCall->first.first==t_idx)
                                    callStk=i_tCall->first.second;
                            }
                        }
                        if (auto itCall = cpn->map_t_call2thread.find(make_pair(t_idx, callStk)) !=
                                          cpn->map_t_call2thread.end())
                            if (update_dis(map_dis, t_idx, t_pa_idx)) {
                                searched.emplace_back(t_idx, callStk);
                            }
                    }
                }
            }
        }
    }
    dis2T.emplace(t_target, map_dis);
}

bool Heuristic::update_dis(map<index_t, NUM_t> &map_dis, index_t t_idx, index_t t_pa) {
    unsigned int pa_dis = map_dis.find(t_pa)->second;
    auto idis = map_dis.find(t_idx);
    if (idis == map_dis.end()) {
        map_dis.emplace(t_idx, pa_dis + 1);
        return true;
    } else if (idis->second > pa_dis + 1) {
        idis->second = pa_dis + 1;
        return true;
    }
    return false;
}

void Heuristic::cal_q2ProgressQT() {
    for (auto i = synch->q_begin_idx; i < synch->q_begin_idx + synch->q_count; i++) {
        auto q = cpn->findP_byindex(i);
        NUM_t dis_q=map_dis2acceptQ.find(i)->second;
        vector<pair<index_t, index_t>> qProgressPT;
        for (auto j = q->get_consumer().begin(); j != q->get_consumer().end(); j++) {
            auto t_con = cpn->findT_byindex(j->idx);
            auto p_con_idx = t_con->get_consumer().front().idx;
            NUM_t dis_con=map_dis2acceptQ.find(p_con_idx)->second;
            if(dis_con<dis_q)
                qProgressPT.emplace_back(p_con_idx, j->idx);
        }
        if (!qProgressPT.empty()) {
            map_q2ProgressQT.emplace(i, qProgressPT);
        }
    }
}

void Heuristic::cal_q2acceptQ() {
    for (auto i = synch->state_accepted.begin(); i != synch->state_accepted.end(); i++) {
        vector<index_t> searched;
        map<index_t, NUM_t> map_dis;//T->dis

        auto t_idx = cpn->findP_byid(*i)->get_producer().front().idx;
        searched.emplace_back(t_idx);
        map_dis.emplace(t_idx, 1);

        for (unsigned int i = 0; i < searched.size(); i++) {
            auto t_pa_idx = searched[i];
            CTransition *tran = cpn->findT_byindex(t_pa_idx);
//            bool control_permit = true;//路径到达函数入口或线程创建处
            for (unsigned int j = 0; j < tran->get_producer().size(); j++) {
                if (tran->get_producer()[j].idx < synch->q_begin_idx)
                    continue;

                CPlace *place = &cpn->getplacearr()[tran->get_producer()[j].idx];
                for (unsigned int k = 0; k < place->get_producer().size(); k++) {
                    if (place->get_producer()[k].idx < synch->ba_t_begin_idx)
                        continue;

                    auto arcType = place->get_producer()[k].arcType;
                    if (arcType == control) {
//                        auto callStk = searched[i].second;
                        auto t_idx = place->get_producer()[k].idx;
//                        auto exitT=cpn->map_call_exitT2enterT.find(tran->getid());
//                        if(exitT!=cpn->map_call_exitT2enterT.end()) {
//                            auto enterT = cpn->map_call_exitT2enterT.find(tran->getid())->second;
//                            vector<string> enterTs;
//                            enterTs.emplace_back(enterT);
////                            callStk.emplace_back(enterTs);
//                        }
                        if (update_dis(map_dis, t_idx, t_pa_idx)) {
                            searched.emplace_back(t_idx);
//                            control_permit = false;
                        }
                    }
                }
            }
        }
        for (auto j = map_dis.begin(); j != map_dis.end(); j++) {
            auto t_idx = j->first;
            auto p_pro_idx = cpn->findT_byindex(t_idx)->get_producer().front().idx;
            map_dis2acceptQ.emplace(p_pro_idx, j->second);
        }
    }
}

index_t Heuristic::get_nextE(vector<PE *> en_PE, Config *C, UNFPDN *unfpdn) {
    index_t e;
    NUM_t dis;
    bool firstE = true;
    for (auto i = en_PE.begin(); i != en_PE.end(); i++) {
        //I变迁优先发生
        auto pe=*i;
        pe_next=*i;
        auto t_idx = pe->t_idx;
//        if(t_idx>synch->ba_t_begin_idx)
//            return *i;
        if (synch->I_idx.find(t_idx) != synch->I_idx.end())
            return pe->e_idx;

        //获得PN下一状态C_plus
        C_plus = get_Config_add(C, *i, unfpdn);

        //对BA每个前进状态，求disBA和disPN相乘的最小值
        NUM_t disMinQ;
        bool firstQ = true;
        auto q_next = map_q2ProgressQT.find(C->q_cur)->second;
        for (auto j = q_next.begin(); j != q_next.end(); j++) {
            is_directWriteGuard=false;

            NUM_t disPN = 1;
            auto t_id=cpn->findT_byindex(j->second)->getid();
            auto iguard = synch->map_t2guard.find(t_id);
            if (iguard == synch->map_t2guard.end()) {
                disPN = 1;
            } else {
                auto guard = iguard->second;
                for (auto ig = guard.begin(); ig != guard.end(); ig++) {
                    switch (synch->judgeF(*ig)) {
                        case LTLF: {
                            disPN += dis2LTLF(*ig);
                            break;
                        }
                        case LTLV: {
                            disPN += dis2LTLV(*ig);
                            break;
                        }
                    }
                }
            }
            auto disBA = get_dis2acceptQ(j->first)+1;
            if (firstQ) {
                disMinQ = disBA * disPN;
                firstQ = false;
            } else {
                disMinQ = disMinQ < disBA * disPN ? disMinQ : disBA * disPN;
            }
        }
        if (firstE) {
            e = pe->e_idx;
            dis = disMinQ;
            firstE = false;
        } else {
            if (dis <= disMinQ)
                continue;
//            else if (dis == disMinQ) {
//                if (e < pe->e_idx)
//                    e = pe->e_idx;
//            }
            else {
                e = pe->e_idx;
                dis = disMinQ;
            }
        }
    }
    return e;
}

Config *Heuristic::get_Config_add(Config *C, PE *pe, UNFPDN *unfpdn) {
    Config *config = new Config;
    auto e = unfpdn->event[pe->e_idx];
    *config = *C;
    config->e_last = pe->e_idx;
    if (config->e_Vec.size() <= pe->e_idx) {
        for (auto i = config->e_Vec.size(); i < pe->e_idx; i++)
            config->e_Vec.emplace_back(false);
        config->e_Vec.emplace_back(true);
    } else {
        config->e_Vec[pe->e_idx] = true;
    }
    config->fired_T.emplace(e.cpn_index);
    auto t_idx = unfpdn->event[pe->e_idx].cpn_index;
    if (t_idx >= synch->ba_t_begin_idx && t_idx < synch->ba_t_begin_idx + synch->ba_t_count) {
        if (synch->I_idx.find(t_idx) != synch->I_idx.end())
            config->is_accept = true;
        else config->is_accept = false;
    }
    //context
    //配置集只需要cut，不用conf_pre和conf_con
    config->cut = new CUT(cpn);
    *config->cut = *C->cut;
    for (auto i = e.consumer.begin(); i != e.consumer.end(); i++) {
        auto p_idx = unfpdn->condition[*i].cpn_index;
        auto token = unfpdn->condition[*i].multiSet.getonlytoken();
        config->cut->add_condition(*i, p_idx, token);
        //heuristic
        if (p_idx >= synch->q_begin_idx && p_idx < synch->q_begin_idx + synch->q_count) {
            C->q_cur = p_idx;
        }
    }
    for (auto i = e.producer.begin(); i != e.producer.end(); i++) {
        auto p_idx = unfpdn->condition[*i].cpn_index;
        auto token = unfpdn->condition[*i].multiSet.getonlytoken();
        config->cut->sub_condition(*i, p_idx, token);
    }

    return config;
}

NUM_t Heuristic::get_dis2acceptQ(index_t q_cur) {
    auto iter = map_dis2acceptQ.find(q_cur);
    return iter->second;
}

NUM_t disOfcolor(const bucket &bkt1, const bucket &bkt2) {
    if (bkt1.icolor != MAXINT && bkt2.icolor != MAXINT) {
        return abs(bkt1.icolor - bkt2.icolor);
    }
    cerr << "color type match error" << endl;
    exit(0);
}

NUM_t Heuristic::dis2LTLF(string s) {
    NUM_t dis=0;
    if (s[0] == '!') //前面带有'!'的is-fireable{}
    {
        /*!{t1 || t2 || t3}：
         * true：t1不可发生 并且 t2不可发生 并且 t3不可发生
         * false： 只要有一个能发生
         * */
         dis=DefaultDis2G;
        s = s.substr(2, s.length() - 2); //去掉“!{”
        while (1) {
            NUM_t dis2t = DefaultDis2G;
            int pos = s.find_first_of(",");
            if (pos < 0)
                break;
            string subs = s.substr(0, pos);            //取出一个变迁
            auto idx = cpn->mapTransition.find(subs)->second;
            T_target=idx;
            //guard
            dis-= get_dis2guard(idx);
            //dis2T
            for (auto i = C_plus->fired_T.begin(); i != C_plus->fired_T.end(); i++) {
                auto map = dis2T.find(idx)->second;
                auto imap = map.find(*i);
                if (imap != map.end()) {
                    auto dis2t_tmp=imap->second ;
                    dis2t = dis2t_tmp < dis2t ? dis2t_tmp : dis2t;
                }
            }
//            DefaultBAstep=max(DefaultBAstep,dis2t*10);
            dis-=dis2t;

            s = s.substr(pos + 1, s.length() - pos);
        }
        return dis;

    } else {         //单纯的is-fireable{}原子命题
        /*{t1 || t2 || t3}:
	     * true: 只要有一个能发生
	     * false: 都不能发生
	     * */
        s = s.substr(1, s.length() - 1);//去掉‘{’

        while (1) {
            NUM_t dis2t=DefaultDis2G;
            int pos = s.find_first_of(",");
            if (pos < 0)
                break;
            string subs = s.substr(0, pos);  //取出一个变迁

            auto idx = cpn->mapTransition.find(subs)->second;
            T_target=idx;
            //guard
            dis+= get_dis2guard(idx);
            //dis2T
            for (auto i = C_plus->fired_T.begin(); i != C_plus->fired_T.end(); i++) {
                auto map = dis2T.find(idx)->second;
                auto imap = map.find(*i);
                if (imap != map.end()) {
                    auto dis2t_tmp=imap->second ;
                    dis2t = dis2t_tmp < dis2t ? dis2t_tmp : dis2t;
                }
            }
//            DefaultBAstep=max(DefaultBAstep,dis2t*10);
            dis+=dis2t;

            s = s.substr(pos + 1, s.length() - pos);
        }
        return dis;
    }
}

void Heuristic::dis2CTN_Token(condition_tree_node *root,NUM_t &dis) {
    if (root == NULL)
        return ;

    if (root->node_type == Tuple) {
        dis=0;
        token singletk;
        dis2CTN_SingleColor(root->left, singletk,dis);
//        tks.emplace_back(singletk);
        if (root->right) {
            if (root->right->node_type == Tuple) {
                NUM_t rdis;
                dis2CTN_Token(root->right,rdis);
                dis+=rdis;
            }
            else {
                token singletk1;
                NUM_t rdis;
                dis2CTN_SingleColor(root->right, singletk1,rdis);
                dis+=rdis;
//                tks.emplace_back(singletk1);
            }
        } else
            throw "Tuple must have right child!";
    } else if (root->node_type == CaseOperator) {
        token condtk;
        dis=0;
        dis2CTN_SingleColor(root->condition, condtk,dis);
        Bucket bkt;
        condtk->getcolor(bkt);
        if (bkt.integer != 0) {
            NUM_t ldis;
            dis2CTN_Token(root->left,ldis);
            dis +=ldis;
        }
        else {
            if (root->right) {
                NUM_t rdis;
                dis2CTN_Token(root->right,rdis);
                dis +=rdis;
            }
            else;
//                throw "ERROR in CaseOperator! don't match any condition!";
        }
    } else {
        token singletk;
        dis=0;
        dis2CTN_SingleColor(root, singletk,dis);
//        tks.emplace_back(singletk);
    }
    return;
}

Bucket operate(const Bucket &lbk,const Bucket &rbk,string Operator,NUM_t dis){
    if(lbk.tid != rbk.tid)
        throw "ERROR in Bucket booloperate!";
    Bucket bkt;
    switch(lbk.tid){
        case Integer:
            bkt.tid = Integer;
            bkt.integer = operate(lbk.integer,rbk.integer,Operator);
            break;
        case Real:
            bkt.tid = Real;
            bkt.real = operate(lbk.real,rbk.real,Operator);
            break;
        case String:
            bkt.tid = String;
            bkt.str = operate(lbk.str,rbk.str,Operator);
            break;
        case dot:
            throw "'operate' can't operate dotsort!";
        case productsort:
            throw "'operate' can't operate productsort!";
    }
    return bkt;
}

template<class T>
bool booloperate(T s1,T s2,string Operator,NUM_t &dis,NUM_t &ldis,NUM_t &rdis) {
    T v1, v2;
    v1 = s1;
    v2 = s2;
    if (Operator == ">") {
        if(v1>v2)
            dis=0;
        else dis=disOfV(s1,s2)+1;
        return v1 > v2;
    }
    else if (Operator == ">=") {
        if(v1>=v2)
            dis=0;
        else dis=disOfV(s1,s2);
        return v1 >= v2;
    }
    else if (Operator == "<") {
        if(v1<v2)
            dis=0;
        else dis=disOfV(s1,s2)+1;
        return v1 < v2;
    }
    else if (Operator == "<=") {
        if(v1<=v2)
            dis=0;
        else dis=disOfV(s1,s2);
        return v1 <= v2;
    }
    else if (Operator == "==") {
        if(v1==v2)
            dis=0;
        else dis=disOfV(s1,s2);
        return v1 == v2;
    }
    else if (Operator == "!=") {
        if(v1!=v2)
            dis=0;
        else dis=1;
        return v1 != v2;
    }
    else if (Operator == "||") {
        if(v1 || v2)
            dis=0;
        else dis= min(ldis,rdis);
        return v1 || v2;
    }
    else if (Operator == "&&") {
        if(v1 && v2)
            dis =0;
        else
            dis=ldis+rdis;
        return v1 && v2;
    }
}

bool booloperate(string s1,string s2,string Operator,NUM_t &dis){
    string v1=s1,v2=s2;
    if (Operator == ">") {
        if(v1 > v2)
            dis=0;
        else dis=1;
        return v1 > v2;
    }
    else if (Operator == ">=") {
        if(v1 >= v2)
            dis=0;
        else dis=1;
        return v1 >= v2;
    }
    else if (Operator == "<") {
        if(v1 < v2)
            dis=0;
        else dis=1;
        return v1 < v2;
    }
    else if (Operator == "<=") {
        if(v1 <= v2)
            dis=0;
        else dis=1;
        return v1 <= v2;
    }
    else if (Operator == "==") {
        if(v1 == v2)
            dis=0;
        else dis=1;
        return v1 == v2;
    }
    else if (Operator == "!=") {
        if(v1 != v2)
            dis=0;
        else dis=1;
        return v1 != v2;
    }
}

bool booloperate(const Bucket &lbk, const Bucket &rbk, string Operator,NUM_t &dis,NUM_t &ldis,NUM_t &rdis) {
    if (lbk.tid != rbk.tid)
        throw "ERROR in Bucket booloperate!";
    switch (lbk.tid) {
        case Integer:
            return booloperate(lbk.integer, rbk.integer, Operator,dis,ldis,rdis);
        case Real:
            return booloperate(lbk.real, rbk.real, Operator,dis,ldis,rdis);
        case String:
            return booloperate(lbk.str, rbk.str, Operator,dis);
        case dot:
            throw "'booloperate' can't operate Dotsort!";
        case productsort:
            throw "'booloperate' can't operate productsort!";
    }
}

void Heuristic::dis2CTN_SingleColor(condition_tree_node *root, token &tk,NUM_t &dis) {
    dis=0;
    if (root == NULL)
        return;

    if (root->node_type == Operator) {
        string Op = root->node_name;
        if (is_BoolOperator(Op)) {
            token ltk, rtk;
            NUM_t ldis,rdis;
            dis2CTN_SingleColor(root->left, ltk,ldis);
            dis2CTN_SingleColor(root->right, rtk,rdis);
            Bucket lbk, rbk, finalbkt;
            ltk->getcolor(lbk);
            rtk->getcolor(rbk);
            tk = (token) (new IntegerSortValue);

            finalbkt.tid = Integer;
            if (booloperate(lbk, rbk, Op,dis,ldis,rdis)) {
//                dis2G+=dis;
                finalbkt.integer = 1;
            }
            else {
//                dis2G+=dis;
                finalbkt.integer = 0;
            }
            tk->setcolor(finalbkt);
        } else {
            if (is_unary(Op)) {
                token ltk;
                NUM_t ldis;//没用
                dis2CTN_SingleColor(root->left, ltk,ldis);
                Bucket lbk, finalbkt;
                ltk->getcolor(lbk);
                finalbkt = operate(lbk, Op);
                switch (lbk.tid) {
                    case Integer:
                        tk = (token) (new IntegerSortValue);
                        break;
                    case Real:
                        tk = (token) (new RealSortValue);
                        break;
                    case String:
                        tk = (token) (new StringSortValue);
                        break;
                }
                tk->setcolor(finalbkt);
            } else {
                token ltk, rtk;
                NUM_t ldis,rdis;
                dis2CTN_SingleColor(root->left, ltk,ldis);
                dis2CTN_SingleColor(root->right, rtk,rdis);
                Bucket lbk, rbk, finalbkt;
                ltk->getcolor(lbk);
                rtk->getcolor(rbk);
                finalbkt = operate(lbk, rbk, Op);
                switch (lbk.tid) {
                    case Integer:
                        tk = (token) (new IntegerSortValue);
                        break;
                    case Real:
                        tk = (token) (new RealSortValue);
                        break;
                    case String:
                        tk = (token) (new StringSortValue);
                        break;
                }
                tk->setcolor(finalbkt);
            }
        }
    } else if (root->node_type == color) {
        string value = root->value;
        auto consttype = JudgeConstType(value);
        Bucket bkt;
        bkt.tid = consttype;
        switch (consttype) {
            case Integer:
                tk = (token) (new IntegerSortValue);
                bkt.integer = atoi(value.c_str());
                break;
            case Real:
                tk = (token) (new RealSortValue);
                bkt.real = atof(value.c_str());
                break;
            case String:
                tk = (token) (new StringSortValue);
                bkt.str = value;
                break;
        }
        tk->setcolor(bkt);
    } else if (root->node_type == variable) {
        string vname = root->node_name;
        if (vname == "pthread_create_v" || vname == "pthread_join_v") {
            //特殊处理pthread_create_v pthread_join_v
            Bucket bkt;
            bkt.tid = Integer;
            bkt.integer = 0;
            tk = (token) (new IntegerSortValue);
            tk->setcolor(bkt);
            return;
        }
        Bucket bkt;
        is_DWG(vname);
        auto iter = cpn->mapVariable.find(vname);
        if (iter == cpn->mapVariable.end())
            throw "invalid CPN variable!";
        else {
            cpn->get_vartable()[iter->second].getvcolor(bkt);
            switch (cpn->get_vartable()[iter->second].gettid()) {
                case Integer:
                    tk = (token) (new IntegerSortValue);
//                    bkt.integer = atoi(value.c_str());
                    break;
                case Real:
                    tk = (token) (new RealSortValue);
//                    bkt.real = atof(value.c_str());
                    break;
                case String:
                    tk = (token) (new StringSortValue);
//                    bkt.str = value;
                    break;
            }
            tk->setcolor(bkt);
        }
    } else if (root->node_type == CaseOperator) {
        token condtk;
        NUM_t ldis,rdis;
        dis2CTN_SingleColor(root->condition, condtk,dis);
        Bucket bkt;
        condtk->getcolor(bkt);
        if (bkt.integer != 0) {
            dis2CTN_SingleColor(root->left, tk, ldis);
            dis=min(dis,ldis);
        }
        else {
            if (root->right) {
                dis2CTN_SingleColor(root->right, tk, rdis);
                dis=min(dis,rdis);
            }
            else
                throw "ERROR in CaseOperator! don't match any condition!";
        }
    }
    return;
}


void Heuristic::dis2CTN(condition_tree_node *root,NUM_t &dis) {
    if (root == NULL)
        return;
    NUM_t ldis=0, rdis=0;
    if (root->node_type == TokenOperator) {
        dis=0;
        dis2CTN(root->left,ldis);
        dis2CTN(root->right,rdis);

        if (root->node_name == "++") {
            dis += ldis;
            dis += rdis;
        } else if (root->node_name == "--") {
            dis += ldis;
            if (ldis < rdis)
                throw "CTN2MS error in --";
            else
                dis -= rdis;
        } else
            throw "ERROR!We just support '++' and '--' for now!";
    } else if (root->node_type == Token) {
//        vector<token> tokens;
        dis2CTN_Token(root->left,dis);
//        if (tokens.size() != 1)
//            throw "CTN2MS ERROR! basic type don't match!";
//
//        Bucket bkt;
//        tokens[0]->getcolor(bkt);
//        ms.generateFromToken(tokens[0]);
//        ms.setTokenCount(root->num);
    } else if (root->node_type == CaseOperator) {
        dis=-1;
        token condtk;
        dis2CTN_SingleColor(root->condition, condtk,dis);
        Bucket bkt;
        condtk->getcolor(bkt);
        if (bkt.integer != 0) {
            dis2CTN(root->left,ldis);
            dis= min(dis,ldis);
        } else {
            if (root->right) {
                dis2CTN(root->right,rdis);
                dis= min(dis,rdis);
            } else;
//                throw "ERROR in CaseOperator! don't match any condition!";
        }
    }
    return ;
}

void Heuristic::cal_guardPath() {
    for(auto it=dis2T.begin();it!=dis2T.end();it++){

        vector<pair<index_t,NUM_t>> guardPath_sort;//t->dis
        vector<index_t> guardPath;
        auto map_dis=it->second;
        for(auto i=map_dis.begin();i!=map_dis.end();i++){
            if(!cpn->findT_byindex(i->first)->get_hasguard())
                continue;
            //按dis排序,降序
            auto ipath=guardPath_sort.begin();
            for(;ipath!=guardPath_sort.end();ipath++){
                if(ipath->second<i->second)
                    break;
            }
            guardPath_sort.emplace(ipath,*i);
        }
        //row相同变迁作为一组
        vector<pair<NUM_t ,vector<index_t>>> guardPath_merge;
        for(auto i=guardPath_sort.begin();i!=guardPath_sort.end();i++){
            auto t=cpn->findT_byindex(i->first);
            auto iter=cpn->map_NoneRow.find(t->getid());
            auto t_row=cpn->getRow_unf(i->first);
            auto j=guardPath_merge.begin();
            for(;j!=guardPath_merge.end();j++){
                if(j->first==t_row){
                    j->second.emplace_back(i->first);
                    break;
                }
            }
            if(j==guardPath_merge.end()){
                vector<index_t> vec_t;
                vec_t.emplace_back(i->first);
                guardPath_merge.emplace_back(t_row,vec_t);
            }
        }

        for(auto i=guardPath_merge.begin();i!=guardPath_merge.end();i++){
            if(i->second.size()>1)
                continue;
            guardPath.emplace_back(i->second.front());
        }
        if(!guardPath.empty())
            map_guardPath.emplace(it->first,guardPath);
    }
}

NUM_t Heuristic::get_dis2guard(index_t t_idx) {
    auto iguardPath=map_guardPath.find(t_idx);
    if(iguardPath==map_guardPath.end())
        return 0;
    auto guardPath=&iguardPath->second;
    //guard可能与多个变迁相关，如if、else对应同一行guard，！guard；
    NUM_t dis;
    auto t = cpn->findT_byindex(guardPath->front());
    auto binding=get_bindingF(t,C_plus->cut);
    BindingVariable(binding,cpn);

    dis = dis2guard(t->get_guard());
    //guardpath中第一个变迁发生后，后续开始计算第二个变迁；
    if(pe_next->t_idx==guardPath->front()){
        guardPath->erase(guardPath->begin());
        if(guardPath->empty())
            map_guardPath.erase(iguardPath);
    }
    else {
        //guard不满足，dis值增大，避免该线程继续探索，优先查找可能使guard满足的路径。
        auto row1 = cpn->getRow_unf(pe_next->t_idx);
        auto row2 = cpn->getRow_unf(guardPath->front());
        if (row1 == row2)
            dis+=DefaultGDelay;
    }
    if(is_directWriteGuard)
        dis=0;

    return dis;
}

NUM_t Heuristic::dis2guard(const condition_tree &guard) {
    if (guard.getexp() == "NULL") {
        return 1;
    }
    NUM_t dis;
    //tid == Integer,sid == 0
    dis2CTN(guard.getroot(),dis);
    DefaultGDelay= max(DefaultGDelay,dis*10);
//    if(dis*10 >DefaultTstep)
//        DefaultTstep=max(DefaultTstep,dis*10);
//    DefaultBAstep=max(DefaultBAstep,dis*100);
    return dis;
}

NUM_t Heuristic::dis2LTLV(string s) {
    int pos = s.find("==");
    if (pos != string::npos) {
        //equality
        if (s[0] == '!') {
            /*!(front == latter)
             * true:front != latter
             * false:front == latter
             * */
            s = s.substr(2, s.length() - 3);   //去除"!{}"
            int p = s.find_first_of("==");
            string front = s.substr(0, p);
            string latter = s.substr(p + 2);
            bucket front_color, latter_color;
            if (!FetchColor(front, front_color))
                return -1;
            if (!FetchColor(latter, latter_color))
                return -1;
            return 100 - disOfcolor(front_color, latter_color);
        } else {
            /*(front == latter)
             * true:front == latter
             * false:front != latter
             * */
            s = s.substr(1, s.length() - 2);   //去除"{}"
            int p = s.find_first_of("==");
            string front = s.substr(0, p);
            string latter = s.substr(p + 2);
            bucket front_color, latter_color;
            if (!FetchColor(front, front_color))
                return -1;
            if (!FetchColor(latter, latter_color))
                return -1;
            return disOfcolor(front_color, latter_color);
        }
    }

    pos = s.find("<=");
    if (pos != string::npos) {
        if (s[0] == '!') {
            /*!(front <= latter)
             * true:front !<= latter
             * false:front <= latter
             * */
            s = s.substr(2, s.length() - 3);   //去除"!{}"
            int p = s.find_first_of("<=");
            string front = s.substr(0, p);
            string latter = s.substr(p + 2);
            bucket front_color, latter_color;
            if (!FetchColor(front, front_color))
                return -1;
            if (!FetchColor(latter, latter_color))
                return -1;
            return disOfcolor(front_color, latter_color);
        } else {
            /*(front <= latter)
             * true:front <= latter
             * false:front !<= latter
             * */
            s = s.substr(1, s.length() - 2);   //去除"{}"
            int p = s.find_first_of("<=");
            string front = s.substr(0, p);
            string latter = s.substr(p + 2);
            bucket front_color, latter_color;
            if (!FetchColor(front, front_color))
                return -1;
            if (!FetchColor(latter, latter_color))
                return -1;
            return disOfcolor(front_color, latter_color);
        }
    }

    if (s[0] == '!') {
        /*!(front < latter)
         * true:front !< latter
         * false:front < latter
         * */
        s = s.substr(2, s.length() - 3);   //去除"!{}"
        int p = s.find_first_of("<");
        string front = s.substr(0, p);
        string latter = s.substr(p + 1);
        bucket front_color, latter_color;
        if (!FetchColor(front, front_color))
            return -1;
        if (!FetchColor(latter, latter_color))
            return -1;
        return disOfcolor(front_color, latter_color);
    } else {
        /*(front < latter)
         * true:front < latter
         * false:front !< latter
         * */
        s = s.substr(1, s.length() - 2);   //去除"{}"
        int p = s.find_first_of("<");
        string front = s.substr(0, p);
        string latter = s.substr(p + 1);
        bucket front_color, latter_color;
        if (!FetchColor(front, front_color))
            return -1;
        if (!FetchColor(latter, latter_color))
            return -1;
        return disOfcolor(front_color, latter_color);
    }
}

bool Heuristic::FetchColor(string s, bucket &color) {
    if (s[0] == 't') {
        auto cpn = synch->cpn_synch;
        string value = s.substr(12, s.length() - 13);
        auto valueVec = split(value, "#");
        if (valueVec.size() != 3)
            throw "token-value转换为字符串后的内容应该由三部分组成!";
        string P_name = valueVec[0], index = valueVec[1], thread = valueVec[2];
        auto iter = cpn->mapPlace.find(P_name);
        if (iter == cpn->mapPlace.end())
            throw "找不到库所" + P_name + " in product.cpp!";
        CPlace &pp = cpn->getplacearr()[iter->second];
//        auto tks = state->marking.mss[iter->second].getmapTokens();
        auto tks = C_plus->cut->cuttable[iter->second].maptoken;
        auto tkiter = tks.begin();
        while (tkiter != tks.end()) {
//            auto tmp_tk = tkiter->first;
            auto tmp_tk = tkiter->second;
            SORTID sid = tmp_tk->getsid();
            auto ps = sorttable.find_productsort(sid);
            if (index == "NULL" ^ ps.get_hasindex() && thread == "NULL" ^ ps.get_hastid()) {
                if (index == "NULL" && thread == "NULL") {
                    //全局普通变量
                    if (tks.size() != 1)
                        throw "FetchColor时全局普通变量出现异常：token数大于1";
                    Bucket tmp_bkt, value_bkt;
                    tmp_tk->getcolor(tmp_bkt);
                    tmp_bkt.getVarValue(value_bkt);
                    color.generateFromBucket(value_bkt);
                    return true;
                } else if (index != "NULL" && thread == "NULL") {
                    //全局数组变量
                    Bucket tmp_bkt, value_bkt, index_bkt;
                    tmp_tk->getcolor(tmp_bkt);
                    tmp_bkt.pro[tmp_bkt.pro.size() - 2]->getcolor(index_bkt);
                    if (index_bkt == atoi(index.c_str())) {
                        tmp_bkt.getVarValue(value_bkt);
                        color.generateFromBucket(value_bkt);
                        return true;
                    }
                } else if (index == "NULL" && thread != "NULL") {
                    //局部普通变量
                    Bucket tmp_bkt, value_bkt, tid_bkt;
                    tmp_tk->getcolor(tmp_bkt);
                    tmp_bkt.pro[tmp_bkt.pro.size() - 1]->getcolor(tid_bkt);
                    if (tid_bkt == thread) {
                        tmp_bkt.getVarValue(value_bkt);
                        color.generateFromBucket(value_bkt);
                        return true;
                    }
                } else if (index != "NULL" && thread != "NULL") {
                    //局部数组变量
                    Bucket tmp_bkt, value_bkt, index_bkt, tid_bkt;
                    tmp_tk->getcolor(tmp_bkt);
                    tmp_bkt.pro[tmp_bkt.pro.size() - 3]->getcolor(index_bkt);
                    tmp_bkt.pro[tmp_bkt.pro.size() - 1]->getcolor(tid_bkt);
                    if (index_bkt == atoi(index.c_str()) && tid_bkt == thread) {
                        tmp_bkt.getVarValue(value_bkt);
                        color.generateFromBucket(value_bkt);
                        return true;
                    }
                }
            } else
                throw "库所" + P_name + "的index或tid和检测性质不匹配!";
            tkiter++;
        }
        return false;
    } else if (s[0] == 'i') {
        string num = s.substr(13, s.length() - 14);
        color.icolor = atoi(num.c_str());
    } else if (s[0] == 'r') {
        string num = s.substr(14, s.length() - 15);
        color.rcolor = atof(num.c_str());
    } else if (s[0] == 's') {
        color.scolor = s.substr(16, s.length() - 17);
    }
    return true;
}

//bool Heuristic::FetchBktV(string v, Config *C, Bucket &bkt) {
//    auto iter=cpn->map_v2place.find(v);
//    if(iter==cpn->map_v2place.end())
//        return false;
//
//    Bucket bkt1;
//    auto p_idx=iter->second;
//    auto p=cpn->findP_byindex(iter->second);
//    auto tokens=C->cut->cuttable[p_idx].maptoken;
//    auto token=tokens.begin();
//    while(token!=tokens.end()){
//        token->second->getcolor(bkt1);
//        bkt1.pro[0]->getcolor(bkt);
//        token++;
//    }
//    //over writer guard
//    auto t_next=cpn->findT_byindex(pe_next->t_idx);
//    for(auto i=t_next->get_consumer().begin();i!=t_next->get_consumer().end();i++){
//        //对同一变量、且写变量
//        if(i->idx!=p_idx || i->arcType==readArc)
//            continue;
//        //在guard变迁的路径上
//        auto idis2T=dis2T.find(T_target)->second;
//        if(idis2T.find(pe_next->t_idx)==idis2T.end())
//            continue;
//        is_directWriteGuard= true;
//    }
//
//    return true;
//}

bool containV(condition_tree_node *root){
    if(root->node_type==variable)
        return true;
    if(root->left!=NULL)
    if(containV(root->left))
        return true;
    if(root->right!=NULL)
    if(containV(root->right))
        return true;
    return false;
}

void Heuristic::cal_overwriteT() {
    auto Pv=cpn->set_varP_normal;
    set<index_t > writeT;
    for(auto i=Pv.begin();i!=Pv.end();i++){
        auto p=cpn->findP_byindex(*i);
        auto p_pro=p->get_producer();
        for(auto j=p_pro.begin();j!=p_pro.end();j++){
            if(j->arcType==readwrite){
                writeT.emplace(j->idx);
            }
        }
    }
    for(auto i=writeT.begin();i!=writeT.end();i++){
        NUM_t count=0;
        bool is_owT= true;
        auto t=cpn->findT_byindex(*i);
        auto t_con=t->get_consumer();
        for(auto j=t_con.begin();j!=t_con.end();j++){
            if(j->arcType==readwrite){
                if(++count>1){
                    is_owT=false;
                    break;
                }
                //left->right有个valuable:=v_id，是固有的,需要跳过。
                if(containV(j->arc_exp.getroot()->left->left)){
                    is_owT= false;
                }
            }
        }
        if(count!=1)
            is_owT=false;
        if(is_owT)
            set_overwriteT.emplace(*i);
    }
}

bool Heuristic::is_DWG(string v) {
    auto iter=cpn->map_v2place.find(v);
    if(iter==cpn->map_v2place.end())
        return false;

    auto p_id=iter->second;
    auto p=cpn->findP_byid(p_id);
    //direct writer guard
    auto t_next=cpn->findT_byindex(pe_next->t_idx);
    for(auto i=t_next->get_consumer().begin();i!=t_next->get_consumer().end();i++){
        auto p_con=cpn->findP_byindex(i->idx);
        //对同一变量
        if(p_con->getid()!=p_id)
            continue;
        //在guard变迁的路径上
        auto idis2T=dis2T.find(T_target)->second;
        if(idis2T.find(pe_next->t_idx)==idis2T.end())
            continue;
        if(set_overwriteT.find(pe_next->t_idx)==set_overwriteT.end())
            continue;
        is_directWriteGuard= true;
        return true;
    }

    return false;
}

Binding_unf *Heuristic::get_bindingF(CTransition *transition, CUT *cut) {
    Binding_unf *binding,*tmpbinding;
    binding = new Binding_unf;
    binding->next = NULL;
    auto T_producer = transition->get_producer();
    for (unsigned int j = 0; j < T_producer.size(); j++) {
        index_t idx = T_producer[j].idx;
        auto pp = cpn->findP_byindex(idx);
        if (!pp->getiscontrolP()) {
            condition_tree_node *root = T_producer[j].arc_exp.getroot();
            while (root) {
                if (root->node_type == CaseOperator && !root->right) {
                    root = root->left;
                    continue;
                }
                if (root->node_type == Token) {

                    tmpbinding = bindingTokenF(root, &cut->cuttable[idx]);
                    if (tmpbinding->next) {
                        Binding_unf *end = tmpbinding->next;
                        while (end->next)
                            end = end->next;
                        end->next = binding->next;
                        binding->next = tmpbinding->next;
                        delete tmpbinding;
                    }
                }
                if (root->left->node_type == Token) {

                    tmpbinding = bindingTokenF(root->left, &cut->cuttable[idx]);
                    if (tmpbinding->next) {
                        Binding_unf *end = tmpbinding->next;
                        while (end->next)
                            end = end->next;
                        end->next = binding->next;
                        binding->next = tmpbinding->next;
                        delete tmpbinding;
                    }
                }
                root = root->right;
            }
        }
    }
    return binding;
}

Binding_unf *Heuristic::bindingTokenF(condition_tree_node *node, CUTINFO *multiset) {
    SORTID sid;
    bool hasindex, hastid;
    Binding_unf *result, *tmpbinding;
    result = new Binding_unf;
    result->next = NULL;

    if (multiset->maptoken.size() == 0)
        return result;

    //binding Integer, for alloc and mutex cond
    if (multiset->tid == Integer) {
        if (node->left->node_name[0] == '_' || isalpha(node->left->node_name[0])) {
            result->next = new Binding_unf;
            result->next->next = NULL;
            result->next->variable = node->left->node_name;
            result->next->value = (token) (new IntegerSortValue);
            Bucket bkt;
            auto token = multiset->getonlytoken();
            token->second->getcolor(bkt);
            result->next->value->setcolor(bkt);
//            color_copy(Integer, 0, tokens->color, result->next->value);
            return result;
        } else
            return result;
    }

    sid = multiset->sid;
    auto ps = sorttable.find_productsort(sid);
    hasindex = ps.get_hasindex();
    hastid = ps.get_hastid();

    int offset = 0;
    token cid;
    cid = (token) (new ProductSortValue(sid));//fpro.generateSortValue(sid);
    Integer_t index;
    condition_tree_node *indexnode, *tidnode;
    indexnode = tidnode = node->left;

    auto tokens = multiset->maptoken;
    if (hasindex && hastid) {
        tidnode = tidnode->right->right;
        while (tidnode->right) {
            indexnode = indexnode->right;
            tidnode = tidnode->right;
            offset++;
        }
        index = atoi(indexnode->left->value.c_str());
        auto token = tokens.begin();
        while (token != tokens.end()) {
            Bucket cid_bkt, tid_bkt, index_bkt;
            token->second->getcolor(cid_bkt);
            cid->setcolor(cid_bkt);
            TID_t sub_tid;
            Integer_t sub_index;
            cid_bkt.pro[offset - 1 + 1]->getcolor(index_bkt);
            cid_bkt.pro[offset - 1 + 3]->getcolor(tid_bkt);
            sub_index = index_bkt.integer;
            sub_tid = tid_bkt.str;
            if (sub_index == index ) {
                tmpbinding = bindingcid_unf(cid_bkt.pro, sid, node);
                Binding_unf *end = tmpbinding;
                while (end->next)
                    end = end->next;
                end->next = result->next;
                result->next = tmpbinding->next;
                delete tmpbinding;
                break;
            }
            token++;
        }
        if (token == tokens.end())
            throw "ERROR!can't binding correctly!";
    } else if (hasindex) {
        tidnode = tidnode->right->right;
        while (tidnode) {
            indexnode = indexnode->right;
            tidnode = tidnode->right;
            offset++;
        }
        index = atoi(indexnode->left->value.c_str());
        auto token = tokens.begin();
        while (token != tokens.end()) {
            Bucket cid_bkt, index_bkt;
            token->second->getcolor(cid_bkt);
            Integer_t sub_index;
            cid_bkt.pro[offset - 1 + 1]->getcolor(index_bkt);
            sub_index = index_bkt.integer;
            if (sub_index == index) {
                tmpbinding = bindingcid_unf(cid_bkt.pro, sid, node);
                Binding_unf *end = tmpbinding;
                while (end->next)
                    end = end->next;
                end->next = result->next;
                result->next = tmpbinding->next;
                delete tmpbinding;
                break;
            }
            token++;
        }
        if (token == tokens.end())
            throw "ERROR!can't binding correctly!";
    } else if (hastid) {
        tidnode = tidnode->right;
        while (tidnode->right) {
            tidnode = tidnode->right;
            offset++;
        }
        auto token = tokens.begin();
        while (token != tokens.end()) {
            Bucket cid_bkt, tid_bkt;
            token->second->getcolor(cid_bkt);
            TID_t sub_tid;
            cid_bkt.pro[offset - 1 + 2]->getcolor(tid_bkt);
            sub_tid = tid_bkt.str;
//            if (sub_tid == tid) {
                tmpbinding = bindingcid_unf(cid_bkt.pro, sid, node);
                Binding_unf *end = tmpbinding;
                while (end->next)
                    end = end->next;
                end->next = result->next;
                result->next = tmpbinding->next;
                delete tmpbinding;
                break;
//            }
            token++;
        }
        if (token == tokens.end())
            throw "ERROR!can't binding correctly!";
    } else {
        tidnode = tidnode->right;
        while (tidnode) {
            tidnode = tidnode->right;
            offset++;
        }
        auto token = tokens.begin();
        while (token != tokens.end()) {
            Bucket cid_bkt;
            token->second->getcolor(cid_bkt);
            {
                tmpbinding = bindingcid_unf(cid_bkt.pro, sid, node);
                Binding_unf *end = tmpbinding;
                while (end->next)
                    end = end->next;
                end->next = result->next;
                result->next = tmpbinding->next;
                delete tmpbinding;
                break;
            }
            token++;
        }
        if (token == tokens.end())
            throw "ERROR!can't binding correctly!";
    }
    //    delete[] cid;
    return result;
}