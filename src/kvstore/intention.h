#ifndef ZLOG_KVSTORE_INTENTION_H
#define ZLOG_KVSTORE_INTENTION_H
#include <deque>
#include <memory>
#include "kvstore/kvstore.pb.h"
#include "node.h"

/*
 * An intention is the after image of a transaction.
 */
class Intention {
 public:
  Intention(const std::shared_ptr<NodePtr>& snapshot, uint64_t rid);

  void Put(const std::string& key, const std::string& val);
  void Delete(std::string key);
  bool Empty();
  void Serialize(std::string *blob);
  void SetCSN(uint64_t pos);

 private:
  const std::shared_ptr<const NodePtr> snapshot_;
  const uint64_t rid_;
  NodeRef root_;
  std::vector<std::string> description_;

  static inline NodePtr& left(NodeRef n) { return n->left; };
  static inline NodePtr& right(NodeRef n) { return n->right; };

  static inline NodeRef pop_front(std::deque<NodeRef>& d) {
    auto front = d.front();
    d.pop_front();
    return front;
  }

  NodeRef insert_recursive(std::deque<NodeRef>& path,
      std::string key, std::string val, const NodeRef& node);

  template<typename ChildA, typename ChildB>
  void insert_balance(NodeRef& parent, NodeRef& nn,
      std::deque<NodeRef>& path, ChildA, ChildB, NodeRef& root);

  template <typename ChildA, typename ChildB >
  NodeRef rotate(NodeRef parent, NodeRef child,
      ChildA child_a, ChildB child_b, NodeRef& root);

  NodeRef delete_recursive(std::deque<NodeRef>& path,
      std::string key, const NodeRef& node);

  void transplant(NodeRef parent, NodeRef removed,
      NodeRef transplanted, NodeRef& root);

  NodeRef build_min_path(NodeRef node, std::deque<NodeRef>& path);

  template<typename ChildA, typename ChildB>
  void mirror_remove_balance(NodeRef& extra_black, NodeRef& parent,
      std::deque<NodeRef>& path, ChildA child_a, ChildB child_b,
      NodeRef& root);

  void balance_delete(NodeRef extra_black,
      std::deque<NodeRef>& path, NodeRef& root);

  // turn a transaction into a serialized protocol buffer
  void serialize_node_ptr(kvstore_proto::NodePtr *dst, NodePtr& src,
      const std::string& dir) const;
  void serialize_node(kvstore_proto::Node *dst, NodeRef node,
      int field_index) const;
  void serialize_intention(kvstore_proto::Intention& out, NodeRef node, int& field_index);

  void set_intention_self_csn_recursive(uint64_t rid, NodeRef node,
      uint64_t pos);

  void set_intention_self_csn(NodeRef root, uint64_t pos);
};

#endif
