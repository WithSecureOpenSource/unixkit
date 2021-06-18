#include <assert.h>
#include <string.h>

#include "unixkit.h"

int main()
{
    assert(unixkit_path_starts_with("/sys/kernel", "/sys"));

    assert(unixkit_path_starts_with("/sys/kernel", "/sys/"));

    assert(!unixkit_path_starts_with("/sys/kernel", "/syss"));

    assert(!unixkit_path_starts_with("/sys/kernel", "/sys/kernel/"));

    avl_tree_t *paths =
        make_avl_tree((int (*)(const void *, const void *)) strcmp);
    avl_elem_t *element;
    avl_tree_put(paths, "/", NULL);
    avl_tree_put(paths, "/sys", NULL);
    avl_tree_put(paths, "/sys$", NULL);
    avl_tree_put(paths, "/sys/kernel", NULL);

    element = unixkit_path_get_lowest_ancestor("/var", paths);
    assert(element);
    assert(!strcmp(avl_elem_get_key(element), "/"));
    destroy_avl_element(avl_tree_pop(paths, "/"));

    element = unixkit_path_get_lowest_ancestor("/sys/kernel", paths);
    assert(element);
    assert(!strcmp(avl_elem_get_key(element), "/sys/kernel"));
    destroy_avl_element(avl_tree_pop(paths, "/sys/kernel"));

    element = unixkit_path_get_lowest_ancestor("/sys/kernel", paths);
    assert(element);
    assert(!strcmp(avl_elem_get_key(element), "/sys"));
    destroy_avl_element(avl_tree_pop(paths, "/sys"));

    assert(!unixkit_path_starts_with_any("/sys/kernel", paths));
    destroy_avl_tree(paths);

    return 0;
}
