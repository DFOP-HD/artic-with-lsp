#include "artic/name_map.h"

#ifdef ENABLE_LSP

#include <cassert>
#include <cstdlib>

#include "artic/ast.h"

namespace artic::ls {

static bool contains(const Loc& loc, const Loc& cursor, bool multiline_check) {
    if (loc.begin.row == loc.end.row) multiline_check = false;
    if (multiline_check) {
        if (cursor.begin.row < loc.begin.row || cursor.end.row > loc.end.row) return false;
        return true;
    } else {
        if (cursor.begin.row != loc.begin.row || cursor.begin.col < loc.begin.col) return false;
        if (cursor.end.row   != loc.end.row   || cursor.end.col > loc.end.col) return false;
        return true;
    }
}

using Ref = NameMap::Ref;
using Decl = NameMap::Decl;

void NameMap::add_type_hint(const ast::Node& node) {
    if (!node.loc.file) return;
    files[*node.loc.file].with_type_hint.push_back(&node);
}

void NameMap::insert(Decl decl, Ref ref) {
    if (!decl || !decl->id.loc.file) return;
    if (std::visit([](auto&& ref) { return ref == nullptr; }, ref)) return;

    files[*get_identifier(ref).loc.file].declaration_of[ref] = decl;
    files[*decl->id.loc.file].references_of[decl].push_back(ref);
}

void NameMap::insert(Decl decl) {
    if (!decl || !decl->id.loc.file) return;
    files[*decl->id.loc.file].references_of.emplace(decl, std::vector<Ref>{});
}

const std::vector<Ref>& NameMap::find_refs(Decl decl) const {
    static const std::vector<Ref> empty;
    if (!decl) return empty;
    if (auto names = files.find(*decl->loc.file); names != files.end()) {
        if (auto def = names->second.references_of.find(decl); def != names->second.references_of.end()) {
            return def->second;
        }
    }
    return empty;
}

Decl NameMap::find_decl(Ref ref) const {
    auto id = get_identifier(ref);
    if (auto names = files.find(*id.loc.file); names != files.end()) {
        if (auto def = names->second.declaration_of.find(ref); def != names->second.declaration_of.end()) {
            return def->second;
        }
    }
    return nullptr;
}

Decl NameMap::find_decl_at(const Loc& loc) const {
    if (!loc.file) return nullptr;
    auto file = files.find(*loc.file);
    if (file == files.end()) return nullptr;
    for (auto& [def, ref] : file->second.references_of) {
        // Note: Does duplicate checks as this is a multi map. However, the check should be fast enough
        if (contains(def->id.loc, loc, false)) return def;
    }
    return nullptr;
}

std::optional<Ref> NameMap::find_ref_at(const Loc& loc) const {
    if (!loc.file) return std::nullopt;
    auto file = files.find(*loc.file);
    if (file == files.end()) return std::nullopt;
    for (auto& [ref, decl] : file->second.declaration_of) {
        auto id = get_identifier(ref);
        if (contains(id.loc, loc, false)) return ref;
    }
    return std::nullopt;
}

const ast::Identifier& NameMap::get_identifier(Ref ref) const {
    return std::visit([](auto&& ref) -> const ast::Identifier& {
        using T = std::decay_t<decltype(ref)>;
        if constexpr (std::is_same_v<T, const ast::Identifier*>)
            return *ref;
        if constexpr (std::is_same_v<T, const ast::Path*>)
            return ref->elems.front().id;
        else if constexpr (std::is_same_v<T, const ast::Path::Elem*>)
            return ref->id;
        else if constexpr (std::is_same_v<T, const ast::ProjExpr*>) {
            if (std::holds_alternative<ast::Identifier>(ref->field))
                return std::get<ast::Identifier>(ref->field);
            else
                assert(false && "tuple indices are not supported for go-to-definition");
        }
        assert(false && "unhandled variant type");
        exit(1);
    }, ref);
}

} // namespace artic::ls

#endif // ENABLE_LSP
