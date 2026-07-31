#ifndef ARTIC_NAME_MAP_H
#define ARTIC_NAME_MAP_H

// Language-server support. Everything in this header is compiled away unless the build
// declares ENABLE_LSP, so the standalone compiler carries none of it.
#ifdef ENABLE_LSP

#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "artic/ast.h"
#include "artic/loc.h"

namespace artic::ls {

/// Stores information related to LSP go-to-definiton & find-references
class NameMap {
public:
    using Decl = const ast::NamedDecl*;
    using Ref = std::variant<const ast::Identifier* /* used for ast::FieldExpr and ast::FieldPtrn */,
                             const ast::Path*, const ast::Path::Elem*,
                             const ast::ProjExpr*>;

    void insert(Decl decl, Ref ref);
    void insert(Decl def);
    void add_type_hint(const ast::Node& node);

    const std::vector<Ref>& find_refs(Decl decl) const;
    Decl find_decl(Ref ref) const;

    Decl find_decl_at(const Loc& loc) const;
    std::optional<Ref> find_ref_at(const Loc& loc) const;

    const ast::Identifier& get_identifier(Ref ref) const;

    struct Names {
        std::unordered_map<Ref, Decl> declaration_of;
        std::unordered_map<Decl, std::vector<Ref>> references_of;
        std::vector<const ast::Node*> with_type_hint;
    };
    std::unordered_map<std::string, Names> files;
};

} // namespace artic::ls

#endif // ENABLE_LSP

#endif // ARTIC_NAME_MAP_H
