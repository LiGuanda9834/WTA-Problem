#ifndef ORA_PRICING_H
#define ORA_PRICING_H

#include <vector>
#include <queue>
#include "Mcfdr.h"
#include "Path.h"
#include "Node.h"


class Label {
public:
    Label() = default;
    Label(const Label & _parent, int j);
    ~Label() = default;
public:
    Label *parent;
    int cur;
    int serviceStartTime;
    int load;
    double cost;
    std::vector<int> onboardOrders;
    std::vector<bool> unreachableOrders; // inst.n+1

    bool pruned; // prune by bound or is dominated
};

auto CompareLabelAscending = [](const Label *a, const Label *b) {
//    return a.score > b.score;
    return a->cost > b->cost;
};

class Pricing {
public:
    Pricing(MCFDR* mcfdr);
    ~Pricing() = default;

    void Set(Node &node);
    void Solve(const std::vector<double> &dual, int* colind);
    void extend(const Label &parent);
    bool dominance(int j);

public:
   MCFDR* mcfdr;
   std::vector<Path> paths; // genereted route with negative reduced cost
   Node *ptrNode;

   std::vector<std::vector<Label>> states;
};


#endif //ORA_PRICING_H
