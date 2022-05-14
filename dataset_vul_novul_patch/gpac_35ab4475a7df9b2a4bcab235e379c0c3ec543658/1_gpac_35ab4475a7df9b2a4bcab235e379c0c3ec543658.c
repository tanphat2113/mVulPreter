GF_Err cat_multiple_files(GF_ISOFile *dest, char *fileName, u32 import_flags, Double force_fps, u32 frames_per_sample, char *tmp_dir, Bool force_cat, Bool align_timelines, Bool allow_add_in_command)
{
	CATEnum cat_enum;
	char *sep;

	cat_enum.dest = dest;
	cat_enum.import_flags = import_flags;
	cat_enum.force_fps = force_fps;
	cat_enum.frames_per_sample = frames_per_sample;
	cat_enum.tmp_dir = tmp_dir;
	cat_enum.force_cat = force_cat;
 	cat_enum.align_timelines = align_timelines;
 	cat_enum.allow_add_in_command = allow_add_in_command;
 
 	strcpy(cat_enum.szPath, fileName);
 	sep = strrchr(cat_enum.szPath, GF_PATH_SEPARATOR);
 	if (!sep) sep = strrchr(cat_enum.szPath, '/');
 	if (!sep) {
 		strcpy(cat_enum.szPath, ".");
 		strcpy(cat_enum.szRad1, fileName);
 	} else {
 		strcpy(cat_enum.szRad1, sep+1);
 		sep[0] = 0;
 	}
 	sep = strchr(cat_enum.szRad1, '*');
 	strcpy(cat_enum.szRad2, sep+1);
 	sep[0] = 0;
 	sep = strchr(cat_enum.szRad2, '%');
 	if (!sep) sep = strchr(cat_enum.szRad2, '#');
 	if (!sep) sep = strchr(cat_enum.szRad2, ':');
 	strcpy(cat_enum.szOpt, "");
 	if (sep) {
 		strcpy(cat_enum.szOpt, sep);
 		sep[0] = 0;
 	}
	return gf_enum_directory(cat_enum.szPath, 0, cat_enumerate, &cat_enum, NULL);
}
