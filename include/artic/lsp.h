#ifndef ARTIC_LSP_H
#define ARTIC_LSP_H

// Language-server support. Everything in this header is compiled away unless the build
// declares ENABLE_LSP, so the standalone compiler carries none of it.
#ifdef ENABLE_LSP

#include <string>

#include "artic/loc.h"

namespace artic::ls {

enum class Severity { Error, Warning, Info, Hint };

/// A diagnostic collected by the language server. The compiler reports through `Logger`,
/// which fills these in whenever a sink is attached to the `Log`.
struct Diagnostic {
    Loc loc;
    std::string message;
    Severity severity;
};

} // namespace artic::ls

#endif // ENABLE_LSP

#endif // ARTIC_LSP_H
