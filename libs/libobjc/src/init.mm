/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libobjc/class.h>
#include <libobjc/module.h>
#include <libobjc/objc.h>
#include <libobjc/runtime.h>

// The function is called by a constructor of each module.
OBJC_EXPORT void __objc_exec_class(struct objc_module* module)
{
    static bool prepared_data_structures = false;

    OBJC_DEBUGPRINT("Called __objc_exec_class, starting to init module\n");

    if (!prepared_data_structures) {
        selector_table_init();
        class_table_init();
        prepared_data_structures = true;
    }

    struct objc_symtab* symtab = module->symtab;
    struct objc_selector* selectors = symtab->refs;

    if (selectors) {
        selector_add_from_module(selectors);
    }

    class_add_from_module(symtab);

    // TODO: Many things to init here.

    class_resolve_all_unresolved();
}
