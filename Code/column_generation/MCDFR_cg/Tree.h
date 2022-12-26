// Branch-and-bound Tree

#ifndef _TREE_H_
#define _TREE_H_

#include "Node.h"

// auto CompareNodeScoreDescending = [](const Node &a, const Node &b)
// {
//    return a.lb > b.lb;
// };

enum SearchStrategy {
    DepthFirst, PrimalBoundFirst
};

class Tree 
{
   public:
      Tree(bool DepthFirst) //: que(CompareNodeScoreDescending)
      {
         if (DepthFirst)
            searchStrategy = SearchStrategy::DepthFirst;
         else
            searchStrategy = SearchStrategy::PrimalBoundFirst;
      }

      ~Tree() = default;

      void pushByMove(const Node &node)
      {
         if (searchStrategy == SearchStrategy::DepthFirst)
         {
            tree.push_back(std::move(node));
         }
         // else
         // {
         //    que.push(std::move(node));
         // }
      }

      void emplace(const Node *parent, long nodeId, bool branchOnNum, int a, int b, int c = -1)
      {
         if (searchStrategy == SearchStrategy::DepthFirst)
         {
            tree.emplace_back(parent, nodeId, branchOnNum, a, b, c);
         }
         // else
         // {
         //    que.emplace(parent, nodeId, branchOnNum, a, b, c);
         // }
      }

      const Node &next()
      {
         if (searchStrategy == SearchStrategy::DepthFirst)
            return tree.back();
         // else
         //    return que.top();
      }

      void pop()
      {
         if (searchStrategy == SearchStrategy::DepthFirst)
            tree.pop_back();
         // else
         //    que.pop();
      }

      bool empty() const
      {
         if (searchStrategy == SearchStrategy::DepthFirst)
            return tree.empty();
         // else
         //    return que.empty();
      }

      int size() const
      {
         if (searchStrategy == SearchStrategy::DepthFirst)
            return tree.size();
         // else
         //    return que.size();
      }

      double updateGlobalLb()
      {
         if (searchStrategy == SearchStrategy::DepthFirst)
         {
            double globalLb = tree.front().lb;
            for (auto &node: tree) {
                if (globalLb > node.lb) {
                    globalLb = node.lb;
                }
            }
            return globalLb;
         }
         // else
         // {
         //    return que.top().lb;
         // }
      }

   public:
      SearchStrategy searchStrategy;

   private:
      std::vector<Node> tree;
      // std::priority_queue<Node, std::vector<Node>, decltype(CompareNodeScoreDescending)> que;

};

#endif //_TREE_H_
