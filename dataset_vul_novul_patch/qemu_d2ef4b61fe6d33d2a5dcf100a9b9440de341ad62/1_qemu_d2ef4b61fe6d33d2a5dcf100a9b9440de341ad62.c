 static int get_int32_le(QEMUFile *f, void *pv, size_t size)
 {
    int32_t loaded;
     int32_t loaded;
     qemu_get_sbe32s(f, &loaded);
 
    if (loaded <= *cur) {
         *cur = loaded;
         return 0;
     }
}
