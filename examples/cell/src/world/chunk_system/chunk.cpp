#include "world/chunk_system/chunk.h"

namespace cell {

    // to avoid linker errors
    // Tell the C++ compiler which instantiations to make while it is compiling the template class’s .cpp file.
    // https://isocpp.org/wiki/faq/templates#templates-defn-vs-decl
    class Cell;
    class Light;
    template class Chunk<Cell>;
    template class Chunk<Light>;

    template<typename T>
    void Chunk<T>::markAsChanged(bool has_changed) {

        _has_changed = has_changed;
    }

    template<typename T>
    bool Chunk<T>::getHasChanged() {

        return _has_changed;
    }

} // cell