#pragma once
#include <memory>
#include <string>
#include <vector>

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <pypa/ast/ast.hh>

#include "src/carnot/planner/objects/qlobject.h"

namespace pl {
namespace carnot {
namespace planner {
namespace compiler {

/**
 * @brief NoneObject represents None in python, the "null" object. This is used as a proxy for void
 * return type in Python interpretation.
 *
 */
class TypeObject : public QLObject {
 public:
  static constexpr TypeDescriptor TypeObjectType = {
      /* name */ "Type",
      /* type */ QLObjectType::kType,
  };

  static StatusOr<std::shared_ptr<TypeObject>> Create(IRNodeType node_type,
                                                      ASTVisitor* ast_visitor) {
    return std::shared_ptr<TypeObject>(new TypeObject(node_type, ast_visitor));
  }

  Status NodeMatches(IRNode* node) {
    // TODO(philkuz) make this nvi and expand it more.
    if (node->type() == node_type_) {
      return Status::OK();
    }
    return node->CreateIRNodeError("Expected '$0', received '$1'", IRNode::TypeString(node_type_),
                                   node->type_string());
  }

  IRNodeType ir_node_type() { return node_type_; }

 protected:
  /**
   * @brief Construct a Type object that represents the null value in Python.
   *
   * @param ast the ast ptr for the
   */
  TypeObject(IRNodeType node_type, ASTVisitor* ast_visitor)
      : QLObject(TypeObjectType, ast_visitor), node_type_(node_type) {}

 private:
  IRNodeType node_type_;
};

}  // namespace compiler
}  // namespace planner
}  // namespace carnot
}  // namespace pl
