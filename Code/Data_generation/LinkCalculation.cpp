#include "LinkCalculation.h"

vector<double> LinkCalculation::cal_meet_point(Target &T, huoli &h)
{
    // z
    double dx = T.x - h.x;
    double dy = T.y - h.y;
    double R2 = (dx * dx + dy * dy);
    double t;
    double a = T.v * T.v - h.v * h.v;
    double b = 2 * (dx * T.vx + dy * T.vy);
    double c = R2;
    if (fabs(T.v - h.v) < 1)
    {
        t = R2 / b;
    }
    else
    {
        double delta = (b * b - 4 * a * c);
        if (delta < 0)
            return {0, 0, 0};
        delta = sqrt(delta);
        double t1 = (-b + delta) / 2 / a;
        double t2 = (-b - delta) / 2 / a;
        if (t1 <= 0 && t2 <= 0)
            return {0, 0, 0};
        if ((t1 <= 0 && t2 > 0) || (t1 > 0 && t2 <= 0))
            t = max(t1, t2);
        else
            t = min(t1, t2);
    }
    return {T.x + t * T.vx, T.y + t * T.vy, t};
    /*clear, clc;

    syms t;
    syms Tx Tvx Ty Tvy;
    syms hx hy;
    syms hv;
    % [ T.x + T.vx * t, T.y + T.vy ]
    % [ h.x, h.y ]
    % h.v
    q = (Tx + Tvx * t - hx) ^ 2 + (Ty + Tvy * t - hy) ^ 2 - (hv * t) ^ 2 == 0;
    w = solve(q, t)
    {
    }*/
}
bool LinkCalculation::check_strong_bound(Target &T, leida &l, huoli &h)
{
    if (l.channle <= 0)
        return false;
    return true;
}
vector<double> LinkCalculation::get_ld_para(leida &l, Target &T)
{

    return {1000, 260000, 75, 100000, 75, 25000, 0};
    //近界、远界、航路角、航路捷径、俯仰角、高界、地界
}
vector<double> LinkCalculation::get_hl_para(huoli &h)
{
    return {1000, 260000, 75, 100000, 75, 25000, 0};
    //近界、远界、航路角、航路捷径、俯仰角、高界、地界
}
vector<double> LinkCalculation::get_cal_para(Target &T, leida &l)
{
    double R = sqrt(pow(T.x - l.x, 2) + pow(T.y - l.y, 2) + pow(T.z, 2));

    double hanglujiao = acos(((T.x - l.x) * (-T.vx) + (T.y - l.y) * (-T.vy)) / T.v / R);
    double hanglujiejin = R * sin(hanglujiao);
    hanglujiao *= 57.3;
    double fuyangjiao = asin(T.z / R); // asin(z/R)
    fuyangjiao *= 57.3;
    double z = T.z;
    return {R, R, hanglujiao, hanglujiejin, fuyangjiao, z, z};
    //距离，距离，航路角，航路捷径，俯仰角，高度，高度
}
vector<double> LinkCalculation::get_cal_para(vector<double> &point, leida &l)
{
    double R = sqrt(pow(point[0] - l.x, 2) + pow(point[0] + l.y, 2));
    double hanglujiao = 0;
    double hanglujiejin = 0;
    double fuyangjiao = 0; // asin(z/R)
    double z = 0;
    return {R, R, hanglujiao, hanglujiejin, fuyangjiao, z, z};
    //距离，距离，航路角，航路捷径，俯仰角，高度，高度
}
vector<double> LinkCalculation::get_cal_para(vector<double> &point, huoli &h, Target &T)
{
    double R = sqrt(pow(point[0] - h.x, 2) + pow(point[0] + h.y, 2));

    double R1 = sqrt(pow(T.x - h.x, 2) + pow(T.y - h.y, 2) + pow(T.z, 2));
    double hanglujiao = acos(((T.x - h.x) * (-T.vx) + (T.y - h.y) * (-T.vy)) / T.v / R1);
    double hanglujiejin = R * sin(hanglujiao);
    temp_Ptr = hanglujiejin;
    double fuyangjiao = 0; // asin(z/R)
    double z = 0;
    return {R, R, hanglujiao, hanglujiejin, fuyangjiao, z, z};
    //距离，距离，航路角，航路捷径，俯仰角，高度，高度
}
bool LinkCalculation::check_ld_sector(leida &l, vector<double> &point)
{
    //基本距离要求？
    double theta = atan2((point[1] - l.y), (point[0] - l.x)); //已x为轴，向y正方向为+；//后续x为北，y为东
    theta *= 57.3;
    theta = theta < 0 ? theta + 360 : theta;
    double delta = fabs(theta - l.chaoxiang);
    return delta < l.half_sector;
}
bool LinkCalculation::check_ld_sector(leida &l, Target &T)
{
    //基本距离要求？
    double theta = atan2((T.y - l.y), (T.x - l.x)); //已x为轴，向y正方向为+；//后续x为北，y为东
    theta *= 57.3;
    theta = theta < 0 ? theta + 360 : theta;
    double delta = fabs(theta - l.chaoxiang);
    return delta < l.half_sector;
}
bool LinkCalculation::check_ld_sector(leida &l, huoli &h)
{
    //基本距离要求？
    double theta = atan2((h.y - l.y), (h.x - l.x)); //已x为轴，向y正方向为+；//后续x为北，y为东
    theta *= 57.3;
    theta = theta < 0 ? theta + 360 : theta;
    double delta = fabs(theta - l.chaoxiang);
    return delta < l.half_sector;
}
bool LinkCalculation::check_ld_point(leida &l, vector<double> &point, Target &T)
{
    if (!check_ld_sector(l, point))
        return false;
    vector<double> p1 = get_ld_para(l, T);
    vector<double> p2 = get_cal_para(point, l);
    // if(l1.type == 0)
    if (p1[0] <= p2[0] && p1[1] >= p2[1] && p1[2] >= p2[2] && p1[3] >= p2[3] && p1[4] >= p2[4] && p1[5] >= p2[5] && p1[6] <= p2[6])
    {
        return true;
    }
    return false;
}
bool LinkCalculation::check_ld_point(leida &l, Target &T)
{
    if (!check_ld_sector(l, T))
        return false;
    vector<double> p1 = get_ld_para(l, T);
    vector<double> p2 = get_cal_para(T, l);
    // if(l1.type == 0)
    if (p1[0] <= p2[0] && p1[1] >= p2[1] && p1[2] >= p2[2] && p1[3] >= p2[3] && p1[4] >= p2[4] && p1[5] >= p2[5] && p1[6] <= p2[6])
    {
        return true;
    }
    return false;
}
bool LinkCalculation::check_hl_point(huoli &h, vector<double> &point, Target &T)
{
    vector<double> p1 = get_hl_para(h);
    vector<double> p2 = get_cal_para(point, h, T);
    // if(l1.type == 0)
    if (p1[0] <= p2[0] && p1[1] >= p2[1] && p1[2] >= p2[2] && p1[3] >= p2[3] && p1[4] >= p2[4] && p1[5] >= p2[5] && p1[6] <= p2[6])
    {
        return true;
    }
    return false;
}
bool LinkCalculation::check_useful_hl(Target &T, huoli &h)
{
    if (h.bullet <= 0)
        return false;
    return true;
}
bool LinkCalculation::match_ld_hl(leida &l, huoli &h)
{
    //没写航路雷达扇区外小雨30km的情况
    double dx = l.x - h.x;
    double dy = l.y - h.y;
    double R2 = (dx * dx + dy * dy);
    if (sqrt(R2) < 30000)
        return true;
    if (!check_ld_sector(l, h))
        return false;
    return true;
}
vector<leida> LinkCalculation::get_useful_leida(Target &T)
{
    vector<leida> leida_opt;
    for (auto &x : ld_set)
    {
        if (check_ld_point(x.second, T))
        {
            leida_opt.push_back(x.second);
        }
    }
    return leida_opt;
}
void LinkCalculation::get_all()
{
    vector<double> t01({1, 5, -60000, 80000, 5000, 300, -400, 500});
    vector<double> t02({2, 10, -80000, 60000, 4500, 400, -300, 500});
    Target t1(t01);
    Target t2(t02);
    Target_pqueue.push(t1);
    Target_pqueue.push(t2);

    vector<double> l01({1, 0, 0});
    leida l1(l01);
    ld_set[1] = l1;

    vector<double> h01({1, -500, 600});
    huoli h1(h01);
    hl_set[1] = h1;

    vector<double> z01({1, 300, 100});
    zhikong z1(z01);
    zk_set[1] = z1;
    // zk_set = init_zk();
}
vector<double> LinkCalculation::get_max_vec(vector<ssl> &ssl_temp)
{
    vector<double> max_vec = {__DBL_MIN__, __DBL_MIN__, __DBL_MIN__, __DBL_MIN__, __DBL_MIN__};
    for (auto &x : ssl_temp)
    {
        for (int i = 0; i < 5; i++)
        {
            if (x.para[i] > max_vec[i])
            {
                max_vec[i] = x.para[i];
            }
        }
    }
    return max_vec;
}
vector<double> LinkCalculation::get_min_vec(vector<ssl> &ssl_temp)
{
    vector<double> min_vec = {__DBL_MAX__, __DBL_MAX__, __DBL_MAX__, __DBL_MAX__, __DBL_MAX__};
    for (auto &x : ssl_temp)
    {
        for (int i = 0; i < 5; i++)
        {
            if (x.para[i] < min_vec[i])
            {
                min_vec[i] = x.para[i];
            }
        }
    }
    return min_vec;
}
void LinkCalculation::normalization(vector<double> &max_vec, vector<double> &min_vec, vector<ssl> &ssl_temp, vector<double> &cur_weight)
{
    for (auto &x : ssl_temp)
    {
        double temp = 0;
        for (int i = 0; i < 5; i++)
        {
            if (max_vec[i] - min_vec[i] <= 0.001)
                x.para[i] = 1;
            else
                x.para[i] = (x.para[i] - min_vec[i]) / (max_vec[i] - min_vec[i]);
            temp = x.para[i] * cur_weight[i];
            x.jc += temp;
        }
    }
}
void LinkCalculation::sort_ssl(vector<ssl> &ssl_temp, vector<double> &cur_weight)
{
    vector<double> max_vec = get_max_vec(ssl_temp);
    vector<double> min_vec = get_min_vec(ssl_temp);
    normalization(max_vec, min_vec, ssl_temp, cur_weight);
    sort(ssl_temp.begin(), ssl_temp.end(), [](ssl &r, ssl &l)
         { return r.jc > l.jc; });
}
int LinkCalculation::kill_NUM()
{
    kill_num++;
    return kill_num;
}
void LinkCalculation::clear_all()
{
    zk_set.clear();
    ld_set.clear();
    hl_set.clear();
    while (!Target_pqueue.empty())
        Target_pqueue.pop();
}
void LinkCalculation::check_state()
{
    while (!undone_Target_pqueue.empty())
    {
        Target T = undone_Target_pqueue.top();
        T.update();
        Target_pqueue.push(T);
        undone_Target_pqueue.pop();
    }
    map<int, Target>::iterator it = done_Target.begin();
    while (it != done_Target.end())
    {
        it->second.update();
        if (it->second.tstop <= 0)
        {
            undo_killchain(it->second.virtual_kill_id);
            done_Target.erase(it++);
        }
        else
        {
            it++;
        }
    }
    map<int, int>::iterator iter = ld_zk_bind.begin();
    while (iter != ld_zk_bind.end())
    {
        auto ld = ld_set.find(iter->first);
        auto zk = zk_set.find(iter->second);
        if (ld == ld_set.end() || zk == zk_set.end())
        {
            iter++; // debug
            continue;
        }
        if (ld->second.channle == ld->second.max_channle && zk->second.channle == zk->second.max_channle)
        {
            ld->second.father_zhikong_bianhao = 0;
            zk->second.father_leida_bianhao = 0;
            ld_zk_bind.erase(iter++);
            continue;
        }
        iter++;
        /*if (ld->second.channle == 8 && zk->second.channle == 8)
        {
            ld_zk_bind.erase(iter++);
            continue;
        }
        */
        // ld or zk is broken
    }
}
void LinkCalculation::undo_killchain(int kill_id)
{
    auto iter = ssl_history_set.find(kill_id);
    if (iter != ssl_history_set.end())
    {
        after_cal_ssl_set[kill_id] = iter->second;
        ofstream f1("20221103.txt", ios::app);
        f1
            << "完成打击撤销No." << kill_id << "杀伤链:";
        f1 << iter->second.t.bianhao << "-" << iter->second.l.bianhao << "-" << iter->second.z.bianhao << "-" << iter->second.h.bianhao << "|||";
        f1 << iter->second.issue_time << "+" << iter->second.time << "=" << iter->second.issue_time + iter->second.time << endl;
        ld_set[iter->second.l.bianhao].channle++;
        zk_set[iter->second.z.bianhao].channle++;
        ssl_history_set.erase(iter);
    }
}
double LinkCalculation::getProb(Target &t, leida &l, huoli &h, vector<double> &point)
{
    /*P = 10 / d_RT + 0.02 / TA.vel - 0.005 * t;
    if (P < 0.4)
        continue;
    end if (P > 1)
        P = 1;
    end return 0.9; */
    return 0.9;
}
double LinkCalculation::getProb(ssl &s)
{
    double p = 0;
    p += 3000 / s.para[1];
    p += 300 / s.t.v;
    p -= 0.004 * s.para[2];
    if (p > 1)
        p = 1;
    /*P = 10 / d_RT + 0.02 / TA.vel - 0.005 * t;
    if (P < 0.4)
        continue;
    end if (P > 1)
        P = 1;
    end return 0.9; */
    return p;
}
vector<ssl> LinkCalculation::one_target_all_ssl(Target &T)
{
    vector<ssl> res;
    vector<leida> le_opt = get_useful_leida(T);
    for (auto hl_iter = hl_set.begin(); hl_iter != hl_set.end(); hl_iter++)
    {
        if (!check_useful_hl(T, hl_iter->second))
            continue;
        vector<double> meet_point = cal_meet_point(T, hl_iter->second);
        if (!check_hl_point(hl_iter->second, meet_point, T))
            continue;

        for (auto ld_iter = le_opt.begin(); ld_iter != le_opt.end(); ld_iter++)
        {
            // 如果强约束不符合 返回
            if (!check_strong_bound(T, *ld_iter, hl_iter->second))
                continue;
            if (!check_ld_point(*ld_iter, meet_point, T))
                continue;
            if (!match_ld_hl(*ld_iter, hl_iter->second))
                continue;
            if (ld_iter->father_zhikong_bianhao != 0)
            {
                ssl temp(T, *ld_iter, zk_set.find(ld_iter->father_zhikong_bianhao)->second, hl_iter->second);
                double dx = T.x - ld_iter->x;
                double dy = T.y - ld_iter->y;
                double R = sqrt(dx * dx + dy * dy);
                temp.time = meet_point[2];
                temp.issue_time = issue_time;
                temp.para[0] = R;
                temp.para[1] = temp_Ptr;
                temp.para[2] = meet_point[2];
                temp.para[3] = getProb(temp);
                temp.prob = temp.para[3];
                temp.para[4] = ld_iter->channle / ld_iter->max_channle + temp.z.channle / temp.z.max_channle + temp.h.bullet / 4;
                res.push_back(temp);
                continue;
            }
            for (auto &x : zk_set)
            {
                if (x.second.father_leida_bianhao != 0)
                {
                    continue;
                }
                ssl temp(T, *ld_iter, x.second, hl_iter->second);
                double dx = T.x - ld_iter->x;
                double dy = T.y - ld_iter->y;
                double R = sqrt(dx * dx + dy * dy);
                temp.time = meet_point[2];
                temp.issue_time = issue_time;
                temp.para[0] = R;
                temp.para[1] = temp_Ptr;
                temp.para[2] = meet_point[2];
                temp.para[3] = getProb(temp);
                temp.prob = temp.para[3];
                temp.para[4] = ld_iter->channle / ld_iter->max_channle + temp.z.channle / temp.z.max_channle + temp.h.bullet / 4;
                res.push_back(temp);
            }
        }
    }
    return res;
}
void LinkCalculation::issue_ssl(ssl &s)
{
    ofstream f1("20221103.txt", ios::app);
    int kill_id = kill_NUM();
    f1
        << "成功下发No." << kill_id << "杀伤链:";
    f1 << s.t.bianhao << "-" << s.l.bianhao << "-" << s.z.bianhao << "-" << s.h.bianhao << "|||" << issue_time << "+" << s.time << endl;
    ssl_history_set[kill_id] = s;
    ld_zk_bind[s.l.bianhao] = s.z.bianhao;
    ld_set[s.l.bianhao].father_zhikong_bianhao = s.z.bianhao;
    zk_set[s.z.bianhao].father_leida_bianhao = s.l.bianhao;
    ld_set[s.l.bianhao].channle--;
    zk_set[s.z.bianhao].channle--;
    hl_set[s.h.bianhao].bullet--;
    s.t.done = true;
    s.t.tstop = s.time;
    s.t.virtual_kill_id = kill_id;
    done_Target[s.t.bianhao] = s.t;
}
void LinkCalculation::issue_the_ssl_virtual(vector<ssl> &ssl_set)
{
    if (ssl_set.size() == 0)
        return;
    for (auto &s : ssl_set)
    {
        if (ld_set[s.l.bianhao].channle <= 0 || hl_set[s.h.bianhao].bullet <= 0 || zk_set[s.z.bianhao].channle <= 0)
            continue;
        if ((ld_set[s.l.bianhao].father_zhikong_bianhao == zk_set[s.z.bianhao].bianhao && zk_set[s.z.bianhao].father_leida_bianhao == ld_set[s.l.bianhao].bianhao) ||
            (ld_set[s.l.bianhao].father_zhikong_bianhao == 0 && zk_set[s.z.bianhao].father_leida_bianhao == 0))
        {
            issue_ssl(s);
            return;
        }
    }
    undone_Target_pqueue.push(ssl_set.front().t);
}
void LinkCalculation::get_weight()
{
    //从数据库里面拿值；
    cur_weight.clear();
    vector<double>
        w{0.1, 0.2, 0.1, 0.2, -0.2};
    cur_weight = w;
}
void LinkCalculation::ManageKillLink()
{
    check_state();
    if (Target_pqueue.size() == 0)
        return;
    while (!Target_pqueue.empty())
    {
        Target T = Target_pqueue.top();
        vector<ssl> ssl_temp = one_target_all_ssl(T);
        sort_ssl(ssl_temp, cur_weight);
        Option_ssw_set.push(ssl_temp);
        if (ssl_temp.size() == 0)
            undone_Target_pqueue.push(T);
        Target_pqueue.pop();
    }

    if (auto_mode)
    {
        while (!Option_ssw_set.empty())
        {
            auto temp = Option_ssw_set.top();
            issue_the_ssl_virtual(temp);
            Option_ssw_set.pop();
        }
    }
}
void LinkCalculation::ManageKillLink(int kill_type)
{
    check_state();
    if (Target_pqueue.size() == 0)
        return;
    while (!Target_pqueue.empty())
    {
        Target T = Target_pqueue.top();
        vector<ssl> ssl_temp = one_target_all_ssl(T);
        sort_ssl(ssl_temp, cur_weight);
        issue_the_ssl_virtual(ssl_temp);
        if (ssl_temp.size() == 0)
            undone_Target_pqueue.push(T);
        Target_pqueue.pop();
    }
}
