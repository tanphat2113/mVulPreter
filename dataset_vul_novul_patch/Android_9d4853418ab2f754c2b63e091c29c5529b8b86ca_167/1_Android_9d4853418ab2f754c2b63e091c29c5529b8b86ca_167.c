 main(int argc, const char **argv)
 {
    const char *  prog = *argv;
    const char *  outfile = NULL;
    const char *  suffix = NULL;
 const char *  prefix = NULL;
 int           done = 0; /* if at least one file is processed */
 struct global global;

   global_init(&global);

 while (--argc > 0)
 {
 ++argv;

 if (strcmp(*argv, "--debug") == 0)
 {
 /* To help debugging problems: */
         global.errors = global.warnings = 1;
         global.quiet = 0;
         global.verbose = 7;
 }

 else if (strncmp(*argv, "--max=", 6) == 0)
 {
         global.idat_max = (png_uint_32)atol(6+*argv);

 if (global.skip < SKIP_UNSAFE)
            global.skip = SKIP_UNSAFE;
 }

 else if (strcmp(*argv, "--max") == 0)
 {
         global.idat_max = 0x7fffffff;

 if (global.skip < SKIP_UNSAFE)
            global.skip = SKIP_UNSAFE;
 }

 else if (strcmp(*argv, "--optimize") == 0 || strcmp(*argv, "-o") == 0)
         global.optimize_zlib = 1;

 else if (strncmp(*argv, "--out=", 6) == 0)
         outfile = 6+*argv;

 else if (strncmp(*argv, "--suffix=", 9) == 0)
         suffix = 9+*argv;

 else if (strncmp(*argv, "--prefix=", 9) == 0)
         prefix = 9+*argv;

 else if (strcmp(*argv, "--strip=none") == 0)
         global.skip = SKIP_NONE;

 else if (strcmp(*argv, "--strip=crc") == 0)
         global.skip = SKIP_BAD_CRC;

 else if (strcmp(*argv, "--strip=unsafe") == 0)
         global.skip = SKIP_UNSAFE;

 else if (strcmp(*argv, "--strip=unused") == 0)
         global.skip = SKIP_UNUSED;

 else if (strcmp(*argv, "--strip=transform") == 0)
         global.skip = SKIP_TRANSFORM;

 else if (strcmp(*argv, "--strip=color") == 0)
         global.skip = SKIP_COLOR;

 else if (strcmp(*argv, "--strip=all") == 0)
         global.skip = SKIP_ALL;

 else if (strcmp(*argv, "--errors") == 0 || strcmp(*argv, "-e") == 0)
         global.errors = 1;

 else if (strcmp(*argv, "--warnings") == 0 || strcmp(*argv, "-w") == 0)
         global.warnings = 1;

 else if (strcmp(*argv, "--quiet") == 0 || strcmp(*argv, "-q") == 0)
 {
 if (global.quiet)
            global.quiet = 2;

 else
            global.quiet = 1;
 }

 else if (strcmp(*argv, "--verbose") == 0 || strcmp(*argv, "-v") == 0)
 ++global.verbose;

#if 0
 /* NYI */
#     ifdef PNG_MAXIMUM_INFLATE_WINDOW
 else if (strcmp(*argv, "--test") == 0)
 ++set_option;
#     endif
#endif

 else if ((*argv)[0] == '-')
         usage(prog);


       else
       {
          size_t outlen = strlen(*argv);
         char temp_name[FILENAME_MAX+1];
 
          if (outfile == NULL) /* else this takes precedence */
          {
 /* Consider the prefix/suffix options */
 if (prefix != NULL)
 {
 size_t prefixlen = strlen(prefix);

 if (prefixlen+outlen > FILENAME_MAX)
 {
                  fprintf(stderr, "%s: output file name too long: %s%s%s\n",
                     prog, prefix, *argv, suffix ? suffix : "");
                  global.status_code |= WRITE_ERROR;
 continue;
 }

               memcpy(temp_name, prefix, prefixlen);
               memcpy(temp_name+prefixlen, *argv, outlen);
               outlen += prefixlen;
               outfile = temp_name;
 }

 else if (suffix != NULL)
               memcpy(temp_name, *argv, outlen);

            temp_name[outlen] = 0;

 if (suffix != NULL)
 {
 size_t suffixlen = strlen(suffix);

 if (outlen+suffixlen > FILENAME_MAX)
 {
                  fprintf(stderr, "%s: output file name too long: %s%s\n",
                     prog, *argv, suffix);
                  global.status_code |= WRITE_ERROR;
 continue;
 }

               memcpy(temp_name+outlen, suffix, suffixlen);
               outlen += suffixlen;
               temp_name[outlen] = 0;
               outfile = temp_name;
 }
 }

 (void)one_file(&global, *argv, outfile);
 ++done;
         outfile = NULL;
 }
 }

 if (!done)
      usage(prog);


    return global_end(&global);
 }
