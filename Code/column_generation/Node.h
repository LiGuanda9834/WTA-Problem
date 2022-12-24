#ifndef MCFDR_NODE_H
#define MCFDR_NODE_H

#include "Path.h"
#include <vector>


enum NodeStatus {
    Unsolved, Infeasible, PrunedByBound, Fractional, Integral
};

class LpSol : public std::vector<std::pair<Path *, double>> {
public:
    double obj;

public:
    LpSol() = default;
    ~LpSol() = default;

    double accumulate() {
        double total = 0;
        for (auto &p: *this) {
            total += p.second;
        }
        return total;
    }

    double cost(int x, int y) {
        double total = 0;
        for (auto &p: *this) {
            for (int i = 0; i + 1 < p.first->size(); ++i) {
                if (x == p.first->at(i) && y == p.first->at(i + 1)) {
                    total += p.second;
                }
            }
        }
        return total;
    }

   //  bool integerFeasible() {
   //      for (auto v: *this) {
   //          if (!huq::integral(v.second)) return false;
   //      }
   //      return true;
   //  }
};

class Node {
public:
    Node() = default;

    Node(const Node *_parent, long _nodeId, bool branchOnNum, int a, int b, int c = -1)
            : parent(_parent), nodeId(_nodeId),
              lbNumVehicle(branchOnNum ? a : _parent->lbNumVehicle),
              ubNumVehicle(branchOnNum ? b : _parent->ubNumVehicle),
              forbidden(_parent->forbidden),
              status(NodeStatus::Unsolved), lpsol(nullptr),
              lb(_parent == nullptr ? 0 : _parent->lb) {
        if (!branchOnNum) {
            if (c == 0) {
                forbidden[a][b] = true;
            } else if (c == 1) {
                auto nVertex = forbidden.size();
                for (int i = 0; i < nVertex; ++i) {
                    if (i != a && !forbidden[i][b]) {
                        forbidden[i][b] = true;
                    }
                    if (i != b && !forbidden[a][i]) {
                        forbidden[a][i] = true;
                    }
                }
            } else {
                std::cerr << "invalid branch\r\n";
            }
        }
    }

    ~Node() {
        parent = nullptr;
        if (lpsol != nullptr) {
            delete lpsol;
        }
    }

    std::string statusStr() const {
        if (status == NodeStatus::Unsolved) return "Unsolved";
        else if (status == NodeStatus::Infeasible) return "Infeasible";
        else if (status == NodeStatus::PrunedByBound) return "Pruned";
        else if (status == NodeStatus::Fractional) return "Fractional";
        else if (status == NodeStatus::Integral) return "Integral";
        else return "Unknown";
    }

    bool valid(const Path &route) const {
        for (int i = 0; i + 1 < route.size(); ++i) {
            int a = route[i], b = route[i + 1];
            if (forbidden[a][b]) {
                return false;
            }
        }
        return true;
    }

public:
    const Node *parent;
    long nodeId;

    int lbNumVehicle;
    int ubNumVehicle;
    std::vector<std::vector<bool>> forbidden;

    NodeStatus status;
    LpSol* lpsol;
    double lb;
};

#endif //MCFDR_NODE_H
