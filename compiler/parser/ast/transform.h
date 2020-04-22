/**
 * transform.h
 * Type checking AST walker.
 *
 * Simplifies a given AST and generates types for each expression node.
 */

#pragma once

#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "parser/ast/ast.h"
#include "parser/ast/format.h"
#include "parser/ast/types.h"
#include "parser/ast/visitor.h"
#include "parser/ast/walk.h"
#include "parser/common.h"
#include "parser/context.h"

namespace seq {
namespace ast {

class TransformVisitor : public ASTVisitor, public seq::SrcObject {
  TypeContext &ctx;
  std::shared_ptr<std::vector<StmtPtr>> prependStmts;
  ExprPtr resultExpr;
  StmtPtr resultStmt;
  PatternPtr resultPattern;

  /// Helper function that handles simple assignments
  /// (e.g. a = b, a.x = b or a[x] = b)
  StmtPtr addAssignment(const Expr *lhs, const Expr *rhs,
                        const Expr *type = nullptr, bool force = false);
  /// Helper function that decomposes complex assignments into simple ones
  /// (e.g. a, *b, (c, d) = foo)
  void processAssignment(const Expr *lhs, const Expr *rhs,
                         std::vector<StmtPtr> &stmts, bool force = false);

  StmtPtr getGeneratorBlock(const std::vector<GeneratorExpr::Body> &loops,
                            SuiteStmt *&prev);
  void prepend(StmtPtr s);

  TypePtr realize(std::shared_ptr<FuncType> type);

  class CaptureVisitor : public WalkVisitor {
    TypeContext &ctx;

  public:
    std::unordered_set<std::string> captures;
    using WalkVisitor::visit;
    CaptureVisitor(TypeContext &ctx);
    void visit(const IdExpr *) override;
  };

public:
  TransformVisitor(TypeContext &ctx,
                   std::shared_ptr<std::vector<StmtPtr>> stmts = nullptr);

  ExprPtr transform(const Expr *e, bool allowTypes = false);
  StmtPtr transform(const Stmt *s);
  PatternPtr transform(const Pattern *p);
  ExprPtr transformType(const ExprPtr &expr);
  StmtPtr realizeBlock(const Stmt *stmt);

public:
  void visit(const NoneExpr *) override;
  void visit(const BoolExpr *) override;
  void visit(const IntExpr *) override;
  void visit(const FloatExpr *) override;
  void visit(const StringExpr *) override;
  void visit(const FStringExpr *) override;
  void visit(const KmerExpr *) override;
  void visit(const SeqExpr *) override;
  void visit(const IdExpr *) override;
  void visit(const UnpackExpr *) override;
  void visit(const TupleExpr *) override;
  void visit(const ListExpr *) override;
  void visit(const SetExpr *) override;
  void visit(const DictExpr *) override;
  void visit(const GeneratorExpr *) override;
  void visit(const DictGeneratorExpr *) override;
  void visit(const IfExpr *) override;
  void visit(const UnaryExpr *) override;
  void visit(const BinaryExpr *) override;
  void visit(const PipeExpr *) override;
  void visit(const IndexExpr *) override;
  void visit(const CallExpr *) override;
  void visit(const DotExpr *) override;
  void visit(const SliceExpr *) override;
  void visit(const EllipsisExpr *) override;
  void visit(const TypeOfExpr *) override;
  void visit(const PtrExpr *) override;
  void visit(const LambdaExpr *) override;
  void visit(const YieldExpr *) override;

  void visit(const SuiteStmt *) override;
  void visit(const PassStmt *) override;
  void visit(const BreakStmt *) override;
  void visit(const ContinueStmt *) override;
  void visit(const ExprStmt *) override;
  void visit(const AssignStmt *) override;
  void visit(const DelStmt *) override;
  void visit(const PrintStmt *) override;
  void visit(const ReturnStmt *) override;
  void visit(const YieldStmt *) override;
  void visit(const AssertStmt *) override;
  void visit(const WhileStmt *) override;
  void visit(const ForStmt *) override;
  void visit(const IfStmt *) override;
  void visit(const MatchStmt *) override;
  void visit(const ExtendStmt *) override;
  void visit(const ImportStmt *) override;
  void visit(const ExternImportStmt *) override;
  void visit(const TryStmt *) override;
  void visit(const GlobalStmt *) override;
  void visit(const ThrowStmt *) override;
  void visit(const FunctionStmt *) override;
  void visit(const ClassStmt *) override;
  void visit(const DeclareStmt *) override;
  void visit(const AssignEqStmt *) override;
  void visit(const YieldFromStmt *) override;
  void visit(const WithStmt *) override;
  void visit(const PyDefStmt *) override;

  void visit(const StarPattern *) override;
  void visit(const IntPattern *) override;
  void visit(const BoolPattern *) override;
  void visit(const StrPattern *) override;
  void visit(const SeqPattern *) override;
  void visit(const RangePattern *) override;
  void visit(const TuplePattern *) override;
  void visit(const ListPattern *) override;
  void visit(const OrPattern *) override;
  void visit(const WildcardPattern *) override;
  void visit(const GuardedPattern *) override;
  void visit(const BoundPattern *) override;

public:
  template <typename Tn, typename... Ts> auto N(Ts &&... args) {
    auto t = std::make_unique<Tn>(std::forward<Ts>(args)...);
    t->setSrcInfo(getSrcInfo());
    return t;
  }
  template <typename Tn, typename... Ts>
  auto Nx(const seq::SrcObject *s, Ts &&... args) {
    auto t = std::make_unique<Tn>(std::forward<Ts>(args)...);
    t->setSrcInfo(s->getSrcInfo());
    return t;
  }
  template <typename Tt, typename... Ts> auto T(Ts &&... args) {
    auto t = std::make_shared<Tt>(std::forward<Ts>(args)...);
    t->setSrcInfo(getSrcInfo());
    return t;
  }
  template <typename T, typename... Ts>
  auto transform(const std::unique_ptr<T> &t, Ts &&... args)
      -> decltype(transform(t.get())) {
    return transform(t.get(), std::forward<Ts>(args)...);
  }
  template <typename T> auto transform(const std::vector<T> &ts) {
    std::vector<T> r;
    for (auto &e : ts)
      r.push_back(transform(e));
    return r;
  }
};

} // namespace ast
} // namespace seq
