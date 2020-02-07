#include "unixkit.h"
#include <assert.h>
#include <string.h>

int main()
{
    assert(unixkit_path_starts_with("/sys/kernel",
                                    "/sys"));

    assert(unixkit_path_starts_with("/sys/kernel",
                                    "/sys/"));

    assert(!unixkit_path_starts_with("/sys/kernel",
                                     "/syss"));

    assert(!unixkit_path_starts_with("/sys/kernel",
                                     "/sys/kernel/"));

    avl_tree_t *paths =
        make_avl_tree((int (*)(const void *, const void *)) strcmp);
    avl_tree_put(paths, "/sys", NULL);
    avl_tree_put(paths, "/sys/kernel", NULL);
    avl_tree_put(paths, "/syss", NULL);
    assert(unixkit_path_starts_with_any("/sys/kernel", paths));
    destroy_avl_element(avl_tree_pop(paths, "/sys/kernel"));
    assert(unixkit_path_starts_with_any("/sys/kernel", paths));
    destroy_avl_element(avl_tree_pop(paths, "/sys"));
    assert(!unixkit_path_starts_with_any("/sys/kernel", paths));
    destroy_avl_tree(paths);

    return 0;
}
