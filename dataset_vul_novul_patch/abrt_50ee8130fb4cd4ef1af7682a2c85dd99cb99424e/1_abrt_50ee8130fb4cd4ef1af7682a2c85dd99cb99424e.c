int main(int argc, char **argv)
{
    /* I18n */
    setlocale(LC_ALL, "");
#if ENABLE_NLS
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);
#endif

    abrt_init(argv);

    /* Can't keep these strings/structs static: _() doesn't support that */
    const char *program_usage_string = _(
        "& [-y] [-i BUILD_IDS_FILE|-i -] [-e PATH[:PATH]...]\n"
        "\t[-r REPO]\n"
        "\n"
        "Installs debuginfo packages for all build-ids listed in BUILD_IDS_FILE to\n"
        "ABRT system cache."
    );

    enum {
        OPT_v = 1 << 0,
        OPT_y = 1 << 1,
        OPT_i = 1 << 2,
        OPT_e = 1 << 3,
        OPT_r = 1 << 4,
        OPT_s = 1 << 5,
    };

    const char *build_ids = "build_ids";
    const char *exact = NULL;
    const char *repo = NULL;
    const char *size_mb = NULL;

    struct options program_options[] = {
        OPT__VERBOSE(&g_verbose),
        OPT_BOOL  ('y', "yes",         NULL,                   _("Noninteractive, assume 'Yes' to all questions")),
        OPT_STRING('i', "ids",   &build_ids, "BUILD_IDS_FILE", _("- means STDIN, default: build_ids")),
        OPT_STRING('e', "exact",     &exact, "EXACT",          _("Download only specified files")),
        OPT_STRING('r', "repo",       &repo, "REPO",           _("Pattern to use when searching for repos, default: *debug*")),
        OPT_STRING('s', "size_mb", &size_mb, "SIZE_MB",        _("Ignored option")),
        OPT_END()
    };
    const unsigned opts = parse_opts(argc, argv, program_options, program_usage_string);

    const gid_t egid = getegid();
    const gid_t rgid = getgid();
    const uid_t euid = geteuid();
    const gid_t ruid = getuid();

    /* We need to open the build ids file under the caller's UID/GID to avoid
     * information disclosures when reading files with changed UID.
     * Unfortunately, we cannot replace STDIN with the new fd because ABRT uses
     * STDIN to communicate with the caller. So, the following code opens a
     * dummy file descriptor to the build ids file and passes the new fd's proc
     * path to the wrapped program in the ids argument.
     * The new fd remains opened, the OS will close it for us. */
    char *build_ids_self_fd = NULL;
    if (strcmp("-", build_ids) != 0)
    {
        if (setregid(egid, rgid) < 0)
            perror_msg_and_die("setregid(egid, rgid)");

        if (setreuid(euid, ruid) < 0)
            perror_msg_and_die("setreuid(euid, ruid)");

        const int build_ids_fd = open(build_ids, O_RDONLY);

        if (setregid(rgid, egid) < 0)
            perror_msg_and_die("setregid(rgid, egid)");

        if (setreuid(ruid, euid) < 0 )
            perror_msg_and_die("setreuid(ruid, euid)");

        if (build_ids_fd < 0)
            perror_msg_and_die("Failed to open file '%s'", build_ids);

        /* We are not going to free this memory. There is no place to do so. */
         build_ids_self_fd = xasprintf("/proc/self/fd/%d", build_ids_fd);
     }
 
    /* name, -v, --ids, -, -y, -e, EXACT, -r, REPO, --, NULL */
    const char *args[11];
     {
         const char *verbs[] = { "", "-v", "-vv", "-vvv" };
         unsigned i = 0;
        args[i++] = EXECUTABLE;
        args[i++] = "--ids";
        args[i++] = (build_ids_self_fd != NULL) ? build_ids_self_fd : "-";
        if (g_verbose > 0)
            args[i++] = verbs[g_verbose <= 3 ? g_verbose : 3];
        if ((opts & OPT_y))
            args[i++] = "-y";
        if ((opts & OPT_e))
        {
            args[i++] = "--exact";
            args[i++] = exact;
        }
        if ((opts & OPT_r))
        {
             args[i++] = "--repo";
             args[i++] = repo;
         }
         args[i++] = "--";
         args[i] = NULL;
     }

    /* Switch real user/group to effective ones.
     * Otherwise yum library gets confused - gets EPERM (why??).
     */
    /* do setregid only if we have to, to not upset selinux needlessly */
    if (egid != rgid)
        IGNORE_RESULT(setregid(egid, egid));
    if (euid != ruid)
    {
        IGNORE_RESULT(setreuid(euid, euid));
        /* We are suid'ed! */
        /* Prevent malicious user from messing up with suid'ed process: */
#if 1

        static const char *whitelist[] = {
            "REPORT_CLIENT_SLAVE", //  Check if the app is being run as a slave
            "LANG",
        };
        const size_t wlsize = sizeof(whitelist)/sizeof(char*);
        char *setlist[sizeof(whitelist)/sizeof(char*)] = { 0 };
        char *p = NULL;
        for (size_t i = 0; i < wlsize; i++)
            if ((p = getenv(whitelist[i])) != NULL)
                setlist[i] = xstrdup(p);

        clearenv();

        for (size_t i = 0; i < wlsize; i++)
            if (setlist[i] != NULL)
            {
                xsetenv(whitelist[i], setlist[i]);
                free(setlist[i]);
            }
#else
        /* Clear dangerous stuff from env */
        static const char forbid[] =
            "LD_LIBRARY_PATH" "\0"
            "LD_PRELOAD" "\0"
            "LD_TRACE_LOADED_OBJECTS" "\0"
            "LD_BIND_NOW" "\0"
            "LD_AOUT_LIBRARY_PATH" "\0"
            "LD_AOUT_PRELOAD" "\0"
            "LD_NOWARN" "\0"
            "LD_KEEPDIR" "\0"
        ;
        const char *p = forbid;
        do {
            unsetenv(p);
            p += strlen(p) + 1;
        } while (*p);
#endif
        /* Set safe PATH */
        char path_env[] = "PATH=/usr/sbin:/sbin:/usr/bin:/bin:"BIN_DIR":"SBIN_DIR;
        if (euid != 0)
            strcpy(path_env, "PATH=/usr/bin:/bin:"BIN_DIR);
        putenv(path_env);

        /* Use safe umask */
         umask(0022);
     }
 
    execvp(EXECUTABLE, (char **)args);
    error_msg_and_die("Can't execute %s", EXECUTABLE);
 }
