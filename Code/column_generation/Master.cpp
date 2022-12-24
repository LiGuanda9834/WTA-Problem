#include "Master.h"

Master::Master(MCFDR* _mcfdr, const AlgoParameter &_param)
        : numLp(0), time(0),
          mcfdr(_mcfdr), param(_param),
          model(env), cplex(model), ptrNumVehicle(nullptr) {
    if (!param.enableCplexLog) {
        cplex.setOut(env.getNullStream());
        cplex.setWarning(env.getNullStream());
    }
    cplex.setParam(IloCplex::Param::RandomSeed, param.seed);
    cplex.setParam(IloCplex::Param::Threads, 1);

    y = IloNumVarArray(env);
    obj = IloMinimize(env);
    model.add(obj);

    rng = IloRangeArray(env, inst.n + 1, 1, param.maxValue);
    model.add(rng);
    if (param.enableBranchOnSum) ptrNumVehicle = &rng[inst.n];
}

void Master::set(const Node &node) {
    if (param.enableBranchOnSum) {
        if (ptrNumVehicle->getLB() != node.lbNumVehicle) ptrNumVehicle->setLb(node.lbNumVehicle);
        if (ptrNumVehicle->getUB() != node.ubNumVehicle) ptrNumVehicle->setUb(node.ubNumVehicle);
    }
    for (int i = 0; i < routePool.size(); ++i) {
        const Route &route = routePool[i];
        if (node.valid(route)) {
            if (y[i].getLB() != 0 || y[i].getUB() != 1) {
                y[i].setBounds(0, 1);
            }
        } else {
            if (y[i].getLB() != 0 || y[i].getUB() != 0) {
                y[i].setBounds(0, 0);
            }
        }
    }
}

bool Master::Solve() {
    timer.on();
    numLp++;
    bool f = cplex.solve();
    time += timer.elapsed();
    return f;
}

void Master::GetDualValues(vector<double> &dual) {
    for (int i = 0; i < inst.n + 1; ++i) {
        dual[i] = cplex.getDual(rng[i]);
    }
}

void Master::addSlackToCardinality() {
    IloNumColumn column(env);
    column += obj(1 << 30);
    column += (*ptrNumVehicle)(-1); // todo: check
    slack = IloNumVar(column, 0, param.maxValue, ILOFLOAT, "slack");
}

void Master::addRoutes(std::vector<Route> &routes) {
    timer.on();
    if (param.debug) {
        bool bug = false;
        for (Route &route: routes) {
            auto it = std::find(routePool.begin(), routePool.end(), route);
            if (it != routePool.end()) {
                Route &other = *it;
                huq::println_tab("route existed in pool", huq::join(route));
                bug = true;
            }
        }
        if (bug) std::exit(-1);
    }

    for (Route &route: routes) {
        auto routeId = routePool.size();
        IloNumColumn column(env);
        column += obj(route.cost);
        for (int i = 1; i + 1 < route.size(); ++i) {
            int p = route[i];
            if (1 <= p && p <= inst.indexLastPickup) {
                column += rng[route[i]](1);
            }
        }
        if (param.enableBranchOnSum) column += (*ptrNumVehicle)(1);
        y.add(IloNumVar(column, 0, param.maxValue, ILOFLOAT, ("y" + std::to_string(routeId)).c_str()));
    }

    time += timer.elapsed();
}

void Master::getSol(Node &node) {
    node.ptr_sol = new LpSol();
    node.ptr_sol->obj = cplex.getObjValue();
    for (int i = 0; i < routePool.size(); ++i) {
        double v = cplex.getValue(y[i]);
        if (v > param.rc_eps) {
            node.ptr_sol->push_back(std::make_pair(&routePool[i], v));
        }
    }
}