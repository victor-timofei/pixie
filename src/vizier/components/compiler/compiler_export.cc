#include "src/vizier/components/compiler/compiler_export.h"

#include <google/protobuf/text_format.h>
#include <memory>
#include <string>
#include <utility>

#include "src/carnot/compiler/compiler.h"
#include "src/carnot/compiler/compiler_state.h"
#include "src/carnot/compiler/registry_info.h"
#include "src/carnot/proto/plan.pb.h"
#include "src/carnot/udf_exporter/udf_exporter.h"
#include "src/common/base/time.h"
#include "src/table_store/schema/relation.h"

namespace internal {
pl::StatusOr<std::unique_ptr<pl::carnot::compiler::RelationMap> > MakeRelationMap(
    std::string relation_pb_str, std::string table_name) {
  pl::table_store::schema::Relation rel;

  pl::table_store::schemapb::Relation relation_pb;
  bool str_merge_success =
      google::protobuf::TextFormat::MergeFromString(relation_pb_str, &relation_pb);
  if (!str_merge_success) {
    return pl::error::InvalidArgument("Couldn't load the relation str as a protobuf.");
  }

  PL_RETURN_IF_ERROR(rel.FromProto(&relation_pb));

  auto rel_map = std::make_unique<pl::carnot::compiler::RelationMap>();
  rel_map->emplace(table_name, rel);
  return rel_map;
}
char *CloneStringToCharArray(std::string str, int *ret_len) {
  *ret_len = str.size();
  char *retval = new char[str.size()];
  memcpy(retval, str.data(), str.size());
  return retval;
}
}  // namespace internal

CompilerPtr CompilerNew() {
  auto compiler_ptr = new pl::carnot::compiler::Compiler();
  return reinterpret_cast<CompilerPtr>(compiler_ptr);
}

char *CompilerCompile(CompilerPtr compiler_ptr, const char *rel_str_c, int rel_str_len,
                      const char *table_name_str_c, int table_name_str_len, const char *query,
                      int query_len, int *resultLen) {
  DCHECK(rel_str_c != nullptr);
  DCHECK(table_name_str_c != nullptr);
  DCHECK(query != nullptr);
  std::string rel_str(rel_str_c, rel_str_c + rel_str_len);
  std::string table_name_str(table_name_str_c, table_name_str_c + table_name_str_len);
  std::string query_str(query, query + query_len);

  auto compiler = reinterpret_cast<pl::carnot::compiler::Compiler *>(compiler_ptr);

  auto registry_info_status = pl::carnot::udfexporter::ExportUDFInfo();
  if (!registry_info_status.ok()) {
    return internal::CloneStringToCharArray(registry_info_status.msg(), resultLen);
  }
  auto registry_info = registry_info_status.ConsumeValueOrDie();

  auto rel_map_status = internal::MakeRelationMap(rel_str, table_name_str);
  if (!rel_map_status.ok()) {
    return internal::CloneStringToCharArray(rel_map_status.msg(), resultLen);
  }
  auto rel_map = rel_map_status.ConsumeValueOrDie();

  // Create a CompilerState obj using the relation map and grabbing the current time.
  auto compiler_state_obj = std::make_unique<pl::carnot::compiler::CompilerState>(
      std::move(rel_map), registry_info.get(), pl::CurrentTimeNS());

  // Pass query into the C++ compile call.
  auto res = compiler->Compile(query_str, compiler_state_obj.get());
  if (!res.ok()) {
    return internal::CloneStringToCharArray(res.msg(), resultLen);
  }
  pl::carnot::carnotpb::Plan plan_pb = res.ConsumeValueOrDie();

  // Serialize the logical plan into bytes.
  std::string ret_str;
  bool serialize = plan_pb.SerializeToString(&ret_str);
  if (!serialize) {
    return internal::CloneStringToCharArray("Couldn't serialize the message", resultLen);
  }
  return internal::CloneStringToCharArray(ret_str, resultLen);
}

void CompilerFree(CompilerPtr compiler_ptr) {
  delete reinterpret_cast<pl::carnot::compiler::Compiler *>(compiler_ptr);
}

void CompilerStrFree(char *str) { delete str; }
