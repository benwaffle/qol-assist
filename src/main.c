/*
 * This file is part of qol-assist.
 *
 * Copyright © 2017 Solus Project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#define _GNU_SOURCE

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "migrate.h"
#include "user-manager.h"
#include "util.h"

#include "migrations/declared.h"

#define MIGRATION(x, y)                                                                            \
        {                                                                                          \
                x, qol_migration_##y                                                               \
        }

/**
 * Simple table describing all of our potential migrations
 */
static QolMigration migration_table[] = {
        MIGRATION("Add users to scanner group", 01_scanner_group),
};

/**
 * Track the maximum number of migrations available
 */
static size_t n_migrations = sizeof(migration_table) / sizeof(migration_table[0]);

/**
 * Dummy code to validate the user management API
 */
static void print_users(QolContext *context)
{
        QolUser *user = NULL;

        /* Wind the user list in reverse */
        for (user = context->user_manager->users; user; user = user->next) {
                if (!qol_user_is_admin(user) || !qol_user_is_active(user)) {
                        continue;
                }
                fprintf(stdout, "User: %s (", user->name);
                for (size_t i = 0; i < user->n_groups; i++) {
                        fprintf(stdout,
                                "%s%s%s",
                                i != 0 ? ":" : "",
                                user->groups[i],
                                i == user->n_groups - 1 ? ")\n" : "");
                }
        }
}

int main(__qol_unused__ int argc, __qol_unused__ char **argv)
{
        QolContext *context = NULL;
        size_t migration_level_start = 0;
        bool ret = false;

        /* Before we go anywhere, kill stdin */
        if (stdin && fileno(stdin) >= 0) {
                close(fileno(stdin));
                stdin = NULL;
        }

        context = qol_context_new();
        if (!context) {
                fprintf(stderr, "Failed to construct QolContext: %s\n", strerror(errno));
                return EXIT_FAILURE;
        }

        print_users(context);

        /* Emulate migration steps */
        for (size_t i = migration_level_start; i < n_migrations; i++) {
                QolMigration *m = &migration_table[i];

                fprintf(stdout, "Begin migration %lu: '%s'\n", i, m->name);
                if (!m->func(context, (int)i)) {
                        fprintf(stderr, "Failed migration %lu: '%s'\n", i, m->name);
                        goto end;
                }
                fprintf(stdout, "Successful migration %lu: '%s'\n", i, m->name);
        }

        ret = true;

end:
        qol_context_free(context);
        return ret ? EXIT_SUCCESS : EXIT_FAILURE;
}

/*
 * Editor modelines  -  https://www.wireshark.org/tools/modelines.html
 *
 * Local variables:
 * c-basic-offset: 8
 * tab-width: 8
 * indent-tabs-mode: nil
 * End:
 *
 * vi: set shiftwidth=8 tabstop=8 expandtab:
 * :indentSize=8:tabSize=8:noTabs=true:
 */
