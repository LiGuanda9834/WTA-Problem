#include <vector>
#include <map>
#include <set>
#include <queue>
#include <fstream>
#include <math.h>
#include <iostream>
#include <algorithm>
using namespace std;

struct Target
{
    Target() {}
    Target(vector<double> &t) : bianhao(t[0]), threat(t[1]), x(t[2]), y(t[3]), z(t[4]), vx(t[5]), vy(t[6]), v(t[7]) {}
    int bianhao;
    int threat;
    double x;
    double y;
    double z;
    double vx;
    double vy;
    double v;
    int type = 256;
    bool done = false;
    double t = 2;
    double tstop = 200;
    double virtual_kill_id = 0;
    void update()
    {
        tstop -= t;
        x = x + vx * t;
        y = t + vy * t;
        if (sqrt(x * x + y * y) < 10000)
            done = true;
    }
};
struct Target_cmp
{
public:
    bool operator()(Target &T1, Target &T2)
    {
        return T1.threat < T2.threat;
    }
};
struct leida
{
    leida() {}
    leida(vector<double> &t) : bianhao(t[0]), x(t[1]), y(t[2]) {}
    int bianhao;
    double chaoxiang = 135;
    double half_sector = 60;
    double x;
    double y;
    int channle = 8; //？？？max_channle
    int max_channle = 8;
    int father_zhikong_bianhao = 0;
};
struct huoli
{
    huoli() {}
    huoli(vector<double> &t) : bianhao(t[0]), x(t[1]), y(t[2]) {}
    int bianhao;
    double x;
    double y;
    double v = 1000;
    int bullet = 4;
};
struct zhikong
{
    zhikong() {}
    zhikong(vector<double> &t) : bianhao(t[0]), x(t[1]), y(t[2]) {}

    int bianhao;
    double x;
    double y;
    int channle = 8;
    int max_channle = 8;
    int father_leida_bianhao = 0;
};

class ssl
{
public:
    ssl() {}
    ssl(Target &_t, leida &_l, zhikong &_z, huoli &_h) : t(_t), l(_l), z(_z), h(_h) {}
    Target t;
    leida l;
    zhikong z;
    huoli h;
    vector<double> para = {0, 0, 0, 0, 0};
    double jc = 0;
    double issue_time = 0;
    double time = 0;
    double prob = 0;
    /*double R;
    double Ptr;
    double time;
    double prob;
    double per;*/
};
struct ssw_cmp
{
public:
    bool operator()(vector<ssl> &T1, vector<ssl> &T2)
    {
        if (T1.size() == 0 && T2.size() == 0)
            return true;
        return T1.size() > T2.size() || (T1.size() == T2.size() && T1.front().t.threat < T2.front().t.threat);
    }
};
typedef priority_queue<Target, vector<Target>, Target_cmp> pqT;
class LinkCalculation
{
public:
    LinkCalculation()
    {
        clear_all();
        get_all();
        get_weight();
        // initiaize this class
    }
    LinkCalculation(pqT &&t1, map<int, leida> &&ld, map<int, zhikong> &&zk, map<int, huoli> &&hl) : Target_pqueue(t1), ld_set(ld), zk_set(zk), hl_set(hl)
    {
        get_weight();
    }
    LinkCalculation(pqT &t1, map<int, leida> &ld, map<int, zhikong> &zk, map<int, huoli> &hl) : Target_pqueue(t1), ld_set(ld), zk_set(zk), hl_set(hl)
    {
        get_weight();
    }
    int kill_NUM();
    void ManageKillLink();                     //主函数
    void ManageKillLink(int kill_type);        //主函数
    vector<ssl> one_target_all_ssl(Target &T); //构建
    void get_weight();                         //数据库 权重
    void issue_ssl(ssl &s);
    void issue_the_ssl_virtual(vector<ssl> &ssl_set); // please consider if there is more need like multi-ssl in futre;
    // void issue_the_ssw_virtual(vector<ssl> &ssl_set);
    void sort_ssl(vector<ssl> &ssl_temp, vector<double> &cur_weight);
    void undo_killchain(int kill_id); //撤销杀伤链
    void check_state();
    void clear_all();
    void get_all();
    vector<leida> get_useful_leida(Target &T);
    vector<double> get_cal_para(vector<double> &point, huoli &h, Target &T);
    bool check_hl_point(huoli &h, vector<double> &point, Target &T);
    vector<double> cal_meet_point(Target &T, huoli &h);
    bool check_strong_bound(Target &T, leida &l, huoli &h);
    vector<double> get_ld_para(leida &l, Target &T);
    vector<double> get_hl_para(huoli &h);
    vector<double> get_cal_para(Target &T, leida &l);
    vector<double> get_cal_para(vector<double> &point, leida &l);
    bool check_ld_sector(leida &l, vector<double> &point);
    bool check_ld_sector(leida &l, Target &T);
    bool check_ld_sector(leida &l, huoli &h);
    bool check_ld_point(leida &l, vector<double> &point, Target &T);
    bool check_ld_point(leida &l, Target &T);
    bool check_useful_hl(Target &T, huoli &h);
    bool match_ld_hl(leida &l, huoli &h);
    double getProb(Target &t, leida &l, huoli &h, vector<double> &point);
    double getProb(ssl &s);
    vector<double> get_max_vec(vector<ssl> &ssl_temp);
    vector<double> get_min_vec(vector<ssl> &ssl_temp);
    void normalization(vector<double> &max_vec, vector<double> &min_vec, vector<ssl> &ssl_temp, vector<double> &cur_weight);

public: // should be private
    priority_queue<Target, vector<Target>, Target_cmp> Target_pqueue;
    priority_queue<Target, vector<Target>, Target_cmp> undone_Target_pqueue;
    map<int, Target> done_Target;
    map<int, ssl> ssl_history_set;
    map<int, ssl> after_cal_ssl_set;
    map<int, zhikong> zk_set;
    map<int, leida> ld_set;
    map<int, huoli> hl_set;
    vector<double> cur_weight;
    priority_queue<vector<ssl>, vector<vector<ssl>>, ssw_cmp> Option_ssw_set;
    map<int, int> ld_zk_bind;
    int kill_num = 0;
    bool auto_mode = true;
    double issue_time = 0;
    double temp_Ptr = 0;
};